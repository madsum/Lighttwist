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

#ifndef UVMAP_WINDOW_H
#define UVMAP_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include "canvasIcon.h"
#include "windowInterface.h"
#include <imgu/imgu.h>
#include "utils/uvmap.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int saveCrop();
extern void cursorWait();
extern void cursorDefault();

#ifdef __cplusplus
}
#endif

class uvmapWindow : public Fl_Double_Window {
private:
  int LINEPOSY;
  int configuration;

  void createMenuWindow();
  void updateLayout();
  void draw();

  static void cb_update_ellipse(Fl_Widget *w, void*);
  static void cb_ok(Fl_Widget *w, void*);
  static void cb_cancel(Fl_Widget *w, void*);
  static void cb_reset(Fl_Widget *w, void*);

public:

  uvmapWindow(int x,int y,int w, int h,const char *l=0) : Fl_Double_Window(x,y,w,h,l){
    createMenuWindow();
    setUVConfiguration(CYCLORAMA_CONFIGURATION);
    loadDefaultValues();
    //set_modal();
    end();
  }

  int setUVConfiguration(int c);
  void loadDefaultValues();

  DnDBox *controls_outer[MAX_NB_UV_CTRLS];
  DnDBox *controls_inner[MAX_NB_UV_CTRLS];
  canvasIcon *background;

  Fl_Scroll *scroll;

  Fl_Spinner *scale_inner;
  Fl_Spinner *scale_outer;
  Fl_Spinner *vcenter;

  Fl_Button *ok;
  Fl_Button *cancel;
  Fl_Button *reset;
};

#endif

