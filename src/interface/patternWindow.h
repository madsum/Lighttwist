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

#ifndef PATTERN_WINDOW_H
#define PATTERN_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "windowInterface.h"
#include "listitem.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

class patternWindow : public Fl_Double_Window {
private:
  int LINEPOSY;

  void createMenuWindow();
  void loadDefaultValues();

  static void cb_ok(Fl_Widget *w, void*);
  static void cb_cancel(Fl_Widget *w, void*);
  static void cb_reset(Fl_Widget *w, void*);

public:

  patternWindow(int x,int y,int w, int h,const char *l=0) : Fl_Double_Window(x,y,w,h,l){
    createMenuWindow();
    loadDefaultValues();
    set_modal();
    end();
  }

  list_item frequency_items[MAX_NB_LIST_ITEMS];
  list_item nb_patterns_items[MAX_NB_LIST_ITEMS];

  Fl_Choice *frequency;
  Fl_Choice *nb_patterns;

  Fl_Button *ok;
  Fl_Button *cancel;
  Fl_Button *reset;
};

#endif

