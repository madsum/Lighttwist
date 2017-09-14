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

#include "playlistLayer.h"

int playlistLayer::handle(int e)
{
    int used_event;
    int ret;
	
    ret=Fl_Box::handle(e);

    used_event=0;

    if (!used_event)
    {
      if (e==FL_PUSH)
      {
          //if (Fl::event_button() == 1) { cb_layer_left_click(this,NULL); }
          if (Fl::event_button() == 1 || Fl::event_button() == 3)
          {
            //fprintf(stderr,"CLICK LAYER\n");
            cb_layer_right_click(this,NULL);
            used_event=1;
          }
      }
    }

    if (used_event) return 1;
    else return ret;
}

void playlistLayer::draw()
{
  int i;
  Fl_Box::draw();

  //fprintf(stderr,"LAYER\n");

  for (i=0;i<MAX_NB_MEDIA;i++)
  {
    if (media[i]->active)
    {
      media[i]->resize(x(),y(),10,h());
      media[i]->draw();
    }
  }
}

void playlistLayer::setScale(int fps_arg,double disp_start_arg,double frames_to_pixels_arg,int offset_arg,int pixel_range)
{
  int i,j;

  for (i=0;i<MAX_NB_MEDIA;i++)
  {
    media[i]->fps=fps_arg;
    media[i]->frames_to_pixels=frames_to_pixels_arg;
    media[i]->disp_start=disp_start_arg;
    media[i]->offset=offset_arg;
    media[i]->pixel_min=offset_arg;
    media[i]->pixel_max=offset_arg+pixel_range;
    for (j=0;j<4;j++)
    {
      media[i]->fade_ctrls[j]->fps=fps_arg;
      media[i]->fade_ctrls[j]->frames_to_pixels=frames_to_pixels_arg;
      media[i]->fade_ctrls[j]->disp_start=disp_start_arg;
      media[i]->fade_ctrls[j]->offset=offset_arg;
      media[i]->fade_ctrls[j]->pixel_min=offset_arg;
      media[i]->fade_ctrls[j]->pixel_max=offset_arg+pixel_range;
    }
  }
  for (i=0;i<MAX_NB_MEDIA;i++)
  {
    media[i]->xmin=0.0;
    media[i]->xmax=MAX_NB_HOURS*60.0*60.0;
  }

  redraw();
}

void playlistLayer::resetMedia()
{
  int i;
  for (i=0;i<MAX_NB_MEDIA;i++)
  {
    media[i]->active=0;
  }
}

playlistMedia *playlistLayer::addMedia(unsigned short type,char *filename,int click_x,double len,double fps)
{
  int i,j;
  for (i=0;i<MAX_NB_MEDIA;i++)
  {
    if (!media[i]->active) break;
  }
  if (i==MAX_NB_MEDIA) return NULL;
  media[i]->color(FL_WHITE);
  media[i]->m_type=type;
  strcpy(media[i]->m_filename,filename);
  media[i]->label(media[i]->m_filename);
  media[i]->resize(0,y(),10,h());
  media[i]->active=1;
  media[i]->m_fps=(float)(fps);
  media[i]->start_x=media[i]->convertPixelToTimeAbsolute(click_x);
  media[i]->len_x=len;
  media[i]->len_x_orig=media[i]->len_x;
  media[i]->updateSize();
  for (j=0;j<4;j++)
  {
    media[i]->fade_ctrls[j]->show();
    if (j==0) media[i]->setControl(j,0.0,0.0);
    else if (j==1) media[i]->setControl(j,0.0,1.0);
    else if (j==2) media[i]->setControl(j,0.0,1.0);
    else media[i]->setControl(j,0.0,0.0);
  }
  media[i]->show();
  win->redraw();

  return media[i];
}

void playlistLayer::resetMenuFiles()
{
  menu_right_click->clear();
}

int playlistLayer::addMenuFile(const char *filename,Fl_Callback *fcn,void *data)
{
  if (filename==NULL) return -1;
  menu_right_click->add(filename,NULL,fcn,data);
  menu_right_click->hide();
  return 0;
}

void playlistLayer::showMenuFile(Fl_Widget *w)
{
  //menu_right_click->hide();
  menu_right_click->position(Fl::event_x(), Fl::event_y());
  if (menu_right_click->size()!=0)
  {
    const Fl_Menu_Item *m = menu_right_click->popup();
    if ( m ) m->do_callback(w, m->user_data());
  }
}



