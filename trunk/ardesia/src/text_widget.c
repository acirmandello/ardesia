/* 
 * Ardesia -- a program for painting on the screen
 * with this program you can play, draw, learn and teach
 * This program has been written such as a freedom sonet
 * We believe in the freedom and in the freedom of education
 *
 * Copyright (C) 2009 Pilolli Pietro <pilolli@fbk.eu>
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

/** Widget for text insertion */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gdk/gdkinput.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <ctype.h>
#include <text_widget.h>
#include <utils.h>
#include <annotate.h>

GtkWidget* text_window = NULL;
cairo_t *text_cr = NULL;
int text_pen_width = 0;

typedef struct
{
  int x;
  int y;
  int x_bearing;
  int y_bearing;
} CharInfo;


typedef struct
{
  int x;
  int y;
} Pos;



int letterc;

Pos* pos;

char* text_color = NULL;

cairo_text_extents_t extents;

int screen_width;
int screen_height;

int max_font_height;

GSList       *letterlist = NULL;


void create_text_window(GtkWindow *parent)
{
  text_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_fullscreen(GTK_WINDOW(text_window));
 
  gtk_window_set_transient_for(GTK_WINDOW(text_window), parent);
  gtk_window_set_modal(GTK_WINDOW(text_window), TRUE);
  
  gtk_window_set_keep_above (GTK_WINDOW(text_window), TRUE);
  gtk_window_stick(GTK_WINDOW(text_window));
  gtk_window_set_decorated(GTK_WINDOW(text_window), FALSE);
  gtk_widget_set_app_paintable(text_window, TRUE);
  gtk_widget_set_double_buffered(text_window, FALSE);
}


void move_editor_cursor()
{
   cairo_move_to(text_cr, pos->x, pos->y);
}


void delete_character()
{
   CharInfo *charInfo = (CharInfo *) g_slist_nth_data (letterlist, 0);

  if (charInfo)
  {  

  cairo_set_operator (text_cr, CAIRO_OPERATOR_CLEAR);
 
  cairo_rectangle(text_cr, charInfo->x + charInfo->x_bearing, charInfo->y + charInfo->y_bearing, screen_width,  max_font_height + text_pen_width+3);
  cairo_fill(text_cr);
  cairo_stroke(text_cr);
  cairo_set_operator (text_cr, CAIRO_OPERATOR_SOURCE);
  pos->x = charInfo->x;
  pos->y = charInfo->y;
  move_editor_cursor();
  letterlist = g_slist_remove(letterlist,charInfo); 
  }
}


/* press keyboard event */
gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)  
{
 
  if ((event->keyval == GDK_BackSpace)
     || (event->keyval == GDK_Delete))
    {
      // undo
      delete_character();
      return FALSE;
    }
   /* is finished the line */
  if ((pos->x + extents.x_advance >= screen_width))
  {
    pos->x = 0;
    pos->y +=  max_font_height;

    /* go to the new line */
    move_editor_cursor();  
  }
  /* is pressed enter */
   else if ((event->keyval == GDK_Return) ||
            (event->keyval == GDK_ISO_Enter) || 	
            (event->keyval == GDK_KP_Enter))
    {
      stop_text_widget();
    } 
  else if (isprint(event->keyval))
    {
      /* The character is printable */
      char *utf8 = g_malloc(2) ;
      sprintf(utf8,"%c", event->keyval);
      
      CharInfo *charInfo = g_malloc (sizeof (CharInfo));
      charInfo->x = pos->x;
      charInfo->y = pos->y; 
      
      cairo_text_extents (text_cr, utf8, &extents);
      cairo_show_text (text_cr, utf8); 
      cairo_stroke(text_cr);
 
      charInfo->x_bearing = extents.x_bearing;
      charInfo->y_bearing = extents.y_bearing;
     
      letterlist = g_slist_prepend (letterlist, charInfo);
      
      /* move cursor to the x step */
      pos->x +=  extents.x_advance;
      move_editor_cursor();  
      
      g_free(utf8);
    }
  return TRUE;
}


void init(GtkWidget *win)
{
     text_cr = gdk_cairo_create(win->window);
     
     cairo_set_operator(text_cr,CAIRO_OPERATOR_SOURCE);
     cairo_set_transparent_color(text_cr);
     cairo_paint(text_cr);
     cairo_stroke(text_cr);   
 
     cairo_set_line_cap (text_cr, CAIRO_LINE_CAP_ROUND);
     cairo_set_line_join(text_cr, CAIRO_LINE_JOIN_ROUND); 
     cairo_set_operator(text_cr, CAIRO_OPERATOR_SOURCE);
     cairo_set_line_width(text_cr, text_pen_width);
     cairo_set_source_color_from_string(text_cr, text_color);
  
     pos = g_malloc (sizeof(Pos));
     pos->x = text_pen_width;
     pos->y = text_pen_width;
     move_editor_cursor();
}


/* The windows has been exposed and now create a transparent window */
static gboolean on_window_text_configure_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
   if (!text_cr)
   {

     init(widget);
   }
  return TRUE;
}

static gboolean on_window_text_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkDisplay* display = gdk_display_get_default ();
    GdkScreen* screen = gdk_display_get_default_screen (display);
    
     screen_width = gdk_screen_get_width (screen);
     screen_height = gdk_screen_get_height (screen);
    return TRUE;
}



void add_shape(GtkWidget *win)
{

  GdkPixmap* shape = gdk_pixmap_new (NULL, screen_width, screen_height, 1);
  cairo_t* shape_cr = gdk_cairo_create(shape);

  cairo_set_operator(shape_cr,CAIRO_OPERATOR_SOURCE);
  
  cairo_set_source_rgba (shape_cr, 0, 0, 0, 0);
  cairo_paint(shape_cr);

  gtk_widget_input_shape_combine_mask(win, shape, 0, 0);
}


/* This is called when the button is pushed */
gboolean press (GtkWidget *win,
                GdkEventButton *ev, 
                gpointer user_data)
{
   add_shape(win);
   
   pos->x = ev->x;
   pos->y = ev->y;
   move_editor_cursor();
   /* This is a trick we must found the maximum height and text_pen_width of the font */
   cairo_set_font_size (text_cr, text_pen_width * 5);
   cairo_text_extents (text_cr, "|" , &extents);
      
   max_font_height = extents.height;
   return TRUE;
}


void start_text_widget(GtkWindow *parent, char* color, int tickness)
{
  
  text_pen_width = tickness;
  text_color = color;
  create_text_window(parent);
      
  gtk_widget_set_events (text_window, GDK_EXPOSURE_MASK
			 | GDK_BUTTON_PRESS_MASK);

  gtk_widget_show_all(text_window);

  g_signal_connect(G_OBJECT(text_window), "configure-event", G_CALLBACK(on_window_text_configure_event), NULL);
  g_signal_connect(G_OBJECT(text_window), "expose-event", G_CALLBACK(on_window_text_expose_event), NULL);

  g_signal_connect (G_OBJECT(text_window), "button_press_event", 
		    G_CALLBACK(press), NULL);

  g_signal_connect (G_OBJECT(text_window), "key_press_event",
		    G_CALLBACK (key_press), NULL);

}


void stop_text_widget()
{
 if (text_cr)
   {
     merge_context(text_cr);
     cairo_destroy(text_cr);       
     text_cr = NULL;
   }
 if (text_window)
   {
    gtk_widget_destroy(text_window);
    text_window= NULL;
   }
}
