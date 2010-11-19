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

#include <pdf_saver.h>
#include <utils.h>
#include <saver.h>


/* internal structure allocated once */
static PdfData *pdf_data;


/* Start the dialog that ask the filename where is being exported the pdf */
gboolean start_save_pdf_dialog(GtkWindow *parent, gchar** workspace_dir, GdkPixbuf *pixbuf)
{
  gboolean ret = TRUE;
   
  GtkWidget *chooser = gtk_file_chooser_dialog_new (gettext("Export as pdf"), 
						    parent, 
						    GTK_FILE_CHOOSER_ACTION_SAVE,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    GTK_STOCK_SAVE_AS, GTK_RESPONSE_ACCEPT,
						    NULL);
  gtk_window_set_modal(GTK_WINDOW(chooser), TRUE);
   
  gtk_window_set_title (GTK_WINDOW (chooser), gettext("Choose a file")); 
 
  /* saving preview */
  GtkWidget*   preview = gtk_image_new ();
  gint preview_width = 128;
  gint preview_height = 128;
  GdkPixbuf*   previewPixbuf = gdk_pixbuf_scale_simple(pixbuf, preview_width, preview_height, GDK_INTERP_BILINEAR);
  gtk_image_set_from_pixbuf (GTK_IMAGE (preview), previewPixbuf);
  
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER(chooser), preview);   
  g_object_unref(previewPixbuf);

  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), *workspace_dir);
   
  gchar* filename = get_default_name();

  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), filename);

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      /* store the folder location this will be proposed the next time */
      g_free(*workspace_dir);
      *workspace_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(chooser)); 

      gchar* supported_extension = ".pdf";
      g_free(filename);
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

      if (!g_str_has_suffix(filename, supported_extension))
        {
          pdf_data->filename = g_strdup_printf("%s%s",filename,supported_extension);
        }      
      else
        {
          pdf_data->filename = g_strdup_printf("%s",filename);
        }
      g_free(filename);     

      if (file_exists(pdf_data->filename))
        {
	  GtkWidget *msg_dialog; 
	  msg_dialog = gtk_message_dialog_new (GTK_WINDOW(chooser), 
					       GTK_DIALOG_MODAL, 
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_YES_NO, gettext("File Exists. Overwrite"));
	  
          gint result = gtk_dialog_run(GTK_DIALOG(msg_dialog));
          if (msg_dialog != NULL)
            { 
	      gtk_widget_destroy(msg_dialog);
            }
	  if ( result == GTK_RESPONSE_NO)
            {
	      ret = FALSE;
	    } 
	}
    }
  
  gtk_widget_destroy (preview);
  if (chooser != NULL)
    {
      gtk_widget_destroy (chooser);
    }
  return ret;
}


/* Initialize the pdf saver */
gboolean init_pdf_saver(GtkWindow *parent, gchar** workspace_dir, GdkPixbuf *pixbuf)
{  
  pdf_data = (PdfData *) g_malloc(sizeof(PdfData));   
  pdf_data->thread = NULL;
  pdf_data->input_filelist = NULL;
  pdf_data->filename = NULL;
   
  /* start the widget to ask the file name where save the pdf */       
  gboolean ret = start_save_pdf_dialog(parent, workspace_dir, pixbuf);

  if (!ret)
    {
        
      return FALSE;
    }
    
  return TRUE;
}


void *pdf_save(void *arg)
{   
  gint height = gdk_screen_height ();
  gint width = gdk_screen_width ();
   
  /* create the cairo surface for pdf */
  cairo_surface_t* pdf_surface = cairo_pdf_surface_create(pdf_data->filename, width, height);
  cairo_t* pdf_cr = cairo_create(pdf_surface);

  gint lenght = g_slist_length(pdf_data->input_filelist);
   
  gint i;
  for (i=lenght-1; i>=0; i--)
    {
      gchar* current_filename = (gchar*) g_slist_nth_data (pdf_data->input_filelist, i);
      // load the filename content   
      GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (current_filename, NULL);
      gdk_cairo_set_source_pixbuf(pdf_cr, pixbuf, 0, 0);
      cairo_paint(pdf_cr);
      cairo_show_page(pdf_cr);
      g_object_unref(pixbuf);
    }

  cairo_surface_flush(pdf_surface);  
  /* destroy */
  cairo_surface_destroy(pdf_surface);
  cairo_destroy(pdf_cr);
  return (void *) NULL;
}


/* wait if there is a pending thread */
void wait_for_pdf_save_pending_thread()
{
  if (pdf_data->thread)
    {
      g_thread_join(pdf_data->thread);
      pdf_data->thread = NULL;
    }
}


/* Add the screenshot to pdf */
void add_pdf_page(GtkWindow *parent, gchar** workspace_dir)
{
  const gchar* tmpdir = g_get_tmp_dir();
  gchar* default_filename = get_default_name();
  gchar* filename  = g_strdup_printf("%s%s%s_screenshoot.png",tmpdir, G_DIR_SEPARATOR_S, default_filename);
  g_free(default_filename);


  GdkPixbuf* pixbuf = grab_screenshot();

  /* Load a surface with the data->annotation_cairo_context content and write the file */
  cairo_surface_t* saved_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gdk_screen_width(), gdk_screen_height());
  cairo_t *cr = cairo_create (saved_surface);
  gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
  cairo_paint(cr);
  cairo_surface_write_to_png (saved_surface, filename);
  cairo_surface_destroy(saved_surface); 
  cairo_destroy(cr);
   
  if (pdf_data == NULL)
    {
      if (!g_thread_supported())
	{
	  g_thread_init(NULL);
	  gdk_threads_init();                  
	  // Called to initialize internal mutex "gdk_threads_mutex".
	  g_printerr("g_thread supported\n");
	}
      if (!init_pdf_saver(parent, workspace_dir, pixbuf))
	{  
	  g_object_unref(pixbuf);
	  return;
	}
    }
  
  g_object_unref(pixbuf);
  pdf_data->input_filelist = g_slist_prepend(pdf_data->input_filelist, filename);  


  wait_for_pdf_save_pending_thread();

  // start save thread
  GError           *err = NULL ;
  if ((pdf_data->thread = g_thread_create((GThreadFunc) pdf_save, (void *) NULL, TRUE, &err)) == NULL)
    {
      g_printerr("Thread create failed: %s!!\n", err->message );
      g_error_free(err) ;
    }   

}


/* Send email */
void send_pdf_with_email(gchar* attachment)
{
  gchar* to = "ardesia-developer@googlegroups.com";
  gchar* subject = "ardesia-contribution";
  gchar* body = "Dear ardesia developer group,\nI want share my work created with Ardesia with you, please for details see the attachment.";
  send_email(to, subject, body, attachment);
}


/* Quit the pdf saver */
void quit_pdf_saver()
{
  if (pdf_data)
    {
      wait_for_pdf_save_pending_thread();

      /* free the list and all the pixbuf inside it */
      while (pdf_data->input_filelist!=NULL)
	{
	  gchar* filename = (gchar*) g_slist_nth_data (pdf_data->input_filelist, 0);
	  if (filename)
	    {
	      g_remove(filename);       
	      pdf_data->input_filelist = g_slist_remove(pdf_data->input_filelist, filename);
	      g_free(filename); 
	    }
	}
      send_pdf_with_email(pdf_data->filename);
      g_free(pdf_data->filename);
      g_free(pdf_data);
    }
}
