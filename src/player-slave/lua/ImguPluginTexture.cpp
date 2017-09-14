/* osgtestvideo
 * test video texture
*/

#include "ImguPluginTexture.h"


// Normalement, si plusieurs images sont dans la queue movie, ou joue le last
// et on flush le reste... (LIFO) sinon, on joue toutes les images (pas LIFO)
#define LIFO

//
// this is where the magic happens...
// the callback is called at every frame
// it can check if anything is available in the queue, and change the texture if needed
// The images are recycled when a new one arrives
//
// tm can be NULL if you don't care about rescaling tex coords
//

class VideoImageUpdateCallback : public osg::StateAttribute::Callback
{
public:

    VideoImageUpdateCallback(osg::Texture2D* texture,osg::Image *im,rqueue *Q):
        _texture(texture)
    {
        _Qlast=Q;
        _img=im; //new osg::Image();
        _currentI=NULL;

        printf("video update callback started\n");
    }

    ~VideoImageUpdateCallback() {
        printf("DESTRUCTEUR VideoImageUpdateCallback\n");
    }

    virtual void operator()(osg::StateAttribute* sa, osg::NodeVisitor* nv)
    {
        imgu *I;
        imgu *J;

    //printf("********** VIDEO Update CALLBACK!!!!\n");

    if( sa==NULL ) { printf("SKIP!!!!!!!!!!!!!!!!!!!\n"); return; }


        // is there an image in the queue?
#ifdef LIFO
        if( RQueueRemoveLast(_Qlast,(unsigned char *)&I) ) return; // no image!
        // flush all other images. Anyway, its too late to display them.
        while( RQueueRemoveFirst(_Qlast,(unsigned char *)&J)==0 ) {
            //int frameNum=atoi(imguGetText(J,"FRAMENUM"));
            imguRecycle(J);
        }
#else
        if( RQueueRemoveFirst(_Qlast,(unsigned char *)&I) ) return; // no image!
#endif

    // send the image to the osgImage, so the texture is refreshed
    //int frameNum=atoi(imguGetText(I,"FRAMENUM"));

#ifdef IMGU8
    if( I->cs==1 ) {
        _img->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==2 ) {
        _img->setImage(I->xs,I->ys, 1, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==3 ) {
        _img->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==4 ) {
        _img->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }
#else
    if( I->cs==1 ) {
        _img->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==2 ) {
        _img->setImage(I->xs,I->ys, 1, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==3 ) {
        _img->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==4 ) {
        _img->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }
#endif

        // free the current image
        if( _currentI ) imguRecycle(_currentI);
        // remember image I as the current one
        _currentI=I;

        //_texture->setImage(_img);

        //printf("Image is %d x %d\n",_img->s(),_img->t());

        // ajuste la textmat pour avoir des coord 0..1 plutot que 0..xs
        //if( _tm ) _tm->setMatrix(osg::Matrix::scale(I->xs,I->ys,1.0));

        //_texture->dirtyTextureObject();
    }

protected:
    osg::Image*         _img;
    osg::Texture2D*     _texture;
    imgu*               _currentI; // image currently used in the texture
    rqueue*             _Qlast; // last plugin queue from which we take the images
};


/******************************************************************************/

ImguPluginTexture::ImguPluginTexture() {

    _nbPlugins=0;
    sprintf(_queueSuffix,"0x%08x",(unsigned int)this);

    printf("Creating new video texture\n");

    // create a send and receive queue
    // the Qrecycle queue is local to us, for recycling.
    _Qrecycle=imguRegisterQueue((char *)"recycle");

    /*
    // add a few images for recycling. Decoding will block when out of images
    imgu *IA=NULL;
    int i;
    for(i=0;i<10;i++) RQueueAddLast(_Qrecycle,(unsigned char *)&IA);
    */

    // add a local texture matrix to scale the coordinates, if desired
    // this MUST be added as an attribute at the same time the texture is added.
    _im=new osg::Image();
    _im->setDataVariance(osg::Object::DYNAMIC);

    this->setImage(_im);

}

/******************************************************************************/

ImguPluginTexture::~ImguPluginTexture()
{
    int i;
    //imguPluginDump();
    printf("destructor imguvideotexture\n");

    printf("[imguplugintexture] Stoping plugins... (nbplugins=%d)\n",_nbPlugins);
    for(i=0;i<_nbPlugins;i++) imguStopPlugin(_tids[i]);

    // ICI on devrait nettoyer la queue de recyclage... a faire!

    printf("[imguplugintexture] Unregistering queues...\n");
    imguUnregisterQueue((char *)"recycle",NULL);
    for(i=0;i<_nbPlugins;i++) imguUnregisterQueue(NULL,_Qplugins[i]);

    printf("[imguplugintexture] done.\n");
    //imguPluginDump();
}


/******************************************************************************/
// le apply de la texture doit appliquer la texmat
void ImguPluginTexture::apply(osg::State& state) const
{
  osg::Texture2D::apply(state);
}


/******************************************************************************/

int ImguPluginTexture::addPlugin(const char *name, const char *classe, const char *in_params) {

    char Qname_out[30];
    char Qname_in[30];

    sprintf(Qname_out,"queue%02d_%s",_nbPlugins,_queueSuffix);
    _Qplugins[_nbPlugins] = imguRegisterQueue(Qname_out);

    char params[300];
    if (_nbPlugins==0) {
      //sprintf(params, "-in recycle -out %s %s", Qname_out, in_params);
	sprintf(Qname_in,"recycle%02d_%s",_nbPlugins,_queueSuffix);
	rqueue *Qtmp = imguRegisterQueue(Qname_in);
	sprintf(params, "-in %s -out %s %s", Qname_in, Qname_out, in_params);


	// add a few images for recycling. Decoding will block when out of images
	imgu *IA=NULL;
	int i;
	for(i=0;i<10;i++) RQueueAddLast(Qtmp,(unsigned char *)&IA);

    } else {
        sprintf(Qname_in,"queue%02d_%s",_nbPlugins-1,_queueSuffix);
        sprintf(params, "-in %s -out %s %s", Qname_in, Qname_out, in_params);
    }

    printf("[ImguPluginTexture] ADDED PLUGIN: %s %s %s\n",name, classe, params);
    _tids[_nbPlugins] = imguStartPlugin(name,classe,params);

    if( _tids[_nbPlugins]<0 ) {
        printf("[ImguPluginTexture] Unable to start added plugin %s : %s : %s\n",name,classe,params);
        return(-1);
    }

    if (strncmp(classe,"camera",6)==0) {
        paramlist *pl_cam=imguGetPluginParameters(_tids[_nbPlugins]);
        paramInvokeCommand(pl_cam,"START");
    }

    _nbPlugins++;

    return 0;
}

/******************************************************************************/

int ImguPluginTexture::doneAddingPlugins() {
    //imguPluginDump();

    if (_nbPlugins==0) {
        printf("ImguPluginTexture requires at least one plugin.\n");
        return -1;
    }

    // set the update callback to cycle through the various min and mag filter modes.
    this->setUpdateCallback(new VideoImageUpdateCallback(this,_im,_Qplugins[_nbPlugins-1]));
    this->setDataVariance(osg::Object::DYNAMIC);
    this->setFilter(osg::Texture::MIN_FILTER,osg::Texture2D::LINEAR);
    this->setFilter(osg::Texture::MAG_FILTER,osg::Texture2D::LINEAR);

    //imguPluginDump();
    return(0);
}

/******************************************************************************/

int ImguPluginTexture::setPluginParam(const char* pluginName, char *param_name, char *param_value) {

    imguPluginInfo *pi;
    int found = -1;

    //printf("imguPluginInfo::setPluginParam by name: [%s] [%s] [%s]\n",
    //       pluginName, param_name, param_value);

    for (int i = 0; i < _nbPlugins; i++) {
        pi = imguGetPluginInfo(_tids[i]);
        if ( strcmp(pi->name, pluginName) == 0 ) {
            found = i;
            break;
        }
    }

    //printf("shit?\n");
    if (found >= 0) {
        paramlist *pl=imguGetPluginParameters(_tids[found]);
        //printf("setting param....\n");
        return paramSetString(pl,param_name,param_value);
    }

    //printf("shit\n");
    return -1;
}


int ImguPluginTexture::setPluginParam(int idx_plugin, char *param_name, char *param_value) {
    paramlist *pl=imguGetPluginParameters(_tids[idx_plugin]);
    return paramSetString(pl,param_name,param_value);
}

/******************************************************************************/

int ImguPluginTexture::invokePluginCommand(const char* pluginName, char *cmd) {

    imguPluginInfo *pi;
    int found = -1;
    for (int i = 0; i < _nbPlugins; i++) {
        pi = imguGetPluginInfo(_tids[i]);
        if ( strcmp(pi->name, pluginName) == 0 ) {
            found = i;
            break;
        }
    }

    if (found >= 0) {
        paramlist *pl=imguGetPluginParameters(_tids[found]);
        return paramInvokeCommand(pl,cmd);
    }
    return -1;
}


int ImguPluginTexture::invokePluginCommand(int idx_plugin, char *cmd) {
    paramlist *pl=imguGetPluginParameters(_tids[idx_plugin]);
    return paramInvokeCommand(pl,cmd);
}

/******************************************************************************/
