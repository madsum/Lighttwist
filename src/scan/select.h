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
 * ltselect.h  -  Main library for the images selection.
 */ 

#ifndef LTSELECT_H
#define LTSELECT_H

#define VERSION	" V.04.09.13"
#define MODULE "ltselect"

#define CONFIDENCE_MINIMUM 50

// 40 patterns
#define NB_PATTERNS_DEFAULT 40

// max # of images in the sequence
#define MAX_IMG	10000

#include "utils/ltsys.h"

int main_ltselect(imgu **T_param,char *out_prefix,int nb_img,int cam,int scale,float contrast_factor,int nb_patterns);

#endif
