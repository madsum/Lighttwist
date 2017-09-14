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

#ifndef LT3DLAYER_H
#define LT3DLAYER_H

#include <osg/CameraNode>
#include <osg/Group>
#include <osg/Node>

#include "ltlayer.h"

class OSG_EXPORT LT3DLayer : public LTLayer
{

 public:

  LT3DLayer( const char* shaderFile,
                osg::Node* scene,
                int rttWidth, // of the pre-render texture
                int rttHeight, double vertical_angle_of_view=25.0,unsigned char stereo_mode=STEREO_NONE );

  LT3DLayer( const char* shaderFile,
                osg::Node* scene,
                int rttWidth, // of the pre-render texture
                int rttHeight,
                double thetaStart, double thetaEnd, double vertical_angle_of_view=25.0,unsigned char stereo_mode=STEREO_NONE );



  virtual ~LT3DLayer();

  void setScene( osg::Node* scene );
  void resetCamera();
  void updateViewMatrix( osg::Matrixd& m );

  static osg::Node* generateDefaultScene();

 protected:

  osg::CameraNode *createSubCamera( float thetaStart,
                                    float thetaEnd,
                                    float thetaOffset,
                                    int x0, int y0,
                                    int xs, int ys, int eye);

  osg::ref_ptr<osg::Group> _scene[2]; //for stereo purposes
  osg::ref_ptr<osg::CameraNode> _cam;
  osg::Matrixd  _viewMatrix;

  osg::ref_ptr<osg::Uniform> _viewAngleUF,_nbCamUF;
  double _vertical_angle_of_view;
  unsigned char _stereo_mode;

  int _width,_height; // of the pre-render texture

  private:

};



#endif // LT3DLAYER_H
