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
#include <broken.h>


/* Number x is roundable to y */
static gboolean is_similar(gdouble x, gdouble y, gint pixel_tollerance)
{
  gdouble delta = abs(x-y); 
  if (delta <= pixel_tollerance) 
    {
      return TRUE;
    }
  return FALSE;
}


/* 
 * The list of point is roundable to a rectangle
 * Note this algorithm found only the rettangle parallel to the axis
 */
static gboolean is_a_rectangle(GSList* list, gint pixel_tollerance)
{
  if (!list)
    {
      return FALSE;
    }
  if ( g_slist_length(list) != 4)
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point0 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 0);
  AnnotateStrokeCoordinate* point1 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 1);
  if (!(is_similar(point0->x, point1->x, pixel_tollerance)))
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point2 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 2);
  if (!(is_similar(point1->y, point2->y, pixel_tollerance)))
    {
      return FALSE; 
    }
  AnnotateStrokeCoordinate* point3 = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 3);
  if (!(is_similar(point2->x, point3->x, pixel_tollerance)))
    {
      return FALSE; 
    }
  if (!(is_similar(point3->y, point0->y, pixel_tollerance)))
    {
      return FALSE; 
    }
  // it is a rectangle
  return TRUE;
}


/* Calculate the media of the point pression */
static gdouble calculate_medium_pression(GSList *list)
{
  gint i = 0;
  gdouble total_pressure = 0;
  while (list)
    {
      AnnotateStrokeCoordinate* cur_point = (AnnotateStrokeCoordinate*)list->data;
      total_pressure = total_pressure + cur_point->pressure;
      i = i +1;
      list=list->next;
    }
  return total_pressure/i;
}


/* Take the list and the rurn the minx miny maxx and maxy points */
static gboolean found_min_and_max(GSList* list, gdouble* minx, gdouble* miny, gdouble* maxx, gdouble* maxy, gdouble* total_pressure)
{
  if (!list)
    {
      return FALSE;
    }
  /* This function has sense only for the list with lenght greater than two */
  gint lenght = g_slist_length(list);
  if (lenght < 3)
    {
      return FALSE;
    }
  AnnotateStrokeCoordinate* out_point = (AnnotateStrokeCoordinate*)list->data;
  *minx = out_point->x;
  *miny = out_point->y;
  *maxx = out_point->x;
  *maxy = out_point->y;
  *total_pressure = out_point->pressure;
  
  while (list)
    {
      AnnotateStrokeCoordinate* cur_point = (AnnotateStrokeCoordinate*)list->data;
      *minx = MIN(*minx, cur_point->x);
      *miny = MIN(*miny, cur_point->y);
      *maxx = MAX(*maxx, cur_point->x);
      *maxy = MAX(*maxy, cur_point->y);
      *total_pressure = *total_pressure + cur_point->pressure;
      list = list->next; 
    }   
  return TRUE;
}


/* The path described in list is similar to a regular poligon */
static gboolean is_similar_to_a_regular_poligon(GSList* list, gint pixel_tollerance)
{
  if (!list)
    {
      return FALSE;
    }
  gint lenght = g_slist_length(list);
  gint i = 0;
  gdouble ideal_distance = -1;
  gdouble total_distance = 0;  

  AnnotateStrokeCoordinate* old_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
              
  for (i=1; i<lenght; i++)
    {
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
      gdouble distance = get_distance(old_point->x, old_point->y, point->x, point->y);
      //printf("(%f,%f); (%f,%f); |%f|\n", old_point->x, old_point->y, point->x, point->y, distance);
      total_distance = total_distance + distance;
      old_point = point;
    }

  ideal_distance = total_distance/lenght;
  
  //printf("Ideal %f\n\n",ideal_distance);

  i = 0;
  old_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
  for (i=1; i<lenght; i++)
    {
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
      /* I have seen that a good compromise allow around 33% of error */
      gdouble threshold =  ideal_distance/3 + pixel_tollerance;
      gdouble distance = get_distance(point->x, point->y, old_point->x, old_point->y);
      if (!(is_similar(distance, ideal_distance, threshold)))
	{
	  // printf("%f not similar to ideal %f, differ %f that is greater than the threshold %f\n", distance, ideal_distance, fabs(distance - ideal_distance), threshold);
	  return FALSE; 
	}
      old_point = point;
    }
  return TRUE;
}


/* Take a path and return the regular poligon path */
static GSList* extract_poligon(GSList* listIn)
{
  if (!listIn)
    {
      return NULL;
    }
  gdouble cx, cy;
  gdouble radius;
  gdouble minx;
  gdouble miny;
  gdouble maxx;
  gdouble maxy;
  gdouble total_pressure;
  gboolean status = found_min_and_max(listIn, &minx, &miny, &maxx, &maxy, &total_pressure);
  if (!status)
    {
      return NULL;
    }
  cx = (maxx + minx)/2;   
  cy = (maxy + miny)/2;   
  radius = ((maxx-minx)+(maxy-miny))/4;   
  gdouble angle_off = M_PI/2;
  gint lenght = g_slist_length(listIn);
  gdouble angle_step = 2 * M_PI / (lenght-1);
  angle_off += angle_step/2;
  gdouble x1, y1;
  gint i;
  gdouble medium_pressure = total_pressure/lenght;
  for (i=0; i<lenght-1; i++)
    {
      x1 = radius * cos(angle_off) + cx;
      y1 = radius * sin(angle_off) + cy;
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (listIn, i);
      point->x = x1;
      point->y = y1;
      point->pressure = medium_pressure;
      angle_off += angle_step;
    }
  AnnotateStrokeCoordinate* pointL = (AnnotateStrokeCoordinate*) g_slist_nth_data (listIn, lenght -1); 
  AnnotateStrokeCoordinate* pointF = (AnnotateStrokeCoordinate*) g_slist_nth_data (listIn, 0); 
  pointL->x = pointF->x;
  pointL->y = pointF->y;
  pointL->pressure = medium_pressure;

  return listIn;  
}


/* Set x-axis of the point */
void putx(gpointer current, gpointer value)
{
  AnnotateStrokeCoordinate* currentpoint = ( AnnotateStrokeCoordinate* ) current;
  gdouble* valuex = ( gdouble* ) value;
  currentpoint->x = *valuex;
}


/* Set y-axis of the point */
static void puty(gpointer current, gpointer value)
{
  AnnotateStrokeCoordinate* currentpoint = ( AnnotateStrokeCoordinate* ) current;
  gdouble* valuey = ( gdouble* ) value;
  currentpoint->y = *valuey;
}


/* Return the degree of the rect beetween two point respect the axis */
static gfloat calculate_edge_degree(AnnotateStrokeCoordinate* pointA, AnnotateStrokeCoordinate* pointB)
{
  gdouble deltax = abs(pointA->x-pointB->x);
  gdouble deltay = abs(pointA->y-pointB->y);
  gfloat directionAB = atan2(deltay, deltax)/M_PI*180;
  return directionAB;   
}


/* Straight the line */
static GSList* straighten(GSList* list)
{  
  GSList* listOut = NULL;
  if (!list)
    {
      return listOut;
    }
  
 
  gint lenght = g_slist_length(list);
  
  gint degree_threshold = 15;

  /* copy the first one point; it is a good point */
  AnnotateStrokeCoordinate* inp_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, 0);
  AnnotateStrokeCoordinate* first_point =  allocate_point(inp_point->x, inp_point->y, inp_point->width, inp_point->pressure);
  listOut = g_slist_prepend (listOut, first_point); 
 
  gint i;
  for (i=0; i<lenght-2; i++)
    {
      AnnotateStrokeCoordinate* pointA = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
      AnnotateStrokeCoordinate* pointB = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i+1);
      AnnotateStrokeCoordinate* pointC = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i+2);
      gfloat directionAB = calculate_edge_degree(pointA, pointB);  
      gfloat directionBC = calculate_edge_degree(pointB, pointC);  

      gfloat delta_degree = abs(directionAB-directionBC);
      if (delta_degree > degree_threshold)
        {
	  //copy B it's a good point
	  AnnotateStrokeCoordinate* point =  allocate_point(pointB->x, pointB->y, pointB->width, pointB->pressure);
	  listOut = g_slist_prepend (listOut, point); 
        } 
      /* else is three the difference degree is minor than the threshold I neglegt B */
    }

  /* Copy the last point; it is a good point */
  AnnotateStrokeCoordinate* last_point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, lenght-1);
  
  AnnotateStrokeCoordinate* last_out_point =  allocate_point(last_point->x, last_point->y, last_point->width, last_point->pressure);
  listOut = g_slist_prepend (listOut, last_out_point);

  /* I reverse the list to preserve the initial order */
  listOut = g_slist_reverse(listOut);


  lenght = g_slist_length(listOut);
  
  if (lenght!=2)  
    {
      return listOut;
    }
  
  /* It is a segment! */   
  gfloat direction = calculate_edge_degree(first_point, last_point);  

  /* is it is closed to 0 degree I draw an horizontal line */
  if ((0-degree_threshold<=direction)&&(direction<=0+degree_threshold)) 
    {
      // y is the average
      gdouble y = (first_point->y+last_point->y)/2;
      // put this y for each element in the list
      g_slist_foreach(listOut, (GFunc)puty, &y);
    } 
  
  /* is it is closed to 90 degree I draw a vertical line */
  if ((90-degree_threshold<=direction)&&(direction<=90+degree_threshold)) 
    {
      // x is the average
      gdouble x = (first_point->x+last_point->x)/2;
      // put this x for each element in the list
      g_slist_foreach(listOut, (GFunc)putx, &x);
    } 

  return listOut; 
}


/* Return a subpath of listInp containg only the meaningful points using the standard deviation */
GSList* extract_relevant_points(GSList *listInp, gboolean close_path, gint pixel_tollerance)
{
  if (!listInp)
    {
      return NULL;
    }

  gint lenght = g_slist_length(listInp);

  /* Initialize the list */
  GSList* listOut = NULL;
  
  gint i = 0;
  AnnotateStrokeCoordinate* pointA = (AnnotateStrokeCoordinate*) g_slist_nth_data (listInp, i);
  AnnotateStrokeCoordinate* pointB = (AnnotateStrokeCoordinate*) g_slist_nth_data (listInp, i+1);
  AnnotateStrokeCoordinate* pointC = (AnnotateStrokeCoordinate*) g_slist_nth_data (listInp, lenght-1);

  gdouble pressure = calculate_medium_pression(listInp);

  gdouble Ax = pointA->x;
  gdouble Ay = pointA->y;
  gint Awidth = pointA->width;

  gdouble Bx = pointB->x;
  gdouble By = pointB->y;
  gint Bwidth = pointB->width;

  gdouble Cx = pointB->x;
  gdouble Cy = pointB->y;
  gint Cwidth = pointB->width;

  AnnotateStrokeCoordinate* first_point =  allocate_point(Ax, Ay, Awidth, pressure);
  // add a point with the coordinates of pointA
  listOut = g_slist_prepend (listOut, first_point);

  gdouble area = 0.;
  gdouble h = 0;
  
  gdouble X1, Y1, X2, Y2;

  for (i = i+2; i<lenght; i++)
    {
      pointC = (AnnotateStrokeCoordinate*) g_slist_nth_data (listInp, i);
      Cx = pointC->x;
      Cy = pointC->y;
      Cwidth = pointC->width;
       
      X1 = Bx - Ax;
      Y1 = By - Ay;
      X2 = Cx - Ax;
      Y2 = Cy - Ay;
    
      area += (gdouble) (X1 * Y2 - X2 * Y1);
    
      h = (2*area)/sqrtf(X2*X2 + Y2*Y2);    
       
      if (fabs(h) >= (pixel_tollerance))
	{
	  // add  a point with the B coordinates
	  AnnotateStrokeCoordinate* new_point =  allocate_point(Bx, By, Bwidth, pressure);
	  listOut = g_slist_prepend (listOut, new_point);
	  area = 0.0;
	  Ax = Bx;
	  Ay = By;
	  Awidth = Bwidth;
	} 
      // put to B the C coordinates
      Bx = Cx;
      By = Cy;
      Bwidth = Cwidth;
    }
  
  /* Add the last point with the coordinates */
  AnnotateStrokeCoordinate* last_point =  allocate_point(Cx, Cy, Cwidth, pressure);
  listOut = g_slist_prepend (listOut, last_point); 

  /* I reverse the list to preserve the initial order */
  listOut = g_slist_reverse(listOut);
  return listOut;
}


/* Return the outbounded rectangle outside the path described to listIn */
GSList*  extract_outbounded_rectangle(GSList* listIn)
{ 
  if (!listIn)
    {
      return NULL;
    }
  gdouble minx;
  gdouble miny;
  gdouble maxx;
  gdouble maxy;
  gdouble total_pressure = 1.0;
  gint lenght = g_slist_length(listIn);
  gboolean status = found_min_and_max(listIn, &minx, &miny, &maxx, &maxy, &total_pressure);
  if (!status)
    {
      return NULL;
    }
  GSList* listOut = NULL;
  gdouble media_pressure = total_pressure/lenght; 
  AnnotateStrokeCoordinate* pointF = (AnnotateStrokeCoordinate*) g_slist_nth_data (listIn, 0);
  gint width = pointF->width;

  AnnotateStrokeCoordinate* point0 = allocate_point (minx, miny, width, media_pressure);
  listOut = g_slist_prepend (listOut, point0);
                       
  AnnotateStrokeCoordinate* point1 =  allocate_point (maxx, miny, width, media_pressure);
  listOut = g_slist_prepend (listOut, point1);

  AnnotateStrokeCoordinate* point2 =  allocate_point (maxx, maxy, width, media_pressure);
  listOut = g_slist_prepend (listOut, point2);

  AnnotateStrokeCoordinate* point3 =  allocate_point (minx, maxy, width, media_pressure);
  listOut = g_slist_prepend (listOut, point3);

  return listOut;
}

       
/* The path described in list is similar to an ellipse,  unbounded_rect is the outbounded rectangle to the eclipse */
gboolean is_similar_to_an_ellipse(GSList* list, GSList* unbounded_rect, gint pixel_tollerance)
{
  if (!list)
    {
      return FALSE;
    }
  gint lenght = g_slist_length(list);
   

  AnnotateStrokeCoordinate* point1 = (AnnotateStrokeCoordinate*) g_slist_nth_data (unbounded_rect, 2);
  AnnotateStrokeCoordinate* point3 = (AnnotateStrokeCoordinate*) g_slist_nth_data (unbounded_rect, 0);

  gdouble a = (point3->x-point1->x)/2;
  gdouble b = (point3->y-point1->y)/2;
  gdouble originx = point1->x + a;
  gdouble originy = point1->y + b;
  gdouble c = 0.0;
  gdouble f1x;
  gdouble f1y;
  gdouble f2x;
  gdouble f2y;

  gdouble aq = powf(a,2);
  gdouble bq = powf(b,2);
  
  if (aq>bq)
    {
      c = sqrtf(aq-bq);
      // F1(x0-c,y0)
      f1x = originx-c;
      f1y = originy;
      // F2(x0+c,y0)
      f2x = originx+c;
      f2y = originy;
    }
  else
    {
      c = sqrtf(bq-aq);
      // F1(x0, y0-c)
      f1x = originx;
      f1y = originy-c;
      // F2(x0, y0+c)
      f2x = originx;
      f2y = originy+c;
    }

  gdouble distancep1f1 = get_distance(point1->x, point1->y, f1x, f1y);
  gdouble distancep1f2 = get_distance(point1->x, point1->y, f2x, f2y);
  gdouble sump1 = distancep1f1 + distancep1f2;

  /* In the ellipse the sum of the distance(p,f1)+distance(p,f2) must be constant */
  gint i=0;   

  for (i=0; i<lenght; i++)
    {
      AnnotateStrokeCoordinate* point = (AnnotateStrokeCoordinate*) g_slist_nth_data (list, i);
      gdouble distancef1 = get_distance(point->x, point->y, f1x, f1y);
      gdouble distancef2 = get_distance(point->x, point->y, f2x, f2y);
      gdouble sum = distancef1 + distancef2;
      gdouble difference = fabs(sum-sump1);
      if (difference>pixel_tollerance)
        {  
	  // the sum is so different from the right one; this is not an ellipse 
	  return FALSE;
        }
    }
  return TRUE;
}

            
/* Take a list of point and return magically the new recognized path */
GSList* broken(GSList* listInp, gboolean close_path, gboolean rectify, gint pixel_tollerance)
{
  if (!listInp)
    {
      return NULL;
    }
  
  GSList* listOut = extract_relevant_points(listInp, close_path, pixel_tollerance);  
   
  if (listOut)
    { 
      if (rectify) 
	{     
	  if (close_path)
	    {
	      // is similar to regular a poligon 
	      if (is_similar_to_a_regular_poligon(listOut, pixel_tollerance))
		{
		  listOut = extract_poligon(listOut);
		} 
	      else
		{

		  if (is_a_rectangle(listOut, pixel_tollerance))
		    {
		      /* is a rectangle */
		      GSList* listOutN = extract_outbounded_rectangle(listOut);
		      g_slist_foreach(listOut, (GFunc)g_free, NULL);
		      g_slist_free(listOut);
		      listOut = listOutN;
		      return listOut;
		    }
		}
	    }
	  else
	    {
	      // try to make straighten
	      GSList* straight_list = straighten(listOut);
	      // free outptr
	      g_slist_foreach(listOut, (GFunc)g_free, NULL);
	      g_slist_free(listOut);
	      listOut = straight_list;
	    }
	} 
      else       
	{
	  return listOut;    
	}  
    }
  return listOut;
}


