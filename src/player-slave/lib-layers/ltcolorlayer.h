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

#ifndef LTCOLORLAYER_H
#define LTCOLORLAYER_H

#include <osg/CameraNode>
#include <osg/Group>
#include <osg/Node>

#include "ltlayer.h"

class OSG_EXPORT LTColorLayer : public LTLayer
{

 public:

    LTColorLayer( float r, float g, float b, float a, int mode = 0 );

  virtual ~LTColorLayer();

  void setColor( float r, float g, float b, float a = 1.0f );
  void setColor( osg::Vec4& col );
  void setColor( osg::Vec3& col );
  osg::Vec4 getColor() { return _color; }

 protected:

  osg::Vec4 _color;
  osg::Uniform* _colorUF;

 private:

};

#endif // LTCOLORLAYER_H
