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

#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Chooser.H>
#include "windowInterface.h"
#include <utils/entry.h>
#include "listitem.h"
#include "remote/remote.h"

#ifdef __APPLE__
  #define BASE_PATH "/Users"
#else
  #define BASE_PATH "/home"
#endif

#define config_path ".lighttwist" //in home directory

#define MAX_NB_SPEAKERS 8

#ifdef __cplusplus
extern "C" {
#endif

//defined in lighttwist.cpp
extern void cb_update_project();
extern void cb_save(Fl_Widget *w, void*);

#ifdef __cplusplus
}
#endif

class projectWindow : public Fl_Double_Window {
private:
  int LINEPOSY;

  char default_dir[PATH_SIZE];
  char *username;
  char remote_path[PATH_SIZE];
  void createMenuWindow();
  void loadDefaultValues();

  static void cb_local_media(Fl_Widget *w, void*);
  static void cb_local_scan(Fl_Widget *w, void*);
  static void cb_local_data(Fl_Widget *w, void*);
  static void cb_local_bin(Fl_Widget *w, void*);
  static void cb_local_lib(Fl_Widget *w, void*);
  static void cb_local_share(Fl_Widget *w, void*);
  static void cb_slider(Fl_Widget *w, void*);
  static void cb_ok(Fl_Widget *w, void*);
  static void cb_reset(Fl_Widget *w, void*);

public:

  projectWindow(int x,int y,int w, int h,const char *l,char *login) : Fl_Double_Window(x,y,w,h,l){
    username=login;
    initItemList(resolution_items);
    initItemList(configuration_items);
    createMenuWindow();
    loadDefaultValues();
    set_modal();
    sprintf(default_dir,"/home/%s/lighttwist",username);
    end();
  }

  int getResolution(int *resx,int *resy);

  Fl_Input *project;
  Fl_Choice *configuration;
  Fl_Check_Button *vmirror;
  Fl_Choice *mode;
  Fl_Spinner *screen_ratio;
  Fl_Spinner *nb_projectors;
  Fl_Choice *resolution;
  Fl_Spinner *nb_speakers;
  Fl_Input *sound_ctrl_ip;
  Fl_Check_Button *enable_jack;
  Fl_Input *local_media;
  Fl_Input *local_scan;
  Fl_Input *local_data;
  Fl_Input *local_bin;
  Fl_Input *local_lib;
  Fl_Input *local_share;
  //Fl_Check_Button *remoteaslocal;
  Fl_Input *remote_scan;
  Fl_Input *remote_data;
  Fl_Input *remote_bin;
  Fl_Input *remote_share;
  list_item resolution_items[MAX_NB_LIST_ITEMS];
  list_item configuration_items[MAX_NB_LIST_ITEMS];

  char project_str[PATH_SIZE];
  char sound_ctrl_ip_str[PATH_SIZE];
  char local_media_str[PATH_SIZE];
  char local_scan_str[PATH_SIZE];
  char local_data_str[PATH_SIZE];
  char local_bin_str[PATH_SIZE];
  char local_lib_str[PATH_SIZE];
  char local_share_str[PATH_SIZE];
  char remote_scan_str[PATH_SIZE];
  char remote_data_str[PATH_SIZE];
  char remote_bin_str[PATH_SIZE];
  char remote_share_str[PATH_SIZE];

  Fl_Button *local_media_but;
  Fl_Button *local_scan_but;
  Fl_Button *local_data_but;
  Fl_Button *local_bin_but;
  Fl_Button *local_lib_but;
  Fl_Button *local_share_but;

  Fl_Button *ok;
  Fl_Button *reset;

};

#endif

