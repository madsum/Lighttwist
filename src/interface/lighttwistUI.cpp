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

#include "lighttwistUI.h"

void lighttwistUI::createUIWindow(char *login,Machine *m)
{
  int templine;
  int i,index;

  username=login;
  machines=m;

  Fl_Menu_Bar* menubar = new Fl_Menu_Bar(0, 0, w(), 20);
  menubar = new Fl_Menu_Bar(0, 0, w(), 25);
  menubar->color(getGrayLevel(MENU_INTENSITY));
  menubar->box(BOX_USER_DRAW);
  if (usertype==USER_ADMIN)
  {
    menubar->add(langGetLabel(LABEL_MENU_PROJECT_OPEN), 0, cb_open, (void*)this);
    menubar->add(langGetLabel(LABEL_MENU_PROJECT_EDIT), 0, cb_project, (void*)this);
    menubar->add(langGetLabel(LABEL_MENU_PROJECT_SAVE), 0, cb_save, (void*)this);
  }
  menubar->add(langGetLabel(LABEL_MENU_PROJECT_QUIT), 0, cb_exit, (void*)this);
  //menubar->add(langGetLabel(LABEL_MENU_MEDIA_EXTENSIONS), 0, cb_media_ext, (void*)this);
  //menubar->add("Media/Add Video...", 0, NULL, (void*)this);
  //menubar->add("Media/Add Audio...", 0, NULL, (void*)this);
  //menubar->add("Media/Add 3D Scene...", 0, NULL, (void*)this);
  //menubar->add("Playlist/Load", 0, NULL, (void*)this);
  //menubar->add("Playlist/Save", 0, NULL, (void*)this);
  menubar->add(langGetLabel(LABEL_MENU_HELP_ONLINE), 0, NULL, (void*)this);
  menubar->add(langGetLabel(LABEL_MENU_HELP_PROBLEM), 0, NULL, (void*)this);
  menubar->add(langGetLabel(LABEL_MENU_HELP_ABOUT), 0, NULL, (void*)this);
  
  watcher.start_stop_thread(true,*this);

  Fl_Gel_Tabs *tabs = new Fl_Gel_Tabs(OFFSET,3*OFFSET,w()-2*OFFSET,h()-4*OFFSET-STATUS_BAR_HEIGHT);
  {
      int gx=tabs->x(), gy=tabs->y()+4*OFFSET, gw=tabs->w(), gh=tabs->h()-2*OFFSET;
      Fl_Group *g;

      if (usertype==USER_ADMIN) tabs->begin();
      else tabs->end();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_SETUP));
      g->color(getGrayLevel(BCKGRND_INTENSITY));

      LINEPOSY=3*LINESPACE+10-8;

      test_pat_header = new Fl_Box(STANDARD_WIDGET, langGetLabel(LABEL_TEST_PATTERN));
      test_pat_header->position(test_pat_header->x()-80,test_pat_header->y());
      setBox(test_pat_header,&LINEPOSY);
      test_pat_header->labelfont(FL_HELVETICA_BOLD);

      LINEPOSY=2*LINESPACE+10;

      test_pat[0] = new Fl_Round_Button(2*OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_TEST_WHITE));
      test_pat[0]->labelsize(FONTSIZE);
      test_pat[0]->type(102);
      test_pat[0]->value(1);
      LINEPOSY+=LINESPACE-8;
      test_pat[1] = new Fl_Round_Button(2*OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_TEST_BLACK));
      test_pat[1]->labelsize(FONTSIZE);
      test_pat[1]->type(102);
      LINEPOSY+=LINESPACE-8;
      test_pat[2] = new Fl_Round_Button(2*OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_TEST_CHECKER));
      test_pat[2]->labelsize(FONTSIZE);
      test_pat[2]->type(102);
      LINEPOSY+=LINESPACE;

      LINEPOSY=5*LINESPACE+3;

      m_scroll=new Fl_Scroll(g->x()+2*OFFSET,5*LINESPACE,g->w()-g->x()-3*OFFSET,g->h()-g->y()-3*LINESPACE,NULL);
      m_scroll->box(FL_NO_BOX);
      m_scroll->type(Fl_Scroll::VERTICAL);

      for (i=0;i<MAX_NB_MACHINES/2;i++)
      {
        index=i+MAX_NB_MACHINES/2;
        m_test[index]=new Fl_Button(-m_scroll->x()+VERY_SMALL_WIDGET,langGetLabel(LABEL_TEST));
        setButton(m_test[index],&templine);
        m_test[index]->callback(cb_test,index);
        //the following if condition is for spacing issue, to align the "Machine" label
        if (index>=10) sprintf(machines[index].label,langGetLabel(LABEL_MACHINE_IP_SPACE),index);
        else sprintf(machines[index].label,langGetLabel(LABEL_MACHINE_IP),index);
        m_ip[index]=new Fl_Input(-m_scroll->x()-m_test[index]->w()+BIG_WIDGET,machines[index].label);
        setInput(m_ip[index],&templine);
        m_test[i]=new Fl_Button(VERY_SMALL_WIDGET,langGetLabel(LABEL_TEST));
        setButton(m_test[i],&templine);
        m_test[i]->callback(cb_test,i);
        sprintf(machines[i].label,langGetLabel(LABEL_MACHINE_IP_SPACE),i);
        m_ip[i]=new Fl_Input(BIG_WIDGET,machines[i].label);
        setInput(m_ip[i],&templine);
        LINEPOSY+=LINESPACE;
      }

      m_scroll->end();

      LINEPOSY=13*LINESPACE+OFFSET;

      test_all_but=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_TEST_ALL));
      setButton(test_all_but,&LINEPOSY);
      test_all_but->callback(cb_test_all,-1);
      LINEPOSY-=LINESPACE;
      connect_all_but=new Fl_Button(-test_all_but->w()-OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_RECONNECT_ALL));
      setButton(connect_all_but,&LINEPOSY);
      connect_all_but->callback(cb_connect,-1);
      
      g->end();
      if (usertype==USER_ARTIST) g->hide();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_SCAN));
      g->color(getGrayLevel(BCKGRND_INTENSITY));

      scan_data=new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_NONE,160,85,NULL);

      LINEPOSY=3*LINESPACE;
      scan_mode_items[0].id=0;
      strcpy(scan_mode_items[0].label,langGetLabel(LABEL_SCAN_NORMAL));
      scan_mode_items[1].id=1;
      strcpy(scan_mode_items[1].label,langGetLabel(LABEL_SCAN_ADVANCED));

      scan_mode=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_MODE));
      scan_mode->add(scan_mode_items[0].label);
      scan_mode->add(scan_mode_items[1].label);
      setChoice(scan_mode,&LINEPOSY);
      scan_mode->callback(cb_scan_mode);

      LINEPOSY+=LINESPACE;

      scan_pattern=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_PATTERN));
      setButton(scan_pattern,&LINEPOSY);
      scan_pattern->callback(cb_scan_pattern);
      scan_grab=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_CAPTURE));
      setButton(scan_grab,&LINEPOSY);
      scan_grab->callback(cb_scan_grab);
      scan_select=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_SELECT));
      setButton(scan_select,&LINEPOSY);
      LINEPOSY-=LINESPACE;
      scan_sum=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_SUM));
      setButton(scan_sum,&LINEPOSY);
      LINEPOSY-=LINESPACE;
      scan_uvmap=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_UVMAP));
      setButton(scan_uvmap,&LINEPOSY);
      scan_uvmap->callback(cb_scan_uvmap);

      scan_calibrate=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_CALIBRATE));
      setButton(scan_calibrate,&LINEPOSY);
      scan_calibrate->callback(cb_scan_calibrate);
      LINEPOSY-=LINESPACE;
      scan_cam2proj=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_CAM2PROJ));
      setButton(scan_cam2proj,&LINEPOSY);
      scan_cam2proj->callback(cb_scan_cam2proj);
      scan_smooth=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_SMOOTH));
      setButton(scan_smooth,&LINEPOSY);
      scan_smooth->callback(cb_scan_smooth);
      scan_gamma=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_GAMMA));
      setButton(scan_gamma,&LINEPOSY);
      scan_gamma->callback(cb_scan_gamma);
      scan_color=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_COLOR));
      setButton(scan_color,&LINEPOSY);
      scan_color->callback(cb_scan_color);
      scan_color->deactivate();
      scan_blend=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_BLEND));
      setButton(scan_blend,&LINEPOSY);
      scan_blend->callback(cb_scan_blend);
      scan_uv2proj=new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SCAN_UV2PROJ));
      setButton(scan_uv2proj,&LINEPOSY);
      scan_uv2proj->callback(cb_scan_uv2proj);

      scan_select->hide();
      scan_sum->hide();

      g->end();
      if (usertype==USER_ARTIST) g->hide();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_SOUND));
      g->color(getGrayLevel(BCKGRND_INTENSITY));

      sound_space=new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_NONE,185,85,NULL);
      for (i=0;i<MAX_NB_SPEAKERS;i++) 
      {
        speakers[i]=new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_BOTH,0,0,NULL);
      }

      LINEPOSY=4*LINESPACE;

      subwoofer_index=new Fl_Spinner(-2*OFFSET+SMALL_WIDGET,langGetLabel(LABEL_SOUND_SUBWOOFER));
      setIntegerSpinner(subwoofer_index,1,MAX_NB_SPEAKERS,&LINEPOSY);
      subwoofer_index->align(FL_ALIGN_TOP);
      subwoofer_index->hide();

      g->end();

      if (usertype==USER_ARTIST) g->hide();
      tabs->begin();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_MEDIA));
      g->color(getGrayLevel(BCKGRND_INTENSITY));

      LINEPOSY=3*LINESPACE;

      media_path_label_header=new Fl_Box(LARGE_WIDGET,langGetLabel(LABEL_MEDIA_TREE));
      media_path_label_header->position(g->x()+OFFSET,media_path_label_header->y());
      setBox(media_path_label_header,&LINEPOSY);
      LINEPOSY-=LINESPACE;
      media_path_label_header->labelfont(FL_HELVETICA_BOLD);
      media_path_label=new Fl_Box(-560+LARGE_WIDGET,NULL);
      setBox(media_path_label,&LINEPOSY);
      media_path_label->align(FL_ALIGN_RIGHT);

      media_scroll=new Fl_Scroll(g->x()+2*OFFSET,4*LINESPACE,g->w()-g->x()-2*OFFSET-240,g->h()-g->y()-2*OFFSET,NULL);
      media_scroll->box(FL_DOWN_BOX);
      media_scroll->end();

      media_extensions_header=new Fl_Box(-65+LARGE_WIDGET,"Extensions: ");
      setBox(media_extensions_header,&LINEPOSY);
      media_extensions_header->labelfont(FL_HELVETICA_BOLD);
      ext_video=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_VIDEO));
      setInput(ext_video,&LINEPOSY);
      ext_3d=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_3D));
      setInput(ext_3d,&LINEPOSY);
      ext_image=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_IMAGE));
      setInput(ext_image,&LINEPOSY);
      ext_audio=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_AUDIO));
      setInput(ext_audio,&LINEPOSY);
      ext_stats=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_STATS));
      setInput(ext_stats,&LINEPOSY);
      ext_stats->hide();
      LINEPOSY-=LINESPACE;
      ext_playlist=new Fl_Input(BIG_WIDGET,langGetLabel(LABEL_EXT_PLAYLIST));
      setInput(ext_playlist,&LINEPOSY);
      ext_playlist->hide();

      LINEPOSY+=LINESPACE+OFFSET;

      media_selected_header=new Fl_Box(-38+LARGE_WIDGET,langGetLabel(LABEL_SELECTED_MEDIA));
      setBox(media_selected_header,&LINEPOSY);
      media_selected_header->labelfont(FL_HELVETICA_BOLD);

      media_default_but = new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_RESET));
      setButton(media_default_but,&LINEPOSY);
      media_default_but->callback(cb_media_default);
      media_default_but->hide();

      LINEPOSY-=LINESPACE;

      //media_refresh_but = new Fl_Button(-media_default_but->w()-OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_REFRESH));
      //setButton(media_refresh_but,&LINEPOSY);
      //media_refresh_but->callback(cb_media_refresh);
      media_refresh_but = new DnDBox(this,DND_STANDARD,cb_media_refresh,NULL,DRAG_NONE,g->x()+g->w()-6*OFFSET,9*LINESPACE+OFFSET,NULL);
      media_refresh_but->label(langGetLabel(LABEL_REFRESH));

      delete_import_but = new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_REMOVE_MEDIA));
      setButton(delete_import_but,&LINEPOSY);
      delete_import_but->callback(cb_media_delete);
      LINEPOSY-=LINESPACE;
      import_but = new Fl_Button(-delete_import_but->w()+STANDARD_WIDGET,langGetLabel(LABEL_IMPORT_MEDIA));
      setButton(import_but,&LINEPOSY);
      import_but->callback(cb_media_precrop);

      g->end();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_PLAYLIST));
      g->color(getGrayLevel(BCKGRND_INTENSITY));

      LINEPOSY=3*LINESPACE;

      pl_scroll=new Fl_Scroll(2*OFFSET,4*LINESPACE,g->w()-242,290,NULL);
      pl_scroll->type(Fl_Scroll::VERTICAL);
      timeline = new playlistTimeline(this,pl_scroll->x(),pl_scroll->y(),g->w()-260,290);
      timeline->end();
      pl_scroll->end();
      timeline->scale_slider->callback(cb_scale);

      //timeline = new playlistTimeline(this,2*OFFSET,4*LINESPACE,g->w()-260,320);
      //timeline->end();

      int pl_x,pl_y;
      pl_x=560;
      pl_y=g->y()+6*OFFSET;
      pl_subdirs_scroll=new Fl_Scroll(pl_x,pl_y,220,g->h()-g->y()-3*LINESPACE,NULL);
      pl_subdirs_scroll->box(FL_NO_BOX);
      pl_subdirs_scroll->end();

      pl_path_label_header=new Fl_Box(LARGE_WIDGET,langGetLabel(LABEL_CURRENT_MEDIA));
      pl_path_label_header->position(g->x()+OFFSET,pl_path_label_header->y());
      setBox(pl_path_label_header,&LINEPOSY);
      LINEPOSY-=LINESPACE;
      pl_path_label_header->labelfont(FL_HELVETICA_BOLD);
      pl_path_label=new Fl_Box(-530+LARGE_WIDGET,NULL);
      setBox(pl_path_label,&LINEPOSY);
      pl_path_label->align(FL_ALIGN_RIGHT);
      pl_path_back=new Fl_Button(pl_x+MEDIA_BUT_HEIGHT,pl_y-MEDIA_BUT_HEIGHT,MEDIA_BUT_HEIGHT,MEDIA_BUT_HEIGHT,"@<");
      pl_path_back->color(fl_rgb_color(COLOR_DIRECTORY));
      pl_path_back->callback(cb_subdirs_back);

      LINEPOSY=13*LINESPACE+2*OFFSET;

      //pl_save_but = new Fl_Button(STANDARD_WIDGET,langGetLabel(LABEL_SAVE_PLAYLIST));
      //setButton(pl_save_but,&LINEPOSY);
      //pl_save_but->callback(cb_save_playlist);

      //LINEPOSY-=LINESPACE;

      //pl_load_but = new Fl_Button(-pl_save_but->w()-OFFSET+STANDARD_WIDGET,langGetLabel(LABEL_LOAD_PLAYLIST));
      //setButton(pl_load_but,&LINEPOSY);
      //pl_load_but->callback(cb_load_playlist);

      pl_new_but = new DnDBox(this,DND_STANDARD,cb_new_playlist,NULL,DRAG_NONE,g->x()+g->w()-21*OFFSET,13*LINESPACE,NULL);
      pl_load_but = new DnDBox(this,DND_STANDARD,cb_load_playlist,NULL,DRAG_NONE,g->x()+g->w()-14*OFFSET,13*LINESPACE,NULL);
      pl_save_but = new DnDBox(this,DND_STANDARD,cb_save_playlist,NULL,DRAG_NONE,g->x()+g->w()-7*OFFSET,13*LINESPACE,NULL);

      g->end();

      g = new Fl_Group(gx,gy,gw,gh,langGetLabel(LABEL_TAB_SHOW));
      g->color(getGrayLevel(BCKGRND_INTENSITY));
      LINEPOSY=3*LINESPACE;

      threading_items[THREADING_SINGLE].id=THREADING_SINGLE;
      strcpy(threading_items[THREADING_SINGLE].label,langGetLabel(LABEL_PLAYER_SINGLE_THREAD));
      threading_items[THREADING_MULTI].id=THREADING_MULTI;
      strcpy(threading_items[THREADING_MULTI].label,langGetLabel(LABEL_PLAYER_MULTI_THREAD));

      threading=new Fl_Choice(-10+SMALL_WIDGET,langGetLabel(LABEL_PLAYER_THREADING));
      threading->add(threading_items[0].label);
      threading->add(threading_items[1].label);
      setChoice(threading,&LINEPOSY);
      threading->align(FL_ALIGN_TOP);
      threading->hide();
      LINEPOSY-=LINESPACE;

      player_items[PLAYER_DEFAULT].id=PLAYER_DEFAULT;
      strcpy(player_items[PLAYER_DEFAULT].label,langGetLabel(LABEL_PLAYER_DEFAULT));
      player_items[PLAYER_LUA].id=PLAYER_LUA;
      strcpy(player_items[PLAYER_LUA].label,langGetLabel(LABEL_PLAYER_LUA));

      player_select=new Fl_Choice(-10+SMALL_WIDGET,langGetLabel(LABEL_PLAYER_SELECT));
      player_select->add(player_items[0].label);
      player_select->add(player_items[1].label);
      setChoice(player_select,&LINEPOSY);
      player_select->align(FL_ALIGN_TOP);

      show_setup_but = new DnDBox(this,DND_STANDARD,cb_show_setup,NULL,DRAG_NONE,g->x()+g->w()-11*OFFSET,5*LINESPACE-2*OFFSET,NULL);
      show_play_but = new DnDBox(this,DND_STANDARD,cb_show_play,NULL,DRAG_NONE,g->x()+g->w()-11*OFFSET,7*LINESPACE-2*OFFSET,NULL);
      show_loop_but = new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_NONE,g->x()+g->w()-11*OFFSET,9*LINESPACE-2*OFFSET,NULL);
      show_sound_but = new DnDBox(this,DND_STANDARD,cb_show_mute,NULL,DRAG_NONE,g->x()+g->w()-11*OFFSET,11*LINESPACE-2*OFFSET,NULL);
      show_joystick_but = new DnDBox(this,DND_STANDARD,cb_show_joystick,NULL,DRAG_NONE,g->x()+g->w()-11*OFFSET,13*LINESPACE-2*OFFSET,NULL);
      LINEPOSY=14*LINESPACE;

      show_live_but = new Fl_Button(BIG_WIDGET,langGetLabel(LABEL_LIVE_WINDOW_TITLE));
      setButton(show_live_but,&LINEPOSY);
      show_live_but->callback(cb_show_live);

      playlist_scroll=new Fl_Scroll(g->x()+2*OFFSET,3*LINESPACE,g->w()-g->x()-11*OFFSET-50,g->h()-g->y()-2*OFFSET,NULL);
      playlist_scroll->box(FL_DOWN_BOX);
      playlist_scroll->end();

      g->end();

      //tabs->resizable(g);
  }
  tabs->end();
  tabs->box(FL_THIN_UP_BOX);

  status=new Fl_Box(tabs->x(),tabs->y()+tabs->h(),tabs->w(),STATUS_BAR_HEIGHT,langGetLabel(LABEL_STATUS));
  setBox(status,&LINEPOSY);
  status->color(getGrayLevel(MENU_INTENSITY));
  status->box(BOX_USER_DRAW);
  status->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  loadDefaultValues();
}

void lighttwistUI::cb_scan_mode(Fl_Widget *w, void*)
{
  lighttwistUI *win=((lighttwistUI *)(w->parent()->parent()->parent()));

  win->updateScanDisplay();
}

void lighttwistUI::updateScanDisplay()
{
  if (scan_mode->value()==0)
  {
    scan_cam2proj->hide();
    scan_smooth->hide();
    scan_gamma->hide();
    scan_color->hide();
    scan_blend->hide();
    scan_uv2proj->hide();
    scan_calibrate->show();
  }
  else
  {
    scan_cam2proj->show();
    scan_smooth->show();
    scan_gamma->show();
    scan_color->show();
    scan_blend->show();
    scan_uv2proj->show();
    scan_calibrate->hide();
  }
}

void lighttwistUI::loadLocalMedia(Directory **mtree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
  loadTree(media_scroll,mtree,dir,x,y,MEDIA_BUT_MARGIN,MEDIA_SHOW_ALL,ext_media,cb_directory,cb_check_directory);
  mediatree=(*mtree);
}

void lighttwistUI::loadLocalPlaylists(Directory **playlisttree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
  loadTree(playlist_scroll,playlisttree,dir,x,y,MEDIA_BUT_MARGIN,MEDIA_SHOW_ALL,ext_media,cb_directory,cb_check_directory);
}

void lighttwistUI::loadImportedMedia(Directory **importedtree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
  loadTree(pl_subdirs_scroll,importedtree,dir,x,y,0,MEDIA_SHOW_SUBDIRS_ONLY,ext_media,cb_directory,cb_check_directory);
  pl_current_dir=(*importedtree);
}

void lighttwistUI::loadTree(Fl_Scroll *scroll,Directory **tree,const char *dir,int x,int y,int margin,unsigned char show_all,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
    char str[PATH_SIZE];
    if (scroll!=NULL)
    {
      scroll->clear();
      scroll->begin();

      dirFreeTree(tree);
      strcpy(str,dir);
      removeSlash(str);

      dirAllocate(tree,NULL,str,scroll->x()+x,scroll->y()+y,show_all,cb_directory,cb_check_directory);
      if ((*tree)!=NULL) 
      {
        //(*tree)->id=0;
        loadExtensions(ext_media);
        loadMediaTree((*tree),(*tree),margin,show_all,cb_directory,cb_check_directory);
        if (show_all) expandMediaTree((*tree));
        else updateMediaTree((*tree));
      }

      scroll->end();
    }
}

void lighttwistUI::loadDefaultValues()
{
  int i;
  char str[DATA_SIZE];

  scan_mode->value(0);

  for (i=0;i<MAX_NB_MACHINES;i++)
  {
    sprintf(str, "%s@192.168.1.%d",username,100+i+1);
    m_ip[i]->value(str);
  }

  for (i=0;i<MAX_NB_SPEAKERS;i++)
  {
    speakers[i]->position(sound_space->x()+10+i*25,sound_space->y()+10);
  }
  subwoofer_index->value(1);

  threading->value(0);
  player_select->value(0);

  loadDefaultExtensions();
}

void lighttwistUI::loadDefaultExtensions()
{
  ext_video->value("avi, mpg, mov");
  ext_audio->value("wav, aif");
  ext_3d->value("osg");
  ext_image->value("png");
  ext_stats->value("stats");
  ext_playlist->value("pll");
}

void lighttwistUI::cb_test(Fl_Widget *w, long index)
{
  unsigned char exec; //if 1: run process; if 0: kill process
  char str[DATA_SIZE];
  lighttwistUI *win;
  int i=(int)(index);

  win=((lighttwistUI *)(w->parent()->parent()->parent()->parent()));

  win->test_pat[0]->selection_color(FL_BLACK);
  win->test_pat[1]->selection_color(FL_BLACK);
  win->test_pat[2]->selection_color(FL_BLACK);

  exec=0;
  if (win->m_i_testing[i]==0)
  //if (1 || win->m_i_testing[i]==0)
  {
    win->m_i_testing[i]=1;
    exec=1;
  }
  else win->m_i_testing[i]=0;
  sprintf(str,"%d",i);

  if (exec) selectMachinesAndExecute(str,start_background);
  else selectMachinesAndExecute(str,stop_background);

  win->redraw();
}

void lighttwistUI::cb_test_all(Fl_Widget *w, long index)
{
  int i;
  unsigned char exec; //if 1: run process; if 0: kill process
  char str[DATA_SIZE];
  lighttwistUI *win;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  win->test_pat[0]->selection_color(FL_BLACK);
  win->test_pat[1]->selection_color(FL_BLACK);
  win->test_pat[2]->selection_color(FL_BLACK);

  if (win->m_all_testing==0)
  //if (1 || win->m_all_testing==0)  
  {
    win->m_all_testing=1;
    exec=1;
    for (i=0;i<MAX_NB_MACHINES;i++) win->m_i_testing[i]=1; 
    win->test_all_but->color(fl_rgb_color(COLOR_DIRECTORY));
  }
  else
  {
    win->m_all_testing=0;
    exec=0;
    for (i=0;i<MAX_NB_MACHINES;i++) win->m_i_testing[i]=0; 
    win->test_all_but->color(FL_GRAY);
  }
  sprintf(str,"%s",langGetLabel(LABEL_CHOICE_ALL));

  if (exec) selectMachinesAndExecute(str,start_background);
  else selectMachinesAndExecute(str,stop_background);

  win->redraw();
}

void lighttwistUI::cb_subdirs_back(Fl_Widget *w, void *data)
{
  lighttwistUI *win;
  char *str;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  if (win->pl_current_dir!=NULL && win->pl_current_dir->parent!=NULL)
  {
    str=win->pl_current_dir->parent->absname;
    cb_imported_directory(NULL,(void *)(str));
  }
}

void lighttwistUI::cb_new_playlist(Fl_Widget *w, void *data)
{
  int i,j;
  char *name;
  char *file;
  int choice;
  int found_media;;

  lighttwistUI *win;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  found_media=0;
  for (i=0;i<NB_DISP_LAYERS;i++)
  {
    for (j=0;j<MAX_NB_MEDIA;j++)
    {
      if (win->timeline->layers[i]->media[j]->active) 
      {  
        found_media=1;         
        break;
      }
    }
  }

  if (found_media)
  {
    choice=fl_choice(langGetLabel(LABEL_NEW_PLAYLIST_SAVE_PREVIOUS),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),langGetLabel(LABEL_UNDO));
    if (choice==1)
    {
      name=win->timeline->playlist_file;
      if (win->timeline->playlist_file[0]=='\0') name=win->mediatree->absname;
      file=fl_file_chooser(langGetLabel(LABEL_PLAYLIST_CHOOSE_FILENAME),NULL,name,Fl_File_Chooser::CREATE);
      if (file!=NULL) 
      {
        strcpy(win->timeline->playlist_file,file);
        win->timeline->savePlaylist(win->mediatree);
      }
    }
    else if (choice!=2)
    {
      win->timeline->resetMedia();
    }
  }
  else win->timeline->resetMedia();
  win->redraw();
}

void lighttwistUI::cb_load_playlist(Fl_Widget *w, void *data)
{
  char *file;
  lighttwistUI *win;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  if (win->mediatree!=NULL)
  {
    file=fl_file_chooser(langGetLabel(LABEL_PLAYLIST_CHOOSE_FILENAME),langGetLabel(LABEL_PLAYLIST_FILES),win->mediatree->absname,Fl_File_Chooser::SINGLE);
    if (file!=NULL) 
    {
      strcpy(win->timeline->playlist_file,file);
      win->timeline->loadPlaylist(win->mediatree);
    }
  }
  win->redraw();
}

void lighttwistUI::cb_save_playlist(Fl_Widget *w, void *data)
{
  char *file;
  char *name;
  lighttwistUI *win;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  name=win->timeline->playlist_file;
  if (win->timeline->playlist_file[0]=='\0') name=win->mediatree->absname;
  file=fl_file_chooser(langGetLabel(LABEL_PLAYLIST_ENTER_FILENAME),NULL,name,Fl_File_Chooser::CREATE);
  if (file!=NULL) 
  {
    strcpy(win->timeline->playlist_file,file);
    win->timeline->savePlaylist(win->mediatree);
  }
  updateLocalPlaylists();
}

void lighttwistUI::cb_media_refresh(Fl_Widget *w, void *data)
{
  updateLocalMedia();
  updateImportedMedia();
  updateLocalPlaylists();
}

void lighttwistUI::cb_media_default(Fl_Widget *w, void *data)
{
  lighttwistUI *win;

  win=((lighttwistUI *)(w->parent()->parent()->parent()));

  win->loadDefaultExtensions();
}

//expects that I is an array of size 3
int lighttwistUI::loadShowImages(imgu **I,const char *path,const char *file_on,const char *file_off,const char *file_disabled)
{
  char file[PATH_SIZE];
  if (I==NULL) return -1;
  if (path==NULL) return -1;
  if (file_on==NULL) return -1;
  if (file_off==NULL) return -1;
  if (file_disabled==NULL) return -1;

  I[0]=NULL;
  I[1]=NULL;
  I[2]=NULL;
  sprintf(file,"%s/%s",path,file_on);
  imguLoad(&I[BUTTON_ON],file,LOAD_16_BITS);
  imguScale(&I[BUTTON_ON],I[BUTTON_ON],SHOW_SCALE_BUT,SHOW_SCALE_BUT);
  sprintf(file,"%s/%s",path,file_off);
  imguLoad(&I[BUTTON_OFF],file,LOAD_16_BITS);
  imguScale(&I[BUTTON_OFF],I[BUTTON_OFF],SHOW_SCALE_BUT,SHOW_SCALE_BUT);
  sprintf(file,"%s/%s",path,file_disabled);
  imguLoad(&I[BUTTON_DISABLED],file,LOAD_16_BITS);
  imguScale(&I[BUTTON_DISABLED],I[BUTTON_DISABLED],SHOW_SCALE_BUT,SHOW_SCALE_BUT);
 
  return 0;
}

void lighttwistUI::updateShowImages()
{
    if (show_setup_flag<=BUTTON_DISABLED) show_setup_but->setImage(show_setup[show_setup_flag]);
    if (show_play_flag<=BUTTON_DISABLED) show_play_but->setImage(show_play[show_play_flag]);
    if (show_sound_flag<=BUTTON_DISABLED) show_sound_but->setImage(show_sound[show_sound_flag]);
    if (show_loop_flag<=BUTTON_DISABLED) show_loop_but->setImage(show_loop[show_loop_flag]);
    if (show_joystick_flag<=BUTTON_DISABLED) show_joystick_but->setImage(show_joystick[show_joystick_flag]);

    redraw();
    Fl::check();  
}

void lighttwistUI::cb_scale(Fl_Widget *w, void*)
{
  lighttwistUI *win=((lighttwistUI *)(w->parent()->parent()->parent()->parent()->parent()));
  win->timeline->setScaleValue(((Fl_Slider *)(w))->value());
}
    

