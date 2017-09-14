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

#ifndef CAM2PROJ_WINDOW_H
#define CAM2PROJ_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "windowInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int selectMachinesAndExecute(const char *str,int (*operfunc)(int i));
extern int do_cam2proj(int i);

#ifdef __cplusplus
}
#endif

class cam2projWindow : public Fl_Double_Window {
private:
  int LINEPOSY;

  void createMenuWindow();
  void loadDefaultValues();

  static void cb_min_range_percentage(Fl_Widget *w, void*);
  static void cb_min_range(Fl_Widget *w, void*);
  static void cb_confidence(Fl_Widget *w, void*);
  static void cb_ok(Fl_Widget *w, void*);
  static void cb_cancel(Fl_Widget *w, void*);
  static void cb_reset(Fl_Widget *w, void*);

  Fl_Slider *min_range_percentage;
  char min_range_percentage_buf[32];
  Fl_Slider *min_range;  
  char min_range_buf[32];
  Fl_Slider *confidence_threshold;
  char confidence_buf[32];

public:

  cam2projWindow(int x,int y,int w, int h,const char *l=0) : Fl_Double_Window(x,y,w,h,l){
    createMenuWindow();
    loadDefaultValues();
    set_modal();
    end();
  }

  Fl_Choice *machine;
  //Fl_Choice *build_type;


  Fl_Box *min_range_percentage_box;
  Fl_Box *min_range_box;

  void setMinRangePercentageValue(double val);
  void setMinRangeValue(double val);
  double getMinRangePercentageValue() {return min_range_percentage->value();};
  double getMinRangeValue() {return min_range->value();};

  Fl_Box *confidence_box;
  void setConfidenceValue(double val);
  double getConfidenceValue() {return confidence_threshold->value();};

  Fl_Check_Button *noise_mode;
  Fl_Check_Button *crop_triangles;

  Fl_Button *ok;
  Fl_Button *cancel;
  Fl_Button *reset;

};

#endif

