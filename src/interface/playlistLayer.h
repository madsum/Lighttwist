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

#ifndef PLAYLIST_LAYER_H
#define PLAYLIST_LAYER_H

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Scroll.H>
#include "canvasIcon.h"
#include "windowInterface.h"
#include "playlistMedia.h"
#include "media.h"
#include <utils/entry.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void cb_layer_right_click(Fl_Widget *w, void*);

#ifdef __cplusplus
}
#endif

#define LAYER_VIDEO_TYPE 0
#define LAYER_AUDIO_TYPE 1

#define MAX_NB_MEDIA 32
#define MAX_NB_HOURS 8

class playlistLayer : public Fl_Box {
private:
  int handle(int e);
  void draw();

  Fl_Window *win;

  char name[DATA_SIZE];

  Fl_Menu_Button *menu_right_click;
  
public:
  unsigned short layer_type;
  playlistMedia *media[MAX_NB_MEDIA];

  playlistLayer(Fl_Window *window,int x,int y,int w,int h,unsigned short t,char *l=0) : Fl_Box(x,y,w,h,NULL) {
    box(BOX_USER_DRAW);
    int i;
    win=window;
    for (i=0;i<MAX_NB_MEDIA;i++)
    {
      //media[i]=NULL;
      media[i]=new playlistMedia(win,NULL,NULL,0.0,1.0,NULL,NULL,NULL);
      media[i]->hide();
      media[i]->fade_ctrls[0]->hide();
      media[i]->fade_ctrls[1]->hide();
      media[i]->fade_ctrls[2]->hide();
      media[i]->fade_ctrls[3]->hide();
    }
    menu_right_click=new Fl_Menu_Button(x,y,w,h,NULL);
    menu_right_click->type(Fl_Menu_Button::POPUP3);
  
    if (l!=NULL) strcpy(name,l);
    layer_type=t;
    type(0);
  }
  ~playlistLayer() {
    int i;
    for (i=0;i<MAX_NB_MEDIA;i++)
    {
      delete media[i];
    }
    delete menu_right_click;
  }

  void setScale(int fps_arg,double disp_start_arg,double frames_to_pixels_arg,int offset_arg,int pixel_range);
  void resetMedia();
  void resetMenuFiles();
  playlistMedia *addMedia(unsigned short type,char *filename,int click_x,double len,double fps);
  int addMenuFile(const char *filename,Fl_Callback *fcn,void *data);
  void showMenuFile(Fl_Widget *w);

};

#endif

