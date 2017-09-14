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

#include <osg/TexMat>
#include "ltcapturelayer.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
class CapLayerTexCallback : public  osg::NodeCallback
{
public:
  CapLayerTexCallback(LTCaptureLayer* cl)
    : osg::NodeCallback(),
      _cl( cl )
  {

  }

  virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
    // printf( "Callback [%s]\n", _tvl->getName().c_str() );
    _cl->nodeCallback();
    traverse(node,nv);
  }

  LTCaptureLayer* _cl;
};

/******************************************************************************/

//int capCallback(capture *cap,imgu **I,int i,int err);

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

rqueue* Qrecycle;
rqueue* Qcam;
int tid_cam;

LTCaptureLayer::LTCaptureLayer( const char* shaderFile, int camid, int exposure, int fps, 
								float xTexMin, float xTexMax, float yTexMin, float yTexMax) 
	: LTLayer( shaderFile, createTexture(512, 512), xTexMin, xTexMax, yTexMin, yTexMax, true) {

	int tid_cam;	// thread id of pattern_camera plugin
	int i,k;
/*
	// create a recycling queue
	Qrecycle=imguRegisterQueue("recycle");
	
	// create the camera image queue
	Qcam=imguRegisterQueue("cam");

	// build the camera plugin command
	char cmd[200];
	sprintf(cmd,"-in recycle -out cam");
	if (camid>=0)
		sprintf(cmd,"%s -camid 0",cmd,camid);
	if (fps>=0)
		sprintf(cmd,"%s -fps %i.0",cmd,fps);

	// create the camera capture plugin
	// ON UTILISE PLUS LE CMD POUR L'INSTANT
	tid_cam=imguStartPlugin("gige","camera","-in recycle -out cam -camid 0 -mcast 1");
	if( tid_cam<0 ) { printf("Could not start the pattern plugin\n");exit(-1); }

	// get access to plugin parameters
	paramlist *pl_cam=imguGetPluginParameters(tid_cam);
	// start the camera
	paramInvokeCommand(pl_cam,"START");
	// the given exposure value is in milliseconds 
	// but has to be changed to nano seconds for the gige plugin
	//paramSetInt(pl_cam,"ExposureValue",exposure*1000);

	// create a couple of empty images in the recycling queue
	// this is the maximum number of images circulating at a single time
	imgu *IA=NULL;
	for(i=0;i<10;i++) RQueueAddLast(Qrecycle,(unsigned char *)&IA);
	
  char capID[30];

  setName( "LTCaptureLayer" );
  printf( "LTCaptureLayer ctor\n" );
  _stopCallback = false;
  _latestImg = NULL;

  this->initScannerEffect(-1.0,1.0,0.2,0.1);
  this->initFreezeEffect();

  this->setUpdateCallback( new CapLayerTexCallback(this) );

  //k = sscanf( capStr, " %[^:]:%s", _capDevice, capID );

  osg::Texture2D* tr = new osg::Texture2D();
  tr->setResizeNonPowerOfTwoHint(false);
  //  tr->setImage( img1.get()  );
  tr->setImage( _capImg );
  tr->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  tr->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
  //tr->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::MIRROR); //REPEAT);
  tr->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER); //REPEAT);
  tr->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
  _texture = tr;

  osg::StateSet* ss = _hud->getStateSet();
  ss->setTextureAttributeAndModes( 0, _texture, osg::StateAttribute::ON );

  osg::TexMat* tm = new osg::TexMat( osg::Matrix::identity());
  tm->setScaleByTextureRectangleSize(true);
  ss->setTextureAttributeAndModes(0, tm, osg::StateAttribute::ON);
*/
  printf( "LTCaptureLayer ctor done\n" );
}


/******************************************************************************/

LTCaptureLayer::~LTCaptureLayer() {
	_stopCallback=true;	
	imguStopPlugin(tid_cam);

	imguUnregisterQueue("recycle",NULL);
	imguUnregisterQueue("cam",NULL);

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*int capCallback(capture *cap,imgu **img,int i,int err) {

  LTCaptureLayer *CL = static_cast<LTCaptureLayer*>(cap->userData);
  //printf("capCallback...........! %d %d\n", i, err);
  if( err ) printf("capCallback! %d %d\n", i, err);

  // on sait que c'est du 8bit... donc on doit faire <<8
  int j;
  imgu* I = img[i];
  unsigned short *p=I->data;
  j=I->xs*I->ys*I->cs;
  //while( --j ) *p++<<=4;
  while( --j ) *p++<<=8; // des bytes en short

  CL->processScanCB(I);
  CL->processFreezeCB(I);

  //static_cast<LTCaptureLayer*>(cap->userData)->setLatestImg(I);
  CL->setLatestImg(I);


  return 1;

}*/

/******************************************************************************/

void LTCaptureLayer::nodeCallback() {

	int i,j,k;
	imgu* I =NULL;
	imgu* IZ=NULL;

	if (RQueueRemoveLast(Qcam,(unsigned char *)&I)!=0)
		return;
	while(RQueueRemoveLast(Qcam,(unsigned char *)&IZ)==0)
		imguRecycle(IZ);

	this->processScanCB(I);
	this->processFreezeCB(I);
	
	this->setLatestImg(I);
	
	imguRecycle(I);
	
	if( this->freezeON==0 || ( this->freezeON==1 && freezeFade>0.0)  ) updateImage(); // 0 and 2 -> update
}

/******************************************************************************/

void LTCaptureLayer::setLatestImg( imgu* i ) {

  if (i != NULL) _latestImg = i;

}

/******************************************************************************/

void LTCaptureLayer::updateImage() {

  if (_stopCallback) return;

  imgu* I = _latestImg;
  if (I == NULL) return;
  if( I->cs==1 ) {
    _capImg->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,
                      (unsigned char *) I->data, osg::Image::NO_DELETE, 1);
  }else if( I->cs==3 ) {
    _capImg->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT,
                      (unsigned char *) I->data, osg::Image::NO_DELETE, 1);
  }else if( I->cs==4 ) {
    _capImg->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,
                      (unsigned char *) I->data, osg::Image::NO_DELETE, 1);
  }

  _latestImg = NULL;

}

/******************************************************************************/
/******************************************************************************/

/** freeze effect **/

void LTCaptureLayer::initFreezeEffect(void)
{
  this->freezeON=0;
  this->freezeFade=0.0;
  this->lastFreeze=NULL;
}

void LTCaptureLayer::freezeEffect(int status)
{
  if( status ) {
    // start freeze
    this->freezeON=2;
    this->freezeFade=1.0;
  }else{
    this->freezeON=0;
  }
}

void freezeDiff(imgu *I,imgu *Ifreeze,double fade)
{
  int i;
  //int a,b,v;
  //double ratio,L;
  unsigned short *p=Ifreeze->data;
  //unsigned short *q=I->data;
  int sz=I->xs*I->ys*I->cs;
  if( fade<0.0 || fade>1.0 ) return;
  for(i=0;i<sz;i++) {
    I->data[i]=I->data[i]*fade+(1-fade)*p[i];
  }
}

// I est l'image courante qu'on vient de recevoir dans le CB
void LTCaptureLayer::processFreezeCB(imgu *I)
{
  // freeze effet
  if( freezeON==2 ) {
    // on sauvegarde la prochaine image comme une nouvelle reference
    imguCopy(&lastFreeze,I);
    freezeON=1;
  }
  if( freezeON==1 && freezeFade>0.0 ) {
    freezeDiff(I,lastFreeze,freezeFade);
    freezeFade-=0.02; // fadeout to freeze
  }
}



/******************************************************************************/
/******************************************************************************/

/*** scanner effect ***/


void LTCaptureLayer::initScannerEffect(double Lmin,double Lmax,double StepIn,double StepOut)
{
  this->LimiteMin=Lmin;
  this->LimiteMax=Lmax;
  this->StepIn=StepIn;
  this->StepOut=StepOut;

  this->lastScan=NULL;
  this->scanMode=0;
  this->scanFade=0.0;
}

void LTCaptureLayer::scannerEffect(int status)
{
  if( status ) {
    // start effect
    if( scanMode==0 ) scanFade=0.0;
    scanMode=1; // get next image!
  }else{
    // stop effect
    printf("STOP scanMode=%d\n",scanMode);
    if( scanMode==2 ) scanFade=1.0; else scanFade=0.0;
    scanMode=0;
    printf("setting mode to 0 and fade to %f\n",scanFade);
  }
}



int LTCaptureLayer::scanComputeDiff(imgu *I,imgu *Ir,double fade)
{
  int i;
  int a,b,v;
  //double ratio;
  double L;
  int sz=I->xs*I->ys*I->cs;
  unsigned short *p=Ir->data;
  unsigned short *q=I->data;
  double ddd=(LimiteMax-LimiteMin)/65535.0;

  for(i=0;i<sz;i++,p++,q++) {
    a= *p;
    b= *q;
    if( a==0 ) a=1;
    if( b==0 ) b=1;
    L=log2((double)b/(double)a);
    //v=(L-LimiteMin)/(LimiteMax-LimiteMin)*65535;
    v=(L-LimiteMin)/ddd;
    if( v<0 ) v=0;
    if( v>65535 ) v=65535;
    if( fade<1.0 ) *q= *q * (1-fade)+fade*v;
    else *q=v;
  }

  return 0;

}



// I est l'image courante qu'on vient de recevoir dans le CB
void LTCaptureLayer::processScanCB(imgu *I)
{
  // scanner effet
  if( scanMode==1 ) {
    // on sauvegarde la prochaine image comme une nouvelle reference
    imguCopy(&lastScan,I);
    scanMode=2;
  }
  if( scanMode==2 ) {
    scanComputeDiff(I,lastScan,scanFade);
    scanFade+=StepIn; // fadein
  }else if( scanFade>0.0 && scanMode==0 ) {
    scanComputeDiff(I,lastScan,scanFade);
    scanFade-=StepOut; // fadein
  }
}


