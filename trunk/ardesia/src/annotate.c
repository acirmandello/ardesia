/* 
 * Annotate -- a program for painting on the screen 
 * Copyright (C) 2009 Pilolli Pietro <pilolli@fbk.eu>
 *
 * Some parts of this file are been copied from gromit
 * Copyright (C) 2000 Simon Budig <Simon.Budig@unix-ag.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkinput.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>

#include "paint_cursor.xpm"
#include "erase_cursor.xpm"

#define ANNOTATE_MOUSE_EVENTS ( GDK_PROXIMITY_IN_MASK |		\
				GDK_PROXIMITY_OUT_MASK |	\
				GDK_POINTER_MOTION_MASK |	\
				GDK_BUTTON_PRESS_MASK |		\
				GDK_BUTTON_RELEASE_MASK)

/* set the DEBUG to 1 to read the debug messages */
#define DEBUG 0

typedef enum
  {
    ANNOTATE_PEN,
    ANNOTATE_ERASER,
  } AnnotatePaintType;


typedef struct
{
  AnnotatePaintType type;
  guint           width;
  guint           arrowsize;
  gchar*          fg_color;
  gdouble         pressure;
} AnnotatePaintContext;


typedef struct
{
  gint x;
  gint y;
  gint width;
} AnnotateStrokeCoordinate;


typedef struct
{
  int untogglexpos;
  int untoggleypos;
  int untogglewidth;
  int untoggleheight;
  cairo_t     *cr;
  GtkWidget   *win;
  GtkWidget   *area;

  GdkDisplay  *display;
  GdkPixmap   *pixmap;

  GdkBitmap   *shape;
 
  AnnotatePaintContext *default_pen;
  AnnotatePaintContext *default_eraser;
  AnnotatePaintContext *cur_context;

  gdouble      lastx;
  gdouble      lasty;
  GList       *coordlist;

  GdkDevice   *device;
  guint        state;

  guint        width;
  guint        maxwidth;
  guint        height;
  gboolean     painted;
  guint        arrow;
  gboolean     debug;
} AnnotateData;

AnnotateData* data;


/* Create a new paint context */
AnnotatePaintContext * annotate_paint_context_new (AnnotatePaintType type,
			                           char* fg_color, 
                                                   guint width, 
                                                   guint arrowsize)
{
  AnnotatePaintContext *context;
  context = g_malloc (sizeof (AnnotatePaintContext));
  context->type = type;
  context->width = width;
  context->arrowsize = arrowsize;
  context->fg_color = fg_color;
  return context;
}


/* Get the annotation window */
GtkWindow* get_annotation_window()
{
   return GTK_WINDOW(data->win);
}


/* Get the cairo context that contains the annotation */
cairo_t* get_annotation_cairo_context()
{
  return data->cr;
}


/* Print paint context informations */
void annotate_paint_context_print (gchar *name, AnnotatePaintContext *context)
{
  g_printerr ("Tool name: \"%-20s\": ", name);
  switch (context->type)
    {
    case ANNOTATE_PEN:
      g_printerr ("Pen,     "); break;
    case ANNOTATE_ERASER:
      g_printerr ("Eraser,  "); break;
    default:
      g_printerr ("UNKNOWN, "); break;
    }

  g_printerr ("width: %3d, ", context->width);
  g_printerr ("arrowsize: %3d, ", context->arrowsize);
  g_printerr ("color: %s\n", context->fg_color);
}


/* Free the memory allocated by paint context */
void annotate_paint_context_free (AnnotatePaintContext *context)
{
  g_free (context);
}


/* Add to the list of the painted point the poin (x,y) with the width width */
void annotate_coord_list_prepend (gint x, gint y, gint width)
{
  AnnotateStrokeCoordinate *point;
  point = g_malloc (sizeof (AnnotateStrokeCoordinate));
  point->x = x;
  point->y = y;
  point->width = width;
  data->coordlist = g_list_prepend (data->coordlist, point);
}


/* Free the list of the painted point */
void annotate_coord_list_free ()
{
  GList *ptr = data->coordlist;

  while (ptr)
    {
      g_free (ptr->data);
      ptr = ptr->next;
    }

  g_list_free (data->coordlist);

  data->coordlist = NULL;
}


gboolean annotate_coord_list_get_arrow_param (AnnotateData* data,
				     	      gint        search_radius,
				              gint       *ret_width,
				              gfloat     *ret_direction)
{
  gint x0, y0, r2, dist;
  gboolean success = FALSE;
  AnnotateStrokeCoordinate  *cur_point, *valid_point;
  GList *ptr = data->coordlist;
  guint width;

  valid_point = NULL;

  if (ptr)
    {
      cur_point = ptr->data;
      x0 = cur_point->x;
      y0 = cur_point->y;
      r2 = search_radius * search_radius;
      dist = 0;

      while (ptr && dist < r2)
        {
          ptr = ptr->next;
          if (ptr)
            {
              cur_point = ptr->data;
              dist = (cur_point->x - x0) * (cur_point->x - x0) +
		(cur_point->y - y0) * (cur_point->y - y0);
              width = cur_point->width * data->cur_context->arrowsize;
              if (width * 2 <= dist &&
                  (!valid_point || valid_point->width < cur_point->width))
		{
		  valid_point = cur_point;
		}
            }
        }

      if (valid_point)
        {
          *ret_width = MAX (valid_point->width * data->cur_context->arrowsize, 2);
          *ret_direction = atan2 (y0 - valid_point->y, x0 - valid_point->x);
          success = TRUE;
        }
    }

  return success;
}


/* Hide the  window with the annotations */
void annotate_hide_window ()
{
  gtk_widget_hide (data->win);
}


/* Show the window with the annotations */
void annotate_show_window ()
{
  gtk_widget_show (data->win);
}


/* Release the mouse pointer */
void annotate_release_grab ()
{           
  gdk_error_trap_push ();

  gdk_display_pointer_ungrab (data->display, GDK_CURRENT_TIME);
  /* inherit cursor from root window */
  gdk_window_set_cursor (data->win->window, NULL);
  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      /* this probably means the device table is outdated, 
	 e.g. this device doesn't exist anymore */
      g_printerr("Error ungrabbing mouse device while ungrabbing all, ignoring.\n");
    }

  
  gtk_widget_input_shape_combine_mask(data->win, data->shape, 0, 0); 
  

  if(data->debug)
    {
      g_printerr ("DEBUG: Ungrabbed pointer.\n");
    }
}


/* Select the default eraser tool */
void annotate_select_eraser()
{
  AnnotatePaintContext *eraser = data->default_eraser;
  data->cur_context = eraser;
}


/* Configure the eraser */
void annotate_configure_eraser(int width)
{
  data->cur_context->width = (width * 10);	
}


/* Select the default pen tool */
void annotate_select_pen()
{
  AnnotatePaintContext *pen = data->default_pen;
  data->cur_context = pen;
}


/* Set the cursor patching the xpm with the selected color */
void set_pen_cursor(char *color)
{
  GdkPixbuf *cursor_src;
  char *line = malloc(12);
  strcpy(line, ". c #");
  strncat(line, color, 6);
  paint_cursor_xpm[2]= line;
  /* now the xpm cursor has been coloured */
  cursor_src = gdk_pixbuf_new_from_xpm_data (paint_cursor_xpm);
  int height = gdk_pixbuf_get_height (cursor_src);
  GdkCursor* cursor = gdk_cursor_new_from_pixbuf (data->display, cursor_src, 1, height-1);
  gdk_window_set_cursor (data->win->window, cursor);
  gdk_flush ();
  g_object_unref (cursor_src);
  gdk_cursor_destroy (cursor);
}


/* Set the eraser cursor */
void set_eraser_cursor()
{
  GdkPixbuf *cursor_src;
  cursor_src = gdk_pixbuf_new_from_xpm_data(erase_cursor_xpm);
  int width = gdk_pixbuf_get_width (cursor_src);
  int height = gdk_pixbuf_get_height (cursor_src);
  GdkCursor* cursor = gdk_cursor_new_from_pixbuf (data->display, cursor_src, width/2, height/2);
  gdk_window_set_cursor (data->win->window, cursor);
  gdk_flush ();
  g_object_unref (cursor_src);
  gdk_cursor_destroy (cursor);
}


/*
 * This is function return if the point (x,y) in inside the ardesia panel area
 * or in a zone where we must unlock the pointer
 */
gboolean in_unlock_area(int x, int y)
{
  int untogglexpos = data->untogglexpos;
  int untoggleypos = data->untoggleypos;
  int untogglewidth = data->untogglewidth;
  int untoggleheight = data->untoggleheight;
  /* rectangle that contains the panel */
  if ((y>=untoggleypos)&&(y<untoggleypos+untoggleheight))
    {
      if ((x>=untogglexpos)&&(x<untogglexpos+untogglewidth))
	{
	  return 1;
	}
    }
  /* top left corner */
  if ((x<5)&&(y<5))
    {
      return 1;
    }
  return 0;
}


/* Grab the cursor */
void annotate_acquire_grab ()
{
  annotate_show_window ();

  GdkGrabStatus result;
  gdk_error_trap_push(); 

  gtk_widget_input_shape_combine_mask(data->win, NULL, 0, 0);   

  result = gdk_pointer_grab (data->area->window, FALSE,
			     ANNOTATE_MOUSE_EVENTS, 0,
			     NULL,
			     GDK_CURRENT_TIME); 

  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      /* this probably means the device table is outdated, 
	 e.g. this device doesn't exist anymore */
      g_printerr("Error grabbing Device while grabbing all, ignoring.");
    }
   
  switch (result)
    {
    case GDK_GRAB_SUCCESS:
      break;
    case GDK_GRAB_ALREADY_GRABBED:
      g_printerr ("Grabbing Pointer failed: %s\n", "AlreadyGrabbed");
      break;
    case GDK_GRAB_INVALID_TIME:
      g_printerr ("Grabbing Pointer failed: %s\n", "GrabInvalidTime");
      break;
    case GDK_GRAB_NOT_VIEWABLE:
      g_printerr ("Grabbing Pointer failed: %s\n", "GrabNotViewable");
      break;
    case GDK_GRAB_FROZEN:
      g_printerr ("Grabbing Pointer failed: %s\n", "GrabFrozen");
      break;
    default:
      g_printerr ("Grabbing Pointer failed: %s\n", "Unknown error");
    }      
  if(data->cur_context && data->cur_context->type == ANNOTATE_ERASER)
    {
      set_eraser_cursor(data);
    } 
  else
    {
      set_pen_cursor(data->cur_context->fg_color);
    } 
  
}


/* Set color */
void annotate_set_color(gchar* color)
{
   data->cur_context->fg_color = color;
}


/* Set width */
void annotate_set_width(guint width)
{
   data->cur_context->width = width;
}


/* Set arrow type */
void annotate_set_arrow(int arrow)
{
  data->arrow = arrow;
  if (arrow>0)
    { 
      /* set the arrowsize with a function depending on line width */
      data->cur_context->arrowsize =  log(data->cur_context->width+7);
    }
  else
    {
      data->cur_context->arrowsize =  0;
    }
}


/* Start to paint */
void annotate_toggle_grab ()
{ 
  annotate_select_pen(data);
  annotate_acquire_grab (data);
}


/* Start to erase */
void annotate_eraser_grab ()
{
  /* Get the with message */
  annotate_select_eraser(data);
  annotate_configure_eraser(data->cur_context->width);
  annotate_acquire_grab (data);
}


/* Repaint the window */
gint repaint ()
{
  gdk_draw_drawable (data->area->window,
                     data->area->style->fg_gc[GTK_WIDGET_STATE (data->area)],
                     data->pixmap,
                     0, 0,
                     0, 0,
                     data->width, data->height);
  return 1;
}


/* Clear cairo context */
void clear_cairo_context(cairo_t* cr)
{
  cairo_set_operator(cr,CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr,0,0,0,0);
  cairo_paint(cr);
}


/* Clear the screen */
void clear_screen()
{
  data->cr=gdk_cairo_create(data->pixmap);
  clear_cairo_context(data->cr);
  repaint();
}


/* Clear the annotations windows */
void annotate_clear_screen ()
{
  clear_screen();
  data->painted = FALSE;
}


/* Select eraser, pen or other tool for tablet */
void annotate_select_tool (GdkDevice *device, guint state)
{
  guint buttons = 0, modifier = 0, len = 0;
  guint req_buttons = 0, req_modifier = 0;
  guint i, j, success = 0;
  AnnotatePaintContext *context = NULL;
  guchar *name;
 
  if (device)
    {
      len = strlen (device->name);
      name = (guchar*) g_strndup (device->name, len + 3);
      
      /* Extract Button/Modifiers from state (see GdkModifierType) */
      req_buttons = (state >> 8) & 31;

      req_modifier = (state >> 1) & 7;
      if (state & GDK_SHIFT_MASK) req_modifier |= 1;

      name [len] = 124;
      name [len+3] = 0;
  
      /*  0, 1, 3, 7, 15, 31 */
      context = NULL;
      i=-1;
      do
        {
          i++;
          buttons = req_buttons & ((1 << i)-1);
          j=-1;
          do
            {
              j++;
              modifier = req_modifier & ((1 << j)-1);
              name [len+1] = buttons + 64;
              name [len+2] = modifier + 48;
            }
          while (j<=3 && req_modifier >= (1 << j));
        }
      while (i<=5 && req_buttons >= (1 << i));

      g_free (name);

      if (!success)
        {
          if (device->source == GDK_SOURCE_ERASER)
            {
              data->cur_context = data->default_eraser;
            }
          else
            {
              data->cur_context = data->default_pen;
	    }
        }
    }
  else
    {
      g_printerr ("ERROR: Attempt to select nonexistent device!\n");
      data->cur_context = data->default_pen;
    }

  data->state = state;
  data->device = device;
}


/* Set the cairo surface color to transparent */
void  cairo_set_transparent_color(cairo_t * cr)
{
  cairo_set_source_rgba (cr, 0.0, 0.0,0.0, 0.0);
}


/* Set the cairo surface color to the RGBA string */
void cairo_set_source_color_from_string( cairo_t * cr, char* color)
{
  int r,g,b,a;
  sscanf (color, "%02X%02X%02X%02X", &r, &g, &b, &a);
  cairo_set_source_rgba (cr, (double) r/256, (double) g/256, (double) b/256, (double) a/256);
}


/* Color selector; if eraser than select the transparent color else alloc the right color */ 
void select_color(AnnotateData *data)
{
  if (!(data->cur_context->type == ANNOTATE_ERASER))
     {
       cairo_set_source_color_from_string(data->cr, data->cur_context->fg_color);
     }
  else
    {
       cairo_set_transparent_color(data->cr);
    }
}


/* Draw line from (x1,y1) to (x2,y2) */
void annotate_draw_line_int (gint x1, gint y1,
			     gint x2, gint y2)
{
  cairo_move_to(data->cr,x1,y1);
  cairo_line_to(data->cr,x2,y2);
  cairo_set_operator(data->cr,CAIRO_OPERATOR_SOURCE);
  cairo_stroke(data->cr);
  data->painted = TRUE;
}


/* 
 * Draw a line; 
 * the algorithm is done to deny the overlapping 
 * of the line with the bar 
 */
void annotate_draw_line (gint x1, gint y1,
		         gint x2, gint y2)
{
  int x0 = data->untogglexpos;
  int y0 = data->untoggleypos;
  int width = data->untogglewidth;
  int height = data->untoggleheight;
  int y = y0;
  /* verify if the candidate y intersects the bar and belong to the line */
  if (((y1<y2)&&(y1<y)&&(y<y2))||((y1>y2)&&(y2<y)&&(y<y1)))
    {
      /* interleaved y found the corrispondent x */
      /* (x,y0) this will be the first point that intersects the rectangle */
      int x=-1;
      /* calculate the x with the formula to calculate the rect passing from (x1.y1) (x2,y2) */
      if (x1!=x2)
	{
	  x=((x2-x1)*(y-y1)/(y2-y1))+x1;
	}
      else
	{
	  x=x1;
	}
      /* verify if the candidate x intersects the bar and belong to the line */
      if (((x0<x)&&(x<x+width))&&(((x1<=x2)&&(x1<=x)&&(x<=x2))||((x1>x2)&&(x2<x)&&(x<x1))))
	{
	  /* interleaved y0+height found the corrispondent xp */
	  /* (xp, y0+height) this will be the second point that intersects the rectangle */
	  int yp = y0 + height;
	  int xp = -1;
	  if (x1!=x2)
	    {
	      xp=((x2-x1)*(yp-y1)/(y2-y1))+x1;
	    }
	  else
	    {
	      xp=x;
	    }
	  /* draw the first and the second segments */
	  if  (y1<y2)
	    {
	      annotate_draw_line_int (x1, y1, x, y);    
	      annotate_draw_line_int (xp, yp, x2, y2);
	      return;    
	    }
	  else
	    {
	      /* viceversa */
	      annotate_draw_line_int (x1, y1, xp, yp);    
	      annotate_draw_line_int (x, y, x2, y2);
	      return;    
	    }
	}
    }
  /* no intersection with the bar draw the full line */ 
  annotate_draw_line_int (x1, y1, x2, y2);    
}


/* Draw an arrow using some polygons */
void annotate_draw_arrow (gint x1, gint y1,
		          gint width, gfloat direction)
{
  GdkPoint arrowhead [4];

  int penwidth = data->cur_context->width;
  int penwidthcos = 2 * penwidth * cos (direction);
  int penwidthsin = 2 * penwidth * sin (direction);
  int widthcos = penwidthcos;
  int widthsin = penwidthsin;

  /* Vertex of the arrow */
  arrowhead [0].x = x1 + penwidthcos;
  arrowhead [0].y = y1 + penwidthsin;

  /* left point */
  arrowhead [1].x = x1 - 2 * widthcos + widthsin ;
  arrowhead [1].y = y1 -  widthcos -  2 * widthsin ;

  /* origin */
  arrowhead [2].x = x1 - widthcos ;
  arrowhead [2].y = y1 - widthsin ;

  /* right point */
  arrowhead [3].x = x1 - 2 * widthcos - widthsin ;
  arrowhead [3].y = y1 +  widthcos - 2 * widthsin ;

  cairo_line_to(data->cr,arrowhead[0].x,arrowhead[0].y);
  cairo_line_to(data->cr,arrowhead[1].x,arrowhead[1].y);
  cairo_line_to(data->cr,arrowhead[2].x,arrowhead[2].y);
  cairo_line_to(data->cr,arrowhead[3].x,arrowhead[3].y);
  cairo_close_path(data->cr);
  cairo_fill_preserve(data->cr);
  cairo_set_operator(data->cr,CAIRO_OPERATOR_SOURCE);
  cairo_stroke(data->cr);
 
  data->painted = TRUE;
}


/* Draw a back arrow using some polygons */
void annotate_draw_back_arrow (gint x1, gint y1,
			       gint width, gfloat direction)
{
  AnnotateData *revertcoordata = g_malloc (sizeof (AnnotateData));
  revertcoordata->cur_context = data->cur_context;
  GList *ptr = data->coordlist;
  if (ptr)
    {
      GList *revptr = g_list_reverse(ptr);
      revertcoordata->coordlist = revptr;
      AnnotateStrokeCoordinate  *first_point;
      first_point = revptr->data;
      int x0 = first_point->x;
      int y0 = first_point->y;
      annotate_coord_list_get_arrow_param (revertcoordata, width * 3,
					   &width, &direction);
      annotate_draw_arrow (x0, y0, width, direction);
    }
}


/*
 * Event-Handlers to perform the drawing
 */

/* Device touch */
gboolean proximity_in (GtkWidget *win,
                       GdkEventProximity *ev, 
                       gpointer user_data)
{
  gint x, y;
  GdkModifierType state;

  gdk_window_get_pointer (data->win->window, &x, &y, &state);
  annotate_select_tool (ev->device, state);

  if(data->debug)
    {
      g_printerr("DEBUG: proximity in device  %s: \n", ev->device->name);
    }
  return TRUE;
}


/* Device lease */
gboolean proximity_out (GtkWidget *win, 
                        GdkEventProximity *ev,
                        gpointer user_data)
{
  
  data->cur_context = data->default_pen;

  data->state = 0;
  data->device = NULL;

  if(data->debug)
    {
      g_printerr("DEBUG: proximity out device  %s: \n", ev->device->name);
    }
  return FALSE;
}


/* This is called when the button is pushed */
gboolean paint (GtkWidget *win,
                GdkEventButton *ev, 
                gpointer user_data)
{

  if (in_unlock_area(ev->x,ev->y))
    /* point is in the ardesia bar */
    {
      /* the last point was outside the bar then ungrab */
      annotate_release_grab ();
      return FALSE;
    }

  data->cr= gdk_cairo_create(data->pixmap);
  cairo_set_line_cap (data->cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width(data->cr,data->cur_context->width);

  select_color(data);  
  GdkModifierType state = (GdkModifierType) ev->state;  
  
  if(data->debug)
    {
      g_printerr("DEBUG: Device '%s': Button %i Down at (x,y)=(%.2f : %.2f)\n",
                 ev->device->name, ev->button, ev->x, ev->y);
    }
 
  if (state & GDK_BUTTON1_MASK)
    { 
      data->lastx = ev->x;
      data->lasty = ev->y;
      annotate_coord_list_prepend (ev->x, ev->y, data->maxwidth);
    }

  return TRUE;
}


/* This shots when the ponter is moving */
gboolean paintto (GtkWidget *win, 
                  GdkEventMotion *ev, 
                  gpointer user_data)
{

  if (in_unlock_area(ev->x,ev->y))
    /* point is in the ardesia bar */
    {
      /* the last point was outside the bar then ungrab */
      annotate_release_grab ();
      return FALSE;
    }

  gdouble pressure = 0;
  GdkModifierType state = (GdkModifierType) ev->state;  
  /* only button1 allowed */
  if (state & GDK_BUTTON1_MASK)
    {
      pressure=0.5;
    }

  if (pressure<=0)
    { 
      /* the button is not pressed */
      data->lastx = ev->x;
      data->lasty = ev->y;
      return FALSE;
    }

  if (ev->device->source == GDK_SOURCE_MOUSE)
    {
      data->maxwidth = data->cur_context->width;
    }
  else
    {
      data->maxwidth = (CLAMP (pressure * pressure,0,1) *
			(double) data->cur_context->width);
    }

  annotate_draw_line (data->lastx, data->lasty, ev->x, ev->y);
  annotate_coord_list_prepend (ev->x, ev->y, data->maxwidth);
    
  data->lastx = ev->x;
  data->lasty = ev->y;
  repaint();
  return TRUE;
}


/* This shots when the button is realeased */
gboolean paintend (GtkWidget *win, GdkEventButton *ev, gpointer user_data)
{
 
  if (in_unlock_area(ev->x,ev->y))
    /* point is in the ardesia bar */
    {
      /* the last point was outside the bar then ungrab */
      annotate_release_grab ();
      return FALSE;
    }

  gint width = data->cur_context->arrowsize * data->cur_context->width / 2;
  gfloat direction = 0;

  if ((ev->x != data->lastx) ||
      (ev->y != data->lasty))
    {
      paintto (win, (GdkEventMotion *) ev, user_data);
    }

  if (data->arrow>0 && data->cur_context->arrowsize > 0 &&
      annotate_coord_list_get_arrow_param (data, width * 3,
					   &width, &direction))
    {
      /* print arrow at the end of the line */
      annotate_draw_arrow (ev->x, ev->y, width, direction);
      if (data->arrow==2)
        {
          /* print arrow at the beginning of the line */
          annotate_draw_back_arrow (ev->x, ev->y, width, direction);
        }
    }

  cairo_stroke(data->cr);
  repaint();
  annotate_coord_list_free (data);
 
  return TRUE;
}


/*
 * Functions for handling various (GTK+)-Events
 */

/* Configure event */
gboolean event_configure (GtkWidget *widget,
                          GdkEventExpose *event,
                          gpointer user_data)
{

  data->pixmap = gdk_pixmap_new (data->area->window, data->width,
                                 data->height, -1);
  clear_screen();

  return TRUE;
}


/* Expose event */
gboolean event_expose (GtkWidget *widget, 
                       GdkEventExpose *event, 
                       gpointer user_data)
{

  gdk_draw_drawable (data->area->window,
                     data->area->style->fg_gc[GTK_WIDGET_STATE (data->area)],
                     data->pixmap,
                     event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return TRUE;
}


/* Do the gtk main loop */
void annotate_main_do_event (GdkEventAny *event, 
                             AnnotateData  *data)
{
  gtk_main_do_event ((GdkEvent *) event);
}


/*
 * Functions for setting up (parts of) the application
 */

/* Setup input device */
void setup_input_devices ()
{
  GList     *tmp_list;

  for (tmp_list = gdk_display_list_devices (data->display);
       tmp_list;
       tmp_list = tmp_list->next)
    {
      GdkDevice *device = (GdkDevice *) tmp_list->data;

      /* Guess "Eraser"-Type devices */
      if (strstr (device->name, "raser") ||
          strstr (device->name, "RASER"))
        {
	  gdk_device_set_source (device, GDK_SOURCE_ERASER);
        }

      /* Dont touch devices with two or more axis - GDK apparently
       * gets confused...  */
      if (device->num_axes > 2)
        {
          g_printerr ("Enabling No. %p: \"%s\" (Type: %d)\n",
                      device, device->name, device->source);
          gdk_device_set_mode (device, GDK_MODE_SCREEN);
        }
    }
}


/* Setup the application */
void setup_app ()
{ 
  /* default color is opaque red */ 
  char *color ="FF0000FF";

  data->display = gdk_display_get_default ();
  GdkScreen   *screen = gdk_display_get_default_screen (data->display);

  data->width = gdk_screen_get_width (screen);
  data->height = gdk_screen_get_height (screen);

  data->win = gtk_window_new (GTK_WINDOW_POPUP);
 
  gtk_window_set_opacity(GTK_WINDOW(data->win), 1); 
  gtk_widget_set_default_colormap(gdk_screen_get_rgba_colormap(screen));
 
  /* DRAWING AREA */
  data->area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (data->area),
                         data->width, data->height);

  gtk_container_add (GTK_CONTAINER (data->win), data->area);

  gtk_window_fullscreen(GTK_WINDOW(data->win));
 
  g_signal_connect (data->area,"configure_event",
                    G_CALLBACK (event_configure), NULL);
  g_signal_connect (data->area, "expose_event",
		    G_CALLBACK (event_expose), NULL);
  g_signal_connect (data->win, "button_press_event", 
		    G_CALLBACK(paint), NULL);
  g_signal_connect (data->win, "motion_notify_event",
		    G_CALLBACK (paintto), NULL);
  g_signal_connect (data->win, "button_release_event",
		    G_CALLBACK (paintend), NULL);
  g_signal_connect (data->win, "proximity_in_event",
		    G_CALLBACK (proximity_in), NULL);
  g_signal_connect (data->win, "proximity_out_event",
		    G_CALLBACK (proximity_out), NULL);

  data->arrow = 0; 
  data->painted = FALSE;

  data->coordlist = NULL;
  data->cr = NULL;

  data->default_pen = annotate_paint_context_new (ANNOTATE_PEN,
						  color, 15, 0);
  data->default_eraser = annotate_paint_context_new (ANNOTATE_ERASER,
						     NULL, 15, 0);

  data->cur_context = data->default_pen;

  data->state = 0;

  setup_input_devices (data);
 
  gtk_widget_show_all(data->win);
  
  clear_screen();
  
  /* SHAPE PIXMAP */
  data->shape = gdk_pixmap_new (NULL, data->width, data->height, 1); 
  cairo_t* shape_cr = gdk_cairo_create(data->shape);
  clear_cairo_context(shape_cr);

}


/* Quit the annotation */
void annotate_quit()
{
  annotate_release_grab(); /* ungrab all */
  cairo_destroy(data->cr);
  g_free (data);
}


/* Init the annotation */
int annotate_init (int x, int y, int width, int height)
{
  data = g_malloc (sizeof (AnnotateData));
  
  data->debug = DEBUG;
  /* Untoggle zone is setted on ardesia zone */
  data->untogglexpos = x;
  data->untoggleypos = y;
  data->untogglewidth = width;
  data->untoggleheight = height;
 
  setup_app ();

  return 0;
}
