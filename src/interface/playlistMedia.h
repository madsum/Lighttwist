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

#ifndef PLAYLIST_MEDIA_H
#define PLAYLIST_MEDIA_H

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include "canvasIcon.h"
#include "windowInterface.h"
#include "media.h"
#include <utils/entry.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void cb_media_right_click(Fl_Widget *w, void*);

#ifdef __cplusplus
}
#endif

#define PL_CMD_TYPE_M 0
#define PL_CMD_TYPE_F 1
#define PL_CMD_TYPE_A 2
#define PL_CMD_TYPE_B 3
#define PL_CMD_TYPE_C 4
#define PL_CMD_TYPE_T 5
#define PL_CMD_TYPE_I 6
#define PL_CMD_TYPE_Z 7

#define FRAME_WIDTH 100  //Number Pixels per frame

#define MEDIA_ENABLE_RESIZE 0
#define MEDIA_MIN_SIZE 50
//MEDIA_MARGIN must be greater than MEDIA_SNAP (this is a bug related to snapping at end of movie)
#define MEDIA_MARGIN 10
#define MEDIA_SNAP 10

class playlistMedia : public canvasIcon {
private:
  Fl_Window *win;
  imgu *skin_cpy;
  imgu *skin_part;
  double start_absolute;

  Fl_Menu_Button *menu_right_click;
  
public:

  playlistMedia(Fl_Window *window,void (*cb_fcn_right)(Fl_Widget *w, void*v),unsigned short type,double s,double l,imgu *skin,imgu *ctrl_low,imgu *ctrl_high) : canvasIcon(window,DND_XSCALABLE,cb_fcn_right,DRAG_HORIZONTAL,0,0,skin) {
    int i;
    win=window;
    m_type=type;
    skin_cpy=NULL;
    skin_part=NULL;
    start_x=s;
    len_x=l;
    imguCopy(&skin_cpy,skin);
    active=0;
    label("media");
    menu_right_click=new Fl_Menu_Button(0,0,10,10,NULL);
    menu_right_click->type(Fl_Menu_Button::POPUP3);
 
    for (i=0;i<4;i++)
    {
      if (i==0 || i==3) fade_ctrls[i]=new DnDBox(win,DND_XSCALABLE,NULL,NULL,DRAG_VERTICAL,0,0,10,10);
      else fade_ctrls[i]=new DnDBox(win,DND_XSCALABLE,NULL,NULL,DRAG_BOTH,0,0,10,10);
    }
  }
  ~playlistMedia() {
    int i;
    for (i=0;i<4;i++)
    {
      if (fade_ctrls[i]!=NULL)
      {
        delete fade_ctrls[i];
        fade_ctrls[i]=NULL;
      }
    }
    imguFree(&skin_cpy);
    imguFree(&skin_part);
  }

  int setFile(char *f,char *n);
  int setControl(int i,double offset,double fade);
  void resetMenuFiles();
  int addMenuFile(const char *filename,Fl_Callback *fcn,void *data);
  void showMenuFile(Fl_Widget *w);
  int handle(int e);
  void draw();

  unsigned char active;
  DnDBox *fade_ctrls[4];
  //playlist command variables
  char m_start_ref[DATA_SIZE];
  float m_fps;
  unsigned short m_type;
  char m_filename[PATH_SIZE];
  char m_tag[DATA_SIZE];
};

#endif


