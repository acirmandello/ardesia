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


#ifdef WIN32
  #include <windows.h>
  #include <winuser.h>    

  #include <gdk/gdk.h>

  #include <gtk/gtk.h>

  /* Ungrab pointer */
  void ungrab_pointer(GdkDisplay* display, GtkWidget *win);


  /* Grab pointer */
  void grab_pointer(GtkWidget *win, GdkEventMask eventmask);

  void setLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

  /* 
   * gdk_cursor_new_from_pixmap is broken on Windows.
   * this is a workaround using gdk_cursor_new_from_pixbuf. 
   */
  GdkCursor* fixed_gdk_cursor_new_from_pixmap(GdkPixmap *source, GdkPixmap *mask,
					    const GdkColor *fg, const GdkColor *bg,
					    gint x, gint y);
  #define gdk_cursor_new_from_pixmap fixed_gdk_cursor_new_from_pixmap
  /* Define other symbols needed to create the transparent layered window */
  #define LWA_COLORKEY	0x00000001
  #define LWA_ALPHA     0x00000002
#endif
