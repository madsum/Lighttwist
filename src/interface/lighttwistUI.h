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

#ifndef lighttwistUI_h
#define lighttwistUI_h

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include "FL/Fl_Round_Button.H"
#include "FL/Fl_Gel_Tabs.H"
#include "FL/Fl_Menu_Bar.H"
#include "DnDBox.h"
#include <stdio.h>
#include "media.h"
#include "windowInterface.h"
#include "projectWindow.h"
#include "playlistTimeline.h"
#include "remote/remote.h"

#include "watchCommand.h"

#define MARGIN_LEFT_STEREO 160
#define MARGIN_LEFT_MONO 275

#define SHOW_SCALE_BUT 0.18

#ifdef __cplusplus
extern "C" {
#endif

//these functions are defined in lighttwist.cpp
extern void cb_project(Fl_Widget* w, void* data);
extern void cb_open(Fl_Widget *w, void*);
extern void cb_save(Fl_Widget *w, void*);
extern void cb_exit(Fl_Widget *w, void*);
extern void cb_connect(Fl_Widget *w, long index);
extern void cb_media_ext(Fl_Widget *w, void*);
extern void cb_scan_pattern(Fl_Widget* w, void* data);
extern void cb_scan_grab(Fl_Widget* w, void* data);
extern void cb_scan_uvmap(Fl_Widget* w, void* data);
extern void cb_scan_cam2proj(Fl_Widget* w, void* data);
extern void cb_scan_smooth(Fl_Widget* w, void* data);
extern void cb_scan_gamma(Fl_Widget* w, void* data);
extern void cb_scan_color(Fl_Widget* w, void* data);
extern void cb_scan_blend(Fl_Widget* w, void* data);
extern void cb_scan_uv2proj(Fl_Widget* w, void* data);
extern void cb_scan_calibrate(Fl_Widget* w, void* data);
extern void cb_media_precrop(Fl_Widget* w, void* data);
extern void cb_media_delete(Fl_Widget* w, void* data);
extern void cb_show_pl_editor(Fl_Widget* w, void* data);
extern void cb_imported_directory(Fl_Widget* w, void *absname);
extern void cb_show_setup(Fl_Widget* w, void* data);
extern void cb_show_play(Fl_Widget* w, void* data);
extern void cb_show_mute(Fl_Widget *w, void*);
extern void cb_show_joystick(Fl_Widget *w, void*);
extern void cb_hook_play(void*);

//extern void cb_show_joystick(Fl_Widget *w, void*);
extern void cb_show_live(Fl_Widget *w, void*);

extern int selectMachinesAndExecute(const char *str,int (*operfunc)(int i));

extern void updateLocalMedia();
extern void updateImportedMedia();
extern void updateLocalPlaylists();

extern int activate_remote_run (int i);
extern int start_background(int i);
extern int stop_background(int i);

#ifdef __cplusplus
}
#endif

class lighttwistUI : public Fl_Double_Window  {
public:
  void createUIWindow(char *login,Machine *m);
  void loadDefaultValues();
  void loadDefaultExtensions();
  void loadLocalMedia(Directory **mediatree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));
  void loadLocalPlaylists(Directory **playlisttree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));
  void loadImportedMedia(Directory **importedtree,const char *dir,int x,int y,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));

  lighttwistUI(int argc, char** argv,int x,int y,int w,int h,const char *l)
    : Fl_Double_Window(x, y, w, h, l) {
    int i;
    usertype=USER_ADMIN;
    for(i=1;i<argc;i++) {
        if( strcmp("-mode",argv[i])==0 && i+1<argc ) {
            if (strcmp(USER_ADMIN_STR,argv[i+1])==0) usertype=USER_ADMIN;
            else if (strcmp(USER_ARTIST_STR_EN,argv[i+1])==0 || strcmp(USER_ARTIST_STR_FR,argv[i+1])==0) usertype=USER_ARTIST;
            i++;continue;
        }
    }
    initItemList(threading_items);
    for(i=0;i<MAX_NB_MACHINES;i++) m_i_testing[i]=0;
    m_all_testing=0;
  };

  Fl_Input *m_ip[MAX_NB_MACHINES];
  Fl_Button *m_test[MAX_NB_MACHINES];
  Fl_Choice *scan_mode;
  DnDBox *scan_data;
  DnDBox *sound_space;
  DnDBox *speakers[MAX_NB_SPEAKERS];
  Fl_Box *status;
  Fl_Round_Button *test_pat[3];
  Fl_Box *media_path_label;
  Fl_Box *pl_path_label;
  Fl_Scroll *pl_subdirs_scroll;
  Directory *pl_current_dir;
  Fl_Scroll *media_scroll;
  Fl_Scroll *playlist_scroll;
  playlistTimeline* timeline;
  
  imgu *show_setup[3];
  imgu *show_play[3];
  imgu *show_sound[3];
  imgu *show_loop[3];
  imgu *show_joystick[3];
  unsigned char show_setup_flag;
  unsigned char show_play_flag;
  unsigned char show_sound_flag;
  unsigned char show_loop_flag;
  unsigned char show_joystick_flag;
  int loadShowImages(imgu **I,const char *path,const char *file_on,const char *file_off,const char *file_disabled);
  void updateShowImages();
  void updateScanDisplay();

  Fl_Spinner *subwoofer_index;
  Fl_Input *ext_video;
  Fl_Input *ext_audio;
  Fl_Input *ext_3d;
  Fl_Input *ext_image;
  Fl_Input *ext_stats;
  Fl_Input *ext_playlist;
  Fl_Choice *threading;
  Fl_Choice *player_select;
  DnDBox *media_refresh_but;
  DnDBox *pl_new_but;
  DnDBox *pl_save_but;
  DnDBox *pl_load_but;
  //Fl_Button *show_joystick_but;
  Fl_Button *show_live_but;

  int usertype;
  bool m_IsTimeToPlay;
  // to avoid Fl::wait() call back
  bool m_ignore_cb;
  WatchCommand watcher;

private:
  static void cb_test(Fl_Widget *w, long index);
  static void cb_test_all(Fl_Widget *w, long index);
  static void cb_scan_mode(Fl_Widget *w, void*);
  static void cb_subdirs_back(Fl_Widget *w, void *data);
  static void cb_new_playlist(Fl_Widget *w, void *data);
  static void cb_load_playlist(Fl_Widget *w, void *data);
  static void cb_save_playlist(Fl_Widget *w, void *data);
  static void cb_media_refresh(Fl_Widget *w, void *data);
  static void cb_media_default(Fl_Widget *w, void *data);
  static void cb_scale(Fl_Widget *w, void*);
  void loadTree(Fl_Scroll *scroll,Directory **tree,const char *dir,int x,int y,int margin,unsigned char show_all,const char *ext_media[NB_EXT_TYPES],void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));

  char *username;
  Machine *machines;
  Directory *mediatree;
  unsigned char m_i_testing[MAX_NB_MACHINES]; //flag indicating if the test pattern is being displayed or not
  unsigned char m_all_testing; //flag indicating if the test pattern is being displayed or not

  int LINEPOSY;

  Fl_Scroll *m_scroll;

  Fl_Button *test_all_but;
  Fl_Button *connect_all_but;

  Fl_Button *scan_pattern;
  Fl_Button *scan_grab;
  Fl_Button *scan_select;
  Fl_Button *scan_sum;
  Fl_Button *scan_uvmap;
  Fl_Button *scan_cam2proj;
  Fl_Button *scan_smooth;
  Fl_Button *scan_blend;
  Fl_Button *scan_gamma;
  Fl_Button *scan_color;
  Fl_Button *scan_uv2proj;
  Fl_Button *scan_calibrate;

  Fl_Button *import_but;
  Fl_Button *delete_import_but;
  Fl_Box *media_path_label_header;

  Fl_Box *media_selected_header;
  Fl_Box *media_extensions_header;
  Fl_Button *media_default_but;

  Fl_Group *pl_subdirs;
  Fl_Box *pl_path_label_header;
  Fl_Button *pl_path_back;
  Fl_Scroll *pl_scroll;

  list_item threading_items[MAX_NB_LIST_ITEMS];
  list_item player_items[MAX_NB_LIST_ITEMS];
  list_item scan_mode_items[MAX_NB_LIST_ITEMS];

  DnDBox *show_setup_but;
  DnDBox *show_play_but;
  DnDBox *show_sound_but;
  DnDBox *show_loop_but;
  DnDBox *show_joystick_but;

  Fl_Box *test_pat_header;

  friend class WatchCommand;
};
#endif
