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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define LINE_SIZE 512
#define PATH_SIZE 256
#define CMD_SIZE 768
#define MSG_SIZE 256
#define FILENAME_SIZE 128
#define DATA_SIZE 48

#define MAX_NB_FILES 4096

//function errors
#define IN_RANGE 0
#define SUCCESS 0
#define GEN_ERROR 1
#define NULL_ERROR 2
#define BUF_ERROR 3
#define PARSE_ERROR 4
#define EXTRACT_ERROR 5
#define IO_ERROR 6
#define MEM_ERROR 7
#define NOT_FOUND_ERROR 8
#define OUTSIDE_RANGE 9
#define DUPLICATES_ERROR 10
#define INCOMPATIBLE_ERROR 11

typedef struct pntr{
  char *comment;
  char key[DATA_SIZE];
  char value[PATH_SIZE];
  struct pntr *next;
}Entry;

#ifdef __cplusplus
extern "C" {
#endif

Entry *entryAdd(Entry **list);
int entryUpdate(Entry **list, const char *key, char *value);
int entryCount(Entry *list);
Entry *entryGet(Entry *list, const char *key);
char *entryGetString(Entry *list, const char *key);
int entryGetLine(char *line,char *buf,int bsize,int *offset);
int entryGetInteger(Entry *list, const char *key,int *val);
int entryGetFloat(Entry *list, const char *key,float *val);
int entryRemove(Entry **list,const char *key);
int entryFree(Entry **list);
int entryReadDataFromFile(const char *filename, Entry **entries);
int entryReadDataFromBuffer(char *buffer,int bsize, Entry **entries);
int entryWriteData(const char *filename, Entry *entries);

#ifdef __cplusplus
}
#endif

#endif
