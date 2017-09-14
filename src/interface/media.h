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

#ifndef LT_MEDIA_H
#define LT_MEDIA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "FL/fl_draw.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Check_Button.H"
#include <utils/entry.h>
#include "windowInterface.h"

#define MEDIA_CHECK_WIDTH 20
#define MEDIA_BUT_WIDTH 1000
#define MEDIA_BUT_HEIGHT 20
#define MEDIA_BUT_MARGIN 25

#define MAX_NB_EXT 256
#define NB_EXT_TYPES 6
#define EXT_UNKNOWN -1
#define EXT_VIDEO 0
#define EXT_AUDIO 1
#define EXT_3D 2
#define EXT_IMAGE 3
#define EXT_STATS 4
#define EXT_PLAYLIST 5

#define COLOR_DIRECTORY 100,134,175

#define MEDIA_SHOW_SUBDIRS_ONLY 0
#define MEDIA_SHOW_ALL 1

#define EXT_DELIMITERS "\t,. "

typedef struct file_next{
  char absname[PATH_SIZE];
  char relname[PATH_SIZE];
  char name[FILENAME_SIZE];
  //int id;
  Fl_Check_Button *check;
  Fl_Box *label;
  unsigned short type;
  file_next *next;
}File;

typedef struct dir_next{
  char absname[PATH_SIZE];
  char name[FILENAME_SIZE];
  //int id;
  int expanded;
  unsigned char show_all;
  Fl_Check_Button *check;
  Fl_Button *but;
  File *files;
  dir_next *subdirs;
  dir_next *next;
  dir_next *parent;
  dir_next *root;
}Directory;

#ifdef __cplusplus
extern "C" {
#endif

#define DIR_HIDE 0
#define DIR_SHOW 1

#define FILE_HIDE 0
#define FILE_SHOW 1

#define DIR_NOT_EXPANDED 0
#define DIR_EXPANDED 1

#define DIR_UNCHECK 0
#define DIR_CHECK 1

int stripFromChar(char *str,char s);
char *startFromChar(const char *str,char s);
char *findRelativePath(const char *root,char *absname);
char *findFilename(char *name);

int dirAllocate(Directory **dir,Directory *root,char *name,int x,int y,int show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));
Directory *dirAddToList(Directory **head,Directory *root,char *name,int x,int y,unsigned char show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));
File *dirAddFile(Directory *head,char *fullname,unsigned char type,int x,int y);
int dirCount(Directory *d);
int dirToggleVisibility(Directory *dir,unsigned char visible);
void dirFree(Directory **dir);
void dirFreeTree(Directory **dir);
void dirFreeList(Directory **dir);
Directory *dirSuccessor(Directory *dir);

int fileAllocate(File **f,char *fullname,char *rootpath,unsigned char type,int x,int y);
File *fileAddToList(File **head,char *fullname,char *rootpath,unsigned char type,int x,int y);
int fileCount(File *f);
int fileToggleVisibility(File *f,unsigned char visible);
void fileFree(File **f);
void fileFreeList(File **f);

int loadExtensions(const char *ext_media[NB_EXT_TYPES]);
int loadMediaTree(Directory *subtree,Directory *root,int margin,unsigned char show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *));
int expandMediaTree(Directory *tree);
//Directory *findDirectoryFromID(Directory *tree,long id);
Directory *findSubDirectoryFromName(Directory *tree,char *name);
Directory *findDirectoryFromAbsname(Directory *tree,char *absname);
File *findFile(Directory *tree,char *fullname,unsigned char absolute);
int selectMediaTree(Directory *tree,int select);
int updateMediaTree(Directory *tree);
int identifyImportedMedia(Directory *media,Directory *imported);

int addSlash(char *arg);
int removeSlash(char *arg);

#ifdef __cplusplus
}
#endif

#endif

