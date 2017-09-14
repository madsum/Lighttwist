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

#include "blendWindow.h"

void blendWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=6*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);    

    LINEPOSY+=LINESPACE;
    posx=0;
    cancel=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_CANCEL));
    cancel->callback(cb_cancel);
    //posx+=-cancel->w()-OFFSET;
    cancel->hide();
    ok=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_OK));
    ok->callback(cb_ok);
    posx+=-ok->w()-OFFSET;
    reset=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_RESET));
    reset->callback(cb_reset);
    reset->hide();
}

void blendWindow::loadDefaultValues()
{

}

void blendWindow::cb_ok(Fl_Widget *w, void*)
{
  blendWindow *win;
  
  win=(blendWindow *)(w->parent());

  win->hide();

  if (win->machine->size()>2) //stereo
  {
    if (!strcmp(win->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL)))
    {
      do_blend(CHANNEL_LEFT);
      do_blend(CHANNEL_RIGHT);
    }
    else if (!strcmp(win->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_RIGHT)))
    {
      do_blend(CHANNEL_RIGHT);
    }
    else
    {
      do_blend(CHANNEL_LEFT);
    }
  }
  else
  {
    do_blend(CHANNEL_MONO);
  }
}
void blendWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((blendWindow *)(w->parent()))->hide();
}
void blendWindow::cb_reset(Fl_Widget *w, void*)
{
  ((blendWindow *)(w->parent()))->loadDefaultValues();
}



