

//
// module play
//
// contient les commandes interfaces avec le player
// ces commandes ne sont pas appellees directement de l'exterieur
// mais plutot de l'interieur (void commands.lua)
//
// En gros:
// le player recoit des commandes a executer.
// il execute directement les commandes dans un interpreteur lua interne.
// Normalement, les commandes sont definiee dans un fichier, genre commands.lua
// La realisation physique passe par des appels aux fonctions du player ici.
// La librarie lua player interface entre les commandes lua recues et l'affichage.
//

#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgDB/WriteFile>
#include <rqueue.h>


#include "luawrapfunctions.h"

#define VERBOSE

///////////
/////////// variables globales definies dans play.h
///////////

int blub[2];

osg::ref_ptr<LTLayer> layers[NB_LAYER]; // valid() dit si actif
LTVideoController* vidCtrls[NB_LAYER];

char dataPath[200];

osg::Group* rootNode;
osg::ClearNode* clearNode;

int machine_num;

char SHADER_FILM_PATH[255];
char SHADER_3D_PATH[255];
char SHADER_STEREO_VERT_PATH[255];
char SHADER_STEREO_FRAG_PATH[255];
int STEREO_MODE;

float xTexMin,xTexMax,yTexMin,yTexMax;

double  timeOffset; // reference_time = local_time() + timeOffset


/******************************************************************************/

int quit() {
    exit(0);
}

/******************************************************************************/

int set_alpha_mask( int num, const char* file ) {


    if( num<0 || num>=NB_LAYER ) return(-1);
    if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }


    osg::Image* img;

    char buf[250];
    sprintf(buf, "%s/%s", dataPath, file);
    img = osgDB::readImageFile( buf );

    if (!img) {
        printf("image [%s] not found\n", file);
        return 0;
    }

    layers[num]->setAlphaMask( img );
    return(0);

}


/******************************************************************************/

int enableLUT( int num, bool b ) {

    if ( num<0 || num>=NB_LAYER ) return(-1);
    if ( !layers[num].valid() )  return(-1);

    layers[num]->enableLUT(b);

    return 0;

}

/******************************************************************************/

int load_lut_blend(const char* lutName, const char* blendName) {

    printf("load_lut_blend: %f, %f, %f, %f BEFORE\n", xTexMin,xTexMax,yTexMin,yTexMax);

    lutTexImage = LTLayer::loadLUT( lutName, &xTexMin, &xTexMax,
                                    &yTexMin, &yTexMax, voffsetx );
    lutTexImage->flipVertical();
    lutTex = LTLayer::createLutTexFromImg( lutTexImage.get() );

    blendTexImage = osgDB::readImageFile( blendName );
    blendTex = LTLayer::createBlendTexFromImg( blendTexImage.get() );

    printf("load_lut_blend: %f, %f, %f, %f AFTER\n", xTexMin,xTexMax,yTexMin,yTexMax);

    for (int i = 0; i < NB_LAYER; i++) {
        printf("reset [%i]\n", i);
        if( layers[i].valid() ) {
            printf("valid\n");
            layers[i]->reset();
        }
    }



    return 0;

}

/******************************************************************************/


int fact(int n) {
  if (n <= 1) return 1;
  else return n*fact(n-1);
}

/******************************************************************************/


void *sebcreate(int a,int b)
{
    printf("create: a is %d, b is %d\n",a,b);
    blub[0]=a;
    blub[1]=b;
    return(blub);
}

int sebprocess(void *z)
{
    int *q=(int *)z;
    int a=q[0];
    int b=q[1];
    printf("process: a is %d, b is %d\n",a,b);
    return(a+b);
}


/******************************************************************************/

//
// unload_layer
//
int unload_layer(int num)
{
#ifdef VERBOSE
  printf("unloading layer %d\n",num);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);

  // flush l'ancienne texture et cyclo
  if( layers[num].valid() ) {
#ifdef VERBOSE
    printf("flushing layer %d\n",num);
#endif
    //THIS USLEEP SHOULD IDEALLY BE REMOVED!
    //it seems that some operations use the cyclo layer, give them time to end
    printf("sleep?\n");
    usleep(200000);
    //rootNode->removeChild(layers[num].get());
    printf("setChild new node..\n");
    rootNode->setChild( NB_LAYER-num-1, new osg::Node() );
    printf("everything null\n");
    layers[num]=NULL;
    vidCtrls[num] = NULL;
  }else{
#ifdef VERBOSE
    printf("no need to flush layer %d\n",num);
#endif
  }
  return(0);
}


/******************************************************************************/

int unload_all() {

  for (int i = 0; i < NB_LAYER; i++) {
    if( layers[i].valid() ) {
      rootNode->setChild( NB_LAYER-i-1, new osg::Node() );
      layers[i]=NULL;
      vidCtrls[i] = NULL;
    }
  }
  
  usleep(200000);
  return 0;

}


/******************************************************************************/


//
// load_layer_image
//
int load_layer_image(int num,char *img_name, int swrap, int twrap)
{
  char img_path[200];

  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  sprintf(img_path,"%s/%s",dataPath,img_name);

  printf("loading layer %d image '%s' (machine %d)\n",num, img_path ,machine_num);

  osg::Texture2D* tex  = new  osg::Texture2D;
  osg::ref_ptr<osg::Image> img1 = osgDB::readImageFile( img_path );

  if( !img1 ) {
    //#ifdef VERBOSE
        printf("load_layer_image: Can't allocate image\n");
  //#endif
        return 0;
  }

  tex->setInternalFormat(GL_RGBA);
  tex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
  tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

  tex->setResizeNonPowerOfTwoHint(false);

  if (swrap) tex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
  else tex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_BORDER);

  if (twrap) tex->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
  else tex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_BORDER);

  tex->setImage(img1.get());

  printf("load_layer_image: texture size = %i x %i\n", tex->getTextureWidth() , tex->getTextureHeight() );
  printf("load_layer_image: image size = %i x %i\n", img1->s(),img1->t() );

  //printf("SHADER is '%s'\n",SHADER_FILM_PATH);

  layers[num] = new LTLayer(SHADER_FILM_PATH,tex, 1.0, 1.0,
                                    xTexMin, xTexMax, yTexMin, yTexMax, true);

  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; //pas beau
  //rootNode->addChild(layers[num].get());
  rootNode->setChild(  NB_LAYER-num-1, layers[num].get() );
  return(0);
}

/******************************************************************************/


//
// load a movie in a specific layer
//

int load_layer_tiled_movie(int num,char *movie_name)
{
  char temp[200];
  char movie_path[200];

  printf("loading layer %d movie '%s' (machine %d)\n",num,movie_name,machine_num);

  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  if( machine_num>=0 ) {
    sprintf(temp,movie_name,machine_num);
  }else{
    strcpy(temp,movie_name);
  }

  if (dataPath[0]=='\0') strcpy(movie_path,temp);
  else sprintf(movie_path,"%s/%s",dataPath,temp);

  layers[num] = new LTTiledVideoLayer(movie_path, SHADER_FILM_PATH, 2048, 2048);
  if ( !layers[num]->isBroken() ) {
      vidCtrls[num] = dynamic_cast<LTVideoController*>(layers[num].get()); // pas beau.  need to rethink this
      layers[num]->setFade(0.0); // default invisible
      rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
  } else {
      unload_layer(num);
  }

  return(0);
}



/******************************************************************************/

int load_layer_movie( int num, char* movie_name, int swrap, int twrap ) {

    char temp[200];
    char movie_path[200];

    printf("loading layer %d movie '%s' (machine %d)\n",num,movie_name,machine_num);

    if( num<0 || num>=NB_LAYER ) return(-1);

    unload_layer(num);

    if( machine_num>=0 ) {
        sprintf(temp,movie_name,machine_num);
    }else{
        strcpy(temp,movie_name);
    }

    if (dataPath[0]=='\0') strcpy(movie_path,temp);
    else sprintf(movie_path,"%s/%s",dataPath,temp);

    LTVideoTexture2D* vt = new LTVideoTexture2D(movie_path, 1, 1);
    layers[num] = new LTVideoLayer(SHADER_FILM_PATH, vt);

    if (swrap) layers[num]->getTexture()->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
    if (twrap) layers[num]->getTexture()->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);

    if ( !layers[num]->isBroken() ) {
        vidCtrls[num] = dynamic_cast<LTVideoController*>(layers[num].get()); // pas beau.  need to rethink this
        layers[num]->setFade(0.0); // default invisible
        rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
    } else {
        unload_layer(num);
    }

}

/******************************************************************************/

int set_color(int num, float r, float g, float b, float a) {


    if( num<0 || num>=NB_LAYER ) return(-1);
    if( !layers[num].valid() ) return(-1);

    LTColorLayer* l = dynamic_cast<LTColorLayer*>(layers[num].get());
    if (!l) return -1;

    l->setColor(r, g, b, a);

    return 0;
}


/******************************************************************************/

//
// prochain frame d'un video!!
//
int frame(int num,int fr)
{
  //printf("frame layer %d, frame=%d\n",num,fr);

  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) return(-1);

  if (vidCtrls[num] != NULL) vidCtrls[num]->setNextFrame(fr);
  return(0);
}



/******************************************************************************/

double getFPS(int num) {
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) return(-1);
  if (vidCtrls[num] != NULL) return vidCtrls[num]->getFPS();
  return(30.0);

}


/******************************************************************************/


int load_layer_multi3d(int num,char *model_name)
{

  printf("---------load_layer_multi3d inside LUAWRAPFUNCTION\n");

  char model_path[200];

  printf("loading layer %d model '%s' (machine %d)\n",num,model_name,machine_num);

  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  sprintf(model_path,"%s/%s",dataPath,model_name);

  osgDB::ReaderWriter::Options* opts = new osgDB::ReaderWriter::Options();
  osgDB::FilePathList fpl = opts->getDatabasePathList();
  fpl.push_back( "." );
  fpl.push_back( dataPath );
  fpl.push_back( osgDB::getFilePath(model_path) );

  /*for (int i = 0; i < fpl.size(); i++) {
    printf("FilePathList[]%i: %s\n", i, fpl[i].c_str() );
    }*/

  osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(model_path, opts);
  if( !loadedModel.valid() ) {
#ifdef VERBOSE
    printf("Unable to load model\n");
#endif
    return(-1);
  }
  /*
  printf("generateDefaultScene...\n");
  loadedModel = LT3DLayer::generateDefaultScene();
  printf("writeNodeFile...\n");
  osgDB::writeNodeFile(*loadedModel, std::string("test_scene.osg"));
  exit(0);*/

  osg::ref_ptr<osg::MatrixTransform> metsCaDrette = new osg::MatrixTransform();
  metsCaDrette->setMatrix(osg::Matrix::rotate(0, 1.0f, 0.0f, 0.0f ));// M_PI/2.0,
  metsCaDrette->addChild( loadedModel.get() );

  LTMulti3DLayer* m3dl = new LTMulti3DLayer(SHADER_3D_PATH, metsCaDrette.get(), 2048, 2048, 25.0, STEREO_MODE);
  printf("----------------CREATING STEREO 3D LAYER: stereo mode: %i\n",STEREO_MODE);
  if (STEREO_MODE!=STEREO_NONE) {
    printf("----------------CREATED STEREO 3D LAYER\n");
    m3dl->setSceneShaders(SHADER_STEREO_VERT_PATH,SHADER_STEREO_FRAG_PATH);
  }

  layers[num] = m3dl;
  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; // laid.  beurk.
  //rootNode->addChild(layers[num].get());
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

  return(0);
}

/******************************************************************************/

int load_layer_color(int num, float r, float g, float b, float a, int mode) {

    if( num<0 || num>=NB_LAYER ) return(-1);

    unload_layer(num);

    LTColorLayer* l = new LTColorLayer( r, g, b, a, mode );

    layers[num] = l;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return 0;
}

/******************************************************************************/
int load_layer_stream_capture(int num, int port, int mtu) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, " -host 226.0.0.1 -port %i -mtu %i ", port, mtu );
    IPL->addPlugin((char*)"streamin", (char*)"camera", str);
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}

/******************************************************************************/
int load_layer_capture(int num, int camid) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, "  -camid %i -mcast 1 ", camid );
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"gige", (char*)"camera", str);
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}

/******************************************************************************/
int load_layer_capture_with_blend(int num, int camid, double ratio) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, "  -camid %i -mcast 1 ", camid );
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"gige", (char*)"camera", str);

    sprintf( str, " -ratio %f ", ratio );
    IPL->addPlugin((char*)"frameblend", (char*)"filter", str);
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}
/******************************************************************************/
int load_layer_capture_with_inverse(int num, int camid) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, "  -camid %i -mcast 1 ", camid );
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"gige", (char*)"camera", str);
    IPL->addPlugin((char*)"inverse", (char*)"filter", (char*)"");
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}


/******************************************************************************/
int load_layer_capture_with_distorsion(int num, int camid, char* map) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, "  -camid %i -mcast 1 ", camid );
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"gige", (char*)"camera", str);

    imgu* I=NULL;
    imguLoad(&I,map,LOAD_AS_IS);
    printf("LOADED MAP:%s (%i,%i)\n",map,I->xs,I->ys);
    imguFree(&I);

    sprintf( str, " -map %s -width %i -height %i", map,I->xs,I->ys );
    IPL->addPlugin((char*)"distorsion", (char*)"filter", str);
    IPL->doneAddingPlugins();


    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}

int load_layer_camera_crayon(int num) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, " ");
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"crayon", (char*)"camera", str);
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}


/******************************************************************************/
int load_layer_capture_with_scanner(int num, int camid) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, "  -camid %i -mcast 1 ", camid );
    //IPL->addPlugin("gige", "camera"," -camid 13139 -mcast 1");
    IPL->addPlugin((char*)"gige", (char*)"camera", str);
    IPL->addPlugin((char*)"scannereffect", (char*)"filter", (char*)"");
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}

/******************************************************************************/
int load_layer_capture_with_plugins(int num, int camid, char* str_filters) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    unload_layer(num);

    // str_filters a la forme:
    // plugin_name<space>plugin_type[<space>plugin_parameters]:plugin_name<>...
    // e.g: "inverse filter:frameblend filter:distorsion gpu filter -map filepath -width x -height y:crayon"

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);

    char params[500];
    sprintf( params, "  -camid %i -mcast 1", camid );
    IPL->addPlugin((char*)"gige", (char*)"camera", params);

    /*
    char plugin_name[100];
    int plugin_namesize;
    char plugin_type[50];
    int plugin_typesize;
    int param_size;

    int i;
    char *token = strtok(str_filters, ":");

    while (token) {
      printf("=================== token: %s\n", token);

    char * type_start;
    type_start = strpbrk( token, " ");

    if (type_start!=NULL) {
        // Get the plugin name (first token)
        plugin_namesize=strlen(token)-strlen(type_start);
        memcpy( plugin_name, token, plugin_namesize );
        plugin_name[plugin_namesize]='\0';
        type_start+=1; // pour enlever l'espace

    } else {
        printf("[load_layer_with_plugins] Plugin type required (e.g. filter)\n");
        return -1;
    }

        // If the string contains a space after the plugin type, we know there are parameters
        char * param_start;
        param_start = strpbrk( type_start, " ");

    if (param_start!=NULL) {
        // Get the plugin type from the remaining string
        plugin_typesize=strlen(type_start)-strlen(param_start);
        memcpy( plugin_type, type_start, plugin_typesize );
        plugin_type[plugin_typesize]='\0';

        param_start+=1; // pour enlever l'espace

        // Get the plugin parameters from the remaining strin
        param_size = strlen(token)-plugin_namesize-plugin_typesize;
        memcpy( params, param_start, param_size );

        printf("param_size: %i\n",param_size);

        // Add the plugin to the plugin queue
        printf( "[load_layer_with_plugins] Calling plugin \"%s\" \"%s\" with params \"%s\"\n", plugin_name, plugin_type, params );
        IPL->addPlugin(plugin_name, plugin_type, params);

    } else {
        // Get the plugin type from the remaining string
        plugin_typesize = strlen(type_start);
        memcpy(plugin_type, type_start, plugin_typesize);
        plugin_type[plugin_typesize]='\0';

        // Add the plugin to the plugin queue
        printf( "[load_layer_with_plugins] Calling plugin \"%s\" \"%s\" with no params\n", plugin_name, plugin_type );
        IPL->addPlugin(plugin_name, plugin_type, (char*)"");
    }

    // Look for the next plugin request in the original string
        printf(" token before: %s\n", token );
    token = strtok(NULL, ":");
    printf(" token after: %s\n", token );

    printf(" str_filter: %s\n", str_filters);
    }
    */

    IPL->addPlugin((char*)"inverse", (char*)"filter", (char*)"");
    IPL->addPlugin((char*)"frameblend", (char*)"filter", (char*)"-ratio 0");
    IPL->addPlugin((char*)"scannereffect", (char*)"filter",(char*)"");
    //IPL->addPlugin((char*)"distorsion", (char*)"gpufilter", (char*)"-map /home/vision/svn3d/danieldanis/demo/distorsion/mosaique.png -width 659 -height 493");
    //IPL->addPlugin((char*)"crayon", (char*)"filter", (char*)"");
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}


/******************************************************************************/
int load_layer_plugin(int num, char* pname, char* pclass, char* param) {

    if( num<0 || num>=NB_LAYER ) return(-1);

    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);

    IPL->addPlugin(pname, pclass, param);
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);
}


/******************************************************************************/

int load_layer_add_plugin(int num, char* pname, char* pclass, char* param) {

    if( num<0 || num>=NB_LAYER ) return(-1);

    LTImguPluginLayer *IPL;
    int k = 0;
    if ( !layers[num].valid() ||
         (IPL = dynamic_cast<LTImguPluginLayer*>( layers[num].get() )) == NULL ) {

        unload_layer(num);
        IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
        IPL->setName((char*)"ipl");
        k = 1;
    }

    //printf("blorg\n");
    IPL->addPlugin(pname, pclass, param);
    IPL->doneAddingPlugins();
    //printf("bloug\n");
    if (k) {
        //printf("adding IPL to layers array...?  come on...\n");
        layers[num] = IPL;
        vidCtrls[num] = NULL;
        rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
    }

    ///IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());
    //if (!IPL) printf("damn..\n");
    //printf("name of layer [%s]\n", IPL->getName().c_str() );
    //printf("name of layer [%s] num=%i\n", layers[num]->getName().c_str(), num );
    return(0);

}

/******************************************************************************/

int load_layer_ultime(int num, int camid) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    char str[50];
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);
    sprintf( str, " -camid %i -mcast 1", camid );

    //IPL->addPlugin((char*)"gige", (char*)"camera", str);
    IPL->addPlugin((char*)"ffmpeg", (char*)"camera",
                   (char*)" -file /home/vision/lighttwist/data/chien2.mpg -fps 10 ");
    IPL->addPlugin((char*)"crayon", (char*)"filter", (char*)"");
    IPL->doneAddingPlugins();

    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);

}

/******************************************************************************/

int load_layer_dl1track(int num) {

    if( num<0 || num>=NB_LAYER ) return(-1);
    unload_layer(num);

    LTImguPluginLayer *IPL = new LTImguPluginLayer(SHADER_FILM_PATH,xTexMin,xTexMax,yTexMin,yTexMax);

    IPL->addPlugin( "v4l2", "camera", "-width 1000 -height 1000 -viewnum 0" );
    IPL->addPlugin( "dl1track", "filter", "-outwidth 1024 -outheight 768" );
    IPL->doneAddingPlugins();
    IPL->enableLUT(false);
    layers[num] = IPL;
    vidCtrls[num] = NULL;
    rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

    return(0);

}

/******************************************************************************/





int set_plugin_param(int num, const char* pluginName, char *param_name, char *param_value) {
    printf("luawrapfunction set_plugin_param by name : %i [%s] [%s] [%s]\n",
           num, pluginName, param_name, param_value);

    //if ( !layers[num].valid() ) printf("layer not valid\n");
    //printf("drumroll please...\n");
    //printf("name of layer [%s] please\n", layers[num]->getName().c_str() );
    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());
    //if ( !IPL ) printf("something is wrong\n");
    //else printf("ouatte de phoque\n");
    if (!IPL) return -1;
    return IPL->setPluginParam(pluginName,param_name,param_value);
}


int set_plugin_param(int num, int idx_plugin, char *param_name, char *param_value) {
    //printf("luawrapfunction set_plugin_param by index:  %i [%i] [%s] [%s]\n",
    //     num, idx_plugin, param_name, param_value);
    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());
    if (!IPL) return -1;
    return IPL->setPluginParam(idx_plugin,param_name,param_value);
}



/******************************************************************************/


int invoke_plugin_command(int num, const char* pluginName, char *cmd) {
    printf("Invoking command: %s on plugin %s on layer %i\n",cmd,pluginName,num);
    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());
    if (!IPL) return -1;
    return IPL->invokePluginCommand(pluginName,cmd);
}

int invoke_plugin_command(int num, int idx_plugin, char *cmd) {
    printf("Invoking command: %s on plugin %i on layer %i\n",cmd,idx_plugin,num);
    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());
    if (!IPL) return -1;
    return IPL->invokePluginCommand(idx_plugin,cmd);
}

/******************************************************************************/

/*
//int load_layer_capture(int num,char *capStr) {
int load_layer_capture(int num, int camid, int exposure, int fps) {

#ifdef VERBOSE
// printf("loading layer %d movie '%s' (machine %d)\n",num,movie,machine_num);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  layers[num] = new LTCaptureLayer(SHADER_FILM_PATH,camid,exposure,fps,xTexMin,xTexMax,yTexMin,yTexMax);
  vidCtrls[num] = NULL;
  layers[num]->setFade(1.0); // default visible
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
  return(0);
}
*/

/*
//
// effet scanned on/off 1/0
//
int capture_scan_effect(int num,int onoff)
{
  if( !layers[num].valid() ) return(-1);

  LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());

  printf("Scanner effect! OnOff = %d\n",onoff);
  IPL->scannerEffect(onoff);
  return(0);
}

//
// effet freeze on/off 1/0
//
int capture_freeze_effect(int num,int onoff)
{
  if( !layers[num].valid() ) return(-1);

  LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());

  printf("Freeze effect! OnOff = %d\n",onoff);
  IPL->freezeEffect(onoff);
  return(0);
}

int capture_inverse_effect(int num,int onoff) {
    if( !layers[num].valid() ) return(-1);

    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());

    printf("Inverse effect! OnOff = %d\n",onoff);
    IPL->inverseEffect(onoff);
    return(0);
}
*/
/*
int capture_canny_effect(int num,int onoff) {
    if( !layers[num].valid() ) return(-1);

    LTImguPluginLayer *IPL = dynamic_cast<LTImguPluginLayer*>(layers[num].get());

    printf("Inverse effect! OnOff = %d\n",onoff);
    IPL->cannyEffect(onoff);
    return(0);
}
*/

/******************************************************************************/

//
// fade un layer
//
//

int fade_out_all(float f) {

    for (int i = 0; i < NB_LAYER; i++) {
        if( layers[i].valid() ) {
            if ( f < layers[i]->getFade() )
            layers[i]->setFade(f);
        }
    }
}

int fade_in_all(float f) {

    for (int i = 0; i < NB_LAYER; i++) {
        if( layers[i].valid() ) {
            if ( f > layers[i]->getFade() )
            layers[i]->setFade(f);
        }
    }
}

int fade(int num,float fd)
{
  printf("fade layer %d, fade=%f\n",num,fd);
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  layers[num]->setFade(fd);
  return(0);
}

int set_clear_node(double r, double g, double b) {
    clearNode->setClearColor(osg::Vec4(r,g,b,1.0));
    //printf("set_clear_node(%f,%f,%f,1.0)\n",r,g,b);
    return(0);
}

/********************************************************************/

double getLocalTime()
{
struct timeval tv;
  double stop;
  gettimeofday(&tv,NULL);
  stop=(double)tv.tv_sec+ (double)tv.tv_usec / 1000000.0;
  return(stop);
}


double getTime()
{
  return( getLocalTime()+timeOffset );
}


void setTime(double t)
{
  timeOffset= t - getLocalTime();
  printf("time=%12.6f local_time=%12.6f  offset=%12.6f\n",t,getLocalTime(),timeOffset);
}




/******************************************************************************/

int scaletrans(int num, float xs, float ys, float xt, float yt) {

  printf("setScaleTrans %i: %f %f %f %f\n", num, xs, ys, xt, yt);

  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  layers[num]->setScaleTrans(xs, ys, xt, yt);
  return(0);

}

/******************************************************************************/
/******************** CRAYON !!!!!!!!!!!!! ************************************/
/******************************************************************************/

int addPoints();
int queuepoint(double x, double y, double z);

#define POINTQ_SIZE 300
rqueue pointQ;
bool qInitialized = false;
osg::Node* crayon = NULL;
osg::MatrixTransform* mtSpin = NULL;
osg::MatrixTransform* mtStill = NULL;
LTMulti3DLayer* crayonLayer = NULL;

typedef struct {
  double x;
  double y;
  double z;
} v3d;

/******************************************************************************/

class NodeFinder: public osg::NodeVisitor
{
public:
  const char* _name;
  osg::Node* _foundNode;

  NodeFinder( const char* name )
    : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
      _name( name ),
      _foundNode( NULL )
  { }

  osg::Node* getFoundNode() {
    return _foundNode;
  }

  virtual void apply (osg::Node& node) {
    if ( strcmp(_name, node.getName().c_str()) == 0)
      _foundNode = &node;

    if (!_foundNode)
      traverse (node); // don't forget this!
  }
};

/******************************************************************************/

class AddPointsCallback : public  osg::NodeCallback
{
public:
  AddPointsCallback()
    : osg::NodeCallback()
  { }

  virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
    // printf( "Callback [%s]\n", _tvl->getName().c_str() );
    addPoints();
    traverse(node,nv);
  }
};

/******************************************************************************/

osg::Geode* createNewLine(const char* name) {

  osg::Geode *g = new osg::Geode();
  osg::Geometry* geom = new osg::Geometry;

  osg::Vec3Array* vertices = new osg::Vec3Array;
  geom->setVertexArray(vertices);

  osg::Vec4Array* colors = new osg::Vec4Array;
  colors->push_back(osg::Vec4(1.0, 1.0, 1.0, 1.0f));
  geom->setColorArray(colors);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::Vec3Array* normals = new osg::Vec3Array;
  normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
  geom->setNormalArray(normals);
  geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

  osg::StateSet* ss = geom->getOrCreateStateSet();
  osg::LineWidth* lw = new osg::LineWidth();
  lw->setWidth(5);
  ss->setAttributeAndModes(lw,osg::StateAttribute::ON);
  ss->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
  ss->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
  ss->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
  geom->setDataVariance(osg::Object::DYNAMIC);
  g->setDataVariance(osg::Object::DYNAMIC);

  geom->setStateSet(ss);

  g->addDrawable(geom);
  g->setName(name);

  printf("new line created: %s\n",name);

  return g;
}


osg::Group* createNewPage(const char* name) {

  osg::Group *g = new osg::Group();
  g->setName(name);

  return g;
}


/******************************************************************************/
double mtSpinAngle = 0.0;

/******************************************************************************/

void queueRandomPoints() {
  double x, y, z;

  srand((unsigned)time(0));
  /*for (int i = 0; i < 150; i++) {
    x = 15.0 - 30.0 * rand() / (double)RAND_MAX;
    y = 3.0 - 6.0 * rand() / (double)RAND_MAX;
    z = 15.0 - 30.0 * rand() / (double)RAND_MAX;
    queuepoint(x, y, z);
    }*/


  for (double t = 0.0; t < 2*M_PI; t+=0.1) {
    x = 3.0* cos(t);
    y = 0.5 - rand() / (double)RAND_MAX;
    z = 3.8* sin(t);
    queuepoint(x, y, z);
  }

}

/******************************************************************************/

int load_layer_crayon(int num)
{

  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  if (!qInitialized) RQueueInit(&pointQ, POINTQ_SIZE, sizeof(v3d), POINTQ_SIZE );
  else RQueueReset(&pointQ);

  qInitialized = true;

  //queueRandomPoints();

  osg::Group* scene = new osg::Group();
  mtSpin = new osg::MatrixTransform( osg::Matrix::identity() );
  mtStill = new osg::MatrixTransform( osg::Matrix::identity() );
  osg::Group *gp = createNewPage("add_lines_here");
  osg::Geode *gl = createNewLine("add_points_here");

  scene->addChild( mtStill );
  scene->addChild( mtSpin );
  gp->addChild( gl );
  mtStill->addChild( gp );

  /*
  // Add a sphere to test the 3D
  osg::Geode* gs = new osg::Geode();
  osg::Sphere* s = new osg::Sphere(osg::Vec3(0, 0, -10), 1);
  osg::ShapeDrawable* sd = new osg::ShapeDrawable(s);
  sd->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
  gs->addDrawable(sd);
  scene->addChild(gs);
  */

  crayon = scene;
  /*
  char vertfp[256];
  char fragfp[256];
  sprintf(vertfp, "%s/%s", sharePath, shaderStereoVertPath);
  sprintf(fragfp, "%s/%s", sharePath, shaderStereoFragPath);
  */
  LTMulti3DLayer* m3dl = new LTMulti3DLayer(SHADER_3D_PATH, NULL, 2048, 2048, 25.0, STEREO_MODE);
  printf("----------------CREATING STEREO CRAYON LAYER: stereo mode: %i\n",STEREO_MODE);

  if (STEREO_MODE!=STEREO_NONE) {
  //if (1) {
    printf("----------------CREATED STEREO CRAYON LAYER\n");
    m3dl->setSceneShaders(SHADER_STEREO_VERT_PATH,SHADER_STEREO_FRAG_PATH);
  }

  layers[num] = m3dl;
  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; // laid.  beurk.
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

  printf("setscene...\n");
  m3dl->setScene(scene);
  crayonLayer = m3dl;
  scene->setUpdateCallback( new AddPointsCallback() );

  return(0);
}

/******************************************************************************/
int nodefound=0;

#define CRAYON_ZOOM_FACTOR  -4.2

int addPoints() {

  osg::Geode* g;
  v3d v;
  size_t nbv = 0;

  if (crayon == NULL) return -1;

  NodeFinder nf("add_points_here");
  mtStill->accept( nf );
  g = (osg::Geode*)nf.getFoundNode();
  if (!g) {
      if (nodefound)
          printf("addPoints: NodeFinder could not find node\n");
    nodefound=0;
    return -1;
  }
  nodefound=1;

  osg::Geometry* geom =  g->getDrawable(0)->asGeometry();
  osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());

  //RQueueDump( &pointQ, NULL );
  int nbPtsAdded = 0;

  while ( RQueueRemoveFirst(&pointQ,(unsigned char*)&v) == 0 ) {
    vertices->push_back( osg::Vec3(v.x, v.y, v.z) );
    nbv = vertices->size();
    //printf("addPoints to geode %s: %f %f %f (%i)\n",
    //       g->getName().c_str(), v.x, v.y, v.z, vertices->size());


    // Add spheres instead of a line
    if (nbv>1) {
      osg::Sphere* s = new osg::Sphere(osg::Vec3(v.x, v.y, v.z), 1);
      osg::ShapeDrawable* sd = new osg::ShapeDrawable(s);
      sd->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
      g->addDrawable(sd);
      //printf("Added sphere at %f %f %f\n",v.x, v.y, v.z);
    }

    //if (nbv>1) geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,vertices->size()-2,2));
    nbPtsAdded++;
    //printf("ADDED PT: %f %f %f\n",v.x, v.y, v.z );
  }

  //if (nbPtsAdded == 0) return 0;

  //mtSpin->setMatrix(  osg::Matrixd::rotate(mtSpinAngle+=0.00015, 0,1,0) );
  osg::LineWidth* lw = new osg::LineWidth();
  lw->setWidth(1.4f);
  osg::StateSet* ss = mtSpin->getOrCreateStateSet();
  ss->setAttributeAndModes(lw,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);



  NodeFinder nf2("add_lines_here");
  mtStill->accept( nf2 );
  osg::Group* gpage = (osg::Group*) nf2.getFoundNode();
  if (!gpage) {
    printf("addPoints: NodeFinder could not find group\n");
    return -1;
  }

  /*
  const osg::BoundingSphere& bs = gpage->getBound();
  if (!bs.valid())  return 0;

  // printf("BoundingSphere: (%f, %f, %f); radius %f\n",
  //     bs.center().x(), bs.center().y(), bs.center().z(), bs.radius() );

  osg::Vec3 eye = bs.center() - osg::Vec3(0.0f,0.0f,CRAYON_ZOOM_FACTOR) * bs.radius();
  osg::Vec3 up(0.0f, 1.0f, 0.0f);
  mtStill->setMatrix( osg::Matrixd::lookAt(eye, bs.center(), up)) ;

  //const osg::BoundingSphere& bspin = mtSpin->getBound();
  //if (!bspin.valid())  return 0;
  */


  //printf("ADDED PT: %f %f %f\n",v.x, v.y, v.z );




  //crayonLayer->resetCamera();

  //if (vertices->size() > 100) {
  //  osgDB::writeNodeFile(*g, std::string("linestest.osg"));
  // }

  return 0;

}

/******************************************************************************/

int queuepoint(double x, double y, double z) {

  printf("queuepoint %f %f %f\n", x, y, z);
  v3d v;
  v.x=x;
  v.y=y;
  v.z=z;
  RQueueAddLast( &pointQ, (unsigned char*)&v);
  return 0;
}


/******************************************************************************/

int newline() {

  NodeFinder nf1("add_points_here");
  crayon->accept( nf1 );

  osg::Node* nline = nf1.getFoundNode();
  if ( !nline ) return -1;

  nline->setName("completed_line");

  RQueueReset(&pointQ);

  osg::Geode *gline = createNewLine("add_points_here");

  NodeFinder nf2("add_lines_here");
  mtStill->accept( nf2 );
  osg::Group* gpage = (osg::Group*) nf2.getFoundNode();
  if (!gpage) return -1;

  gpage->addChild( gline );
  //mtStill->addChild( gpage );
  //mtStill->accept( nf );
  //

  printf("new line\n");

  return 0;
}


int newpage() {

    NodeFinder nf("add_lines_here");
    mtStill->accept( nf );
    osg::Group* gpage = (osg::Group*) nf.getFoundNode();
    if ( !gpage ) return -1;

    printf("new page\n");

    osg::MatrixTransform* mtStillCopy = new osg::MatrixTransform();
    mtStillCopy->setMatrix( mtStill->getMatrix() );
    osg::MatrixTransform* mtAntiSpin = new osg::MatrixTransform();
    mtAntiSpin->setMatrix( osg::Matrixd::rotate(-mtSpinAngle, 0,1,0) );

    mtStillCopy->addChild(gpage);
    mtAntiSpin->addChild(mtStillCopy);
    mtSpin->addChild(mtAntiSpin);
    mtStill->removeChild(gpage);

    gpage->setName("completed_page");
    printf("completed page\n");

    osg::Group *gp = createNewPage("add_lines_here");
    mtStill->addChild( gp );

    newline();

    //osg::Geode *gl = createNewLine("add_points_here");
    //gp->addChild(gl);

    return 0;
}

///
///
/// GIMP!
///
///

// painture de gimp (le data est binaire)
int paint(int num,int bpp,int x,int y,int w,int h,void *data)
{
unsigned char *bin=(unsigned char *)data;
    if( num<0 || num>=NB_LAYER ) return(-1);
    if( !layers[num].valid() ) return(-1);

    //printf("[player] paint num=%d bpp=%d, xy=%d,%d wh=%d,%d data=0x%08lx\n",num,bpp,x,y,w,h,bin);
    //printf("bin data is %c%c%c%c\n",p[0],p[1],p[2],p[3]);
    layers[num]->paintTexture(bpp,x,y,w,h,bin);
    return(0);
}

