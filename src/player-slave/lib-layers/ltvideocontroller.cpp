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

#include "ltvideocontroller.h"

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

LTVideoController::LTVideoController() {
  _nbVids = 0;
  _nextFrame = 0;
  for (int i = 0; i < MAX_NB_TILES; i++) _vids[i] = NULL;
}

/*****************************************************************************/

LTVideoController::~LTVideoController() {
  for (int i = 0; i < MAX_NB_TILES; i++)
  {
    if (_vids[i] != NULL)
    {
      //delete _vids[i];
      _vids[i] = NULL;
    }
  }
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/


bool LTVideoController::addVideoTexture( const char* path ) {



  return true;
}


/*****************************************************************************/

bool LTVideoController::addVideoTexture( LTVideoTexture2D* vt ) {

  if (!vt) return false;

  _vids[_nbVids] = vt;
  _nbVids++;

  return true;
}

/*****************************************************************************/

bool LTVideoController::go( int64_t f ) {

  bool err = false;

  //printf("\nLTVideoController::go(%Ld) **************************************\n", f);

  for (int i = 0; i < _nbVids; i++) {
    if (!_vids[i]) {
      err = true;
      continue;
    }

    err = _vids[i]->go(f);
  }
  return err;
}

/*****************************************************************************/

double LTVideoController::getFPS() {

  for (int i = 0; i < _nbVids; i++) {
    if (_vids[i]) return _vids[i]->getFPS();
  }
  
  return 0.0;    
}


/*****************************************************************************/

