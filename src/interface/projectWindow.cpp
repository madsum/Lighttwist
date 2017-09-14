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

#include "projectWindow.h"

void projectWindow::createMenuWindow()
{
    int templine;

    resolution_items[0].id=0;
    strcpy(resolution_items[0].label,"1920x1080");
    resolution_items[1].id=1;
    strcpy(resolution_items[1].label,"1600x1200");
    resolution_items[2].id=2;
    strcpy(resolution_items[2].label,"1280x1024");
    resolution_items[3].id=3;
    strcpy(resolution_items[3].label,"1280x800");
    resolution_items[4].id=4;
    strcpy(resolution_items[4].label,"1200x800");
    resolution_items[5].id=5;
    strcpy(resolution_items[5].label,"1024x768");
    resolution_items[6].id=6;
    strcpy(resolution_items[6].label,"800x600");

    configuration_items[PLANAR_CONFIGURATION].id=PLANAR_CONFIGURATION;
    strcpy(configuration_items[PLANAR_CONFIGURATION].label,"Rectangle");
    configuration_items[HOMOGRAPHY_CONFIGURATION].id=HOMOGRAPHY_CONFIGURATION;
    strcpy(configuration_items[HOMOGRAPHY_CONFIGURATION].label,langGetLabel(LABEL_CONFIGURATION_HOMOGRAPHY));
    configuration_items[CYCLORAMA_CONFIGURATION].id=CYCLORAMA_CONFIGURATION;
    strcpy(configuration_items[CYCLORAMA_CONFIGURATION].label,"Panorama");
    configuration_items[BEZIER_CONFIGURATION].id=BEZIER_CONFIGURATION;
    strcpy(configuration_items[BEZIER_CONFIGURATION].label,"Bezier");

    LINEPOSY=2*OFFSET;

    project=new Fl_Input(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_NAME));
    setInput(project,&LINEPOSY);
    LINEPOSY+=LINESPACE;
    configuration=new Fl_Choice(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_CONFIGURATION));
    configuration->add(configuration_items[0].label);
    configuration->add(configuration_items[1].label);
    configuration->add(configuration_items[2].label);
    configuration->add(configuration_items[3].label);
    setChoice(configuration,&LINEPOSY);
    vmirror=new Fl_Check_Button(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_VMIRROR));
    setCheckButton(vmirror,&LINEPOSY);
    //vmirror->hide();
    //LINEPOSY-=LINESPACE;
    mode=new Fl_Choice(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_MODE));
    mode->add(langGetLabel(LABEL_MODE_MONO));
    mode->add(langGetLabel(LABEL_MODE_STEREO_RC));
    mode->add(langGetLabel(LABEL_MODE_STEREO_P));
    setChoice(mode,&LINEPOSY);
    screen_ratio=new Fl_Spinner(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_RATIO));
    setFloatSpinner(screen_ratio,0.05,20.0,&LINEPOSY);
    nb_projectors=new Fl_Spinner(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_NB_PROJECTORS));
    setIntegerSpinner(nb_projectors,1,MAX_NB_MACHINES/2,&LINEPOSY);
    resolution=new Fl_Choice(-SMALL_WIDGET_WIDTH+STANDARD_WIDGET,langGetLabel(LABEL_PROJECT_RESOLUTION));
    resolution->add(resolution_items[0].label);
    resolution->add(resolution_items[1].label);
    resolution->add(resolution_items[2].label);
    resolution->add(resolution_items[3].label);
    resolution->add(resolution_items[4].label);
    resolution->add(resolution_items[5].label);
    resolution->add(resolution_items[6].label);
    resolution->add(resolution_items[7].label);
    setChoice(resolution,&LINEPOSY);
    nb_speakers=new Fl_Spinner(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_NB_SPEAKERS));
    setIntegerSpinner(nb_speakers,0,MAX_NB_SPEAKERS,&LINEPOSY);
    sound_ctrl_ip=new Fl_Input(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_SOUND_CTRL_IP));
    setInput(sound_ctrl_ip,&LINEPOSY);
    enable_jack=new Fl_Check_Button(-SMALL_WIDGET_WIDTH+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_ENABLE_JACK));
    setCheckButton(enable_jack,&LINEPOSY);
    LINEPOSY+=LINESPACE;
    local_media_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_media_but,&templine);
    local_media_but->callback(cb_local_media);
    local_media=new Fl_Input(-local_media_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_MEDIA_DIRECTORY));
    setInput(local_media,&LINEPOSY);
    local_scan_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_scan_but,&templine);
    local_scan_but->callback(cb_local_scan);
    local_scan=new Fl_Input(-local_scan_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_LOCAL_SCAN_DIRECTORY));
    setInput(local_scan,&LINEPOSY);
    local_data_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_data_but,&templine);
    local_data_but->callback(cb_local_data);
    local_data=new Fl_Input(-local_data_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_LOCAL_DATA_DIRECTORY));
    setInput(local_data,&LINEPOSY);
    local_bin_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_bin_but,&templine);
    local_bin_but->callback(cb_local_bin);
    local_bin=new Fl_Input(-local_bin_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_LOCAL_BIN_DIRECTORY));
    setInput(local_bin,&LINEPOSY);
    local_lib_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_lib_but,&templine);
    local_lib_but->callback(cb_local_lib);
    local_lib=new Fl_Input(-local_lib_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_LOCAL_LIB_DIRECTORY));
    setInput(local_lib,&LINEPOSY);
    local_share_but=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_BROWSE));
    setButton(local_share_but,&templine);
    local_share_but->callback(cb_local_share);
    local_share=new Fl_Input(-local_share_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_LOCAL_SHARE_DIRECTORY));
    setInput(local_share,&LINEPOSY);

    //remoteaslocal=new Fl_Check_Button(STANDARD_WIDGET, "Remote as local");
    //setCheckButton(remoteaslocal,&LINEPOSY);
    //remoteaslocal->callback(cb_remoteaslocal);

    LINEPOSY+=LINESPACE;
    remote_scan=new Fl_Input(-local_share_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_REMOTE_SCAN_DIRECTORY));
    setInput(remote_scan,&LINEPOSY);
    remote_data=new Fl_Input(-local_share_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_REMOTE_DATA_DIRECTORY));
    setInput(remote_data,&LINEPOSY);
    remote_bin=new Fl_Input(-local_share_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_REMOTE_BIN_DIRECTORY));
    setInput(remote_bin,&LINEPOSY);
    remote_share=new Fl_Input(-local_share_but->w()+VERY_LARGE_WIDGET,langGetLabel(LABEL_PROJECT_REMOTE_SHARE_DIRECTORY));
    setInput(remote_share,&LINEPOSY);

    LINEPOSY+=LINESPACE/2;
    ok=new Fl_Button(SMALL_WIDGET,langGetLabel(LABEL_APPLY));
    ok->callback(cb_ok);
    reset=new Fl_Button(-ok->w()-OFFSET+SMALL_WIDGET,langGetLabel(LABEL_RESET));
    reset->callback(cb_reset);
}

void projectWindow::loadDefaultValues()
{
  if (langGet()==LANG_ENGLISH) sprintf(project_str,"%s/%s/lighttwist/projects/my_first_project.ltproj",BASE_PATH,username);
  else sprintf(project_str,"%s/%s/lighttwist/projets/mon_premier_projet.ltproj",BASE_PATH,username);
  sprintf(sound_ctrl_ip_str,"localhost");
  sprintf(local_media_str,"%s/%s/lighttwist/media",BASE_PATH,username);
  sprintf(local_scan_str,"%s/%s/lighttwist/scan",BASE_PATH,username);
  sprintf(local_data_str,"%s/%s/lighttwist/data",BASE_PATH,username);
  //sprintf(local_bin_str,"/usr/local/bin");
  //sprintf(local_lib_str,"/usr/local/lib");
  //sprintf(local_share_str,"/usr/local/share/lighttwist");
  sprintf(local_bin_str,"/usr/bin");
  sprintf(local_lib_str,"/usr/lib");
  sprintf(local_share_str,"/usr/share/lighttwist");
  sprintf(remote_scan_str,"~/lighttwist/scan");
  sprintf(remote_data_str,"~/lighttwist/data");
  //sprintf(remote_bin_str,"/usr/local/bin");
  //sprintf(remote_share_str,"/usr/local/share/lighttwist");
  sprintf(remote_bin_str,"/usr/bin");
  sprintf(remote_share_str,"/usr/share/lighttwist");

  project->value(project_str);
  configuration->value(0);
  vmirror->value(0);
  mode->value(0);
  screen_ratio->value(16.0/9.0);
  //nb_projectors->value(MAX_NB_MACHINES/2);
  nb_projectors->value(8);
  resolution->value(0);
  nb_speakers->value(8);
  sound_ctrl_ip->value(sound_ctrl_ip_str);
  enable_jack->value(0);
  local_media->value(local_media_str);
  local_scan->value(local_scan_str);
  local_data->value(local_data_str);
  local_bin->value(local_bin_str);
  local_lib->value(local_lib_str);
  local_share->value(local_share_str);
  //remoteaslocal->value(0);
  remote_scan->value(remote_scan_str);
  remote_data->value(remote_data_str);
  remote_bin->value(remote_bin_str);
  remote_share->value(remote_share_str);
}

int projectWindow::getResolution(int *resx,int *resy)
{
  if (resx==NULL || resy==NULL) return -1;

  sscanf(resolution->mvalue()->label(),"%dx%d",resx,resy);

  return 0;
}

void projectWindow::cb_local_media(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_media->value(dir);
}

void projectWindow::cb_local_scan(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_scan->value(dir);
}

void projectWindow::cb_local_data(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_data->value(dir);
}

void projectWindow::cb_local_bin(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_bin->value(dir);
}

void projectWindow::cb_local_lib(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_lib->value(dir);
}

void projectWindow::cb_local_share(Fl_Widget *w, void*)
{
  char *dir;
  projectWindow *win=(projectWindow *)(w->parent());
  dir=fl_dir_chooser(langGetLabel(LABEL_CHOOSE_DIRECTORY),win->default_dir,0);
  if (dir!=NULL) win->local_share->value(dir);
}

void projectWindow::cb_ok(Fl_Widget *w, void*)
{
  ((projectWindow *)(w->parent()))->hide();
  cb_update_project();
  //cb_save(NULL,NULL);
}
void projectWindow::cb_reset(Fl_Widget *w, void*)
{
  ((projectWindow *)(w->parent()))->loadDefaultValues();
}





