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

#include "gammaWindow.h"

void gammaWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=6*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);

    gamma_manual=new Fl_Check_Button(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_MANUAL));
    setCheckButton(gamma_manual,&LINEPOSY);
    gamma_manual->callback(cb_manual);

    gamma_projector[0]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_PROJECTOR_R));
    setFloatSpinner(gamma_projector[0],1.0,3.0,&LINEPOSY);
    gamma_projector[1]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_PROJECTOR_G));
    setFloatSpinner(gamma_projector[1],1.0,3.0,&LINEPOSY);
    gamma_projector[2]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_PROJECTOR_B));
    setFloatSpinner(gamma_projector[2],1.0,3.0,&LINEPOSY);

    gamma_camera[0]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_CAMERA_R));
    setFloatSpinner(gamma_camera[0],0.1,1.0,&LINEPOSY);
    gamma_camera[1]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_CAMERA_G));
    setFloatSpinner(gamma_camera[1],0.1,1.0,&LINEPOSY);
    gamma_camera[2]=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_GAMMA_CAMERA_B));
    setFloatSpinner(gamma_camera[2],0.1,1.0,&LINEPOSY);

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

void gammaWindow::loadDefaultValues()
{
    gamma_manual->value(1);
    gamma_camera[0]->value(1.0);
    gamma_camera[1]->value(1.0);
    gamma_camera[2]->value(1.0);
    gamma_projector[0]->value(2.2);
    gamma_projector[1]->value(2.2);
    gamma_projector[2]->value(2.2);
}

void gammaWindow::cb_manual(Fl_Widget *w, void*)
{
  gammaWindow *win;

  win=((gammaWindow *)(w->parent()));

  if (win->gamma_manual->value())
  {
    win->gamma_camera[0]->deactivate();
    win->gamma_camera[1]->deactivate();
    win->gamma_camera[2]->deactivate();
    win->gamma_projector[0]->activate();
    win->gamma_projector[1]->activate();
    win->gamma_projector[2]->activate();
  }
  else
  {
    win->gamma_camera[0]->activate();
    win->gamma_camera[1]->activate();
    win->gamma_camera[2]->activate();
    win->gamma_projector[0]->deactivate();
    win->gamma_projector[1]->deactivate();
    win->gamma_projector[2]->deactivate();
  }
}
void gammaWindow::cb_ok(Fl_Widget *w, void*)
{
  gammaWindow *win;
  const char *str;

  win=((gammaWindow *)(w->parent()));

  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_gamma);
}
void gammaWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((gammaWindow *)(w->parent()))->hide();
}
void gammaWindow::cb_reset(Fl_Widget *w, void*)
{
  ((gammaWindow *)(w->parent()))->loadDefaultValues();
}
void gammaWindow::cb_undo(Fl_Widget *w, void*)
{
  gammaWindow *win;
  const char *str;

  win=((gammaWindow *)(w->parent()));

  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_gamma_reset);
}




