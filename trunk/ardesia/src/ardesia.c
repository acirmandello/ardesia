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


#include <utils.h>
#include <annotation_window.h>
#include <background_window.h>
#include <project_dialog.h>
#include <share_confirmation_dialog.h>
#include <bar.h>


#ifdef HAVE_BACKTRACE
#include <glibc_backtrace.h>
#endif

#ifdef _WIN32
#include <windows_backtrace.h>
#endif


/* Print the version of the tool and exit */
static void print_version()
{
  g_printf("Ardesia %s; the free digital sketchpad\n\n", PACKAGE_VERSION);
  exit(EXIT_FAILURE);
}


/* Print the command line help */
static void print_help()
{
  gchar* year = "2009-2010";
  gchar* author = "Pietro Pilolli";
  g_printf("Usage: %s [options] [filename]\n\n", PACKAGE_NAME);
  g_printf("Ardesia the free digital sketchpad\n\n");
  g_printf("options:\n");
  g_printf("  --verbose ,\t-V\t\tEnable verbose mode to see the logs\n");
  g_printf("  --decorate,\t-d\t\tDecorate the window with the borders\n");
  g_printf("  --gravity ,\t-g\t\tSet the gravity of the bar. Possible values are:\n");
  g_printf("  \t\t\t\teast [default]\n");
  g_printf("  \t\t\t\twest\n");
  g_printf("  \t\t\t\tnorth\n");
  g_printf("  \t\t\t\tsouth\n");
  g_printf("  --help    ,\t-h\t\tShows the help screen\n");
  g_printf("  --version ,\t-v\t\tShow version information and exit\n");
  g_printf("\n");
  g_printf("filename:\t  \t\tThe interactive Whiteboard Common File (iwb)\n");
  g_printf("\n");
  g_printf("%s (C) %s %s\n", PACKAGE_STRING, year, author);
  exit(EXIT_SUCCESS);
}


#ifndef _WIN32
/* Call the dialog that inform the user to enable a composite manager */
static void run_missing_composite_manager_dialog()
{
  GtkWidget *msg_dialog; 
  msg_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, 
				      GTK_BUTTONS_OK, 
				      gettext("In order to run Ardesia you need to enable a composite manager")
                                      );
                 
  gtk_dialog_run(GTK_DIALOG(msg_dialog));
   
  if (msg_dialog != NULL)
    {
      gtk_widget_destroy(msg_dialog);
    }
  exit(EXIT_FAILURE);
}


/* Check if a composite manager is active */
static void check_composite_manager(GdkScreen* screen)
{
  gboolean composite = gdk_screen_is_composited(screen);
  if (!composite)
    {
      /* start the enable composite manager dialog */
      run_missing_composite_manager_dialog();
    }
}
#endif


/* Set the best colormap available on the system */
static void set_the_best_colormap()
{
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen   *screen = gdk_display_get_default_screen(display);
  GdkVisual* visual = NULL;

  /* In FreeBSD operating system you might have a composite manager */
#if ( defined(__freebsd__) || defined(__freebsd) || defined(_freebsd) || defined(freebsd) )
  check_composite_manager(screen);
#endif

  /* In linux operating system you must have a composite manager */
#ifdef linux
  check_composite_manager(screen);
#endif

  GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
  if (colormap)
    {
      gtk_widget_set_default_colormap(colormap);
    }
  else
    {
      g_warning("The screen does not support the alpha channel\n");
      visual = gdk_screen_get_rgba_visual(screen);
      if (visual == NULL)
	{
	  g_warning("The screen does not support the rgba visual!\n");
#ifndef _WIN32
	  exit(EXIT_FAILURE);
#endif
	}
    }
}


/* Parse the command line in the standar getopt way */
static CommandLine* parse_options(gint argc, char *argv[])
{
  CommandLine* commandline = g_malloc((gsize) sizeof(CommandLine)); 
  
  commandline->position = EAST;
  commandline->debug = FALSE;
  commandline->iwbfile = NULL;
  commandline->decorated=FALSE;

  /* getopt_long stores the option index here. */
  while (1)
    {
      gint c;
      static struct option long_options[] =
	{
	  /* These options set a flag. */
	  {"help", no_argument,       0, 'h'},
          {"decorated", no_argument,  0, 'd'},
	  {"verbose", no_argument,    0, 'V' },
          {"version", no_argument,    0, 'v'},
	  /* These options don't set a flag.
	     We distinguish them by their indices. */
	  {"gravity", required_argument, 0, 'g'},
	  {0, 0, 0, 0}
	};

      gint option_index = 0;
      c = getopt_long(argc, argv, "hdvVg:",
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
	  print_version();
	  break;
        case 'd':
	  commandline->decorated=TRUE;
	  break;
	case 'V':
	  commandline->debug=TRUE;
	  break;
	case 'g':
	  if (g_strcmp0(optarg, "east") == 0)
	    {
	      commandline->position = EAST;
	    }
	  else if (g_strcmp0(optarg, "west") == 0)
	    {
	      commandline->position = WEST;
	    }
          else if (g_strcmp0(optarg, "north") == 0)
	    {
	      commandline->position = NORTH;
	    }
          else if (g_strcmp0(optarg, "south") == 0)
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
      commandline->iwbfile = argv[optind];
    } 
  return commandline;
}


/* Enable the localization support with gettext */
static void enable_localization_support()
{
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain(GETTEXT_PACKAGE);
#endif

  gtk_set_locale();
}


/* 
 * This function create a segmentation fault; 
 * it is useful to test the segmentation fault handler
 */
/*
static void create_segmentation_fault()
{
  int *f=NULL;
  *f = 0;
}
*/


/* Create a shorcut to the workspace on the desktop */
static void create_workspace_shortcut(gchar* workspace_dir)
{
  gchar* desktop_entry_filename = g_strdup_printf("%s%s%s_workspace", get_desktop_dir(), G_DIR_SEPARATOR_S, PACKAGE_NAME);
#ifdef _WIN32
  windows_create_link(workspace_dir , desktop_entry_filename, "%SystemRoot%\\system32\\imageres.dll", 123);
#else
  xdg_create_link(workspace_dir , desktop_entry_filename, "folder-documents");
#endif
  g_free(desktop_entry_filename);  
}


/* Setup the workspace */
static gchar* configure_workspace(gchar* project_name)
{
  /* The workspace dir is set in the documents ardesia folder */
  gchar* workspace_dir = g_strdup_printf("%s%s%s", get_documents_dir(), G_DIR_SEPARATOR_S, PACKAGE_NAME);

  create_workspace_shortcut(workspace_dir);
  
  return workspace_dir;
}


static gchar* create_default_project_dir(gchar* workspace_dir, gchar* project_name)
{
  gchar* project_dir = g_strdup_printf("%s%s%s", workspace_dir, G_DIR_SEPARATOR_S, project_name);

  if (!g_file_test(project_dir, G_FILE_TEST_EXISTS)) 
    {
      if (g_mkdir_with_parents(project_dir, 0700)==-1)
        {
           g_warning("Unable to create folder %s\n", project_dir);
        } 
    }
  return project_dir;
} 


/* This is the starting point */
int
main(gint argc, char *argv[])
{
  CommandLine *commandline = NULL;
  gchar* project_name = "";
  gchar* project_dir = "";
  gchar* iwbfile = "";
  GtkWidget* background_window = NULL; 
  GtkWidget* annotation_window = NULL; 
  GtkWidget* ardesia_bar_window = NULL; 
  GSList * artifact_list = NULL;
  
  /* Enable the localization support with gettext */
  enable_localization_support();
  
  gtk_init(&argc, &argv);

#ifdef HAVE_BACKTRACE
#ifdef HAVE_LIBSIGSEGV
  glibc_backtrace_register();
#endif
#endif
#ifdef _WIN32
  windows_backtrace_register();
#endif

  /*
   * Uncomment this and the create_segmentation_fault function 
   * to create a segmentation fault 
   */
  //create_segmentation_fault();

  set_the_best_colormap();

  commandline = parse_options(argc, argv);

  if (commandline->iwbfile)
    {
       iwbfile = commandline->iwbfile;
       int initpos = g_substrlastpos(commandline->iwbfile, G_DIR_SEPARATOR_S); 
       int endpos  = g_substrlastpos(commandline->iwbfile, ".");
       project_name = g_substr(commandline->iwbfile, initpos+1, endpos-1);   
       project_dir = g_substr(commandline->iwbfile, 0, initpos-1); 
    }
  else
    {
       /* show the project name wizard */
       project_name = start_project_dialog(NULL);
       gchar* workspace_dir = configure_workspace(project_name);
       project_dir = create_default_project_dir(workspace_dir, project_name);
       g_free(workspace_dir);
       /* will be putted in the project dir */
       gchar* extension = "iwb";
       /* the zip is the iwb in the project inside the ardesia workspace */
       iwbfile = g_strdup_printf("%s%s%s.%s", project_dir, G_DIR_SEPARATOR_S, project_name, extension);
    }
  set_project_name(project_name);
  set_project_dir(project_dir);
  set_iwbfile(iwbfile);

  background_window = create_background_window(); 
  
  if (background_window == NULL)
    {
      destroy_background_window();
      g_free(commandline);
      exit(EXIT_FAILURE);
    }

  gtk_window_set_keep_above(GTK_WINDOW(background_window), TRUE); 
  gtk_widget_show(background_window);
  
  set_background_window(background_window);
  
  /* init annotate */
  annotate_init(background_window, commandline->iwbfile, commandline->debug); 

  annotation_window = get_annotation_window();  

  if (annotation_window == NULL)
    {
      annotate_quit();
      destroy_background_window();
      g_free(commandline);
      exit(EXIT_FAILURE);
    }

  /* annotation window is valid */
  gtk_window_set_keep_above(GTK_WINDOW(annotation_window), TRUE);


  gtk_widget_show(annotation_window);
  
  ardesia_bar_window = create_bar_window(commandline, annotation_window);
  
  if (ardesia_bar_window == NULL)
    {
      annotate_quit();
      destroy_background_window();
      g_free(commandline);
      exit(EXIT_FAILURE);
    }

  g_free(commandline);

  gtk_window_set_keep_above(GTK_WINDOW(ardesia_bar_window), TRUE);
  gtk_widget_show(ardesia_bar_window);
  gtk_main();
  
  artifact_list = get_artifacts();

  if (artifact_list)
    {  
       start_share_dialog(NULL);
       free_artifacts();  
    }
         
  g_free(project_name);
  g_free(project_dir);
  g_free(iwbfile);

  return 0;
}


