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

#ifndef LTTILEDVIDEOLAYER_H
#define LTTILEDVIDEOLAYER_H

#include <osg/Node>

#include "lt3dlayer.h"
#include "ltvideocontroller.h"
#include "lttileinfo.h"


class OSG_EXPORT LTTiledVideoLayer : public LT3DLayer, public LTVideoController {

  public:

    LTTiledVideoLayer( const char* fileName,
                       const char* shaderFile,
                       int rttWidth, int rttHeight );

    ~LTTiledVideoLayer();

  protected:

};

#endif // LTTILEDVIDEOLAYER_H
