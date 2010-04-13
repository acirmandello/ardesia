/* 
 * Ardesia -- a program for painting on the screen
 * with this program you can play, draw, learn and teach
 * This program has been written such as a freedom sonet
 * We believe in the freedom and in the freedom of education
 *
 * Copyright (C) 2009 Pilolli Pietro <pilolli@fbk.eu>
 *
 * Ardesia is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Ardesia is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <config.h>

#include <gtk/gtk.h>
#include "stdlib.h"


/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif



#include "callbacks.h"
#include "annotate.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "getopt.h"
#include "utils.h"

#ifdef _WIN32
  #include <gdkwin32.h>
  #define UI_FILE "ardesia.ui"
#else
  #define UI_FILE PACKAGE_DATA_DIR"/ardesia/ui/ardesia.ui"
#endif 



int NORTH=1;
int SOUTH=2;

/* space from border in pixel */
int SPACE_FROM_BORDER = 25;

GtkBuilder *gtkBuilder;



typedef struct
{
  char* backgroundimage;
  gboolean debug;
  int position;
} CommandLine;


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
  gint dwidth = gdk_screen_width ();
  gint dheight = gdk_screen_height ();
  calculate_centered_position(ardesiaBarWindow, dwidth, dheight, x, y, wwidth, wheight, position);
  return 0; 
}


/* Print command line help */
void print_help()
{
  char* version = "0.3";
  char* year = "2009-2010";
  char* author = "Pietro Pilolli";
  printf("Usage: ardesia [options] [filename]\n\n");
  printf("options:\n");
  printf("  --verbose,\t-v\t\tEnable verbose mode to see the logs\n");
  printf("  --gravity,\t-g\t\tSet the gravity of the bar. Possible values are:\n");
  printf("  \t\t\t\tnorth\n");
  printf("  \t\t\t\tsouth\n");
  printf("  --help,   \t-h\t\tShows the help screen\n");
  printf("\n");
  printf("filename:\t  \t\tThe file containig the image to be be used as background\n");
  printf("\n");
  printf("Ardesia %s (C) %s %s\n", version, year, author);
  exit(1);
}


/* check if a composite manager is active */
void check_composite()
{
  #ifndef _WIN32
    GdkDisplay *display = gdk_display_get_default ();
    GdkScreen   *screen = gdk_display_get_default_screen (display);
    gboolean composite = gdk_screen_is_composited (screen);
    if (!composite)
      {
        /* composite manager must be enabled */
        GtkWidget *msg_dialog; 
        msg_dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, 
                                             GTK_BUTTONS_OK, gettext("In order to run Ardesia you need to enable the Compiz composite manager"));
        if (STICK)
        {
          gtk_window_stick((GtkWindow*)msg_dialog);
        }
                 
        gtk_dialog_run(GTK_DIALOG(msg_dialog));
        if (msg_dialog != NULL)
          {
	    gtk_widget_destroy(msg_dialog);
          }
        exit(0);
      }
    GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
    if (colormap)
      {
        gtk_widget_set_default_colormap(colormap);
      }
  #endif
}


CommandLine* parse_options(int argc, char *argv[])
{
  CommandLine* commandline = g_malloc(sizeof(CommandLine)); 
  commandline->position = SOUTH;
  commandline->debug = FALSE;
  commandline->backgroundimage = NULL;

  /* getopt_long stores the option index here. */
  while (1)
    {
      int c;
      static struct option long_options[] =
	{
	  /* These options set a flag. */
	  {"help", no_argument,       0, 'h'},
	  {"verbose", no_argument,       0, 'v'},
	  /* These options don't set a flag.
	     We distinguish them by their indices. */
	  {"gravity", required_argument, 0, 'g'},
	  {0, 0, 0, 0}
	};

     
      int option_index = 0;
      c = getopt_long (argc, argv, "hvg:",
		       long_options, &option_index);
     
      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
	case 'h':
	  print_help();
	  break;
	case 'v':
	  commandline->debug=TRUE;
	  break;
	case 'g':
	  if (strcmp (optarg, "north") == 0)
	    {
	      commandline->position = NORTH;
	    }
	  else if (strcmp (optarg, "south") == 0)
	    {
	      commandline->position = SOUTH;
	    }
	  else
	    {
	      print_help();
	    }
	  break;
	default:
	  print_help();
	  break;
	} 
    }
  if (optind<argc)
    {
      commandline->backgroundimage = argv[optind];
    } 
  return commandline;
}

/* Set the defult width of the pen line */
void setInitialWidth(int val)
{
  GtkWidget* widthWidget = GTK_WIDGET(gtk_builder_get_object(gtkBuilder,"thickScale"));
  GtkHScale* hScale = (GtkHScale *) widthWidget;
  gtk_range_set_value(&hScale->scale.range, val);
}


GtkWidget*
create_window (void)
{
  GtkWidget *window;
  GError* error = NULL;

  gtkBuilder = gtk_builder_new ();
  if (!gtk_builder_add_from_file (gtkBuilder, UI_FILE, &error))
    {
      g_warning ("Couldn't load builder file: %s", error->message);
      g_error_free (error);
    }

  /* This is important; connect all the callback from gtkbuilder xml file */
  gtk_builder_connect_signals (gtkBuilder, NULL);
  window = GTK_WIDGET (gtk_builder_get_object (gtkBuilder, "winMain"));
  /* Set the width to 15 in the thick scale */
  setInitialWidth(15);

  return window;
}


int
main (int argc, char *argv[])
{
  GtkWidget *ardesiaBarWindow ;

  #ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
  #endif


  gtk_set_locale ();
  gtk_init (&argc, &argv);

  check_composite();

  ardesiaBarWindow = create_window ();
  /*
   * The following code create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  int width;
  int height;
  gtk_window_get_size (GTK_WINDOW(ardesiaBarWindow) , &width, &height);

  int x, y;

  CommandLine* commandline = parse_options(argc, argv);

  /* calculate_initial_position the window in the desired position */
  if (calculate_initial_position(ardesiaBarWindow,&x,&y,width,height,commandline->position) < 0)
    {
      exit(0);
    }
  gtk_window_move(GTK_WINDOW(ardesiaBarWindow),x,y);  

  /** Init annotate */
  annotate_init(x, y, width, height, commandline->debug, commandline->backgroundimage); 

  gtk_widget_show (ardesiaBarWindow);
  gtk_window_present(get_annotation_window());

  gtk_main ();
  g_object_unref (gtkBuilder);	
  return 0;
}
