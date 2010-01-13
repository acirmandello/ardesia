/* 
 * Ardesia-- a program for painting on the screen 
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



#include <math.h>
#include <glib.h>

#include <glib.h>
#include <gdk/gdkinput.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <annotate.h>
#include <broken.h>

double     tollerance = 15;


/* x is roundable to y */
gboolean is_similar(int x, int y)
{
  int delta = abs(x-y); 
  if (delta <= tollerance) 
    {
      return TRUE;
    }
  return FALSE;
}
 

/* 
 * The list of point is roundable to a rectangle
 * Note this algorithm found only the rettangle parallel to the axis
 * in other case we must reason on degrees 
 */
gboolean is_a_rectangle(GSList* list)
{
  if ( g_slist_length(list) != 4)
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point0 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 0);
  AnnotateStrokeCoordinate* point1 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 1);
  if (!(is_similar(point0->x, point1->x)))
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point2 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 2);
  if (!(is_similar(point1->y, point2->y)))
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point3 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 3);
  if (!(is_similar(point2->x, point3->x)))
    {
      return FALSE; 
    }
  if (!(is_similar(point3->y, point0->y)))
    {
      return FALSE; 
    }
  // is a rectangle
  return TRUE;
}


/* Return the relevant point of the line */
GSList* extract_relevant_points(GSList *listInp, gboolean close_path)
{
  AnnotateStrokeCoordinate* inp_point = (AnnotateStrokeCoordinate*)listInp->data;
  double H;
  int Ax, Ay, Bx, By, Cx, Cy;
  int X1,X2,Y1,Y2;
  double  area;
  GSList* listOut = NULL;
  
  /*copy the first one point */
  AnnotateStrokeCoordinate* first_point =  g_malloc (sizeof (AnnotateStrokeCoordinate));
  first_point->x = inp_point->x;
  first_point->y = inp_point->y;
  first_point->width = inp_point->width;
 
  listOut = g_slist_prepend (listOut, first_point); 

  area = 0.;
  Ax = inp_point->x;
  Ay = inp_point->y;

  listInp = listInp->next;   
  inp_point = (AnnotateStrokeCoordinate*)listInp->data;
  Bx = inp_point->x;
  By = inp_point->y;
  Cx = inp_point->x;
  Cy = inp_point->y;
  
  listInp = listInp->next;   

  while (listInp)
    {
      inp_point = (AnnotateStrokeCoordinate*)listInp->data;
      Cx = inp_point->x;
      Cy = inp_point->y;
      X1 = Bx-Ax;
      Y1 = By-Ay;
      X2 = Cx-Ax;
      Y2 = Cy-Ay;
      area += (double)(X1*Y2 - X2*Y1);
 
      H = (2*area)/sqrt(X2*X2+Y2*Y2);
   
      if (abs(H) > tollerance)
	{   
	  Ax = Bx;
	  Ay = By;
	  // Take a further point with standard deviation greater than the tollerance
	  inp_point = (AnnotateStrokeCoordinate*)listOut->data;
	  if (abs(Bx-inp_point->x)<tollerance)
	    {
	      Bx=inp_point->x;
	    }
	  if (abs(By-inp_point->y)<tollerance)
	    {
	      By=inp_point->y;
	    }
	  AnnotateStrokeCoordinate* add_point =  g_malloc (sizeof (AnnotateStrokeCoordinate));
	  add_point->x = Bx;
	  add_point->y = By;
	  add_point->width = inp_point->width;
	  listOut = g_slist_prepend (listOut, add_point);
	  area = 0.;
	}
      Bx = Cx;
      By = Cy;
      listInp = listInp->next;   
    }

  if (!close_path)
    {
      AnnotateStrokeCoordinate* last_point =  g_malloc (sizeof (AnnotateStrokeCoordinate));
      last_point->x = Cx;
      last_point->y = Cy;
      last_point->width = inp_point->width;
      listOut = g_slist_prepend (listOut, last_point);
    }

  return listOut;
}


void found_min_and_max(GSList* listOut, gint* minx, gint* miny, gint* maxx, gint* maxy, gint* total_width)
{
  AnnotateStrokeCoordinate* out_point = (AnnotateStrokeCoordinate*)listOut->data;
  *minx = out_point->x;
  *miny = out_point->y;
  *maxx = out_point->x;
  *maxy = out_point->y;
  *total_width = out_point->width;
  
  while (listOut)
    {
      AnnotateStrokeCoordinate* cur_point = (AnnotateStrokeCoordinate*)listOut->data;
      *minx = MIN(*minx, cur_point->x);
      *miny = MIN(*miny, cur_point->y);
      *maxx = MAX(*maxx, cur_point->x);
      *maxy = MAX(*maxy, cur_point->y);
      *total_width = *total_width + cur_point->width;
      listOut = listOut->next; 
    }   
}


gboolean is_similar_to_a_regular_poligon(GSList* list)
{
  gint lenght = g_slist_length(list);
  double old_distance = 0;
  int i = 0;
  AnnotateStrokeCoordinate* old_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
  for (i=1; i<lenght; i++)
    {
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
      double distance = (sqrt(pow(point->x-old_point->x,2)+pow(point->y-old_point->y,2)));
      if (old_distance!=0)
        {
          if (!(is_similar(distance, old_distance)))
            {
              return FALSE; 
            }
        }
      old_distance = distance;
      old_point = point;
    }
   return TRUE;
}


GSList* extract_poligon(GSList* listIn)
{
  int cx, cy;
  int radius;
  gint minx;
  gint miny;
  gint maxx;
  gint maxy;
  gint total_width;
  gint lenght = g_slist_length(listIn);
  found_min_and_max(listIn, &minx, &miny, &maxx, &maxy, &total_width);
  cx = (maxx + minx)/2;   
  cy = (maxy + miny)/2;   
  radius = ((maxx-minx)+(maxy-miny))/4;   
  double angle_off = M_PI/2;
  double angle_step = 2 * M_PI / lenght;
  angle_off += angle_step/2;
  double x1, y1;
  int i;
  for (i=0; i<lenght; i++)
    {
      x1 = radius * cos(angle_off) + cx;
      y1 = radius * sin(angle_off) + cy;
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (listIn, i);
      point->x = x1;
      point->y = y1;
      angle_off += angle_step;
    }
  return listIn;  
}


GSList*  extract_outbounded_rectangle(GSList* listIn)
{ 
  gint minx;
  gint miny;
  gint maxx;
  gint maxy;
  gint total_width;
  gint lenght = g_slist_length(listIn);
  found_min_and_max(listIn, &minx, &miny, &maxx, &maxy, &total_width);
  GSList* listOut = NULL;
  gint media_width = total_width/lenght; 
  AnnotateStrokeCoordinate* point0 =  g_malloc (sizeof (AnnotateStrokeCoordinate));
  point0->x = minx;
  point0->y = miny;
  point0->width = media_width;
  listOut = g_slist_prepend (listOut, point0);
                       
  AnnotateStrokeCoordinate* point1 =  g_malloc (sizeof (AnnotateStrokeCoordinate));
  point1->x = maxx;
  point1->y = miny;
  point1->width = media_width;
  listOut = g_slist_prepend (listOut, point1);

  AnnotateStrokeCoordinate* point2 =  g_malloc (sizeof (AnnotateStrokeCoordinate));
  point2->x = maxx;
  point2->y = maxy;
  point2->width = media_width;
  listOut = g_slist_prepend (listOut, point2);

  AnnotateStrokeCoordinate* point3 =  g_malloc (sizeof (AnnotateStrokeCoordinate));
  point3->x = minx;
  point3->y = maxy;
  point3->width = media_width;
  listOut = g_slist_prepend (listOut, point3);
  return listOut;
}


GSList* broken(GSList* listInp, gboolean* close_path, gboolean rectify)
{
     
  *close_path = FALSE;
  
  AnnotateStrokeCoordinate* inp_point = (AnnotateStrokeCoordinate*)listInp->data;
  guint lenght = g_slist_length(listInp); 
  AnnotateStrokeCoordinate* end_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (listInp, lenght-1);

  if ((is_similar(end_point->x,inp_point->x)) &&(is_similar(end_point->y,inp_point->y)))
    {
      /* close path */
      *close_path = TRUE;
    } 
  
  GSList* listOut = extract_relevant_points(listInp, *close_path);  
  
  if (*close_path)
    {
      /* close path */
      if (rectify)
	{
          if (!(is_a_rectangle(listOut)))
            {
             if ( g_slist_length(listOut) > 4)
               {
                 // is similar to regular a poligoin
                 if (is_similar_to_a_regular_poligon(listOut))
                   {
                     listOut = extract_poligon(listOut);
                   }  
                 return listOut;
               }
             else
               {
                 return listOut;
               } 
	    }
          else
            {
              /* is a rectangle */
              GSList* listOutN = extract_outbounded_rectangle(listOut);
              g_slist_foreach(listOut, (GFunc)g_free, NULL);
	      g_slist_free(listOut);
              listOut = listOutN;
              return listOut;
            }
	}
      else
        {
           /*  make the outbounded rectangle that will used to draw the ellipse */
           GSList* listOutN = extract_outbounded_rectangle(listOut);
           g_slist_foreach(listOut, (GFunc)g_free, NULL);
	   g_slist_free(listOut);
           listOut = listOutN;
           return listOut;
        }  
    }

  return listOut;
}
