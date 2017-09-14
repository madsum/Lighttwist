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

#ifndef CAPTURE_WINDOW_H
#define CAPTURE_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include "windowInterface.h"

//#define CAPTURE_SIMULTANEOUS_NOISE

#ifdef __cplusplus
extern "C" {
#endif

extern int selectMachinesAndExecute(const char *str,int (*operfunc)(int i));
extern int do_stl(int i);
extern int do_grab(int i);
extern int do_grab_noise(int i);
extern int do_select (int i);
extern int do_select_noise (int i);
extern int start_background(int i);
extern int setTestPattern(int p);
extern void cb_test_camera(Fl_Widget *w, void*);
extern int fioReadFolderFiles(char namelist[MAX_NB_FILES][FILENAME_SIZE],char *inpath);

extern int do_rename_patterns(int i);
extern int do_rename_gamma(int i);
extern int do_rename_color(int i);
extern int do_rename_noise(int i);

#ifdef __cplusplus
}
#endif

class captureWindow : public Fl_Double_Window {
private:
  int LINEPOSY;

  void createMenuWindow();
  void loadDefaultValues();

  static void cb_ok(Fl_Widget *w, void*);
  static void cb_cancel(Fl_Widget *w, void*);

public:

  captureWindow(int x,int y,int w, int h,const char *l=0) : Fl_Double_Window(x,y,w,h,l){
    createMenuWindow();
    loadDefaultValues();
    set_modal();
    end();
  }
  Fl_Input *camId;
  Fl_Spinner *camDistoK1;
  Fl_Spinner *camDistoK2;
  Fl_Choice *machine;
  Fl_Choice *mode;
  Fl_Choice *cam;
  Fl_Spinner *exposure_time;
  Fl_Choice *transfer;
  Fl_Check_Button *stl_only;

  char filenames[MAX_NB_FILES][FILENAME_SIZE];
  int nbfiles;
  int findex;

  Fl_Button *ok;
  Fl_Button *cancel;
  Fl_Button *test_camera;

};

#endif

