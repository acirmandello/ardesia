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
#include <string.h> 

#include "stdio.h"
#include "time.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gconf/gconf-client.h>

#include <math.h>
#include <pngutils.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


/* annotation is visible */
gboolean     visible = TRUE;

/* pencil is selected */
gboolean     pencil = TRUE;

/* selected color */
char*        color="FF0000";

/* selected line width */
int          tickness=15;

/* pid of the ffmpeg process for the recording */
int ffmpegpid = -1;

/* pid of the current running instance of the annotate client */
int annotateclientpid = -1;

/* arrow=0 mean disabled, arrow=1 mean arrow, arrow=2 mean double arrow */
char* arrow = "0";

/* Used to record the history of the last color selected */
GdkColor*  gdkcolor= NULL;


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

/* Return if the recording is active */
gboolean is_recording()
{
  if (ffmpegpid==-1)
    {
      return FALSE;
    }
  return TRUE;
}

/* Called when close the program */
gboolean 
quit()

{
  extern int annotatepid;
  gboolean ret=FALSE;
  if(is_recording())
    {
      kill(ffmpegpid,9);
    }       
  if (annotateclientpid != -1)
    {
      kill(annotateclientpid,9);
    }
  kill(annotatepid,9);
  remove_background();
  /* Disalloc */
  g_object_unref ( G_OBJECT(gtkBuilder) ); 

  gtk_main_quit();; 
  exit(ret);
}

/* Start to paint calling annotate */
void
paint()

{

  pencil=TRUE;
  char* ticknessa = (char*) malloc(16*sizeof(char));
  sprintf(ticknessa,"%d",tickness);

  callAnnotate("--toggle", color, ticknessa, arrow);   
 
  free(ticknessa);

}


/* Start to erase calling annotate */
void
erase()

{

  pencil=FALSE;
  char* ticknessa = (char*) malloc(16*sizeof(char));
  sprintf(ticknessa,"%d",tickness);

  callAnnotate("--eraser", ticknessa, NULL, NULL); 
  
  free(ticknessa);
   
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

/*
 * Get the desktop folder;
 * Now this function use gconf to found the folder,
 * this means that this rutine works properly only
 * with the gnome desktop environment
 * We can investigate how-to generalize this 
 */
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


/* Get the current date and format in a printable format */
char* get_date()
{
  struct tm* t;
  time_t now;
  time( &now );
  t = localtime( &now );

  char* date = malloc(64*sizeof(char));
  sprintf(date, "%d-%d-%d_%d:%d:%d", t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec);
  return date;
}


/* 
 * This function is called when the thick property is changed;
 * start paint with pen or eraser depending on the selected tool
 */
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


/*
 * Get the current selected color and store it in the esadecimal format
 * in the pickedcolor variable
 */
void 
get_selected_color		 (char* pickedcolor, GtkColorSelection   *colorsel)

{
  gtk_color_selection_get_current_color   (colorsel, gdkcolor);
  /* set color */
  gchar* col = gdk_color_to_string(gdkcolor);
  /* e.g. color #20200000ffff */ 
  char tmpcolor[7]; 
  tmpcolor[0]=col[1];
  tmpcolor[1]=col[2];
  tmpcolor[2]=col[5];
  tmpcolor[3]=col[6];
  tmpcolor[4]=col[9];
  tmpcolor[5]=col[10];
  tmpcolor[6]=0;
 
  strncpy(&pickedcolor[0],tmpcolor,7);
  /* e.g. pickedcolor 2000ff */ 
}

/* Return if a file exists */
gboolean file_exists(char* filename)
{
  struct stat statbuf;
  if(stat(filename, &statbuf) < 0) {
    if(errno == ENOENT) {
      return FALSE;
    } else {
      fprintf(stderr, "could not stat %s\n", filename);
      perror("");
      exit(0);
    }
  }
  return TRUE;
}


/* Start event handler section */


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
  if (visible)
    {
      visible=FALSE;
      /* set tooltip to unhide */
      gtk_tool_item_set_tooltip_text((GtkToolItem *) toolbutton,"Unhide");
    }
  else
    {
      visible=TRUE;
      /* set tooltip to hide */
      gtk_tool_item_set_tooltip_text((GtkToolItem *) toolbutton,"Hide");
    }
}


void
on_toolsScreenShot_activate	       (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  char * date = get_date();
  const gchar* desktop_dir = get_desktop_dir();	
  char* filename =  malloc(256*sizeof(char));

  sprintf(filename,"ardesia_%s", date);
  GtkWidget *chooser = gtk_file_chooser_dialog_new ("Save image as png", NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    GTK_STOCK_SAVE_AS, GTK_RESPONSE_ACCEPT,
						    NULL);
  
  gtk_window_set_title (GTK_WINDOW (chooser), "Select a file");
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), desktop_dir);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), filename);
  
  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {

      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
      desktop_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));

      filename = (gchar *) realloc(filename,  (strlen(filename) + 4 + 1) * sizeof(gchar)); 
      (void) strcat((gchar *)filename, ".png");
               
 
      gtk_widget_destroy (chooser);
      if (file_exists(filename) == TRUE)
        {
	  GtkWidget *msg_dialog; 
                   
	  msg_dialog = gtk_message_dialog_new (GTK_WINDOW(toolbutton), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,  GTK_BUTTONS_YES_NO, "File Exists. Overwrite");

                 
	  if (gtk_dialog_run(GTK_DIALOG(msg_dialog)) == GTK_RESPONSE_NO)
            { 
	      gtk_widget_destroy(msg_dialog);
	      g_free(filename);
	      return; 
	    } 
	  else 
            {
              gtk_widget_destroy(msg_dialog);
            }
	}
      makeScreenshot(filename);
    }
  else
    {
      gtk_widget_destroy (chooser);
    } 

  paint();
  free(date);
  free(filename);
}


void
on_toolsRecorder_activate              (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  if(is_recording())
    {
      kill(ffmpegpid,9);
      ffmpegpid=-1;
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
   
      char * date = get_date();
      const gchar* desktop_dir = get_desktop_dir();	
      char* filename =  malloc(256*sizeof(char));

      sprintf(filename,"ardesia_%s", date);
      GtkWidget *chooser = gtk_file_chooser_dialog_new ("Save video as mpeg", NULL , GTK_FILE_CHOOSER_ACTION_SAVE,
							GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							GTK_STOCK_SAVE_AS, GTK_RESPONSE_ACCEPT,
							NULL);
  
      gtk_window_set_title (GTK_WINDOW (chooser), "Select a file");
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), desktop_dir);
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), filename);
  
      if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{

	  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	  desktop_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser));

	  filename = (gchar *) realloc(filename,  (strlen(filename) + 5 + 1) * sizeof(gchar)); 
	  (void) strcat((gchar *)filename, ".mpeg");
               
 
	  gtk_widget_destroy (chooser);
	  if (file_exists(filename) == TRUE)
	    {
	      GtkWidget *msg_dialog; 
                   
	      msg_dialog = gtk_message_dialog_new (GTK_WINDOW(toolbutton), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,  GTK_BUTTONS_YES_NO, "File Exists. Overwrite");

                 
	      if (gtk_dialog_run(GTK_DIALOG(msg_dialog)) == GTK_RESPONSE_NO)
		{ 
		  gtk_widget_destroy(msg_dialog);
		  g_free(filename);
		  return; 
		} 
	      else 
		{
		  gtk_widget_destroy(msg_dialog);
		}
	    }
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
	  free(screen_dimension);
	  /* put icon to stop */
	  char* location = PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps/stop.png" ;
	  GtkWidget * label_widget = gtk_image_new_from_file (location);
	  gtk_tool_button_set_label_widget (toolbutton, label_widget);
	  /* set stop tooltip */ 
	  gtk_tool_item_set_tooltip_text((GtkToolItem *) toolbutton,"Stop");
	  /* show */ 
	  gtk_widget_show(label_widget);

	}
      else
        {
	  gtk_widget_destroy (chooser);
        } 

      paint();
      free(date);
      free(filename);
 
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

  GtkToggleToolButton *button = GTK_TOGGLE_TOOL_BUTTON(toolbutton);
  GtkColorSelection *colorsel;
  if (gtk_toggle_tool_button_get_active(button))
    {
      /* open color widget */
      GtkWidget* colorDialog = gtk_color_selection_dialog_new ("Changing color");
      if (gdkcolor==NULL)
	{
	  gdkcolor = g_malloc (sizeof (GdkColor));
	}
      else
	{
	  colorsel = GTK_COLOR_SELECTION ((GTK_COLOR_SELECTION_DIALOG (colorDialog))->colorsel);
	  gtk_color_selection_set_current_color(colorsel, gdkcolor);
	  gtk_color_selection_set_previous_color(colorsel, gdkcolor);
	} 

      if (annotateclientpid != -1)
	{
	  kill(annotateclientpid,9);
	}  
      gint result = gtk_dialog_run((GtkDialog *) colorDialog);

      /* Wait for user to select OK or Cancel */
      switch (result)
	{
	case GTK_RESPONSE_OK:
	  colorsel = GTK_COLOR_SELECTION ((GTK_COLOR_SELECTION_DIALOG (colorDialog))->colorsel);
          char*   pickedcolor= malloc(7*sizeof(char));;
	  get_selected_color(pickedcolor,colorsel);
          color = pickedcolor; 
	  gtk_widget_destroy(colorDialog);
	  paint(); 
	  break;
      
	default:
	  /* If dialog did not return OK then it was canceled */
	  //gtk_toggle_tool_button_set_active(button, FALSE); 
	  gtk_widget_destroy(colorDialog);
	  paint(); 
	  break;
	}

    }

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
on_buttonClear_activate                (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
  
  callAnnotate("--clear", NULL, NULL, NULL);   

}
