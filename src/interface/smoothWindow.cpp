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

#include "smoothWindow.h"

void smoothWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=4*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);    

    nb_patches_x=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_SMOOTH_NB_PATCHES_X));
    setChoice(nb_patches_x,&LINEPOSY);
    nb_patches_x->add("8");
    nb_patches_x->add("12");
    nb_patches_x->add("16");
    nb_patches_x->add("24");
    nb_patches_x->add("32");
    nb_patches_y=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_SMOOTH_NB_PATCHES_Y));
    setChoice(nb_patches_y,&LINEPOSY);
    nb_patches_y->add("8");
    nb_patches_y->add("12");
    nb_patches_y->add("16");
    nb_patches_y->add("24");
    nb_patches_y->add("32");

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

void smoothWindow::loadDefaultValues()
{
    nb_patches_x->value(2);
    nb_patches_y->value(2);
}

void smoothWindow::cb_ok(Fl_Widget *w, void*)
{
  smoothWindow *win;
  const char *str;

  win=((smoothWindow *)(w->parent()));
  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_smooth);
}
void smoothWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((smoothWindow *)(w->parent()))->hide();
}
void smoothWindow::cb_reset(Fl_Widget *w, void*)
{
  ((smoothWindow *)(w->parent()))->loadDefaultValues();
}
void smoothWindow::cb_undo(Fl_Widget *w, void*)
{
  smoothWindow *win;
  const char *str;

  win=((smoothWindow *)(w->parent()));
  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_smooth_reset);
}






