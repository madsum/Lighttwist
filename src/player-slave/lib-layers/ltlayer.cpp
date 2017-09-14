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

/// LAYER


//#include <osg/StateAttribute>
//#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/FragmentProgram>
#include <osg/CameraNode>
#include <osg/TexMat>

#include <osgDB/ReadFile>

#include "ltlayer.h"
#include "utils/gamma.h"


/******************************************************************************/


// init private static members
osg::Texture2D* LTLayer::_lutTex = NULL;
osg::Image* LTLayer::_lutTexImage = NULL;
osg::Texture2D* LTLayer::_blendTex = NULL;
osg::Image* LTLayer::_blendTexImage = NULL;

bool LTLayer::_lutNormalized = false;

vector3 LTLayer::_gamma = {0,0,0};
matrix3 LTLayer::_color = {0,0,0,0,0,0,0,0,0};
vector4 LTLayer::_lutBB = {0,0,0,0};

float LTLayer::_xLutScale = 1.0;
float LTLayer::_yLutScale = 1.0;

float LTLayer::_xLutMin = 0.0;
float LTLayer::_xLutMax = 1.0;
float LTLayer::_yLutMin = 0.0;
float LTLayer::_yLutMax = 1.0;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/




/// sert pour les video
LTLayer::LTLayer( const char* shaderFile,
                  osg::Texture2D* tex,
                  float xLutScale,float yLutScale,
                  float xRttTexMin,float xRttTexMax,
                  float yRttTexMin,float yRttTexMax,
                  bool restrictContentToLUT,float slit )
{
  setName( "LTLayer" );
  _broken = false;

  // this is the maximum coord of the texture image...
  // normally, set to 1.0
  // if set to, say, 0.5, then the texture 0..1 should be mapped to 0.. 0.5
  // we assume that 0,0 is always valid
  _xLutScale=xLutScale;
  _yLutScale=yLutScale;

  // Attention: tex pourrait etre null (puis defini par setTexture)
  _texture = tex; //createTexture( _texWidth, _texHeight);

  if (restrictContentToLUT) _hud = createHUD( 0,1,0,1 );
  else _hud = createHUD( xRttTexMin,xRttTexMax,yRttTexMin,yRttTexMax );

  _hud->setDataVariance( osg::Object::DYNAMIC );

  _restrictContentToLUT = restrictContentToLUT;

  osg::ref_ptr<osg::StateSet> stateset = _hud->getOrCreateStateSet();

  this->addChild( _hud.get() );

  //Init animation parameters
  aspect_x = xLutScale;
  aspect_y = yLutScale;
  scale = 1;
  rot_angle = 0;
  rot_cx = rot_cy = 0.5;
  tx = ty = 0;

  setDataVariance( osg::Object::DYNAMIC );

  osg::ref_ptr<osg::TexMat> tm = new osg::TexMat( osg::Matrixd::identity() );
  stateset->setTextureAttribute( TEX_ID , tm.get());
  stateset->setTextureAttributeAndModes(TEX_ID, tm.get(), osg::StateAttribute::ON);

  //_gamma = gamma;
  _enableLUTUF = new osg::Uniform( "enableLUT", true );
  _gammaUF0 = new osg::Uniform( "gamma0",  (float)(_gamma[0]) );
  _gammaUF1 = new osg::Uniform( "gamma1",  (float)(_gamma[1]) );
  _gammaUF2 = new osg::Uniform( "gamma2",  (float)(_gamma[2]) );
  osg::ref_ptr<osg::Uniform> colorUF0 = new osg::Uniform( "color0",  (float)(_color[0]) );
  osg::ref_ptr<osg::Uniform> colorUF1 = new osg::Uniform( "color1",  (float)(_color[1]) );
  osg::ref_ptr<osg::Uniform> colorUF2 = new osg::Uniform( "color2",  (float)(_color[2]) );
  osg::ref_ptr<osg::Uniform> colorUF3 = new osg::Uniform( "color3",  (float)(_color[3]) );
  osg::ref_ptr<osg::Uniform> colorUF4 = new osg::Uniform( "color4",  (float)(_color[4]) );
  osg::ref_ptr<osg::Uniform> colorUF5 = new osg::Uniform( "color5",  (float)(_color[5]) );
  osg::ref_ptr<osg::Uniform> colorUF6 = new osg::Uniform( "color6",  (float)(_color[6]) );
  osg::ref_ptr<osg::Uniform> colorUF7 = new osg::Uniform( "color7",  (float)(_color[7]) );
  osg::ref_ptr<osg::Uniform> colorUF8 = new osg::Uniform( "color8",  (float)(_color[8]) );
  _xLutScaleUF = new osg::Uniform( "xLutScale",  _xLutScale );
  _yLutScaleUF = new osg::Uniform( "yLutScale",  _yLutScale );
  _xRttTexMinUF = new osg::Uniform( "xRttTexMin",  xRttTexMin );
  _xRttTexMaxUF = new osg::Uniform( "xRttTexMax",  xRttTexMax );
  _yRttTexMinUF = new osg::Uniform( "yRttTexMin",  yRttTexMin );
  _yRttTexMaxUF = new osg::Uniform( "yRttTexMax",  yRttTexMax );
  _xLutMinUF = new osg::Uniform( "lutxmin", _xLutMin );
  _xLutMaxUF = new osg::Uniform( "lutxmax", _xLutMax );
  // normalement un param du layer
  _fade=1.0;
  _fadeUF = new osg::Uniform( "fade",  _fade );
  _homographyUF = new osg::Uniform("homography",_homography);
  _slit = new osg::Uniform( "slit",  slit );
  stateset->setTextureAttributeAndModes(LUT_ID, _lutTex, osg::StateAttribute::ON);
  osg::ref_ptr<osg::Uniform> lutSampler = new osg::Uniform("lut_tex", LUT_ID);
  stateset->setTextureAttributeAndModes(BLEND_ID, _blendTex, osg::StateAttribute::ON);
  osg::ref_ptr<osg::Uniform> blendSampler = new osg::Uniform("blend_tex", BLEND_ID);
  _alphaTex = NULL;
  _alphaTexImage = NULL;
  stateset->setTextureAttributeAndModes(ALPHA_ID, _alphaTex, osg::StateAttribute::OFF);
  osg::ref_ptr<osg::Uniform> alphaSampler = new osg::Uniform("alphaTex", ALPHA_ID);
  _useAlphaUF = new osg::Uniform("useAlpha", false);
  if( _texture.valid() )
      stateset->setTextureAttributeAndModes( TEX_ID,
                                             _texture.get(), osg::StateAttribute::ON );
  osg::ref_ptr<osg::Uniform> texSampler = new osg::Uniform("texture", TEX_ID );

  loadGLSLShader( shaderFile, stateset.get() );

    stateset->addUniform(_enableLUTUF.get());
    stateset->addUniform(_gammaUF0.get());
    stateset->addUniform(_gammaUF1.get());
    stateset->addUniform(_gammaUF2.get());
    stateset->addUniform(colorUF0.get());
    stateset->addUniform(colorUF1.get());
    stateset->addUniform(colorUF2.get());
    stateset->addUniform(colorUF3.get());
    stateset->addUniform(colorUF4.get());
    stateset->addUniform(colorUF5.get());
    stateset->addUniform(colorUF6.get());
    stateset->addUniform(colorUF7.get());
    stateset->addUniform(colorUF8.get());

    // on doit passer le facteur d'echelle en parametre, parce qu'on veut
    // pouvoir reutiliser la LUT entre les differents films
    // pour le precrop, il faudra en plus du scale avoir un offset....

    // echelle est en fait la limite de la texture

    stateset->addUniform(_xLutScaleUF.get());
    stateset->addUniform(_yLutScaleUF.get());

    printf("rtttex vs xylutminmax?\n");
    printf("%f == %f\n", xRttTexMin, _xLutMin );
    printf("%f == %f\n", xRttTexMax, _xLutMax );
    printf("%f == %f\n", yRttTexMin, _yLutMin );
    printf("%f == %f\n", yRttTexMax, _yLutMax );

    stateset->addUniform(_xRttTexMinUF.get());
    stateset->addUniform(_xRttTexMaxUF.get());
    stateset->addUniform(_yRttTexMinUF.get());
    stateset->addUniform(_yRttTexMaxUF.get());

    stateset->addUniform(_homographyUF.get());

    this->setScaleTrans(1.0,1.0,0.0,0.0);

    stateset->addUniform(_xLutMaxUF.get());
    stateset->addUniform(_xLutMinUF.get());

    stateset->addUniform(_fadeUF.get());
    stateset->addUniform(_slit.get());
    stateset->addUniform(lutSampler.get());
    stateset->addUniform(blendSampler.get());
    stateset->addUniform(alphaSampler.get());
    stateset->addUniform(_useAlphaUF.get());
    stateset->addUniform(texSampler.get());

  //// mode blending a ON (pour permettre le fade)
  osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
  bf->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
  stateset->setAttributeAndModes(bf.get(),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
  stateset->setMode(GL_BLEND,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

  //// mode depth_test a OFF, puisque c'est un quad dans tout l'ecran.
  stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
  stateset->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
  stateset->setMode(GL_POLYGON_SMOOTH, osg::StateAttribute::ON);
}

//////
///
/// pour ajuster la texture apres avoir creer le ltlayer
///
/////
void LTLayer::setTexture(osg::Texture2D* tex) {
    _texture=tex;
    if( _texture.valid() ) {
        osg::StateSet* stateset = _hud->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes( TEX_ID, _texture.get(), osg::StateAttribute::ON );
    }
}


/******************************************************************************/

LTLayer::~LTLayer()
{
  printf("KILL CYCLO LAYER\n");
}


/******************************************************************************/
/******************************************************************************/

void LTLayer::reset() {

    printf(" LTLayer::reset!!\n");
    _xLutScaleUF->set( _xLutScale );
    _yLutScaleUF->set( _yLutScale );
    _xRttTexMinUF->set( _xLutMin );
    _xRttTexMaxUF->set( _xLutMax );
    _yRttTexMinUF->set( _yLutMin );
    _yRttTexMaxUF->set( _yLutMax );
    _xLutMinUF->set( _xLutMin );
    _xLutMaxUF->set( _xLutMax );

    printf("reset lut: %f, %f, %f, %f\n",
           _xLutMin, _xLutMax, _yLutMin, _yLutMax );

    if (_restrictContentToLUT) {
        reshapeHud( -1, 1, -1, 1, 0,
                    0,1,0,1 );
    } else {
        reshapeHud( -1, 1, -1, 1, 0,
                    _xLutMin, _xLutMax,
                    _yLutMin, _yLutMax );
    }

    osg::StateSet* ss =   _hud->getOrCreateStateSet();
    ss->setTextureAttributeAndModes(LUT_ID, _lutTex, osg::StateAttribute::ON);
    ss->setTextureAttributeAndModes(BLEND_ID, _blendTex, osg::StateAttribute::ON);

}



/******************************************************************************/

void LTLayer::enableShader( bool b )
{
  osg::StateSet* ss = _hud->getOrCreateStateSet();

  printf("LTLayer: Enabling SHADERS\n");

  if (b) {

    ss->setTextureAttributeAndModes( LUT_ID, _lutTex, osg::StateAttribute::ON );
    ss->setTextureAttributeAndModes( BLEND_ID, _blendTex, osg::StateAttribute::ON );
    ss->setAttribute( _shaderProgram.get() );

  } else {

    ss->setTextureAttributeAndModes( LUT_ID, _lutTex, osg::StateAttribute::OFF );
    ss->setTextureAttributeAndModes( BLEND_ID, _blendTex, osg::StateAttribute::OFF );
    //ss->setAttributeAndModes( _shaderProgram, osg::StateAttribute::OFF ); DOESN'T WORK!!! OSG KNOWS THIS!
    ss->removeAttribute( _shaderProgram.get() );
  }

}

/******************************************************************************/

void LTLayer::setAlphaMask( osg::Image* img ) {

    if (!img) {
        printf("setAlphaMask: invalid image\n");
        return;
    }
    osg::StateSet* ss = _hud->getOrCreateStateSet();

    _alphaTexImage = img;
    _alphaTexImage->setDataVariance( osg::Object::DYNAMIC );
    printf("creating texture\n");
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureSize( img->s(), img->t() ); // useless??
    tex->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
    tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    tex->setImage( img );

    _alphaTex = tex;
    _alphaTex->setDataVariance( osg::Object::DYNAMIC );
    printf("settextureattributeandmodes on\n");
    ss->setTextureAttributeAndModes(ALPHA_ID, _alphaTex, osg::StateAttribute::ON);

    _alphaTexImage->dirty();
    printf("enableAlphaMask\n");
    _useAlphaUF->set(true);
    printf("setAlphaMask done\n");
}

void LTLayer::setAlphaMask( osg::Texture2D* tex ) {

    osg::StateSet* ss = _hud->getOrCreateStateSet();

    _alphaTex = tex;
    _alphaTexImage = tex->getImage();

    ss->setTextureAttributeAndModes(ALPHA_ID, _alphaTex, osg::StateAttribute::ON);
    enableAlphaMask(true);
}

/******************************************************************************/

void LTLayer::enableAlphaMask( bool b ) {

    if (_alphaTex != NULL) {
        _useAlphaUF->set(b);
    } else {
        _useAlphaUF->set(false);
    }

}




/******************************************************************************/

void LTLayer::reshapeHud( double left, double right,
                          double bottom, double top, double zoff,
                          double xRTTMin, double xRTTMax,
                          double yRTTMin, double yRTTMax ) {

  //_hud->setProjectionMatrix(osg::Matrixd::ortho2D(left, right, bottom, top));

  osg::Geometry *g = _hudQuad->getDrawable(0)->asGeometry();
  osg::Vec3Array* coords = (osg::Vec3Array*) g->getVertexArray();
  (*coords)[0].set( left,  top,    0 );
  (*coords)[1].set( left,  bottom, 0 );
  (*coords)[2].set( right, bottom, 0 );
  (*coords)[3].set( right, top,    0 );

  osg::Vec2Array* tcoords = (osg::Vec2Array*) g->getTexCoordArray(0);
  (*tcoords)[0].set( xRTTMin, yRTTMax );
  (*tcoords)[1].set( xRTTMin, yRTTMin );
  (*tcoords)[2].set( xRTTMax, yRTTMin );
  (*tcoords)[3].set( xRTTMax, yRTTMax );
}


/******************************************************************************/

void LTLayer::updateSlit( float slit )
{
  _slit->set( slit );
}

void LTLayer::updateGamma( vector3 gamma )
{
  _gammaUF0->set((float)(gamma[0]));
  _gammaUF1->set((float)(gamma[1]));
  _gammaUF2->set((float)(gamma[2]));
}

/******************************************************************************/

void LTLayer::setScaleTrans( float xs, float ys,
                             float xt, float yt )
{
  _homography=osg::Matrix3(xs,0,xt,0,ys,yt,0,0,1);

  _homographyUF->set( _homography );

  //printf("setScaleTrans: scale by (%f, %f); trans by (%f, %f)\n", xs, ys, xt, yt);

}


/******************************************************************************/
#define HEADERSIZE  27

//
// commande de gimp
// "pain %c%5d%5d%5d%5d" , xb,yb,wb,hb
void LTLayer::paintTexture( int bpp,int x,int y,int width,int height,unsigned char *bin )
{
  int xx,yy,j;
  unsigned char *data;

  data=_texture->getImage()->data();

    int wref,href,bppref;
    wref=_texture->getTextureWidth();
    href=_texture->getTextureHeight();
    bppref=_texture->getImage()->r();

    bppref=4;
    //printf("bppref=%d bpp=%d res:(%d,%d) (%d,%d)  %d %d\n",bppref,bpp,wref,href,x,y,width,height);

//  printf("format is %d\n",_texture->getImage()->getInternalTextureFormat());

  for(yy=0;yy<height;yy++)
  {
    for(xx=0;xx<width;xx++)
    {
      //printf("(");
      for(j=0;j<bpp && j<bppref;j++)
      {
        data[((href-1-y-yy)*wref+(x+xx))*bppref+j]=bin[(yy*width+xx)*bpp+j];
        //printf("%3d",buf[20+(yy*width+xx)*bpp+j]);
      }
      //printf(") ");
    }
    //printf("\n");
  }

  _texture->getImage()->dirty();
}

/******************************************************************************/

//
// commande de gimp
// "pain %c%5d%5d%5d%5d" , xb,yb,wb,hb
void LTLayer::paintTexture( unsigned char *buf )
{

  int x,y,width,height;
  int bpp;
  char buffer[30]; // this is > headersize(27)
  unsigned char *data;

  data=_texture->getImage()->data();

    int i;
    for(i=0;buf[i];i++) buffer[i]=buf[i];
    buffer[i]=0;

    height=atoi(buffer+21);buffer[21]=0;
    width=atoi(buffer+16);buffer[16]=0;
    y=atoi(buffer+11);buffer[11]=0;
    x=atoi(buffer+6);buffer[6]=0;
    bpp=atoi(buffer+5);

    this->paintTexture(bpp,x,y,width,height,buf+HEADERSIZE);
}

/******************************************************************************/


// on cree une camera en plus, sur le top...
osg::CameraNode* LTLayer::createHUD(float xRTTMin, float xRTTMax,
                                    float yRTTMin,float yRTTMax )
{

  printf("createHUD with x(%f, %f) y(%f, %f)\n", xRTTMin, xRTTMax, yRTTMin, yRTTMax);

  osg::Geode* hudModel = createQuadGeode(0, 0, 0, xRTTMin,xRTTMax,yRTTMin,yRTTMax);
  hudModel->setName( "hudquad" );

  osg::StateSet* ss = hudModel->getOrCreateStateSet();
  ss->setDataVariance( osg::Object::DYNAMIC );

  osg::ref_ptr<osg::TexEnv> te = new osg::TexEnv(osg::TexEnv::DECAL);
  ss->setTextureAttribute(0,te.get());

  _hudQuad = hudModel;

  return createHUDCamera(hudModel);
}


osg::CameraNode* LTLayer::createHUDCamera(osg::Node* hudModel)
{
  // The camera to view the hud. This is what we control, in reality
  // the camera for prerender will be controlled from outside...

  osg::CameraNode* camera = new osg::CameraNode;
  camera->setName( "hudcam" );

  osg::StateSet* ss = camera->getOrCreateStateSet();
  ss->setDataVariance( osg::Object::DYNAMIC );

  // set up the background color and clear mask.
  camera->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
  //camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //camera->setClearMask( GL_DEPTH_BUFFER_BIT);
  camera->setClearMask(0);
  camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER );

  // set up projection.

  camera->setProjectionMatrix(osg::Matrixd::ortho2D(-1, 1, -1, 1));

  // setup view of the model
  /*
    camera->setProjectionMatrixAsPerspective( 90.0, 1.0, -0.5, 0.5 );
    camera->setViewMatrixAsLookAt( osg::Vec3(0.0f,0.0f,-1.0f),
    osg::Vec3(0.0f,0.0f,0.0f),
    osg::Vec3(0.0f,1.0f,0.0f) );
  */

  camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
  camera->setRenderOrder(osg::CameraNode::POST_RENDER);

  camera->addChild(hudModel);

  return(camera);
}

/******************************************************************************/

osg::Geode* LTLayer::createQuadGeode(float xoff, float yoff, float zoff,
                                     float xRTTMin, float xRTTMax,
                                     float yRTTMin, float yRTTMax )
{

  return createQuadGeode( -1.0f+xoff, 1.0f+xoff,
                          -1.0f+yoff, 1.0f+yoff,
                          zoff,
                          xRTTMin, xRTTMax, yRTTMin, yRTTMax);

}

/******************************************************************************/

osg::Geode* LTLayer::createQuadGeode(float xmin, float xmax,
                                     float ymin, float ymax, float zoff,
                                     float xRTTMin, float xRTTMax,
                                     float yRTTMin, float yRTTMax)
{

  // set up the drawstate.
  osg::ref_ptr<osg::StateSet> dstate = new osg::StateSet;
  dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
  dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

  // set up the geoset.
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  geom->setStateSet(dstate.get());

  osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array(4);
  (*coords)[0].set( xmin, ymax, zoff );
  (*coords)[1].set( xmin, ymin, zoff );
  (*coords)[2].set( xmax, ymin, zoff );
  (*coords)[3].set( xmax, ymax, zoff );
  geom->setVertexArray(coords.get());

  osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array(4);

  (*tcoords)[0].set( xRTTMin, yRTTMax );
  (*tcoords)[1].set( xRTTMin, yRTTMin );
  (*tcoords)[2].set( xRTTMax, yRTTMin );
  (*tcoords)[3].set( xRTTMax, yRTTMax );

  /*(*tcoords)[0].set( 0.0f, 1.0f );
    (*tcoords)[1].set( 0.0f, 0.0f );
    (*tcoords)[2].set( 1.0f, 0.0f );
    (*tcoords)[3].set( 1.0f, 1.0f );*/

  geom->setTexCoordArray(0,tcoords.get());

  osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array(1);
  (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
  geom->setColorArray(colours.get());
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::ref_ptr<osg::DrawArrays> da=new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);
  geom->addPrimitiveSet(da.get());

  // set up the geode.
  osg::Geode *geode = new osg::Geode;
  geode->addDrawable(geom.get());

  return geode;
}

/******************************************************************************/

osg::Texture2D* LTLayer::createTexture(unsigned int tex_width, unsigned int tex_height)
{
  printf("CREATING Texture (%d,%d)\n",tex_width,tex_height);
  osg::Texture2D* texture2D = new osg::Texture2D;
  texture2D->setTextureSize(tex_width, tex_height);
  texture2D->setInternalFormat(GL_RGBA);
  texture2D->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  texture2D->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  texture2D->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER); //REPEAT
  texture2D->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
  return(texture2D);
}

/******************************************************************************/

osg::Texture2D* LTLayer::createLutTexFromImg( osg::Image* img )
{

  osg::Texture2D* tex = new osg::Texture2D;

  tex->setImage( img );
  //tex->setTextureSize( img->s(), img->t() ); // useless??

  tex->setInternalFormat( GL_LUMINANCE16_ALPHA16 );
  //tex->setInternalFormat( GL_RGBA16F_ARB );
  tex->setSourceType( GL_FLOAT );
  //tex->setSourceType( GL_SHORT );
  tex->setSourceFormat( GL_LUMINANCE_ALPHA );
  tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  tex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
  tex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
  //tex->setMaxAnisotropy(16);

  _lutTexImage = img;
  _lutTex = tex;

  _lutTexImage->setDataVariance( osg::Object::DYNAMIC );
  _lutTex->setDataVariance( osg::Object::DYNAMIC );

  _lutTexImage->dirty();

  return tex;

}

/******************************************************************************/

osg::Texture2D* LTLayer::createBlendTexFromImg( osg::Image* img )
{

  osg::Texture2D* tex = new osg::Texture2D;
  tex->setTextureSize( img->s(), img->t() ); // useless??
  tex->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
  tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  tex->setImage( img );

  _blendTexImage = img;
  _blendTex = tex;

  _blendTex->setDataVariance( osg::Object::DYNAMIC );
  _blendTexImage->setDataVariance( osg::Object::DYNAMIC );

  _blendTexImage->dirty();

  return tex;

}

/******************************************************************************/

/// precrop =1 : resize le bounding box sur l'intervalle (0,1)

osg::Image* LTLayer::loadLUT( const char* fileName,
                              float* xMin, float* xMax,
                              float* yMin, float* yMax, float voffsetx)
{
  /*![xs*ys] coord u,v -> x,y*/
  imgu *UV,*UV_cmap;

  float* data ;
  int width, height;
  float minx, miny, maxx, maxy;
  int loopall;

  _lutNormalized = false;

  UV = NULL;
  UV_cmap = NULL;
  if (imguLoad(&UV_cmap, fileName, LOAD_16_BITS)) return NULL;
  imguConvertToComplexUV(&UV, UV_cmap);

  minx = miny = 999999.0f;
  maxx = maxy = -999999.0f;

  width = UV->xs;
  height = UV->ys;
  printf( "LoadLUT: width = %d; height = %d\n", width, height );

  printf( "loatLUT: creating osg::Image...\n" );
  osg::Image* image = new osg::Image;
  image->setImage( width, height, 1,            // 1 = 3rd dimension..
                   GL_LUMINANCE16_ALPHA16,
                   GL_LUMINANCE_ALPHA,          // glformat
                   GL_FLOAT,
                   (unsigned char*) malloc( width * height * 2 * sizeof(float) ),
                   osg::Image::USE_MALLOC_FREE );

  image->setDataVariance( osg::Object::DYNAMIC );
  data = (float*)image->data();

  if(!data){
    printf("load_lut: Can't allocate data\n");
    exit(0);
  }

  float datax, datay;

  for( int i = 0; i < height; i++ )
    {
      for( int j = 0; j < width; j++ )
        {
          datax = (float) UV->complex[ i*width + j ][0];
          datay = (float) UV->complex[ i*width + j ][1];

          data[width*2*i + 2*j] = datax;
          data[width*2*i + 2*j + 1] = datay;

          if ( datax < minx && datax >= 0.0f ) minx = datax;
          if ( datax > maxx && datax >= 0.0f ) maxx = datax;
          if ( datay < miny && datay >= 0.0f ) miny = datay;
          if ( datay > maxy && datay >= 0.0f ) maxy = datay;
        }
    }

  //this offsets changes the 0 position on the cyclo
  if (voffsetx!=0.0)
    {
      loopall=0;
      if (minx+voffsetx>1.0) loopall=1;
      for( int i = 0; i < height; i++ )
        {
          for( int j = 0; j < width; j++ )
            {
              // check blend
              if (loopall)
                {
                  data[width*2*i + 2*j] += voffsetx-1.0;
                }
              else
                {
                  data[width*2*i + 2*j] += voffsetx;
                }
            }
        }

      minx+=voffsetx;
      if (loopall)
        {
          minx-=1.0;
          maxx+=voffsetx-1.0;
        }
      else
        {
          maxx+=voffsetx;
        }
    }

  imguFree(&UV);
  imguFree(&UV_cmap);

  printf("minx=%f maxx=%f miny=%f maxy=%f (pre-norm)\n", minx, maxx, miny, maxy);

  *xMin = minx;
  *xMax = maxx;
  *yMin = miny;
  *yMax = maxy;

  _xLutMin=minx;
  _xLutMax=maxx;
  _yLutMin=miny;
  _yLutMax=maxy;

  _lutBB[0] = minx;
  _lutBB[1] = maxx;
  _lutBB[2] = miny;
  _lutBB[3] = maxy;
  _lutTexImage = image;

  normalizeLUT(_lutTexImage,_xLutMin,_xLutMax,_yLutMin,_yLutMax,_xLutScale,_yLutScale);

  return image;
}

/******************************************************************************/

osg::Image* LTLayer::generateLUT(int machine_num,int nb_machines,int height,int width,
                                 float* xMin, float* xMax,
                                 float* yMin, float* yMax,
                                 float leftborder, float rightborder)
{
  float* data ;
  float minx, miny, maxx, maxy;
  //float xoffset,tile_width;

  if (nb_machines<1) return NULL;

  _lutNormalized = false;

  if (machine_num==0) leftborder = 0.0;
  if (machine_num==nb_machines-1) rightborder = 0.0;

  minx = miny = 999999.0f;
  maxx = maxy = -999999.0f;

  printf( "LoadLUT: width = %d; height = %d\n", width, height );

  printf( "loatLUT: creating osg::Image...\n" );
  osg::Image* image = new osg::Image;
  image->setImage( width, height, 1,            // 1 = 3rd dimension..
                   GL_LUMINANCE16_ALPHA16,
                   GL_LUMINANCE_ALPHA,          // glformat
                   //GL_SHORT,
                   GL_FLOAT,                    // gltype
                   (unsigned char*) malloc( width * height * 2 * sizeof(float) ),
                   osg::Image::USE_MALLOC_FREE );

  data = (float*)image->data();
  if(!data){
    printf("load_lut: Can't allocate data\n");
    exit(0);
  }

  /*Generer la LUT*/
  double cadreXmin,cadreXmax,cadreYmin,cadreYmax;
  float datax, datay;

  //tile_width=((double)(width-1.0))/nb_machines;
  //xoffset=machine_num*tile_width;

  cadreXmin = (double)machine_num/nb_machines + leftborder/nb_machines;
  cadreXmax = (double)(machine_num+1)/nb_machines - rightborder/nb_machines;
  cadreYmin = 0.0;
  cadreYmax = 1.0;

  for( int i = 0; i < height; i++ )
    {
      for( int j = 0; j < width; j++ )
        {
          datax = j/(width-1.0) * (cadreXmax-cadreXmin) + cadreXmin;
          datay = i/(height-1.0) * (cadreYmax-cadreYmin) + cadreYmin;
          //datax = (xoffset+j/(double)(nb_machines))/(width-1.0);
          //datay = i/(height-1.0);
          //printf("(%f, %f)\n", datax, datay);

          data[width*2*i + 2*j] = datax;
          data[width*2*i + 2*j + 1] = datay;

          if ( datax < minx && datax >= 0.0f ) minx = datax;
          if ( datax > maxx && datax >= 0.0f ) maxx = datax;
          if ( datay < miny && datay >= 0.0f ) miny = datay;
          if ( datay > maxy && datay >= 0.0f ) maxy = datay;
        }
    }

  printf("minx=%f maxx=%f miny=%f maxy=%f (pre-norm)\n", minx, maxx, miny, maxy);

  *xMin = minx;
  *xMax = maxx;
  *yMin = miny;
  *yMax = maxy;

  _xLutMin=minx;
  _xLutMax=maxx;
  _yLutMin=miny;
  _yLutMax=maxy;

  _lutBB[0] = minx;
  _lutBB[1] = maxx;
  _lutBB[2] = miny;
  _lutBB[3] = maxy;
  _lutTexImage = image;

  normalizeLUT(_lutTexImage,_xLutMin,_xLutMax,_yLutMin,_yLutMax,_xLutScale,_yLutScale);

  return image;
}

/******************************************************************************/

void  LTLayer::normalizeLUT(osg::Image* img, float xmin,float xmax,float ymin,float ymax,float xscale,float yscale)
{
  float xf,yf,xc,yc,xm,ym;
  //static int done=0;
  int width,height;
  float v;

  printf("NORMALIZE    DONE=%d\n",_lutNormalized);
  if( _lutNormalized ) return;


  //xf=-xmin/(xmax-xmin)*xscale;
  //yf=-ymin/(ymax-ymin)*yscale;
  //xc=xscale/(xmax-xmin);
  //yc=yscale/(ymax-ymin);

  xm=xscale;
  ym=yscale;

  xf=-xmin/(xmax-xmin);
  yf=-ymin/(ymax-ymin);
  xc=1.0/(xmax-xmin);
  yc=1.0/(ymax-ymin);

  width=img->s();
  height=img->t();

  float *data = (float*)img->data();
  for( int i = 0; i < height; i++ )
    for( int j = 0; j < width; j++ ) {
      v=data[width*2*i + 2*j];
      //v=(v*xc+xf)-xm*((int)((v*xc+xf)/xm));
      v=(v*xc+xf);
      data[width*2*i + 2*j]=v;

      v=data[width*2*i + 2*j + 1];
      //v=(v*yc+yf)-ym*((int)((v*yc+yf)/ym));
      v=(v*yc+yf);
      data[width*2*i + 2*j + 1]=v;
    }

  _lutNormalized = true;

}


/******************************************************************************/

void LTLayer::enableLUT( bool b ) 
{
    _enableLUTUF->set(b);
}


/******************************************************************************/

int LTLayer::loadGLSLShader( const char* fileName, osg::StateSet* stateset )
{
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    _shaderProgram = new osg::Program;
    osg::ref_ptr<osg::Shader> fragObj = new osg::Shader( osg::Shader::FRAGMENT );
    printf( "loading shader...\n" );
    _shaderProgram->addShader( fragObj.get() );
    if (fileName[0]=='\0' || fragObj->loadShaderSourceFromFile( fileName )==false) 
    {
        printf("ERROR loading shader.\n");
        exit(-1);
    }
    printf( "loading shader... done\n" );
    stateset->setAttributeAndModes( _shaderProgram.get(), osg::StateAttribute::ON );

    printf("[ltlayer] Returning from loadGLSLShader\n");
    return 0;

}

/******************************************************************************/

int LTLayer::loadGLSLShaderSource( const char* source, osg::StateSet* stateset )
{
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    _shaderProgram = new osg::Program;
    osg::ref_ptr<osg::Shader> fragObj = new osg::Shader( osg::Shader::FRAGMENT );
    _shaderProgram->addShader( fragObj.get() );
    fragObj->setShaderSource( source );
    printf( "loading shader... done\n" );
    stateset->setAttributeAndModes( _shaderProgram.get(), osg::StateAttribute::ON );

    return true;
}

/**********************************************************/
