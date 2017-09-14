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

#include <osg/Group>
#include <osg/TexEnv>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Camera>
#include <osg/Matrix>

#include <osgDB/ReadFile>

#include "lttiledvideolayer.h"


/*****************************************************************************/

int next_pow2(int val)
{
  int pow = 1;
  while(pow<val) pow*=2;
  return pow;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

class TexCallback : public  osg::NodeCallback
{
public:
  TexCallback(LTTiledVideoLayer* tvl)
    : osg::NodeCallback()
  {
    _tvl = tvl;

  }

  virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
    //printf( "Callback [%s] (go!)\n", _tvl->getName().c_str() );
    _tvl->go();
    traverse(node,nv);
  }

  LTTiledVideoLayer* _tvl;
};



/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

LTTiledVideoLayer::LTTiledVideoLayer( const char* fileName,
                                      const char* shaderFile,
                                      int rttWidth, int rttHeight )
    : LT3DLayer( shaderFile, NULL, rttWidth, rttHeight ),
      LTVideoController()
{

  int nbTiles;
  int lutBBPix[4];
  tile_info *tiles;

  setName( "LTTiledVideoLayer" );

  tiles=(tile_info *)(malloc(sizeof(tile_info)*MAX_NB_TILES));

  this->setUpdateCallback( new TexCallback(this) );

  nbTiles = get_tiles_coord( fileName, _lutBB, tiles, lutBBPix);

  if( nbTiles <= 0 ) { // trouble ahead!
    printf("nbTiles <= 0!!  BORKED!  returning...\n");
    _broken = true;
    return;
  }

  double qxMin, qyMin, qxMax, qyMax, qtxMin, qtxMax, qtyMin, qtyMax;

  //part of the movie covered by the lut
  double lutMovieX = lutBBPix[XMIN];
  double lutMovieY = lutBBPix[YMIN];
  double lutMovieWidth = lutBBPix[XMAX] - lutBBPix[XMIN] + 1;
  double lutMovieHeight = lutBBPix[YMAX] - lutBBPix[YMIN] + 1;

  /*if (movieWidth > rttWidth || movieHeight > rttHeight ) {
    printf("rtt size is screwy\n");
    exit(1);
  }*/

  printf( "lut BB: x[%f, %f]; y[%f, %f]\n", lutMovieX, lutMovieWidth, lutMovieY, lutMovieHeight );

  osg::Vec2Array* tcoords = (osg::Vec2Array*)_hudQuad->getDrawable(0)->asGeometry()->getTexCoordArray(0);
  (*tcoords)[0].set(0,1);
  (*tcoords)[1].set(0,0);
  (*tcoords)[2].set(1,0);
  (*tcoords)[3].set(1,1);

  //movie,rtt ratio
  double xScaleHud = lutMovieWidth / rttWidth;
  double yScaleHud = lutMovieHeight / rttHeight;

  if (xScaleHud>1.0) _xLutScaleUF->set( 1.0f );
  else _xLutScaleUF->set( (float) xScaleHud );
  if (yScaleHud>1.0) _yLutScaleUF->set( 1.0f );
  else _yLutScaleUF->set( (float) yScaleHud );

  double viewWidth,viewHeight;
  viewWidth=lutMovieWidth;
  if (lutMovieWidth>rttWidth) viewWidth=rttWidth;
  viewHeight=lutMovieHeight;
  if (lutMovieHeight>rttHeight) viewHeight=rttHeight;

  _cam->setViewport(0, 0, viewWidth, viewHeight);
  _cam->setProjectionMatrix( osg::Matrixd::ortho2D(0, viewWidth, viewHeight, 0) );

  _cam->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  _cam->setViewMatrix(osg::Matrix::identity());

  osg::ref_ptr<osg::Group> video = new Group();
  osg::StateSet *ss;

  for (int i = 0; i < nbTiles; i++ ) {

    //these are redeclared for every iteration
    osg::ref_ptr<osg::Geode> quad;
    osg::ref_ptr<osg::TexEnv> tenv;
    osg::ref_ptr<LTVideoTexture2D> vt;

    //minimum and maximum pixels of the lut coverted by the tile
    qxMin = (double)(tiles[i].x + tiles[i].bb_x) - lutMovieX;
    qyMin = (double)(tiles[i].y + tiles[i].bb_y) - lutMovieY;
    qxMax = (double)(tiles[i].x + tiles[i].bb_x + tiles[i].bb_width) - lutMovieX;
    qyMax = (double)(tiles[i].y + tiles[i].bb_y + tiles[i].bb_height) - lutMovieY;
    if (xScaleHud>1.0)
    {
      qxMin/=xScaleHud;
      qxMax/=xScaleHud;
    }
    if (yScaleHud>1.0)
    {
      qyMin/=yScaleHud;
      qyMax/=yScaleHud;
    }

    //texture coordinates that represent the used part of the tile (w.r.t. the tile)
    qtxMin = (double)(tiles[i].bb_x) / (double)next_pow2(tiles[i].width);
    qtyMin = (double)(tiles[i].bb_y) / (double)next_pow2(tiles[i].height);
    qtxMax = (double)(tiles[i].bb_x + tiles[i].bb_width) / (double)next_pow2(tiles[i].width);
    qtyMax = (double)(tiles[i].bb_y + tiles[i].bb_height) / (double)next_pow2(tiles[i].height);

    printf( "file: %s\nquad: %f, %f, %f, %f ... tex: %f, %f, %f, %f\n",
            tiles[i].filename,
            qxMin, qxMax, qyMin, qyMax,
            qtxMin, qtxMax, qtyMin, qtyMax );

    quad = createQuadGeode( qxMin, qxMax, qyMin, qyMax, 0,
                            qtxMin, qtxMax, qtyMin, qtyMax );

    vt = new LTVideoTexture2D( tiles[i].filename, 0, 0 );
    addVideoTexture( vt.get() );

    ss = quad->getOrCreateStateSet();
    ss->setDataVariance( osg::Object::DYNAMIC );
    ss->setTextureAttributeAndModes(0, vt.get(), osg::StateAttribute::ON);
    tenv = new osg::TexEnv(osg::TexEnv::DECAL);
    ss->setTextureAttribute(0,tenv.get());

    video->addChild( quad.get() );
  }

  setScene( video.get() );

  //disableShader();
  //enableShader();

  free(tiles);
}


/*****************************************************************************/

LTTiledVideoLayer::~LTTiledVideoLayer() {
}

/*****************************************************************************/
