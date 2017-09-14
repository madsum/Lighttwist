
#include "ltvideolayer.h"
#include "ltvideotexture.h"

/******************************************************************************/
class LTVideoCtlCallback : public  osg::NodeCallback
{
public:
  LTVideoCtlCallback(LTVideoController* vctl)
    : osg::NodeCallback()
  {
    _vctl = vctl;

  }

  virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
    //printf( "Callback [%s] (go!)\n", _tvl->getName().c_str() );
    _vctl->go();
    traverse(node,nv);
  }

  LTVideoController* _vctl;
};

/******************************************************************************/

LTVideoLayer::LTVideoLayer( const char* shaderFile, LTVideoTexture2D* vt )
    : LTLayer( shaderFile, vt, vt->getXRatio(), vt->getYRatio(),
               _xLutMin, _xLutMax, _yLutMin, _yLutMax, true ),
      LTVideoController()
{

    if (!vt) {
        printf("LTVideoLayer: borked!\n");
        _broken = true;
        return;
    }

    addVideoTexture( vt );
    setUpdateCallback( new LTVideoCtlCallback(this) );

}

/******************************************************************************/

LTVideoLayer::~LTVideoLayer()
{


    printf("LTVideoLayer destroyed... \n");


}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
