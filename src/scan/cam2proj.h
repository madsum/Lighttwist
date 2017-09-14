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
 * cam2proj.h  -  Main library for ltrecon. 
 *               This file contains all the default values used to run ltrecon.
 * 
 */

#ifndef LTCAM2PROJ_H
#define LTCAM2PROJ_H

#include <imgu/imgu.h>

#define MODULE "LTCAM2PROJ"
#define INVALID -99999

// Types of resolution
#define ORIG 0
#define SMOOTH 1
#define MARKOV_SIMPLE 2
#define MARKOV_CONT 3
#define MARKOV_JP 4


// main call
int main_ltcam2proj(imgu **Tselect_param, imgu *Trange_param, int img_size_x, int img_size_y, char *out_prefix, int type, float mrf_beta_param, int cam, int imap_xs, int imap_ys, int smooth_ws, int confidence_min, int range_min, char *uv_filename, char *uvimg_filename, int proj_res_x, int proj_res_y,int nb_pattern,int xy_threshold);


// ******************************************************
// **************
// Default Values
// **************

//Bout de code qui ne sont pas vraiment selon Markov, mais qui ne devrait pas changer tant que ca la solution
#define OPTI_MRF 

#define MRF_COOL_DOWN 1.0

#define HOMO_TO_CODE

//#define MRF_VOTE
#define MRF_BETA 0.5
//float MRF_BETA = 10000.;

#define PROJ_RES_X_DEFAULT 1024
#define PROJ_RES_Y_DEFAULT 768
#define PROJ_CODE 1024

// To skip some unuseful operations and the saves
// pour sauter les operations inutiles et les saves
//#define QUICK

// Sub-pixel shift (in number of bits) for [xy]code and [xy]conf.
// shift sous-pixel (en nb de bits) pour [xy]code et [xy]conf. Normalement 3.
#define SUBPIX	0

// 4 bits of sub-pixel for the mean
// 4 bits de sous-pixel pour le moyennage
#define MOYSUBPIX	0

//#define VERBOSE_FIX
//#define VERBOSE_RECOVER

// symetric mean (preserve image borders)
// moyennage symetrique (preserve les bords de l'image)
#define SYMETRIC

// neighborhood will be -MWS..MWS in X and Y
#define SMOOTH_WINDOW_SIZE_DEFAULT	2


// validity threshold
// On compte le nombre de bits reconstruits correctement.
// le VALIDITY est le nombre minimum de bits qui doivent etre valides...
// Ca prend 10 bits fiables sur 20 pour etre valide
#define VALIDITY_THRESHOLD     8

// pour le fichier triangles osg
// a la fin, on est plus rigide, apres le lissage
//#define OUTPUT_VALIDITY_THRESHOLD	5
#define OUTPUT_VALIDITY_THRESHOLD	8

// validity threshold for inverse_mapping
//#define INVERSE_MAP_VALIDITY_THRESHOLD	4
// you must pick low enough to have a dense enough inversion
// too low -> errors.
#define INVERSE_MAP_VALIDITY_THRESHOLD	8

// Minimum confidence in % of the range of the pixel
// Normally = 50%, but if the intensity is low, lower this value a bit 
// en % du range du pixel...
// normalement 50%, mais si l'intensite est faible, on baisse un peu...
#define CONFIDENCE_MINIMUM_DEFAULT 50

// This value must be adapted to the exposure time.
// It's equal to the black image noise level X2 (normally 15 or 31)
// ce parametre doit etre adapte au temps d'exposition....
// C'est le niveau de bruit dans une image noire, X2
// normalement 15 ou 31
#define RANGE_MIN_DEFAULT 61

#define XY_THRESHOLD_DEFAULT 75

// Open Scene Graph (OSG) constants
#define OSG_TEXTURED 1 // Type of OSG created:  0=not textured  1=textured
#define TRIANGLE_SIZE 5 // Triangle size for the OSG mapping


// ** Inverse map settings
#define IMAP_XS_DEFAULT 1024
#define IMAP_YS_DEFAULT 1024
// **

#define C2P_DEBUG 0

//#define INVERSE_Y

#endif
