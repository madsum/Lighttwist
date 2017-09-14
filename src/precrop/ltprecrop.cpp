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

/*
 * precropTiles.c  -  Program for precropping movies in tiles. 
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <imgu/imgu.h>
#include <utils/ltsys.h>
#include <utils/entry.h>

#include "PrecropMain.h"

#define BUF_SIZE 255
#define VERSION	"PRECROP_TILES - V.01"

void help(char* prog)
{
  printf("\nUsage: %s [OPTIONS]\n\n", prog);
  printf("Options: \n");
  printf("   -i filename, --in_movie filename\n");
  printf("         Specify the filename of the full original movie.\n\n");
  printf("   -o filename, --out_movie filename\n");
  printf("         Specify the filename of the croped movie.\n\n");
  printf("   -w int, --width int\n");
  printf("         Specify the width of the original movie.\n\n");
  printf("   -h int, --height int\n");
  printf("         Specify the height of the original movie.\n\n");
  printf("   -f float, --fps float\n"); 
  printf("         Specify the fps of the croped movie.\n\n");
  printf("   -tw int, --tile_width int\n");
  printf("         Specify the width (in pixels) of each tile.\n\n");
  printf("   -th int, --tile_height int\n");
  printf("         Specify the height (in pixels) of each tile.\n\n");
  printf("   -V, --version\n");
  printf("         Show the version of the program.\n\n");
  printf("   --help\n");
  printf("         Show the help screen.\n\n");
}


int main(int argc, char **argv) 
{
  int i;
  char movie_in_name[PATH_SIZE];
  char movie_out_name[PATH_SIZE];
  char path_out[PATH_SIZE];
  int movie_in_xs, movie_in_ys;
  float fps;
  int tile_width,tile_height;
  PrecropMain *Main;

  // reset variables
  path_out[0]='\0';
  movie_in_name[0]='\0';
  movie_out_name[0]='\0';
  movie_in_xs = 0;
  movie_in_ys = 0;
  fps = 29.97;
  tile_width = 0;

  //Option parameters
  for(i=1;i<argc;i++) 
  {
    if( (strcmp(argv[i],"-i")==0 || strcmp(argv[i],"--movie_in")==0) && i+1<argc ) 
    {
      strcpy(movie_in_name,argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-o")==0 || strcmp(argv[i],"--movie_out")==0) && i+1<argc ) 
    {
      strcpy(movie_out_name,argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-opath")==0 || strcmp(argv[i],"--path_out")==0) && i+1<argc ) 
    {
      strcpy(path_out,argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-w")==0 || strcmp(argv[i],"--width")==0) && i+1<argc ) 
    {
      movie_in_xs = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--height")==0) && i+1<argc ) 
    {
      movie_in_ys = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-f")==0 || strcmp(argv[i],"--fps")==0) && i+1<argc ) 
    {
      fps = atof(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-tw")==0 || strcmp(argv[i],"--tile_width")==0) && i+1<argc ) 
    {
      tile_width = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-th")==0 || strcmp(argv[i],"--tile_height")==0) && i+1<argc ) 
    {
      tile_height = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0 ) 
    {
      printf("%s\n\n",VERSION);
      exit(0);
    }
    if( strcmp(argv[i],"--help")==0 ) 
    {
      help(argv[0]);
      exit(0);
    }
  }

  if(tile_width == 0)
    tile_width = movie_in_xs;


  Main=new PrecropMain(movie_in_name,path_out,movie_out_name,movie_in_xs,movie_in_ys,tile_width,tile_height,fps);

  LT_EXIT(0);
}


