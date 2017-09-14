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

#ifndef LT_GAMMA
#define LT_GAMMA

#define DEFAULT_GAMMA_R 1.0f
#define DEFAULT_GAMMA_G 1.0f
#define DEFAULT_GAMMA_B 1.0f

#include <stdio.h>
#include <string.h>
#include <imgu/matrixmath.h>

#ifdef __cplusplus
extern "C" {
#endif

void read_gamma_and_color(char *path,int mindex,vector3 gamma,matrix3 color);
int read_gamma_and_max(double *gamma,double *max, char *filename);
int write_gamma_and_max(double *gamma,double *max, char *filename);

#ifdef __cplusplus 
}
#endif

#endif
