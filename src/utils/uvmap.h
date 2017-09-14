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
 * uvmap.h  -  This program will test if a pixel lies inside or outside
 *                the cropped area. int dual_circle_crop() return 0 if the 
 *                pixel is outside, 1 if it's inside.
 *                Typically called by ltrecon.                
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <imgu/imgu.h>
#include "triangle.h"

#ifndef LT_UVMAP_H
#define LT_UVMAP_H

#define MAX_NB_UV_CTRLS 4
#define NB_UV_COORD_MAX 2*MAX_NB_UV_CTRLS
//#define UV_NONE -1

#define UV_RECTANGLE_STR "RECTANGLE"
#define UV_HOMOGRAPHY_STR "HOMOGRAPHY"
#define UV_DUAL_CIRCLES_STR "DUAL_CIRCLES"
#define UV_BEZIER_STR "BEZIER"

#define UV_RECTANGLE 0
#define UV_HOMOGRAPHY 1
#define UV_DUAL_CIRCLES 2
#define UV_BEZIER 3

#define BEZIER_NORMALIZE

#define NB_TRIANGLES 64

typedef struct
{
	  int x,y;
} uv_coord_struct;

#ifdef __cplusplus
extern "C" {
#endif

void uvGetCoords(uv_coord_struct *coord);
void uvSetCoords(uv_coord_struct *coord);
void uvGetScales(double *inner, double *outer);
void uvSetScales(double inner, double outer);
void uvGetVCenter(double *center);
void uvSetVCenter(double center);
void uvGetType(int *type);
void uvSetType(int type);
void uvGetUVMap(imgu **uvmap);

void uvEllipseCenter( float x1,float y1, float x2,float y2, float x3,float y3,float *xc,float *yc,float *r);
int uvCenter(double xy[2]);

int uvReadFile(char *crop_filename);
int uvSaveFile(char *crop_filename);
int uvLoadImg(char *uvmap_filename);

int uvCrop(double x,double y,double uv[2]);
int uvCropInverse(double u,double v,double xy[2]);
int uvCheck(double x, double y);
int uvCropImg(imgu *img);
int uvComputeImg(char *uvmap_filename,int resx,int resy);
//interpolation in the uv img
int uvImgClosest(double x,double y,double uv[2]);
double uvImgBilinear(double x,double y,double uv[2]);

#ifdef __cplusplus
}
#endif

#endif
