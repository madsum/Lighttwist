/*
 * This file is part of Lighttwist.
 * 
 * @Copyright 2004-2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Louis Bouchard (lwi.bouchard@gmail.com)
 *   Jean-Philippe Tardif
 *   Patrick Holloway
 *   Nicolas Martin
 *   Vlad Lazar
 *   Jamil Draréni 
 *   Marc-Antoine Drouin
 * @Copyright 2005-2007 Société des arts technologiques
 *
 * Lighttwist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Lighttwist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Lighttwist.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "canvasIcon.h"

void canvasIcon::draw()
{
    int i;

    DnDBox::draw();

    for (i=0;i<MAX_NB_PRIMITIVES;i++)
    {
      fl_color(primitives[i][1]);
#ifndef __APPLE__ //this line causes fltk problems in Mac OS when primitives are not properly initialized
				  //until fixed, simply don't call this function when running on Mac
		fl_line_style(primitives[i][2],primitives[i][3]); 
#endif      
		if (primitives[i][0]==PRIMITIVE_LINE)
      {
          fl_line(primitives[i][4],primitives[i][5],primitives[i][6],primitives[i][7]);
      }
      else if (primitives[i][0]==PRIMITIVE_ELLIPSE)
      {
          fl_arc(primitives[i][4],primitives[i][5],primitives[i][6],primitives[i][7],0.0,360.0);
      }
      else if (primitives[i][0]==PRIMITIVE_RECTANGLE)
      {
          fl_rect(primitives[i][4],primitives[i][5],primitives[i][6],primitives[i][7]);
      }
      else if (primitives[i][0]==PRIMITIVE_CURVE)
      {
          fl_begin_line();
          fl_curve(primitives[i][4],primitives[i][5],primitives[i][6],primitives[i][7],primitives[i][8],primitives[i][9],primitives[i][10],primitives[i][11]);
          fl_end_line();
      }
    }
}

int canvasIcon::addLine(int color,int style,int lwidth,int x1,int y1,int x2,int y2)
{
    int i=0;

    while(primitives[i][0]!=PRIMITIVE_NONE && i<MAX_NB_PRIMITIVES)
    {
      i++;
    }
    if (i==MAX_NB_PRIMITIVES) return -1;
   
    primitives[i][0]=PRIMITIVE_LINE;
    primitives[i][1]=color;
    primitives[i][2]=style;
    primitives[i][3]=lwidth;
    primitives[i][4]=x1;
    primitives[i][5]=y1;
    primitives[i][6]=x2;
    primitives[i][7]=y2;
       
    return 0;
}

int canvasIcon::addEllipse(int color,int style,int lwidth,int x,int y,int hradius,int vradius)
{
    int i=0;

    while(primitives[i][0]!=PRIMITIVE_NONE && i<MAX_NB_PRIMITIVES)
    {
      i++;
    }
    if (i==MAX_NB_PRIMITIVES) return -1;
   
    primitives[i][0]=PRIMITIVE_ELLIPSE;
    primitives[i][1]=color;
    primitives[i][2]=style;
    primitives[i][3]=lwidth;
    primitives[i][4]=x;
    primitives[i][5]=y;
    primitives[i][6]=hradius;
    primitives[i][7]=vradius;
       
    return 0;
}

int canvasIcon::addRectangle(int color,int style,int lwidth,int x,int y,int width,int height)
{
    int i=0;

    while(primitives[i][0]!=PRIMITIVE_NONE && i<MAX_NB_PRIMITIVES)
    {
      i++;
    }
    if (i==MAX_NB_PRIMITIVES) return -1;
   
    primitives[i][0]=PRIMITIVE_RECTANGLE;
    primitives[i][1]=color;
    primitives[i][2]=style;
    primitives[i][3]=lwidth;
    primitives[i][4]=x;
    primitives[i][5]=y;
    primitives[i][6]=width;
    primitives[i][7]=height;
       
    return 0;
}

int canvasIcon::addCurve(int color,int style,int lwidth,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4)
{
    int i=0;

    while(primitives[i][0]!=PRIMITIVE_NONE && i<MAX_NB_PRIMITIVES)
    {
      i++;
    }
    if (i==MAX_NB_PRIMITIVES) return -1;
   
    primitives[i][0]=PRIMITIVE_CURVE;
    primitives[i][1]=color;
    primitives[i][2]=style;
    primitives[i][3]=lwidth;
    primitives[i][4]=x1;
    primitives[i][5]=y1;
    primitives[i][6]=x2;
    primitives[i][7]=y2;
    primitives[i][8]=x3;
    primitives[i][9]=y3;
    primitives[i][10]=x4;
    primitives[i][11]=y4;

    return 0;
}

void canvasIcon::removePrimitives()
{
    int i;
    for (i=0;i<MAX_NB_PRIMITIVES;i++)
    {
      removePrimitive(i);
    }
}

int canvasIcon::removePrimitive(int id)
{
    if (id<0 || id>=MAX_NB_PRIMITIVES) return -1;
    
    primitives[id][0]=PRIMITIVE_NONE;

    return 0;
}


