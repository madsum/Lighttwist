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

#include "colorWindow.h"

void colorWindow::createMenuWindow()
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
    posx+=-reset->w()-OFFSET;
    undo=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_UNDO));
    undo->callback(cb_undo);
}

void colorWindow::loadDefaultValues()
{

}

void colorWindow::cb_ok(Fl_Widget *w, void*)
{
  colorWindow *win;
  const char *str;

  win=((colorWindow *)(w->parent()));
  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_color);
}
void colorWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((colorWindow *)(w->parent()))->hide();
}
void colorWindow::cb_reset(Fl_Widget *w, void*)
{
  ((colorWindow *)(w->parent()))->loadDefaultValues();
}
void colorWindow::cb_undo(Fl_Widget *w, void*)
{
  colorWindow *win;
  const char *str;

  win=((colorWindow *)(w->parent()));
  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_color_reset);
}






