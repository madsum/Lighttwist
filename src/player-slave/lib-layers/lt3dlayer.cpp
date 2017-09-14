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

#include <osg/CameraNode>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Node>
#include <osg/Geode>
#include <osg/MatrixTransform>

#include <cstdlib>
#include <ctime>

#include "lt3dlayer.h"

LT3DLayer::LT3DLayer( const char* shaderFile,
                                 osg::Node* scene,
                                 int rttWidth, int rttHeight, double vertical_angle_of_view,unsigned char stereo_mode)
: LTLayer( shaderFile, createTexture(rttWidth, rttHeight) )
{

    printf("LT3DLayer ctor...\n");
    setName( "LT3DLayer" );
    _width=rttWidth;
    _height=rttHeight;

    _scene[0]=new osg::Group();
    _scene[1]=new osg::Group();

    _vertical_angle_of_view=vertical_angle_of_view;
    _stereo_mode=stereo_mode;

    _scene[0]->addChild( scene );
    if (_stereo_mode==STEREO_RC) _scene[1]->addChild( scene );

    printf( "_xLutMin = %f; _xLutMax = %f; _yLutMin = %f; _yLutMax = %f\n",
            _xLutMin, _xLutMax, _yLutMin, _yLutMax );

    float thetaStart = _xLutMin * 2.0 * M_PI;
    float thetaEnd = _xLutMax * 2.0 * M_PI;

    _viewMatrix = osg::Matrix::identity();
    _cam = createSubCamera( thetaStart, thetaEnd, M_PI, // <- offset
                            0, 0, _width, _height,-1 );
    this->addChild( _cam.get() );

    osg::StateSet* ss = _hud->getOrCreateStateSet();
    _viewAngleUF = new osg::Uniform( "viewAngle", (float)fabs(thetaEnd - thetaStart) );
    ss->addUniform(_viewAngleUF.get());
    _nbCamUF = new osg::Uniform( "nbCams", 1.0f );
    ss->addUniform(_nbCamUF.get());

    printf("LT3DLayer ctor done\n");
}


/******************************************************************************/

LT3DLayer::LT3DLayer( const char* shaderFile,
                                 osg::Node* scene,
                                 int rttWidth, int rttHeight,
                                 double thetaStart, double thetaEnd, double vertical_angle_of_view,unsigned char stereo_mode )
: LTLayer( shaderFile, createTexture(rttWidth, rttHeight) )
{

    printf("LT3DLayer ctor...\n");
    setName( "LT3DLayer" );
    _width=rttWidth;
    _height=rttHeight;

    _scene[0]=new osg::Group();
    _scene[1]=new osg::Group();

    _vertical_angle_of_view=vertical_angle_of_view;
    _stereo_mode=stereo_mode;

    _scene[0]->addChild( scene );
    if (_stereo_mode==STEREO_RC) _scene[1]->addChild( scene );

    printf( "_xLutMin = %f; _xLutMax = %f; _yLutMin = %f; _yLutMax = %f\n",
            _xLutMin, _xLutMax, _yLutMin, _yLutMax );

    _viewMatrix = osg::Matrix::identity();
    _cam = createSubCamera( thetaStart, thetaEnd, M_PI, // <- offset
                            0, 0, _width, _height,-1 );
    this->addChild( _cam.get() );

    osg::StateSet* ss = _hud->getOrCreateStateSet();
    _viewAngleUF = new osg::Uniform( "viewAngle", (float)fabs(thetaEnd - thetaStart) );
    ss->addUniform(_viewAngleUF.get());
    _nbCamUF = new osg::Uniform( "nbCams", 1.0f );
    ss->addUniform(_nbCamUF.get());

    printf("LT3DLayer ctor done\n");
}

/******************************************************************************/

LT3DLayer::~LT3DLayer()
{
    printf("KILL Cyclo 3DLayer\n");
}

/******************************************************************************/


void LT3DLayer::setScene( osg::Node* scene )
{
  _scene[0]->removeChild( 0,1 );
  _scene[1]->removeChild( 0,1 );

  scene->setDataVariance( osg::Object::DYNAMIC );

  _scene[0]->addChild( scene );
  if ( _stereo_mode==STEREO_RC) _scene[1]->addChild( scene );
}

/******************************************************************************/

void LT3DLayer::resetCamera() {

    const osg::BoundingSphere& bs = _cam->getBound();

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
    _cam->setViewMatrixAsLookAt(eye, bs.center(),osg::Vec3(0.0f,0.0f,1.0f));

    //Vec3 ctr, up;
    //_cam->getViewMatrixAsLookAt(eye, ctr, up);
    //printf("resetCamera: eye = (%f, %f, %f)\n", eye.x(), eye.y(), eye.z());
    //printf("resetCamera: ctr = (%f, %f, %f)\n", ctr.x(), ctr.y(), ctr.z());
    //printf("resetCamera: up = (%f, %f, %f)\n", up.x(), up.y(), up.z());

    return;
}


/******************************************************************************/

void LT3DLayer::updateViewMatrix( osg::Matrixd& m ) {

  _cam->setViewMatrix( m * _viewMatrix );

}

/******************************************************************************/



osg::Node* LT3DLayer::generateDefaultScene() {

  osg::Geode *g = new osg::Geode();

  double x, y, z;
  double min = - 100;
  double max = 100;
  double rad = 1.0;
  double gap = 40.0;

  osg::ShapeDrawable* shape;

  srand((unsigned)time(0));

  for (x = min; x <= max; x+= rad*10.0)
    for (y = min; y <= max; y+= rad*10.0)
      for (z = min; z <= max; z+= rad*10.0) {
        if (fabs(x) > gap || fabs(y) > gap || fabs(z) > gap ) {
          shape = new osg::ShapeDrawable( new osg::Box(osg::Vec3(x,y,z), rad) );
          shape->setColor( osg::Vec4(matRandNumber(), matRandNumber(), matRandNumber(), 1.0) );
          g->addDrawable(shape);
        }
      }

  return g;

}


/******************************************************************************/


osg::CameraNode *LT3DLayer::createSubCamera(float thetaStart, float thetaEnd,
        float thetaOffset, int x0, int y0, int xs, int ys, int eye)
{
  if (eye < -1 || eye>=2 ) return NULL;

  if ( thetaEnd < thetaStart ) thetaEnd += 2 * M_PI;

  float thetaCenter = thetaOffset + ( thetaStart + thetaEnd ) / 2.0;
  float thetaSpan = fabs( thetaEnd - thetaStart );

  osg::CameraNode *camera = new osg::CameraNode();

  printf("adding scene for eye %d\n",eye);
  if (eye==-1)
  {
    camera->addChild( _scene[0].get() );
    printf("createSubCamera: scene added\n");
    if (_stereo_mode==STEREO_RC) {
      camera->addChild( _scene[1].get() );
      printf("createSubCamera: right scene added for stereo\n");
    }
  }
  else
  {
    camera->addChild( _scene[eye].get() );
    printf("createSubCamera: scene added\n");
  }

  camera->setName( "scenecam" );

  camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (_stereo_mode!=STEREO_RC) camera->setColorMask(1,1,1,1);
  else if (eye==0) camera->setColorMask(1,0,0,1);
  else camera->setColorMask(0,1,1,1);
  if (!(eye==1 && _stereo_mode==STEREO_RC))
  {
    camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f)); // ALPHA!! ZERO!!!
  }

  // pour le view seulement
  //camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
  camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  camera->setTransformOrder(osg::CameraNode::POST_MULTIPLY);
  camera->setProjectionMatrix(osg::Matrix::identity());

  camera->setViewMatrix(osg::Matrix::identity());
  _viewMatrix = osg::Matrix::rotate( thetaCenter, 0.0f, 1.0f, 0.0f );

  //osg::Matrix::rotate( thetaCenter, 0.0f, 1.0f, 0.0f );
  //osg::Matrix::rotate( M_PI/2.0, 0.0f, 0.0f, 1.0f );
  camera->setViewMatrix( _viewMatrix );

  /* osg::Vec3 eye, ctr, up;
  camera->getViewMatrixAsLookAt(eye, ctr, up);
  printf("createSubCamera: eye = (%f, %f, %f)\n", eye.x(), eye.y(), eye.z());
  printf("createSubCamera: ctr = (%f, %f, %f)\n", ctr.x(), ctr.y(), ctr.z());
  printf("createSubCamera: up = (%f, %f, %f)\n", up.x(), up.y(), up.z());*/

  camera->setViewport(x0, y0, xs, ys);

  // set the camera to render before the main camera.
  camera->setRenderOrder(osg::CameraNode::PRE_RENDER);
  // tell the camera to use OpenGL frame buffer object where supported.
  camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);
  // attach the texture and use it as the color buffer.
  camera->attach(osg::CameraNode::COLOR_BUFFER, _texture.get());

  osg::StateSet *ss = camera->getOrCreateStateSet();

  // ici pour le depth test?????
  ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
  // on devrait mettre ca a off si la lumiere est disponible...
  ss->setMode(GL_LIGHTING,osg::StateAttribute::ON);
  ss->setMode(GL_BLEND,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

  ss->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
  ss->setMode(GL_POLYGON_SMOOTH, osg::StateAttribute::ON);

  osg::Matrix f;

  double near = 0.001;
  double far = 1000000.0;
  double left =   -near * tan( thetaSpan * 0.5 );
  double right =  -left;

  double phi = _vertical_angle_of_view * M_PI / 180.0;
  double h = near / cos( thetaSpan * 0.5 );
  double top = h * tan( phi / 2.0 );
  double bottom = -top;

  printf( "h = %f\n", h );
  printf( "left = %f; right = %f; top = %f; bottom %f \n",
          left, right, top, bottom );

  f.makeFrustum( left, right, bottom, top, near, far );

  camera->setProjectionMatrix( f );

  //the vertex shader makes visible vertices that were outside the frustrum (culling is no longer valid)
  //however, the following solution makes renderer very slow...
//  camera->setCullingMode(osg::CullSettings::DEFAULT_CULLING&(!osg::CullSettings::VIEW_FRUSTUM_SIDES_CULLING));

  printf("3d layer: _cam created\n");

  return(camera);

}


/******************************************************************************/

