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

#ifndef LIVE_WINDOW_H
#define LIVE_WINDOW_H

#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include "windowInterface.h"
#include "playlistTimeline.h"
#include "player-master/default/playlist_parse.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void cb_show_live_fade(Fl_Widget* w, long i);
extern void cb_show_live_volume(Fl_Widget* w, long i);
extern void cb_show_live_stereo(Fl_Widget* w, long i);
extern void cb_show_live_spk_scale(Fl_Widget* w, long i);
extern void cb_show_live_gamma(Fl_Widget* w, long i);

#ifdef __cplusplus
}
#endif

class liveWindow : public Fl_Double_Window {
private:
  void createMenuWindow();

  static void cb_reset(Fl_Widget *w, void*);
  static void cb_b_scan(Fl_Widget *w, void*);

  Fl_Scroll *scroll;

  Fl_Button *B_scan;

public:

  liveWindow(int x,int y,int w, int h,const char *l=0) : Fl_Double_Window(x,y,w,h,l){
    createMenuWindow();
    loadDefaultValues();
    //set_modal();
    end();
  }

  void loadDefaultValues();
  void sendAllValues();

  Fl_Slider *live_fade[NB_DISP_VIDEO_LAYERS+1]; //+1 because we need space for master fade control
  Fl_Slider *live_volume[NB_DISP_AUDIO_LAYERS+1]; //+1 because we need space for master volume control

  Fl_Spinner *live_stereo_R;
  Fl_Spinner *live_stereo_B;
  Fl_Spinner *live_stereo_distortion;
  Fl_Spinner *live_stereo_slit;
  Fl_Spinner *live_speaker_scale;

  Fl_Slider *live_gamma[3];

  Fl_Button *reset;
};

#endif

