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

#include "PrecropMain.h"

PrecropMain::PrecropMain(char *movie_in_name,char *path_out,char *movie_out_name,int movie_in_xs, int movie_in_ys,int tile_width,int tile_height,float fps)
{
  int i,finished;
  int nb_tiles;
  // init loop
  nb_tiles_x = 0;
  for(tile_xmin=0; tile_xmin < movie_in_xs; tile_xmin += tile_width)
  {
    nb_tiles_x++;
  }
  nb_tiles_y = 0;
  for(tile_ymin=0; tile_ymin < movie_in_ys; tile_ymin += tile_height)
  {
    nb_tiles_y++;
  }

  nb_tiles=nb_tiles_x*nb_tiles_y;
  if (nb_tiles>MAX_NB_TILES)
  {
    LT_PRINTF("Warning: number of tiles (%d) is larger than %d!\n",nb_tiles,MAX_NB_TILES);
  }

  tile_no = 0;
  for(tile_ymin=0; tile_ymin < movie_in_ys; tile_ymin += tile_height)
  {
    for(tile_xmin=0; tile_xmin < movie_in_xs; tile_xmin += tile_width)
    {
      LT_PRINTF("\n\n**** Processing Tile #%i ****\n\n",tile_no);
      precrop_finished[tile_no]=0;

      // find the box
      if(tile_xmin + tile_width < movie_in_xs) box_width = tile_width;
      else box_width = movie_in_xs - tile_xmin;
      if(tile_ymin + tile_height < movie_in_ys) box_height = tile_height;
      else box_height = movie_in_ys - tile_ymin;

      // encode the frames in a movie
      
      if(path_out[strlen(path_out)-1]=='/')sprintf(movie_out_name_full,"%s%s_Tile%05ix%05ix%05ix%05i.avi",path_out,movie_out_name, tile_xmin, tile_ymin, box_width,box_height);
      else sprintf(movie_out_name_full,"%s/%s_Tile%05ix%05ix%05ix%05i.avi",path_out,movie_out_name, tile_xmin, tile_ymin, box_width,box_height);
      //normal msg level for tile_no==0, no msgs otherwise
      sprintf(cmd,"ffmpeg -i %s -an -r %f -cropleft %d -croptop %d -cropright %d -cropbottom %d -vcodec mjpeg -y -sameq %s",movie_in_name,fps,tile_xmin,tile_ymin,movie_in_xs-tile_xmin-box_width,movie_in_ys-tile_ymin-box_height,movie_out_name_full);
      LT_PRINTF("ltprecrop_tiles cmd: %s\n", cmd);
      new PrecropThread(tile_no,cmd,(ThreadInterface *)(this));
        
      // next tile
      tile_no++;
      if (tile_no>=MAX_NB_TILES)
      { //this is done to break out of loop
        tile_xmin=movie_in_xs;
        tile_ymin=movie_in_ys;
      } 
    }
  }

  while(1)
  {
    sleep(1);
    finished=1;
    for (i=0;i<nb_tiles;i++)
    {
      finished&=precrop_finished[i];
    }
    if (finished) break;
  }
}

void PrecropMain::ThreadCallback(int index,int status)
{ 
  LT_PRINTF("FINISHED TILE %d\n",index);
  precrop_finished[index]=1;
}



