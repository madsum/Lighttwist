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

#ifndef LTMULTI3DLAYER_H
#define LTMULTI3DLAYER_H

#include <osg/Node>
#include <osg/Group>
#include <vector>
#include "lt3dlayer.h"
#include "player-master/default/playlist_parse.h"

class OSG_EXPORT LTMulti3DLayer : public LT3DLayer {


  public:

    LTMulti3DLayer( const char* shaderFile, osg::Node* scene, int width, int height, double vertical_angle_of_view=25.0,unsigned char stereo_mode=STEREO_NONE );
    ~LTMulti3DLayer();

    void resetCamera();
    void updateViewMatrix( osg::Matrixd& m );
    void setScene( osg::Node* scene );

    void setSceneShaders( char* vertPath, char* fragPath,double B=STEREO_B_DEFAULT,double R=STEREO_R_DEFAULT,double distortion=STEREO_DISTORTION_DEFAULT);
    void updateStereoConfiguration( double B,double R,double distortion);
    void updateEyeSelection( int vtoggle);
    void enableLighting();
    void disableLighting();

  protected:

  private:
    std::vector< osg::ref_ptr<osg::CameraNode> > _sceneCams;
    std::vector<osg::Matrixd> _viewMatrices;
    std::vector<double> _view_orientation;

    osg::ref_ptr<osg::Uniform> _B[2],_R,_distortion;
    osg::ref_ptr<osg::Uniform> _lighting;
    double _sign[2];


}; // class LTMulti3DLayer

#endif // LTMULTI3DLAYER_H
