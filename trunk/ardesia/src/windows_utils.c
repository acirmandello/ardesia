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

/* All this file will be build only on windows */
#ifdef _WIN32

#include <windows_utils.h>
  
BOOL (WINAPI *setLayeredWindowAttributesProc) (HWND hwnd, COLORREF crKey,
					       BYTE bAlpha, DWORD dwFlags) = NULL;
	

/* This is needed to wrap the setLayeredWindowAttributes throught the windows user32 dll */
void setLayeredGdkWindowAttributes(GdkWindow* gdk_window, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
  HWND hwnd = GDK_WINDOW_HWND(gdk_window);
  HINSTANCE hInstance = LoadLibraryA("user32");		

  setLayeredWindowAttributesProc = (BOOL (WINAPI*)(HWND hwnd,
						   COLORREF crKey, BYTE bAlpha, DWORD dwFlags))
    GetProcAddress(hInstance,"SetLayeredWindowAttributes");

  setLayeredWindowAttributesProc(hwnd, crKey, bAlpha, dwFlags);
}


/* Is the two color similar */
gboolean colors_too_similar(const GdkColor *colora, const GdkColor *colorb)
{
  return (abs(colora->red - colorb->red) < 256 &&
	  abs(colora->green - colorb->green) < 256 &&
	  abs(colora->blue - colorb->blue) < 256);
}


/* 
 * The function gdk_cursor_new_from_pixmap is broken on Windows.
 * this is a workaround using gdk_cursor_new_from_pixbuf. 
 * Thanks to Dirk Gerrits for this contribution 
 */
GdkCursor* fixed_gdk_cursor_new_from_pixmap(GdkPixmap *source, GdkPixmap *mask,
					    const GdkColor *fg, const GdkColor *bg,
					    gint x, gint y)
{
  GdkPixmap *rgb_pixmap;
  GdkGC *gc;
  GdkPixbuf *rgb_pixbuf, *rgba_pixbuf;
  GdkCursor *cursor;
  gint width, height;

  /*
   * @HACK  It seems impossible to work with RGBA pixmaps directly in
   * GDK-Win32.  Instead we pick some third color, different from fg
   * and bg, and use that as the '<b style="color: black; background-color: rgb(255, 153, 153);">transparent</b> color'.  We do this using
   * colors_too_similar (see above) because two colors could be
   * unequal in GdkColor's 16-bit/sample, but equal in GdkPixbuf's
   * 8-bit/sample. 
   *
   */
  GdkColor candidates[3] = {{0,65535,0,0}, {0,0,65535,0}, {0,0,0,65535}};
  GdkColor *trans = &candidates[0];
  if (colors_too_similar(trans, fg) || colors_too_similar(trans, bg)) 
    {
      trans = &candidates[1];
      if (colors_too_similar(trans, fg) || colors_too_similar(trans, bg))
        {
          trans = &candidates[2];
        }
    }
  /* trans is now guaranteed to be unique from fg and bg */

  /* create an empty pixmap to hold the cursor image */
  gdk_drawable_get_size(source, &width, &height);
  rgb_pixmap = gdk_pixmap_new(NULL, width, height, 24);

  /* blit the bitmaps defining the cursor onto a <b style="color: black; background-color: rgb(255, 153, 153);">transparent</b> background */
  gc = gdk_gc_new(rgb_pixmap);
  gdk_gc_set_fill(gc, GDK_SOLID);
  gdk_gc_set_rgb_fg_color(gc, trans);
  gdk_draw_rectangle(rgb_pixmap, gc, TRUE, 0, 0, width, height);
  gdk_gc_set_fill(gc, GDK_OPAQUE_STIPPLED);
  gdk_gc_set_stipple(gc, source);
  gdk_gc_set_clip_mask(gc, mask);
  gdk_gc_set_rgb_fg_color(gc, fg);
  gdk_gc_set_rgb_bg_color(gc, bg);
  gdk_draw_rectangle(rgb_pixmap, gc, TRUE, 0, 0, width, height);
  gdk_gc_unref(gc);

  /* create a cursor out of the created pixmap */
  rgb_pixbuf = gdk_pixbuf_get_from_drawable(
					    NULL, 
					    rgb_pixmap, 
					    gdk_colormap_get_system(),
					    0, 0, 0, 0,
					    width, 
					    height);
  gdk_pixmap_unref(rgb_pixmap);
  rgba_pixbuf = gdk_pixbuf_add_alpha(rgb_pixbuf, TRUE, trans->red, trans->green, trans->blue);
  gdk_pixbuf_unref(rgb_pixbuf);
  cursor = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), rgba_pixbuf, x, y);
  gdk_pixbuf_unref(rgba_pixbuf);

  return cursor;
}

#endif

