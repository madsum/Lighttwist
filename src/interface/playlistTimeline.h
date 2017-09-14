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

#ifndef PLAYLIST_TIMELINE_H
#define PLAYLIST_TIMELINE_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Dial.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "playlistLayer.h"
#include "player-master/default/playlist_parse.h"

#define NB_SCALES 17   //Number of ranges in the zoom scale
#define SCALE_WIDTH     100  //Number of pixels between dividers 

#define TL_LAYER_START 100
#define TL_LAYER_HEIGHT 50
#define TL_POINTER_Y 130

#define TL_LEAD 0  //Number of pix before first time mark
#define DRAWN_FRAME_SIZE 50  //Pixel width of a single frame

#define ZOOM_LABEL "Zoom"
#define ZOOM_NB_FORMAT "00:00:00.00"

#define NO_TAG "no_tag"
#define ZERO_ABSOLUTE "0.0"

class playlistTimeline : public Fl_Scroll {
private:
  void draw();

  int timeline_end;
  int fps;

  double disp_start;     //start of displayed data
  double disp_nb_frames;

  int  scales[NB_SCALES];
  int  scale_pos;

  double  scale;
  double  scale_factor;

  void scaleCompute(int prev_scale_pos);
  void setDefaultScales();

  Fl_Box *corner;
  char scale_buf[32];

  Fl_Button *layers_box[NB_DISP_LAYERS];
  Fl_Button *layers_lbl[NB_DISP_LAYERS];
  Fl_Button *timeline_header;

  Fl_Window *win;

  int resolveTime(char *tag,char *start_ref,double *abs_start_time,double *start_time,double *end_time);
  playlistMedia *findTagMedia(char *tag);

protected:


public:
  playlistTimeline(Fl_Window *window,int x,int y,int width,int height,const char *l = 0):Fl_Scroll(x,y,width,height,l){
    int LINEPOSY;
    win=window;
    box(FL_NO_BOX);
    color(FL_GRAY);
    type(0);

    playlist_file[0]='\0';
	setDefaultScales(); 
    corner=new Fl_Box(x,y,TL_LAYER_START,TL_LAYER_HEIGHT);
    corner->box(FL_FLAT_BOX);
    corner->deactivate();
    timeline_header=new Fl_Button(x+TL_LAYER_START,y,width-TL_LAYER_START,TL_LAYER_HEIGHT);
    timeline_header->color(fl_rgb_color(COLOR_DIRECTORY));
    timeline_header->labelsize(FONTSIZE);
    timeline_header->deactivate();
    initLayers();
    pointer_line=new DnDBox(win,DND_XSCALABLE,NULL,NULL,DRAG_HORIZONTAL,0, 0, 10,10,NULL);

	scale_pos=NB_SCALES-1;
	setFPS(60);
    LINEPOSY=0;
    scale_slider=new Fl_Slider(SMALL_WIDGET,"Confidence threshold: ");
    setSlider(scale_slider,&LINEPOSY);
    scale_slider->position(x+13,y+25);
    scale_slider->align(FL_ALIGN_TOP);
    setScaleValue(1.0);

    setDispStart(0);
    setTimelineEnd(MAX_NB_HOURS*60*60*fps); //8 hours
    setDispStart(60*fps);
  }
  ~playlistTimeline() {
    delete scale_slider;
    resetLayers();
  }

  void initLayers();
  void resetLayers()
  {
    int i;
    for (i=0;i<NB_DISP_LAYERS;i++)
    {
      if (layers[i]!=NULL)
      {
        delete layers[i];
        layers[i]=NULL;
      }
    }
  }

  void setDispStart(double t){disp_start=t;scaleCompute(-1);};
  void setTimelineEnd(int t){timeline_end=t;scaleCompute(-1);};

  int getFPS(){return fps;}
  void setFPS(int s)
  {
    fps=s;
    scaleCompute(-1);
    setDefaultScales();
  }

  void setPointerImg(imgu *I)
  {
    pointer_line->setImage(I);
    pointer_line->setPosition(x()+TL_LAYER_START+TL_LEAD,TL_POINTER_Y);
    pointer_line->xmin=(double)(pointer_line->x());
    pointer_line->xmax=(double)(pointer_line->xmin+timeline_header->w()-2*TL_LEAD);
    pointer_line->center_flag=1;
    pointer_line->convert_width=0;
    scaleCompute(-1);
    setScaleValue(scale_slider->value());
  }

  void resetMedia();
  int loadPlaylist(Directory *mediatree);
  int savePlaylist(Directory *mediatree);
  void setScaleValue(double val);

  Fl_Dial *layers_angle[NB_DISP_AUDIO_LAYERS];
  char playlist_file[PATH_SIZE];
  playlistLayer *layers[NB_DISP_LAYERS];
  DnDBox *pointer_line;
  Fl_Slider *scale_slider;
};

#endif

