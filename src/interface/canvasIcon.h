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

#ifndef CANVAS_ICON_H
#define CANVAS_ICON_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "DnDBox.h"
#include "windowInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#define MAX_NB_PRIMITIVES 10
#define MAX_NB_PARAMS 12

#define PRIMITIVE_NONE 0
#define PRIMITIVE_LINE 1
#define PRIMITIVE_ELLIPSE 2
#define PRIMITIVE_RECTANGLE 3
#define PRIMITIVE_CURVE 4

class canvasIcon : public DnDBox {
private:
  int primitives[MAX_NB_PRIMITIVES][MAX_NB_PARAMS];

protected:  
  void draw();

public:

  canvasIcon(Fl_Window *win,unsigned char opts_type, void (*cb_fcn_right)(Fl_Widget *w, void*v),unsigned char d,int x,int y,imgu *I) : DnDBox(win,opts_type,NULL,cb_fcn_right,d,x,y,I){
      removePrimitives();
  }

  int addLine(int color,int style,int lwidth,int x1,int y1,int x2,int y2);
  int addEllipse(int color,int style,int lwidth,int x,int y,int hradius,int vradius);
  int addRectangle(int color,int style,int lwidth,int x,int y,int width,int height);
  int addCurve(int color,int style,int lwidth,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
  void removePrimitives();
  int removePrimitive(int id);

};

#endif

