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

#include "patternWindow.h"

void patternWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=6*OFFSET;


    frequency_items[0].id=0;
    strcpy(frequency_items[0].label,"16");
    frequency_items[1].id=1;
    strcpy(frequency_items[1].label,"32");
    frequency_items[2].id=2;
    strcpy(frequency_items[2].label,"64");

    nb_patterns_items[0].id=0;
    strcpy(nb_patterns_items[0].label,"300");
    nb_patterns_items[1].id=1;
    strcpy(nb_patterns_items[1].label,"200");
    nb_patterns_items[2].id=2;
    strcpy(nb_patterns_items[2].label,"100");

    frequency=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_PATTERN_FREQUENCY));
    frequency->add(frequency_items[0].label);
    frequency->add(frequency_items[1].label);
    frequency->add(frequency_items[2].label);
    setChoice(frequency,&LINEPOSY);
    nb_patterns=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_PATTERN_NB));
    nb_patterns->add(nb_patterns_items[0].label);
    nb_patterns->add(nb_patterns_items[1].label);
    nb_patterns->add(nb_patterns_items[2].label);
    setChoice(nb_patterns,&LINEPOSY);

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

void patternWindow::loadDefaultValues()
{
  frequency->value(0);
  nb_patterns->value(0);
}

void patternWindow::cb_ok(Fl_Widget *w, void*)
{
  patternWindow *win;
  
  win=(patternWindow *)(w->parent());

  win->hide();
}
void patternWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((patternWindow *)(w->parent()))->hide();
}
void patternWindow::cb_reset(Fl_Widget *w, void*)
{
  ((patternWindow *)(w->parent()))->loadDefaultValues();
}




