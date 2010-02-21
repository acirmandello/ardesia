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


/*
 * Initial main.c.
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <gtk/gtk.h>

#include "annotate.h"
#include "interface.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "background.h"
#include "gettext.h"

#include <X11/Xutil.h>

int NORTH=1;
int SOUTH=2;

/* space from border in pixel */
int SPACE_FROM_BORDER = 25;


/* Get the screen resolution asking to the Xorg server throught the xlib libraries */
int getScreenResolution(Display *display, int *width, int *height)
{
  Screen *screen = XDefaultScreenOfDisplay(display);
  *width  = XWidthOfScreen (screen);
  *height = XHeightOfScreen(screen);
  return 0;
}


/* 
 * Calculate the position where calculate_initial_position the main window 
 * the centered position upon the window manager bar
 */
void calculate_centered_position(GtkWidget *ardesiaBarWindow, int dwidth, int dheight, int *x, int *y, int wwidth, int wheight, int position)
{
  *x = (dwidth - wwidth)/2;
  if (position==NORTH)
    {
      *y = SPACE_FROM_BORDER; 
    }
  else if (position==SOUTH)
    {
      /* south */
      *y = dheight - SPACE_FROM_BORDER - wheight;
    }  
  else
    {
      /* invalid position */
      perror ("Valid position are NORTH or SOUTH\n");
      exit(0);
    }
}


/* 
 * Calculate initial position
 * Here can be beatiful have a configuration file
 * where put the user can decide the position of the window
 */
int calculate_initial_position(GtkWidget *ardesiaBarWindow, int *x, int *y, int wwidth, int wheight, int position)
{
  Display *display = XOpenDisplay (0);  
  int dwidth, dheight;
  if (getScreenResolution(display, &dwidth, &dheight)<0) 
    {
      perror ("Fatal error while getting screen resolution\n");
      return -1;
    }
  calculate_centered_position(ardesiaBarWindow,dwidth,dheight, x, y, wwidth, wheight, position);
  XCloseDisplay (display);
  return 0; 
}


/* Print command line help */
void print_help()
{
  printf("Usage: ardesia [options]\n\n");
  printf("options:\n");
  printf("--gravity,\t-g\t\tSet the gravity of the bar. Possible values are:\n");
  printf("\t\t\t\tnorth\n");
  printf("\t\t\t\tsouth\n");
  printf("--help,   \t-h\t\tShows the help screen\n");
  printf("\n");
}


/* check if a composite manager is active */
void check_composite()
{
  GdkDisplay *display = gdk_display_get_default ();
  GdkScreen   *screen = gdk_display_get_default_screen (display);
  gboolean composite = gdk_screen_is_composited (screen);
  if (!composite)
    {
      /* composite manager must be enabled */
      GtkWidget *msg_dialog; 
      msg_dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, 
                                           GTK_BUTTONS_OK, gettext("In order to run Ardesia you need to enable the Compiz composite manager"));
      gtk_window_stick((GtkWindow*)msg_dialog);
                 
      gtk_dialog_run(GTK_DIALOG(msg_dialog));
      if (msg_dialog != NULL)
        {
	  gtk_widget_destroy(msg_dialog);
        }
      exit(0);
    }
  gtk_widget_set_default_colormap(gdk_screen_get_rgba_colormap(screen));
 }


/* 
 * The main entry of the program;
 * not all the koders 
 * start to read the code by here 
 * a lot of 
 * are only patchers 
 * bad merchants
 * hopeless 
 * and without any horizon;
 * avoid to 
 * stay closed to 
 * this kind of people
 * if you are not immune from 
 * the enemy that 
 * tains the world: 
 * the slavery
 */
int main (int argc, char *argv[])
{
  int position = SOUTH;
  char* arg = NULL;
  gboolean loadbackground = FALSE;

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);
#endif

  gboolean debug = FALSE;
 
  int i=1;
  if (argc>i)
    {
     arg = argv[i];
     if ((strcmp (arg, "--verbose") == 0) 
	  || (strcmp (arg, "-v") == 0))
	{
           debug=TRUE;
          if (argc>i+1)
          {
            i = i+1;
          }
        }
      arg = argv[i];
      if ((strcmp (arg, "--gravity") == 0) 
	  || (strcmp (arg, "-g") == 0))
	{
	  if (argc>i+1)
	    {
	      arg = argv[i+1];
	      if (strcmp (arg, "north") == 0)
		{
		  position = NORTH;
		}
	      else if (strcmp (arg, "south") == 0)
		{
		  position = SOUTH;
		}
	      else
		{
		  print_help();
		  exit(1);
		}
	      if (argc>i+2)
		{
                  arg=argv[i+2];
		}

              else
                {
                  arg=NULL;
                }
	    }
	  else
	    {
	      printf("Required missing argument\n");
	      print_help();
	      exit(0);  
	    }
	}
      else if ((strcmp (arg, "--help") == 0) 
	       || (strcmp (arg, "-h") == 0))
	{
	  print_help();
	  exit(1); 
	}
      if (arg)
      {
        if (!(strncmp (arg, "-",1) == 0))
          {
            loadbackground = TRUE;   
          }
      }
    } 

  gtk_set_locale ();
  gtk_init (&argc, &argv);
  check_composite();
  
  /*
   * The following code create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  GtkWidget *ardesiaBarWindow = create_mainWindow ();
  int width;
  int height;
  gtk_window_get_size (GTK_WINDOW(ardesiaBarWindow) , &width, &height);

  int x, y;

  /* calculate_initial_position the window in the desired position */
  if (calculate_initial_position(ardesiaBarWindow,&x,&y,width,height,position) < 0)
    {
      exit(0);
    }
  gtk_window_move(GTK_WINDOW(ardesiaBarWindow),x,y);  
  
  /** Init annotate */
  annotate_init(x, y, width, height, debug);

  if (loadbackground)
  {
     change_background_image(arg); 
  }
 
  GtkWindow* annotation_window = get_annotation_window();  
  if (annotation_window)
  {
    gtk_window_set_transient_for(GTK_WINDOW(ardesiaBarWindow), annotation_window );
  }

  gtk_widget_show (ardesiaBarWindow);
  
  gtk_main ();
 
  gtk_widget_destroy(ardesiaBarWindow); 

  return 0;
}
