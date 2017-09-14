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

#include "liveWindow.h"

void liveWindow::createMenuWindow()
{
    int i;
    int LINEPOSY;
    int offset;
    char label[32];

    LINEPOSY=2*LINESPACE;

    scroll=new Fl_Scroll(0,0,w(),h()-3*LINESPACE,NULL);
    scroll->box(FL_NO_BOX);

    offset=OFFSET;
    for (i=0;i<=NB_DISP_VIDEO_LAYERS;i++)
    {
      if (i==0) strcpy(label,langGetLabel(LABEL_MASTER_VIDEO));
      else sprintf(label,"%d",i);
      live_fade[i] = new Fl_Slider(x()+offset,y()+8*OFFSET,20,75,strdup(label));
      live_fade[i]->labelsize(FONTSIZE);
      live_fade[i]->callback(cb_show_live_fade,i);
      if (i==0) offset+=10*OFFSET;
      else offset+=5*OFFSET;
    }

    offset=OFFSET;
    for (i=0;i<=NB_DISP_AUDIO_LAYERS;i++)
    {
      if (i==0) strcpy(label,langGetLabel(LABEL_MASTER_AUDIO));
      else sprintf(label,"%d",i);
      live_volume[i] = new Fl_Slider(x()+offset,y()+8*OFFSET+125,20,75,strdup(label));
      live_volume[i]->labelsize(FONTSIZE);
      live_volume[i]->callback(cb_show_live_volume,i);
      if (i==0) offset+=10*OFFSET;
      else offset+=5*OFFSET;
    }

    offset=OFFSET;
    for (i=0;i<3;i++)
    {
      strcpy(label,"Gamma (");
      if (i==0) strcat(label,langGetLabel(LABEL_RED));
      else if (i==1) strcat(label,langGetLabel(LABEL_GREEN));
      else strcat(label,langGetLabel(LABEL_BLUE)); 
      strcat(label,")"); 
      live_gamma[i] = new Fl_Slider(x()+offset,y()+8*OFFSET+250,20,75,strdup(label));
      live_gamma[i]->labelsize(FONTSIZE);
      live_gamma[i]->callback(cb_show_live_gamma,i);
      offset+=10*OFFSET;
    }

    LINEPOSY=LINESPACE;

    live_stereo_R = new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_STEREO_R));
    setFloatSpinner(live_stereo_R,5.0,5000.0,&LINEPOSY);
    live_stereo_R->step(0.5);
    live_stereo_R->callback(cb_show_live_stereo,0);

    live_stereo_B = new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_STEREO_B));
    setFloatSpinner(live_stereo_B,-100.0,100.0,&LINEPOSY);
    live_stereo_B->step(0.05);
    live_stereo_B->callback(cb_show_live_stereo,0);

    B_scan = new Fl_Button(STANDARD_WIDGET,"Scan Baseline");
    setButton(B_scan,&LINEPOSY);
    B_scan->callback(cb_b_scan,0);

    live_stereo_distortion = new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_STEREO_DISTORT));
    setFloatSpinner(live_stereo_distortion,0.0,1.0,&LINEPOSY);
    live_stereo_distortion->step(0.01);
    live_stereo_distortion->callback(cb_show_live_stereo,0);

    live_stereo_slit = new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_STEREO_SLIT));
    setFloatSpinner(live_stereo_slit,0.0,180.0,&LINEPOSY);
    live_stereo_slit->step(1.0);
    live_stereo_slit->callback(cb_show_live_stereo,0);

    live_speaker_scale = new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_SOUND_SCALE));
    setFloatSpinner(live_speaker_scale,0.01,10000.0,&LINEPOSY);
    live_speaker_scale->step(0.05);
    live_speaker_scale->callback(cb_show_live_spk_scale,0);

    scroll->end();

    LINEPOSY=h()-2*LINESPACE;

    reset=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_RESET));
    reset->callback(cb_reset);
}

void liveWindow::loadDefaultValues()
{
  int i;
  for (i=0;i<=NB_DISP_VIDEO_LAYERS;i++)
  {
    live_fade[i]->value(0.0);
  }

  for (i=0;i<=NB_DISP_AUDIO_LAYERS;i++)
  {
    live_volume[i]->value(0.0);
  }
  live_volume[0]->value(1.0-1.0);

  live_gamma[0]->value(0.26666);
  live_gamma[1]->value(0.26666);
  live_gamma[2]->value(0.26666);

  live_stereo_R->value(STEREO_R_DEFAULT);
  live_stereo_B->value(STEREO_B_DEFAULT);
  live_stereo_distortion->value(STEREO_DISTORTION_DEFAULT);
  live_stereo_slit->value(STEREO_SLIT_DEFAULT);
  live_speaker_scale->value(SPEAKER_SCALE_DEFAULT);
}

void liveWindow::cb_reset(Fl_Widget *w, void*)
{
  liveWindow *win;
  win=(liveWindow *)(w->parent());
  win->loadDefaultValues();

  win->sendAllValues();

  win->redraw();
}

void liveWindow::sendAllValues()
{
  int i;

  for (i=0;i<=NB_DISP_VIDEO_LAYERS;i++)
  {
    cb_show_live_fade(NULL,i);
  }
  for (i=0;i<=NB_DISP_AUDIO_LAYERS;i++)
  {
    cb_show_live_volume(NULL,i);
  }
  cb_show_live_stereo(NULL,0);
  //cb_show_live_gamma(NULL,0);
}

void liveWindow::cb_b_scan(Fl_Widget *w, void*)
{
  int i,ires;
  liveWindow *win;
  win=(liveWindow *)(w->parent()->parent());

  ires=50;

  for (i=0;i<ires;i++)
  {
    win->live_stereo_B->value(2.5-i*5.0/(ires-1));
    cb_show_live_stereo(NULL,0);
    usleep(10000);
    Fl::check();
  }
  for (i=0;i<ires;i++)
  {
    win->live_stereo_B->value(-2.5+i*5.0/(ires-1));
    cb_show_live_stereo(NULL,0);
    usleep(10000);
    Fl::check();
  }

  win->redraw();
}

