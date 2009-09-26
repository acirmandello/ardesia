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
 
/* This is the file where put the callback of the events generated by the ardesia interface
 * If you are trying to add some functionality and you have already add a button in interface
 * you are in the right place. In any case good reading!
 * Pay attentioni; all the things are delegated to the annotate commandline tool
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "interface.h"

#include "stdlib.h"
#include "unistd.h"

#include "stdio.h"
#include "time.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gconf/gconf-client.h>

#include <png.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>



int     recording = 0;

char*   color="FF0000";
char    pickedcolor[7];
int     tickness=15;


int ffmpegpid;
int annotateclientpid = -1;
char* arrow = "0";
int pencil = TRUE;

GtkColorSelectionDialog* colorDialog = NULL;


/* 
 * Create a annotate client process the annotate
 * that talk with the server process 
 */
int 
callAnnotate(char *arg1, char* arg2, char* arg3, char* arrow)

{
  if (annotateclientpid != -1)
    {
      kill(annotateclientpid,9);
    }
  pid_t pid;

  pid = fork();

  if (pid < 0)
    {
      return -1;
    }
  if (pid == 0)
    {
      char* annotate="annotate";
      char* annotatebin = (char*) malloc(160*sizeof(char));
      sprintf(annotatebin,"%s/../bin/%s", PACKAGE_DATA_DIR, annotate);
      execl(annotatebin, annotate, arg1, arg2, arg3, arrow, NULL);
      free(annotatebin);
      _exit(127);
    }
  annotateclientpid = pid;
  return pid;
}


gboolean 
quit()

{
  extern int annotatepid;
  gboolean ret=FALSE;
  if(recording)
    {
      kill(ffmpegpid,9);
    }       
  if (annotateclientpid != -1)
    {
      kill(annotateclientpid,9);
    }
  kill(annotatepid,9);
   
  /* Disalloc */
  g_object_unref ( G_OBJECT(gtkBuilder) ); 

  gtk_main_quit();; 
  exit(ret);
}


gboolean
on_quit                                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return quit();

}


gboolean
on_winMain_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return quit();

}


void
paint()

{

  pencil=TRUE;
  char* ticknessa = (char*) malloc(16*sizeof(char));
  sprintf(ticknessa,"%d",tickness);

  callAnnotate("--toggle", color, ticknessa, arrow);   
 
  free(ticknessa);

}


void
erase()

{

  pencil=FALSE;
  char* ticknessa = (char*) malloc(16*sizeof(char));
  sprintf(ticknessa,"%d",tickness);

  callAnnotate("--eraser", ticknessa, NULL, NULL); 
  
  free(ticknessa);
   
}


void
on_toolsEraser_activate                (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  erase();
}


void
on_toolsArrow_activate               (GtkToolButton   *toolbutton,
				      gpointer         user_data)
{
  if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton)))
    {
       /* if single arrow is active release it */
      GtkToggleToolButton* doubleArrowToolButton = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(gtkBuilder,"buttonDoubleArrow"));
      if (gtk_toggle_tool_button_get_active(doubleArrowToolButton))
        {
	  gtk_toggle_tool_button_set_active(doubleArrowToolButton, FALSE); 
        }
      arrow="1";
    }
  else
    {
      arrow="0";
    }
  if (pencil)
   {
     paint();
   }
   else
   {
     erase();
   }
}

void
on_toolsDoubleArrow_activate               (GtkToolButton   *toolbutton,
				      gpointer         user_data)
{
  if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton)))
    {
      /* if single arrow is active release it */
      GtkToggleToolButton* arrowToolButton = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(gtkBuilder,"buttonArrow"));
      if (gtk_toggle_tool_button_get_active(arrowToolButton))
        {
	  gtk_toggle_tool_button_set_active(arrowToolButton, FALSE); 
        }
      arrow="2";
    }
  else
    {
      arrow="0";
    }
  if (pencil)
   {
     paint();
   }
   else
   {
     erase();
   }
}

void
on_toolsVisible_activate               (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  callAnnotate("--visibility", NULL, NULL, NULL );
}


/* 
 * Create a annotate client process the annotate
 * that talk with the server process 
 */
int
startFFmpeg(char *argv[])

{
  pid_t pid;

  pid = fork();

  if (pid < 0)
    {
      return -1;
    }
  if (pid == 0)
    {
      execvp(argv[0], argv);
    }
  return pid;
}


const gchar *
get_desktop_dir (void)
{
  GConfClient *gconf_client = NULL;
  gboolean desktop_is_home_dir = FALSE;
  const gchar* desktop_dir = NULL;

  gconf_client = gconf_client_get_default ();
  desktop_is_home_dir = gconf_client_get_bool (gconf_client,
                                               "/apps/nautilus/preferences/desktop_is_home_dir",
                                               NULL);
  if (desktop_is_home_dir)
    {
      desktop_dir = g_get_home_dir ();
    }
  else
    {
      desktop_dir = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
    }

  g_object_unref (gconf_client);

  return desktop_dir;
}


gboolean
save_png (GdkPixbuf *pixbuf,
	  const char *filename)
{
  FILE *handle;
  int width, height, depth, rowstride;
  gboolean has_alpha;
  guchar *pixels;
  png_structp png_ptr;
  png_infop info_ptr;
  png_text text[2];
  int i;

  handle = fopen (filename, "w");
  if (!handle) {
    return FALSE;
  }

  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    return FALSE;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    png_destroy_write_struct (&png_ptr, (png_infop *) NULL);
    return FALSE;
  }

  if (setjmp (png_ptr->jmpbuf)) {
    /* Error handler */
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return FALSE;
  }

  png_init_io (png_ptr, handle);
  
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

  if (has_alpha) {
    png_set_IHDR (png_ptr, info_ptr, width, height,
		  depth, PNG_COLOR_TYPE_RGB_ALPHA,
		  PNG_INTERLACE_NONE,
		  PNG_COMPRESSION_TYPE_DEFAULT,
		  PNG_FILTER_TYPE_DEFAULT);
  } else {
    png_set_IHDR (png_ptr, info_ptr, width, height,
		  depth, PNG_COLOR_TYPE_RGB,
		  PNG_INTERLACE_NONE,
		  PNG_COMPRESSION_TYPE_DEFAULT,
		  PNG_FILTER_TYPE_DEFAULT);
  }

  /* Some text to go with the image */
  text[0].key = "Title";
  text[0].text = (char *)filename;
  text[0].compression = PNG_TEXT_COMPRESSION_NONE;
  text[1].key = "Software";
  text[1].text = "Ardesia";
  text[1].compression = PNG_TEXT_COMPRESSION_NONE;
  png_set_text (png_ptr, info_ptr, text, 2);

  png_write_info (png_ptr, info_ptr);
  for (i = 0; i < height; i++) {
    png_bytep row_ptr = pixels;

    png_write_row (png_ptr, row_ptr);
    pixels += rowstride;
  }

  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);

  fflush (handle);
  fclose (handle);

  return TRUE;
}


void makeScreenshot(char* filename)
{

  gint height = gdk_screen_height ();
  gint width = gdk_screen_width ();

  GdkWindow* root = gdk_get_default_root_window ();
  GdkPixbuf* buf = gdk_pixbuf_get_from_drawable (NULL, root, NULL,
						 0, 0, 0, 0, width, height);
  save_png (buf, filename);
  g_object_unref (G_OBJECT (buf));

}


char* getDate()
{
  struct tm* t;
  time_t now;
  time( &now );
  t = localtime( &now );

  char* date = malloc(64*sizeof(char));
  sprintf(date, "%d-%d-%d_%d:%d:%d", t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec);
  return date;
}


void
on_toolsScreenShot_activate	       (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  char * date = getDate();
  const gchar* desktopDir = get_desktop_dir();	
  char* filename =  malloc(256*sizeof(char));
  sprintf(filename,"%s/ardesia_%s.png", desktopDir, date);
  makeScreenshot(filename);
  free(date);
  free(filename);
  paint();
}


void
on_toolsRecorder_activate              (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if(recording)
    {
      kill(ffmpegpid,9);
      recording=0;
      /* put icon to record */
      char* location = PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps/record.png" ;
      GtkWidget * label_widget = gtk_image_new_from_file (location);
      gtk_tool_button_set_label_widget (toolbutton, label_widget);
      /* set stop tooltip */ 
      gtk_tool_item_set_tooltip_text((GtkToolItem *) toolbutton,"Record");
      /* show */ 
      gtk_widget_show(label_widget);
    }
  else
    {       
      char* screen_dimension=malloc(16*sizeof(char));
      gint screen_height = gdk_screen_height ();
      gint screen_width = gdk_screen_width ();
      sprintf(screen_dimension,"%dx%d", screen_width, screen_height);

      char* filename =  malloc(128*sizeof(char));
      char* date = getDate();
      const gchar* desktopDir = get_desktop_dir();	

      sprintf(filename,"%s/ardesia_%s.mpeg", desktopDir, date);
      free(date);
      char* argv[11];
      argv[0] = "ffmpeg";
      argv[1] = "-f";
      argv[2] = "x11grab";
      argv[3] = "-r";
      argv[4] = "25";
      argv[5] = "-s";
      argv[6] = screen_dimension;
      argv[7] = "-i";
      argv[8] = ":0.0";
      argv[9] = filename;
      argv[10] = (char*) NULL ;
      ffmpegpid = startFFmpeg(argv);
      recording=1;
      free(screen_dimension);
      free(filename);
      /* put icon to stop */
      char* location = PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps/stop.png" ;
      GtkWidget * label_widget = gtk_image_new_from_file (location);
      gtk_tool_button_set_label_widget (toolbutton, label_widget);
      /* set stop tooltip */ 
      gtk_tool_item_set_tooltip_text((GtkToolItem *) toolbutton,"Stop");
      /* show */ 
      gtk_widget_show(label_widget);
  
    }
  paint();
}


void thick()

{

  if (pencil)
    {
      paint();
    }
  else
    {
      erase();
    }

}


void
on_thickScale_value_changed		(GtkHScale   *hScale,
					 gpointer         user_data)
{
  tickness=gtk_range_get_value(&hScale->scale.range);
  thick();

}


void
on_colorBlack_activate                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color="000000";
  paint();

}


void
on_colorBlue_activate                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "3333CC";
  paint();

}


void
on_colorRed_activate                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "FF0000";
  paint();

}


void
on_colorGreen_activate                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "008000";
  paint();

}


void
on_buttonPicker_activate	        (GtkToolButton   *toolbutton,
					 gpointer         user_data)
{
  if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton)))
    {
      /* open color widget */
      colorDialog = GTK_COLOR_SELECTION_DIALOG(gtk_builder_get_object(gtkBuilder,"colorselectiondialog"));
  
      if (annotateclientpid != -1)
	{
	  kill(annotateclientpid,9);
	}  
      gtk_dialog_run((GtkDialog *) colorDialog);
    }

}


void
on_colorsel_color_selection_color_changed(GtkColorSelection   *colorsel,
                                          gpointer         user_data)
{
  GdkColor          *color= g_malloc (sizeof (GdkColor));
  gtk_color_selection_get_current_color   (colorsel, color);
  gtk_color_selection_set_previous_color(colorsel, color);
  /* set color */
  gchar* col = gdk_color_to_string(color);
  /* e.g. color #20200000ffff */ 
  pickedcolor[0]=col[1];
  pickedcolor[1]=col[2];
  pickedcolor[2]=col[5];
  pickedcolor[3]=col[6];
  pickedcolor[4]=col[9];
  pickedcolor[5]=col[10];
  pickedcolor[6]=0;
  /* e.g. pickedcolor 2000ff */ 
  g_free(color);

}


void
on_colorsel_ok_button_activate          (GtkButton   *okbutton,
					 gpointer         user_data)
{
  /* close dialog */
  gtk_widget_hide((GtkWidget*) colorDialog);
  color=pickedcolor;
  paint();
  
}


void
on_colorsel_cancel_button_activate          (GtkButton   *okbutton,
					     gpointer         user_data)
{
  /* close dialog */
  gtk_widget_hide((GtkWidget*) colorDialog);
  paint();
  
}


void
on_colorLightBlue_activate             (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "00C0FF";
  paint();

}


void
on_colorLightGreen_activate            (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "00FF00";
  paint();

}


void
on_colorMagenta_activate               (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
 
  color = "FF00FF";
  paint();

}


void
on_colorOrange_activate                (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "FF8000";
  paint();

}


void
on_colorYellow_activate                (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "FFFF00";
  paint();

}


void
on_colorWhite_activate                 (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{

  color = "FFFFFF";
  paint();

}
