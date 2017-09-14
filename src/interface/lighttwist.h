
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

#ifndef LIGHTTWIST_H
#define LIGHTTWIST_H

#include <FL/fl_ask.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Window.H>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <bmc/bimulticast.h>
#include <utils/pd_send.h>
#include "lighttwistUI.h"
#include "listitem.h"
#include "utils/uvmap.h"
#include <utils/graycode.h>
#include <imgu/imgu.h>
#include "media.h"
#include <utils/entry.h>
#include <utils/gamma.h>
#include "blendWindow.h"
#include "cam2projWindow.h"
#include "uv2projWindow.h"
#include "colorWindow.h"
#include "uvmapWindow.h"
#include "shellWindow.h"
#include "gammaWindow.h"
#include "captureWindow.h"
#include "liveWindow.h"
#include "patternWindow.h"
#include "playWindow.h"
#include "precropWindow.h"
#include "projectWindow.h"
#include "smoothWindow.h"

#include "remote/ltports.h"

//#include "watchCommand.h"
//#include "my_log.h"
#ifdef __cplusplus
extern "C" {
#endif

void updateUVMapWindow();
void updateMachineConfiguration();
void updateSpeakerConfiguration();
void cb_connect(Fl_Widget *w, long index);
void cb_media_directory(Fl_Widget* w, void *absname);
void cb_media_check_directory(Fl_Widget* w, void *absname);
void cb_playlist_directory(Fl_Widget* w, void *absname);
void cb_playlist_check_directory(Fl_Widget* w, void *absname);
void cb_imported_directory(Fl_Widget* w, void *absname);
void cb_imported_check_directory(Fl_Widget* w, void *absname);

int fioCreateDirectories(const char *path,unsigned char remote,int i);

int updateDataImage(const char *filename);
void cursorWait();
void cursorDefault();

int do_stats_delete (const char *name,int type);
int do_sum(int channel);
int do_grab_camera(int m_index,const char *name,const char *outdir,int nbimgs,int color,int sync,int all);
int do_uvmap();

int stop_background(int i);
int start_background(int i);
int stop_jack();
int start_jack();
int stop_joystick_ctrl();
int start_joystick_ctrl();
int start_player_slave(int i);
int stop_player_slave(int i);
int start_player_master();
int stop_player_master();
int stop_playlist();

#define PROJECT_KEY "PROJECT"
#define EXT_VIDEO_KEY "EXT_VIDEO"
#define EXT_AUDIO_KEY "EXT_AUDIO"
#define EXT_3D_KEY "EXT_3D"
#define EXT_IMAGE_KEY "EXT_IMAGE"
#define EXT_PLAYLIST_KEY "EXT_PLAYLIST"
#define PLAYER_THREADING "PLAYER_THREADING"
#define PLAYER_SELECT "PLAYER_SELECT"
#define config_filename "lighttwist.cfg"

#ifdef __cplusplus
}
#endif

#endif

