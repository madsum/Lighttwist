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

#ifndef LT_PRECROP_MAIN_H
#define LT_PRECROP_MAIN_H

#include "utils/ThreadInterface.h"
#include "PrecropThread.h"

#include <stdio.h>
#include <unistd.h>     //header file for sleep()

#include <utils/ltsys.h>
#include <utils/entry.h>

#define MAX_NB_TILES 1024

class PrecropMain: public ThreadInterface
{
private:
  char movie_out_name_full[PATH_SIZE];
  int tile_xmin, tile_xmax, tile_ymin, tile_ymax, tile_no, nb_tiles_x,nb_tiles_y;
  int box_width, box_height;
  char cmd[CMD_SIZE];
  unsigned char precrop_finished[MAX_NB_TILES];

public:
  PrecropMain(char *movie_in_name,char *path_out,char *movie_out_name,int movie_in_xs, int movie_in_ys,int tile_width,int tile_height,float fps);
  void ThreadCallback(int index,int status);
};

#endif


