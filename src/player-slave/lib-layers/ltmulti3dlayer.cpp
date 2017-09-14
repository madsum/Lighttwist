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

#include "ltmulti3dlayer.h"

#define MULTI_CAMERA_SPAN_MAX (M_PI / 4.0)

/*****************************************************************************/

int nextPow2(int val)
{
  int pow = 1;
  while(pow<val) pow*=2;
  return pow;
}


/*****************************************************************************/

LTMulti3DLayer::LTMulti3DLayer( const char* shaderFile,
                                 osg::Node* scene,
                                 int width, int height, double vertical_angle_of_view, unsigned char stereo_mode )
  : LT3DLayer( shaderFile, scene, width, height, vertical_angle_of_view,stereo_mode )
{
  double t_start,t_end;

  printf("LTMulti3DLayer ctor...\n");
  setName( "LTMulti3DLayer" );

  double thetaStart = _xLutMin * 2.0 * M_PI;
  double thetaEnd = _xLutMax * 2.0 * M_PI;

  //thetaStart = 0;
  //thetaEnd = 2.0 * M_PI;


  printf("LTMulti3DLayer: thetaStart = %f; thetaEnd = %f; MULTI_CAMERA_SPAN_MAX = %f\n",
         thetaStart, thetaEnd, MULTI_CAMERA_SPAN_MAX );

  if ( thetaEnd < thetaStart ) thetaEnd += 2 * M_PI;

  int nbCams = 1;
  double thetaIncr = thetaEnd - thetaStart;
  double widthIncr = _width;

  //multiple cameras creates culling problems with stereo vertex shader
  //a vertex not originally visible is culled, but the shader will make it visible!
  /*if ( thetaIncr > MULTI_CAMERA_SPAN_MAX ) {
    nbCams = ceil( thetaIncr / MULTI_CAMERA_SPAN_MAX );
    nbCams = nextPow2( nbCams );
    thetaIncr = thetaIncr / (double) nbCams;
    widthIncr = widthIncr / (double) nbCams;
  }*/

  printf("LTMulti3DLayer: nb of 3d cams = %i; thetaIncr = %f; widthIncr = %f\n",
         nbCams, thetaIncr, widthIncr);

  osg::CameraNode* cam;

  _sign[0]=-1.0;
  _sign[1]=1.0;
  if (_stereo_mode==STEREO_P_RIGHT) _sign[0]=1.0;

  for (int eye=0;eye<2;eye++)
  {
    if (eye && _stereo_mode!=STEREO_RC) break;
    for ( int i = 0; i < nbCams; i++) {

      printf("LTMulti3DLayer: new 3dlayer theta = %f to %f; width = %f to %f\n",
           thetaStart + i*thetaIncr, thetaStart + (i+1)*thetaIncr, i*widthIncr, (i+1)*widthIncr);

      //printf("ORIENTATION: %f\n",M_PI+ ( t_start + t_end ) / 2.0);

      t_start=thetaStart + i*thetaIncr;
      t_end=thetaStart + (i+1)*thetaIncr;
      cam = createSubCamera(t_start, t_end, M_PI,
                          i*widthIncr, 0, widthIncr, _height,eye );
      _sceneCams.push_back( cam );

      _viewMatrices.push_back( cam->getViewMatrix() );
      _view_orientation.push_back( M_PI+ ( t_start + t_end ) / 2.0 );
      this->addChild( cam );
    }
  }

  _cam->removeChild( _scene[0].get() );
  _cam->removeChild( _scene[1].get() );
  this->removeChild( _cam.get() ); // not needed anymore, replaced by _sceneCams vector

  osg::StateSet* ss = _hud->getOrCreateStateSet();
  osg::Uniform* nbCamUF = ss->getUniform("nbCams");
  nbCamUF->set( (float) nbCams );

  printf("LTMulti3DLayer ctor done\n");
}

/*****************************************************************************/

LTMulti3DLayer::~LTMulti3DLayer() {
  _sceneCams.clear();
  _viewMatrices.clear();
}

void LTMulti3DLayer::setSceneShaders( char* vertPath, char* fragPath, double B,double R,double distortion )
{
  size_t i;

  for (int eye=0;eye<2;eye++)
  {
    if (eye && _stereo_mode!=STEREO_RC) break;

    _B[eye]=new osg::Uniform("B", (float) (B*_sign[eye]));
    if (eye==0)
    {
      _R=new osg::Uniform("R", (float) R);
      _distortion=new osg::Uniform("distortion", (float) distortion);
      _lighting = new osg::Uniform("lighting", 1 );
    }

    i=0;
    if (eye==1) i=_sceneCams.size()/2;
    for ( ; i < _sceneCams.size(); i++ )
    {
      //printf("setSceneShaders ...(%d)\n",i);
      osg::StateSet* sss = _sceneCams[i]->getOrCreateStateSet();
      osg::Program* shaders = new osg::Program;
      osg::Shader* vObj = new osg::Shader( osg::Shader::VERTEX );
      shaders->addShader( vObj );
      //printf( "loading v shader...(%d)\n",i );
      vObj->loadShaderSourceFromFile( vertPath );

      sss->addUniform( _B[eye].get() );
      sss->addUniform( _R.get() );
      sss->addUniform( _distortion.get() );

      sss->addUniform( _lighting.get() );

      sss->addUniform( new osg::Uniform("camangle", (float) _view_orientation[i] ) );

      sss->setAttributeAndModes( shaders, osg::StateAttribute::ON );
    }
  }
}

void LTMulti3DLayer::updateStereoConfiguration( double B,double R,double distortion)
{
  if (_stereo_mode==STEREO_NONE) return;

  //printf("UPDATING STEREO: %f %f %f\n",B,R,distortion);

  _B[0]->set((float)(B*_sign[0]));
  if (_stereo_mode==STEREO_RC) _B[1]->set((float)(B*_sign[1]));
  _R->set((float)(R));
  _distortion->set((float)(distortion));
}

void LTMulti3DLayer::updateEyeSelection( int vtoggle)
{
  float Btmp;
  if (_stereo_mode==STEREO_P_LEFT)
  {
    _B[0]->get(Btmp);
    if (vtoggle) _B[0]->set((float)(fabs(Btmp)*_sign[0]));
    else _B[0]->set((float)(fabs(Btmp)*-_sign[0]));
  }
}

void LTMulti3DLayer::enableLighting()
{
  if (_stereo_mode==STEREO_NONE) return;

  printf("LIGHTS ENABLED\n");
  _lighting->set(1);
}

void LTMulti3DLayer::disableLighting()
{
  if (_stereo_mode==STEREO_NONE) return;

  printf("LIGHTS DISABLED\n");
  _lighting->set(0);
}

/*****************************************************************************/

void LTMulti3DLayer::resetCamera() {

    if (_scene[0]->getChild(0) == NULL) return;

    const osg::BoundingSphere& bs = _scene[0]->getChild(0)->getBound();

    if (!bs.valid()) {
        return;
    }

    float znear = 1.0f*bs.radius();
    float zfar  = 3.0f*bs.radius();

    // 2:1 aspect ratio as per flag geometry below.
    //float proj_top   = 0.25f*znear;
    //float proj_right = 0.5f*znear;

    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.

    // set view
    //_cam->setReferenceFrame(osg::Transform::ABSOLUTE_RF);


    //_cam->setProjectionMatrixAsFrustum(-proj_right,proj_right,
    //                                     -proj_top,proj_top,
    //                                     znear,zfar);
    osg::Vec3 eye = bs.center()-osg::Vec3(0.0f,1.0f,-1.0f)*bs.radius();
    osg::Matrixd m;
    m.makeLookAt(eye, bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    updateViewMatrix(m);

    //Vec3 ctr, up;
    //_cam->getViewMatrixAsLookAt(eye, ctr, up);
    //printf("resetCamera: eye = (%f, %f, %f)\n", eye.x(), eye.y(), eye.z());
    //printf("resetCamera: ctr = (%f, %f, %f)\n", ctr.x(), ctr.y(), ctr.z());
    //printf("resetCamera: up = (%f, %f, %f)\n", up.x(), up.y(), up.z());

    return;
}
/*****************************************************************************/

void LTMulti3DLayer::updateViewMatrix( osg::Matrixd& m ) {

  for ( size_t i = 0; i < _sceneCams.size(); i++ )
    _sceneCams[i]->setViewMatrix( m * (_viewMatrices[i]) );
}

/*****************************************************************************/

void LTMulti3DLayer::setScene( osg::Node* scene ) {
  if (scene!=NULL) scene->setCullingActive(false);

  _scene[0]->removeChild(0,1);
  _scene[1]->removeChild(0,1);

  _scene[0]->addChild( scene );
  if ( _stereo_mode==STEREO_RC) _scene[1]->addChild( scene );
}


/*****************************************************************************/
