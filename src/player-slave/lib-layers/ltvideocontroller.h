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

#ifndef LTVIDEOCONTROLLER_H
#define LTVIDEOCONTROLLER_H

#include "ltvideotexture.h"
#include <precrop/PrecropMain.h> // for MAX_NB_TILES

class LTVideoController {

 public:
  LTVideoController();
  virtual ~LTVideoController();

  virtual bool addVideoTexture( const char* path );
  virtual bool addVideoTexture( LTVideoTexture2D* vt );
  virtual void setNextFrame( int64_t f ) { _nextFrame = f; }
  virtual bool go() { return this->go(_nextFrame); }
  virtual bool go( int64_t f );
  virtual double getFPS();

 protected:
  LTVideoTexture2D* _vids[MAX_NB_TILES];
  int _nbVids;
  int64_t _nextFrame;
};

#endif // LTVIDEOCONTROLLER_H
