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

#include "captureWindow.h"

void captureWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=6*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);
    mode=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_MODE));
    mode->add(langGetLabel(LABEL_CHOICE_PATTERNS));
    mode->add(langGetLabel(LABEL_CHOICE_GAMMA));
    mode->add(langGetLabel(LABEL_CHOICE_COLOR));
    mode->add(langGetLabel(LABEL_CHOICE_NOISE));
    mode->add(langGetLabel(LABEL_CHOICE_ALL));
    setChoice(mode,&LINEPOSY);
    stl_only=new Fl_Check_Button(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_STL_ONLY));
    setCheckButton(stl_only,&LINEPOSY);
    cam=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_CAMERA));
    cam->add(CAMERA_GPHOTO2);
    cam->add(CAMERA_V4L);
    cam->add(CAMERA_PROSILICA);
    setChoice(cam,&LINEPOSY);

    camId=new Fl_Input(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_CAMID));
    setInput(camId, &LINEPOSY);

    camDistoK1=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_DISTO_K1));
    setFloatSpinner(camDistoK1,-4.0,4.0,&LINEPOSY);
    camDistoK1->step(0.0001);
    camDistoK1->format("%.04f");

    camDistoK2=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_DISTO_K2));
    setFloatSpinner(camDistoK2,-4.0,4.0,&LINEPOSY);
    camDistoK2->step(0.0001);
    camDistoK2->format("%.04f");

    exposure_time=new Fl_Spinner(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_INTERVAL));
    setFloatSpinner(exposure_time,0.001,10.0,&LINEPOSY);
    exposure_time->step(0.001);
    exposure_time->format("%.03f");
    //exposure_time->hide();
    //LINEPOSY-=LINESPACE;
    transfer=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_TRANSFER));
    transfer->add(langGetLabel(LABEL_CHOICE_AUTOMATIC));
    transfer->add(langGetLabel(LABEL_CHOICE_MANUAL));
    setChoice(transfer,&LINEPOSY);

    LINEPOSY+=LINESPACE;
    posx=0;
    cancel=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_CANCEL));
    cancel->callback(cb_cancel);
    //posx+=-cancel->w()-OFFSET;
    cancel->hide();
    ok=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_OK));
    ok->callback(cb_ok);
    posx+=-ok->w()-OFFSET;
    test_camera=new Fl_Button(posx+STANDARD_WIDGET,langGetLabel(LABEL_CAPTURE_TEST));
    test_camera->callback(cb_test_camera);
}

void captureWindow::loadDefaultValues()
{
  mode->value(0);
  cam->value(0);
  camDistoK1->value(1.0);
  camDistoK2->value(1.0);
  exposure_time->value(0.5);
  transfer->value(0);
}

void captureWindow::cb_ok(Fl_Widget *w, void*)
{
  int i;
  captureWindow *win;
  const char *str;
  char cam[DATA_SIZE];
  int choice;

  win=((captureWindow *)(w->parent()));

  strcpy(cam,win->cam->mvalue()->label());

  if (strcmp(cam,CAMERA_GPHOTO2)==0 && strcmp(win->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_AUTOMATIC))==0)
  {
    if (win->stl_only->value()==0)
    {
      choice=fl_choice(langGetLabel(LABEL_DELETE_CAMERA_FILES),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),NULL);
    }
    else choice=1;
  }
  else choice=1;

  if (choice==1)
  {
    win->hide();
    if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0)
    {
      for (i=0;i<3;i++)
      {
        if (i>=2) continue; //skip colors
        win->mode->value(i);
        printf("LABEL %s\n",win->mode->mvalue()->label());
        //same as in 'else' condition...
        setTestPattern(TEST_PATTERN_BLACK);
        selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),start_background);
        str=win->machine->mvalue()->label();
        if (win->stl_only->value()) selectMachinesAndExecute(str,do_stl);
        else
        {
          selectMachinesAndExecute(str,do_grab);
          if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0) selectMachinesAndExecute(str,do_select);
        }
      }
      win->mode->value(3);
    }
    else
    {
      setTestPattern(TEST_PATTERN_BLACK);
      selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),start_background);
      str=win->machine->mvalue()->label();
      if (win->stl_only->value()) selectMachinesAndExecute(str,do_stl);
      else
      {
        if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_NOISE))==0)
        {
#ifdef CAPTURE_SIMULTANEOUS_NOISE
          selectMachinesAndExecute(str,do_grab_noise);
          selectMachinesAndExecute(str,do_select_noise);
#else
          selectMachinesAndExecute(str,do_grab);
          selectMachinesAndExecute(str,do_select);
#endif
        }
        else
        {
          selectMachinesAndExecute(str,do_grab);
          if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0) selectMachinesAndExecute(str,do_select);
        }
      }
    }
  }

  //rename and move files for manual transfer 
  if (strcmp(cam,CAMERA_GPHOTO2)==0 && strcmp(win->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_MANUAL))==0)
  {
    char *dir;
    dir=fl_dir_chooser(langGetLabel(LABEL_CAMERA_FILE_LOCATION),"/media/EOS_DIGITAL/DCIM/",0);
    if (dir!=NULL)
    {
      str=win->machine->mvalue()->label();
      win->findex=0;
      win->nbfiles=fioReadFolderFiles(win->filenames,dir);
      if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0 || strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0)
      {
        selectMachinesAndExecute(str,do_rename_patterns);
      }
      if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_GAMMA))==0 || strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0)
      {
        selectMachinesAndExecute(str,do_rename_gamma);
      }
      if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_COLOR))==0)
      {
        selectMachinesAndExecute(str,do_rename_color);
      }
      if (strcmp(win->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_NOISE))==0)
      {
        selectMachinesAndExecute(str,do_rename_noise);
      }
    }
  }
}

void captureWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((captureWindow *)(w->parent()))->hide();
}



