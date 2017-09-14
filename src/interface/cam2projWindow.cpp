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

#include "cam2projWindow.h"

void cam2projWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=6*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);    
    //build_type=new Fl_Choice(STANDARD_WIDGET,"Type of reconstruction: ");
    //build_type->add("Original");
    //build_type->add("Simple Smooth");
    //build_type->add("Markov Smooth");
    //setChoice(build_type,&LINEPOSY);
    min_range=new Fl_Slider(STANDARD_WIDGET,langGetLabel(LABEL_CAM2PROJ_MIN_RANGE));
    setSlider(min_range,&LINEPOSY);
    min_range->callback(cb_min_range);
    min_range_box=new Fl_Box(STANDARD_WIDGET);
    setBox(min_range_box,&LINEPOSY);
    min_range_percentage=new Fl_Slider(STANDARD_WIDGET,langGetLabel(LABEL_CAM2PROJ_MIN_CONFIDENCE));
    setSlider(min_range_percentage,&LINEPOSY);
    min_range_percentage->callback(cb_min_range_percentage);
    min_range_percentage_box=new Fl_Box(STANDARD_WIDGET);
    setBox(min_range_percentage_box,&LINEPOSY);

    confidence_threshold=new Fl_Slider(STANDARD_WIDGET,langGetLabel(LABEL_CAM2PROJ_CONFIDENCE_THRESHOLD));
    setSlider(confidence_threshold,&LINEPOSY);
    confidence_threshold->callback(cb_confidence);

    confidence_box=new Fl_Box(STANDARD_WIDGET,"");
    setBox(confidence_box,&LINEPOSY);

    noise_mode=new Fl_Check_Button(STANDARD_WIDGET,langGetLabel(LABEL_CAM2PROJ_NOISE));
    setCheckButton(noise_mode,&LINEPOSY);

    crop_triangles=new Fl_Check_Button(STANDARD_WIDGET,langGetLabel(LABEL_CAM2PROJ_CROP));
    setCheckButton(crop_triangles,&LINEPOSY);
    crop_triangles->hide();
    //LINEPOSY+=LINESPACE;
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
}

void cam2projWindow::loadDefaultValues()
{
    //build_type->value(0);
    setMinRangePercentageValue(0.25);
    setMinRangeValue(0.15);
    setConfidenceValue(0.30);
    noise_mode->value(0);
    crop_triangles->value(1);
}

void cam2projWindow::setMinRangePercentageValue(double val)
{
  min_range_percentage->value(val);
  sprintf(min_range_percentage_buf,"%d",(int)(val*100));  
  min_range_percentage_box->label(min_range_percentage_buf);
}

void cam2projWindow::setMinRangeValue(double val)
{
  min_range->value(val);
  sprintf(min_range_buf,"%d",(int)(val*255));  
  min_range_box->label(min_range_buf);
}

void cam2projWindow::cb_min_range_percentage(Fl_Widget *w, void*)
{
  cam2projWindow *win=((cam2projWindow *)(w->parent()));
  win->setMinRangePercentageValue(((Fl_Slider *)(w))->value());
}

void cam2projWindow::cb_min_range(Fl_Widget *w, void*)
{
  cam2projWindow *win=((cam2projWindow *)(w->parent()));
  win->setMinRangeValue(((Fl_Slider *)(w))->value());
}

void cam2projWindow::setConfidenceValue(double val)
{
  confidence_threshold->value(val);
  sprintf(confidence_buf,"%d",(int)(val*255));  
  confidence_box->label(confidence_buf);
}

void cam2projWindow::cb_confidence(Fl_Widget *w, void*)
{
  cam2projWindow *win=((cam2projWindow *)(w->parent()));
  win->setConfidenceValue(((Fl_Slider *)(w))->value());
}

void cam2projWindow::cb_ok(Fl_Widget *w, void*)
{
  cam2projWindow *win;
  const char *str;

  win=((cam2projWindow *)(w->parent()));
  str=win->machine->mvalue()->label();
  win->hide();
  selectMachinesAndExecute(str,do_cam2proj);
}
void cam2projWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((cam2projWindow *)(w->parent()))->hide();
}
void cam2projWindow::cb_reset(Fl_Widget *w, void*)
{
  ((cam2projWindow *)(w->parent()))->loadDefaultValues();
}




