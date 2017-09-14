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
 * graycode.h  -  Library for the calculation of the Gray code.
 */
 
#ifndef LTGRAYCODE_H
#define LTGRAYCODE_H

#ifdef __cplusplus
extern "C" {
#endif

int encode(int n);
int decode(int g, int nb);
int getnbbits(int w);
int getpowerof2(int e);

//////////////////////////////////////////
// shuffle of data....
// 0,1,2,3,4,5   n=6   ==>   0,5,2,3,4,1
// 0,1,2,3,4     n=5   ==>   0,3,2,1,4
int shuffle(int i,int n);

//int get_color(int position, int pattern_id, int nb_bits);
int get_position(int r, int nb_bits);

int decode_table_x(int g,int nb);
int decode_table_y(int g,int nb);
void init_gray_tables (int nb_x,int nb_y);

#ifdef __cplusplus
}
#endif

#endif


