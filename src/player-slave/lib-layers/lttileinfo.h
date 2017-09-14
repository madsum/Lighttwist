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

#ifndef LTLAYERTILES_H
#define LTLAYERTILES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <interface/windowInterface.h>
#include <utils/entry.h>

#include <precrop/PrecropMain.h>
#include <imgu/imgu.h>

typedef struct
{
  char filename[128];
  int x,y,width,height; //position and width of tile in original movie
  int bb_x,bb_y,bb_width,bb_height;  //position and width of used tile (intersection with bounding box of lut) w.r.t to its position
} tile_info;


#define XMIN 0
#define XMAX 1
#define YMIN 2
#define YMAX 3

int get_tiles_coord(const char *movie_filename,vector4 lut_bb,tile_info* tiles, int lut_bb_pixels[4]);

#endif // LTLAYERTILES_H

