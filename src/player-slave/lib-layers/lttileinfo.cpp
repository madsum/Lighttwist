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

#include "lttileinfo.h"

int test_inside(int cpos,int clen,int bmin,int bmax)
{
  if (cpos<=bmax && cpos+clen-1>=bmin) return 1;
  else return 0;
}

int fill_tiles_tab(const char* movie_filename, int origW, int origH, vector4 lut_bb, tile_info* tiles, int lut_bb_pixels[4] )
{
    int i,nb_tiles;
    FILE *pfp;
    char line[128];
    char cmd[128];
    char movie_data_path[512];
    char *pt_char;

    int tile_inside;
    int len_path;

    //region covered by lut in original movie
    lut_bb_pixels[XMIN]=(int)(lut_bb[XMIN] * (origW-1) + 0.5);
    lut_bb_pixels[XMAX]=(int)(lut_bb[XMAX] * (origW-1) + 0.5);
    lut_bb_pixels[YMIN]=(int)(lut_bb[YMIN] * (origH-1) + 0.5);
    lut_bb_pixels[YMAX]=(int)(lut_bb[YMAX] * (origH-1) + 0.5);

    printf("lut_x_min=%d ; lut_x_max=%d\n",lut_bb_pixels[XMIN],lut_bb_pixels[XMAX]);
    printf("lut_y_min=%d ; lut_y_max=%d\n",lut_bb_pixels[YMIN],lut_bb_pixels[YMAX]);

    strcpy (movie_data_path, movie_filename);
    //remove file extension from filename
    pt_char=strrchr(movie_data_path,'.');
    if (pt_char!=NULL) pt_char[0]='\0';
    len_path = strlen(movie_data_path);

    sprintf(cmd,"ls %s_Tile*",movie_data_path);
    pfp=popen(cmd, "r");
    if (pfp == NULL) return -1;

    i=0;
    while (fgets(line, sizeof(line), pfp)!=NULL && i<MAX_NB_TILES)
    {
      //remove file extension from filename
      strcpy(tiles[i].filename,line);
      pt_char=strrchr(line,'.');
      if (pt_char!=NULL) pt_char[0]='\0';
      sscanf(line+len_path,"_Tile%dx%dx%dx%d",&tiles[i].x,&tiles[i].y,&tiles[i].width,&tiles[i].height);

      tile_inside=0;

      //printf("TEST: %d %d %d %d\n",tiles[i].x,tiles[i].y,tiles[i].width,tiles[i].height);

      if (test_inside(tiles[i].x,tiles[i].width,lut_bb_pixels[XMIN],lut_bb_pixels[XMAX]) &&
          test_inside(tiles[i].y,tiles[i].height,lut_bb_pixels[YMIN],lut_bb_pixels[YMAX]))
      { //tile is overlapping lut
        tile_inside=1;
      }
      else if (lut_bb_pixels[XMAX]>origW-1 &&
          test_inside(tiles[i].x,tiles[i].width,lut_bb_pixels[XMIN]-origW,lut_bb_pixels[XMAX]-origW))
      { //tile is overlapping lut with tile overlaping horizontal boundary
        tile_inside=1;
        tiles[i].x+=origW;
      }
      else if (lut_bb_pixels[YMAX]>origH-1 &&
          test_inside(tiles[i].y,tiles[i].height,lut_bb_pixels[YMIN]-origH,lut_bb_pixels[YMAX]-origH))
      { //tile is overlapping lut with tile overlaping vertical boundary
        tile_inside=1;
        tiles[i].y+=origH;
      }

      if (tile_inside)
      {
        //initialize texture coordinates
        tiles[i].bb_x=0;
        tiles[i].bb_y=0;
        tiles[i].bb_width=tiles[i].width;
        tiles[i].bb_height=tiles[i].height;

        pt_char=strrchr(tiles[i].filename,'\n');
        if (pt_char!=NULL) pt_char[0]='\0';
        i++;
      }
    }
    pclose(pfp);

    if (i==MAX_NB_TILES) i--;

    tiles[i].filename[0]='\0';

    nb_tiles=i;
    for (i=0;i<nb_tiles;i++)
    {
        //update texture coordinates
        if (tiles[i].x-lut_bb_pixels[XMIN]<0) tiles[i].bb_x=lut_bb_pixels[XMIN]-tiles[i].x;
        if (tiles[i].y-lut_bb_pixels[YMIN]<0) tiles[i].bb_y=lut_bb_pixels[YMIN]-tiles[i].y;
        if (tiles[i].x+tiles[i].width-1>lut_bb_pixels[XMAX]) tiles[i].bb_width=lut_bb_pixels[XMAX]-tiles[i].x+1;
        if (tiles[i].y+tiles[i].height-1>lut_bb_pixels[YMAX]) tiles[i].bb_height=lut_bb_pixels[YMAX]-tiles[i].y+1;
        tiles[i].bb_width-=tiles[i].bb_x;
        tiles[i].bb_height-=tiles[i].bb_y;

        printf("TILE %d: %d %d %d %d | TEXTURE: %d %d %d %d (%s)\n",
               i,tiles[i].x,tiles[i].y,tiles[i].width,tiles[i].height,
               tiles[i].bb_x,tiles[i].bb_y,tiles[i].bb_width,tiles[i].bb_height, tiles[i].filename);
    }

    return nb_tiles;
}

int get_tiles_coord(const char *movie_filename,vector4 lut_bb,tile_info* tiles, int lut_bb_pixels[4])
{
    int ret;
    int origW,origH;
    char movie_without_suffix[500];
    char stats_filename[500];
    const char *pt_char;
    int pt_char_no;
    Entry *config;

    if (tiles==NULL) return -1;

    origW=0;
    origH=0;
    config=NULL;

    //get stats
    pt_char=strrchr(movie_filename,'.');
    pt_char_no = pt_char - movie_filename;
    strncpy (movie_without_suffix, movie_filename, pt_char_no);
    movie_without_suffix[pt_char_no]='\0';
    sprintf(stats_filename,"%s.stats",movie_without_suffix);
    printf("stats_filename = %s\n",stats_filename);

    if (entryReadDataFromFile(stats_filename,&config))
    {
      fprintf(stderr,"Error in opening stats file '%s'\n",stats_filename);return 0;
    }
    ret=entryGetInteger(config,STATS_WIDTH,&origW);
    ret=entryGetInteger(config,STATS_HEIGHT,&origH);
    printf("origW= %i  ;  origH= %i\n",origW,origH);

    entryFree(&config);

    return fill_tiles_tab(movie_filename, origW, origH, lut_bb,tiles, lut_bb_pixels);
}


