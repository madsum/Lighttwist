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

/* load une texture */


#include "luawrapfunctions.h"

// pur utiliser LUA
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

    // declare l'initialisation du module lua (play)
    extern int luaopen_player(lua_State* L);
    // pousse un pointeur en mode "compatible SWIG"
    extern void luawrappointer(lua_State* L,void *p);
}

/////////
/////////
/////////

// port par defaut pour le controle
// peut etre change par -port
#define DEFAULT_CMD_PORT  10000

// definir pour utiliser gimp
/* #define USE_GIMP */

#define GIMP_DRAW_BASE_PORT 15000

//////////
//////////
//////////

//#define USE_PROFILER

#ifdef HAVE_PROFILER
  #include <profiler.h>

#endif

#include "utils/gamma.h"
#include <imgu/imgu.h>


// difference avec le maitre...
tile_info* tiles;

//osg::Group* rootNode; [LUA]

int stereo_mode = STEREO_NONE;

// ajouter des textures pour ces rigolos...
char outPath[200];
//char dataPath[200];
char lutName[200];
char blendName[200];

char shaderFilmPath[255];
char shader3dPath[255];
char sharePath[255];
char shaderStereoFragPath[255];
char shaderStereoVertPath[255];


//int machine_num; [lua]
int nb_machines;

/// on conserve la LUT (image et texture) ici pour partager
//float xTexMin,xTexMax,yTexMin,yTexMax; [lua]

// LUAAA
osg::ref_ptr<osg::Image> lutTexImage;
osg::ref_ptr<osg::Texture2D> lutTex;
osg::ref_ptr<osg::Image> blendTexImage;
osg::ref_ptr<osg::Texture2D> blendTex;
float voffsetx; /// LUA!!!

osg::Matrixd mat_tmp;

//see bmc_network.h for explanation on why we have 2 channels
bimulticast bmc;
udpcast udp;
char cmd[50];

int frameNb,prev_frameNb;
int FRAME_COUNT;

/// offset entre le temps de reference (PD) et le temps local
// On a: reference_time = local_time() + timeOffet
// Quand on recoit un message time x, on doit ajuster:
// timeOffset = x - local_time()
// On peut donner l'ajustement au viewer aussi: setReferenceTime(local_time()+timeOffset)
//double timeOffset;

#define VERBOSE
#undef DEBUG

/******************************************************************************/
double timeClock; ////,timeOffset;
double timeLastRefresh,timeDiff;
/******************************************************************************/

/******************************************************************************/

int load_layer_3d(int num,char *model_name)
{
  char model_path[200];
  //#ifdef VERBOSE
  //printf("loading layer %d model '%s' (machine %d)\n",num,model_name,machine_num);
  //#endif
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

  layers[num] = new LT3DLayer(SHADER_3D_PATH, loadedModel.get(), 2048, 2048);
  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; // laid.  beurk.
  //rootNode->addChild(layers[num].get());
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

  return(0);
}

/******************************************************************************/
#ifdef SKIP_LUA
int load_layer_multi3d(int num,char *model_name)
{

  printf("---------load_layer_multi3d inside LTPLAYERLUA\n");

  char model_path[200];
  //#ifdef VERBOSE
  //printf("loading layer %d model '%s' (machine %d)\n",num,model_name,machine_num);
  //#endif
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

  char vertfp[256];
  char fragfp[256];
  sprintf(vertfp, "%s/%s", sharePath, shaderStereoVertPath);
  sprintf(fragfp, "%s/%s", sharePath, shaderStereoFragPath);

  layers[num] = new LTMulti3DLayer(SHADER_3D_PATH, loadedModel.get(), 2048, 2048, 25.0, stereo_mode);
  if (stereo_mode!=STEREO_NONE)
  {
    layers[num]->setSceneShaders(vertfp, fragfp);
  }

  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; // laid.  beurk.
  //rootNode->addChild(layers[num].get());
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

  return(0);
}
#endif

/******************************************************************************/

#ifdef SKIP_MOVED_TO_LUA
int load_layer_image(int num,char *img_name)
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
  tex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
  tex->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
  tex->setImage(img1.get());

  printf("load_layer_image: size = %i x %i\n", tex->getTextureWidth() , tex->getTextureHeight() );

  osg::TexMat* tm = new osg::TexMat;
  float p[16];
  for(int i=0;i<16;i++) p[i]=0.0;
  p[0]=1.0; p[5]=1.0; p[10]=1.0; p[15]=1.0;

  osg::StateSet* ss = new osg::StateSet();
  tm->setMatrix(osg::Matrixd(p));
  ss->setTextureAttributeAndModes(0, tm  ,osg::StateAttribute::ON);
  ss->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

  /*float asp_x , asp_y;
  asp_x = 1;
  asp_y = (float)img1->s()/img1->t();

  //To ensure that aspect are between 0..1
  if( img1->s() > img1->t() ){
    asp_x = 1;
    asp_y = (float)img1->s()/img1->t();
  }else{&xTexMin, &xTexMax, &yTexMin, &yTexMax
    asp_x = (float)img1->t()/img1->s();
    asp_y = 1;
  }*/

  layers[num] = new LTLayer(SHADER_FILM_PATH,tex, 1.0, 1.0,
                                    xTexMin, xTexMax, yTexMin, yTexMax, true);

  layers[num]->setFade(0.0); // default invisible
  vidCtrls[num] = NULL; //pas beau
  //rootNode->addChild(layers[num].get());
  rootNode->setChild(  NB_LAYER-num-1, layers[num].get() );
  return(0);
}
#endif

/******************************************************************************/

#ifdef SKIP_LUA
int frame(int num,int fr)
{
  int ret;
  ret=0;
#ifdef VERBOSE
  printf("frame layer %d, frame=%d\n",num,fr);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  if (vidCtrls[num] != NULL) vidCtrls[num]->setNextFrame(fr);

    //dynamic_cast<LTTiledVideoLayer*>(layers[0].get())->go(fr);

  return(ret);
}
#endif

/******************************************************************************/

#ifdef SKIP_LUA
int fade(int num,float fd)
{
  //printf("fade layer %d, fade=%f\n",num,fd);
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  layers[num]->setFade(fd);
  return(0);
}
#endif

/******************************************************************************/

// t est le vrai temps externe de reference (PD)

/***
void adjustTimeOffset(double t,osgViewer::Viewer* viewer)
{
  timeOffset= t - getLocalTime();
  printf("time=%12.6f local_time=%12.6f  offset=%12.6f\n",t,getLocalTime(),timeOffset);
}
***/

/// load <layer> <movie>
/// unload <layer>
/// frame <layer> <frame_num>
/// fade <layer> <fade_val>

// retourne 0 si ok, -1 si err
int execLua(lua_State *L,const char *buf)
{
int status;
  //printf("lua exec '%s'\n",buf);
  status=luaL_dostring(L,buf);
        if (status) {
    fprintf(stderr, "error %d: %s\n", status,lua_tostring(L, -1));
    lua_pop(L,1);
    return(-1);
  }
  return(0);
}


/******************************************************************************/

/*
 * Chaque commande est une suite de token et devient une fonction + args
 *
 * si fulllen <= strlen(buf)+1, alors on suppose que la commande est complete.
 * sinon, on a fulllen - (strlen(buf)+1) bytes binaires a envoyer a la commande
 *
 */

void process_cmd_lua(char *buf,lua_State *L,int fulllen)
{
    char *p;
    int nb=0;
    int k,before,after,len;

    //printf("process_cmd_lua: [%s]\n", buf);

    len=strlen(buf);
    before=lua_gettop(L);  // etat du stack avant la function

  // skip starting spaces
  while(*buf && isspace(*buf) ) buf++;

    // kill ending spaces or ;
  int i;
  for(i=len-1;i>=0;i--) if( buf[i]!=';' && !isspace(buf[i]) ) break;
  buf[i+1]=0;

  p = strtok( buf, " " );
  if( p==NULL ) { return; }

  //printf("function = '%s'\n",p);

  // first field is function to call
  lua_getfield(L, LUA_GLOBALSINDEX, p); /* function to be called */

  if( lua_isnil(L,-1) ) {
    printf("lua error: function '%s' not defined\n",p);
    lua_pop(L,1);
    return;
  }

  // the arguments are pushed... only strings... we let lua convert as needed

  nb=0;
  while( p != NULL ) {
    p = strtok( NULL, " " );
    if ( p != NULL ) {
      //printf ("arg : %s\n",p);
      lua_pushstring(L,p);
      nb++;
    }
  }

  // extra binary data
  if( fulllen>len+1 ) {
    //printf("adding a binary parameter for the %d extra bytes 0x%08lx\n",fulllen-(len+1),(buf+len+1));

    // must be SWIG compatible here... call a wrapper from luawrapfunctions
    luawrappointer(L,(void *)(buf+len+1));
    nb++;
  }

  // call with nb args and expect NO returned results
  k=lua_pcall(L, nb, 0,0);
  if( k ) {
    printf("lua error %d: %s\n", k,lua_tostring(L, -1));
    lua_pop(L,1);
  }
  after=lua_gettop(L);
  //printf("nb of results is %d\n",after-before);

  lua_pop(L,after-before); // inutile pour 0 results
}



/*****************************************************************************/


/******************************************************************************/



int main( int argc, char **argv )
{
  char blub[2000];
  int i,k,n,b;
  char *ctemp;
  int zero_loc = 0;
  int dlutsx,dlutsy;
  int sync;
  //float voffsetx; /// LUA!!!
  imgu *img;
  int vpxmin, vpymin, vpwidth, vpheight;
  float leftborder, rightborder;
  int threading;
  int cmd_port; // port de commande
  int gimp_port; // reception des images de gimp (dessins)
  char cmdbuf[20];
  int bmcthreads; // use threading for bmc?

  // l'interpreteur LUA
  lua_State *L;

  // init LUA!
  printf("LUA: init...\n");
  L=luaL_newstate();;
  luaL_openlibs(L);
  printf("LUA: done libs\n");
  k=luaopen_player(L); // init le module player (wrapped luawrapfunctions.i)
  printf("LUA: done player k=%d.\n",k);

  printf("Lua initialize by running commands.lua ...\n");
  k=execLua(L,"dofile(\"commands.lua\")");
  if( k<0 ) {
  k=execLua(L,"dofile(\"/usr/local/share/lighttwist/commands.lua\")");
  if( k<0 ) {
    printf("Unable to find 'commands.lua', even in /usr/local/share/lighttwist :-(\n");
    exit(0);
  }
  }

  printf("commands.lua loaded.\n");

/**
  {
  int status;
  status=luaL_dostring(L,"print(player.fact(4))");
  printf("status=%d\n",status);
  }
**/

/**
  {
  int status;
  status=luaL_dostring(L,"z=player.sebcreate(1,2);print(z);print(player.sebprocess(z))");
  printf("status=%d\n",status);

  }
**/

//sprintf(blub,"pain 1    0    0   15   15");
//for(i=0;i<20;i++) blub[27+i]='A'+i;
//printf("len is %d\n",strlen(blub));
//process_cmd_lua(blub,L,27+20);

//exit(0);


  //osg::FrameStamp * fs;

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
  profiler_init();
  #endif
#endif

  img=NULL;
  tiles=NULL;

  /*for (i=0;i<NB_LAYER;i++)
  {
    multi_textures[i]=NULL;
    }*/

  threading=0;
  sync=0;
  lutName[0]='\0';
  dataPath[0]='\0';
  outPath[0]='\0';
  blendName[0]='\0';
  machine_num=0;
  nb_machines=1;
  STEREO_MODE=STEREO_NONE;
  SHADER_FILM_PATH[0]=0;
  SHADER_3D_PATH[0]=0;
  SHADER_STEREO_VERT_PATH[0]=0;
  SHADER_STEREO_FRAG_PATH[0]=0;
  //strcpy(SHADER_FILM_PATH,"./deform_video.glsl");
  //strcpy(SHADER_3D_PATH,"./deform_3d.glsl");
  dlutsx=1024;
  dlutsy=1024;
  FRAME_COUNT=0;
  vpxmin = 0;
  vpymin = 0;
  vpwidth = 512; // si on met 0, ca plante...
  vpheight = 512; // si on met 0, ca plante...
  leftborder = 0.0;
  rightborder = 0.0;
  cmd_port=DEFAULT_CMD_PORT;
  gimp_port=GIMP_DRAW_BASE_PORT;
    bmcthreads=1; // use threading by default

  for(i=1;i<argc;i++) {
      printf("arg: [%s]\n", argv[i]);
    if( strcmp(argv[i],"-nobmcthreads")==0 ) {
        bmcthreads=0;
        printf("NOT USGING BMC THREADS.\n");
        continue;
    }
    if( strcmp(argv[i],"-path")==0 && i+1<argc ) {
      strcpy(dataPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-m")==0 && i+1<argc ) {
      machine_num=atoi(argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-n")==0 && i+1<argc ) {
      nb_machines=atoi(argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-lut")==0 && i+1<argc ) {
      strcpy(lutName,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-blend")==0 && i+1<argc ) {
      strcpy(blendName,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-video_shader")==0 && i+1<argc ) {
      strcpy(shaderFilmPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-3d_shader")==0 && i+1<argc ) {
      strcpy(shader3dPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-vstereo_shader")==0 && i+1<argc ) {
      strcpy(shaderStereoVertPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-fstereo_shader")==0 && i+1<argc ) {
      strcpy(shaderStereoFragPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-share")==0 && i+1<argc ) {
      strcpy(sharePath,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-stereo_p_right")==0 ) {
      stereo_mode = STEREO_P_RIGHT;
      printf("----------READ stereo_p_right option: stereo_mode: %i\n",stereo_mode);
    }
    if( strcmp(argv[i],"-stereo_p_left")==0 ) {
      stereo_mode = STEREO_P_LEFT;
    }
    if( strcmp(argv[i],"-stereo_rc")==0 ) {
      stereo_mode = STEREO_RC;
    }
    if( strcmp(argv[i],"-dlutsx")==0 && i+1<argc ) {
      dlutsx=atoi(argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-dlutsy")==0 && i+1<argc ) {
      dlutsy=atoi(argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-zero")==0 && i+1<argc ) {
      zero_loc=atoi(argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-sync")==0) {
      sync=1;
      continue;
    }
    if( strcmp(argv[i],"-vpxmin")==0) {
      vpxmin=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpymin")==0) {
      vpymin=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpwidth")==0) {
      vpwidth=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpheight")==0) {
      vpheight=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-leftborder")==0) {
      leftborder=atof(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-rightborder")==0) {
      rightborder=atof(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-threading")==0 && i+1<argc) {
      threading=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-port")==0 && i+1<argc ) {
      cmd_port=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-gimpport")==0 && i+1<argc ) {
      gimp_port=atoi(argv[i+1]);
      i++;
      continue;
    }

  }

  sprintf(SHADER_FILM_PATH,"%s/%s",sharePath,shaderFilmPath);
  sprintf(SHADER_3D_PATH,"%s/%s",sharePath,shader3dPath);
  sprintf(SHADER_STEREO_VERT_PATH,"%s/%s",sharePath,shaderStereoVertPath);
  sprintf(SHADER_STEREO_FRAG_PATH,"%s/%s",sharePath,shaderStereoFragPath);
  STEREO_MODE = stereo_mode;

  strcpy(outPath,lutName);
  ctemp=strrchr(outPath,'/');
  if (ctemp==NULL)
  {
    //printf("Error: no outPath!\n");exit(1);
  }
  else (*ctemp)='\0';

#ifdef VERBOSE
  printf("LOADING LUT\n");
#endif

  voffsetx=zero_loc/360.0f;
  if (voffsetx<0.0) voffsetx=0.0;
  if (voffsetx>1.0) voffsetx=0.0;

  lutTexImage = LTLayer::loadLUT( lutName,
          &xTexMin, &xTexMax, &yTexMin, &yTexMax,voffsetx);

  if (lutTexImage==NULL)
  {
    //load default lut
    lutTexImage = LTLayer::generateLUT(machine_num,nb_machines,dlutsx,dlutsy,
                                               &xTexMin, &xTexMax, &yTexMin, &yTexMax,
                                               leftborder, rightborder);
  } else {

    lutTexImage->flipVertical();
  }

#ifdef VERBOSE
  printf("LUT TEXTURE\n");
#endif
  lutTex = LTLayer::createLutTexFromImg( lutTexImage.get() );

#ifdef VERBOSE
  printf("DONE\n");

  printf("BLEND... is '%s'\n",blendName);
#endif

  blendTexImage = osgDB::readImageFile( blendName );
  if( blendTexImage==NULL )
  {
    //load default blend
    unsigned char *data;

    blendTexImage = new osg::Image;
    blendTexImage->setImage( dlutsx, dlutsy, 1,            // 1 = 3rd dimension..
        GL_LUMINANCE,
        GL_LUMINANCE,          // glformat
        GL_UNSIGNED_BYTE,                    // gltype
        (unsigned char *) malloc(dlutsx * dlutsy * sizeof(unsigned char)),
        osg::Image::USE_MALLOC_FREE );

    data = (unsigned char*)blendTexImage->data();
    if(!data){
        printf("load_blend: Can't allocate data\n");
        exit(0);
    }

    for( int i = 0; i < dlutsy; i++ )
    {
      for( int j = 0; j < dlutsx; j++ )
      {
        data[i*dlutsx + j] = (unsigned char)(255);
      }
    }
  }

  //blendTexImage->flipVertical();
  blendTex = LTLayer::createBlendTexFromImg( blendTexImage.get() );
#ifdef VERBOSE
  printf("DONE\n");
#endif

#ifdef VERBOSE
  printf("GAMMA + COLOR\n");
#endif
  vector3 gamma;
  matrix3 color;

  read_gamma_and_color(outPath,machine_num,gamma,color);
  LTLayer::setGamma(gamma);
  LTLayer::setColor(color);

#ifdef VERBOSE
  printf("DONE\n");
#endif


  osg::ArgumentParser arguments(&argc,argv);

  // construct the viewer.
  osgViewer::Viewer viewer;

  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

  traits->x = vpxmin;
  traits->y = vpymin;
  if (vpwidth > 0) traits->width = vpwidth;
  if (vpheight > 0) traits->height = vpheight;
  traits->windowDecoration = false;
  traits->doubleBuffer = true;
  traits->sharedContext = 0;
    traits->vsync = true;
  traits->useMultiThreadedOpenGLEngine = false;

  osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

  osg::Camera* viewerCam = viewer.getCamera();
  viewerCam->setGraphicsContext(gc.get());
  viewerCam->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));

  // fond noir par defaut!!!
  viewerCam->setClearColor(osg::Vec4f(0,0,0,1));

  /*printf("%i .. %i .. %i\n",
         viewer.getEndBarrierPosition(),
         osgViewer::Viewer::BeforeSwapBuffers,
         osgViewer::Viewer::AfterSwapBuffers     ); // this one by default!

  //exit(0);

  */

  if (threading) {
      printf("osg multithreading on.\n");
  //viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
  //viewer.setThreadingModel( osgViewer::Viewer::CullDrawThreadPerContext  );
  //viewer.setThreadingModel( osgViewer::Viewer::ThreadPerContext    );
  //viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
  viewer.setThreadingModel( osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext   );
  //viewer.setThreadingModel( osgViewer::Viewer::ThreadPerCamera   );
  //viewer.setThreadingModel( osgViewer::Viewer::AutomaticSelection  );
  } else {
      printf("osg multithreading off.\n");
    viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
  }

  // set up the value with sensible default event handlers.
  //viewer.setUpViewer(osgViewer::Viewer::STANDARD_ETTINGS);

  // get details on keyboard and mouse bindings used by the viewer.
  //viewer.getUsage(*arguments.getApplicationUsage());

  osg::Group* rootOfRoot = new osg::Group();

  rootNode = new osg::Group();
  rootNode->setName( "root" );

  osg::StateSet* stateset = rootNode->getOrCreateStateSet();
  stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

  for (int i = 0; i < NB_LAYER; i++) {
    rootNode->addChild( new osg::Node() );
  }

  clearNode = new osg::ClearNode();
  clearNode->setClearColor(osg::Vec4(0.0,0.0,0.0,1.0));
  clearNode->setName( "clear" );

  rootOfRoot->addChild(clearNode);
  rootOfRoot->addChild(rootNode);

  // le layer 0 sera dessine en premier

  for(i=0;i<NB_LAYER;i++) {
    layers[i] = NULL;
    vidCtrls[i] = NULL;
  }

  // pass the loaded scene graph to the viewer.
  viewer.setSceneData(rootOfRoot);


  //osg::DisplaySettings * ds = osg::DisplaySettings::instance();
  //ds->setNumMultiSamples(16);
  //viewer.setDisplaySettings(ds);
  printf("setwindowrectangle!\n");
  osgViewer::Viewer::Windows windows;
  viewer.getWindows( windows, true );
  if (windows[0]) {
      windows[0]->setWindowRectangle( vpxmin, vpymin, vpwidth, vpheight );
      windows[0]->setCursor( osgViewer::GraphicsWindow::NoCursor );
  }

  printf("realize...\n");
  // create the windows and run the threads.
  viewer.realize();
  //viewer.setEndBarrierPosition( osgViewer::Viewer::BeforeSwapBuffers );
 printf("realized!\n");
  // Force la dimension et la position de la fenetre
  // Les preferences de fenetre de l'OS semblent avoir preseance
  // sur le GraphicsContext, et donc la barre de menu de GNOME empeche
  // la fenetre d'etre en (0,0).


  //viewer.setView(osg::Matrix::identity());
  //viewer.setView(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), 1.0f, 0.0f, 0.0f));
  viewer.getCamera()->setViewMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), 1.0f, 0.0f, 0.0f));


  n=0;
  b=0;

  int refresh_rate,usleep_time;
  osg::Timer* timer = osg::Timer::instance();
  timer->setStartTick();
  timeOffset=timer->time_s();
  timeClock=timer->time_s()-timeOffset;
  timeLastRefresh=timeClock;
  refresh_rate=60;

  double fps = 0;
  if (vidCtrls[0] != NULL) fps = vidCtrls[0]->getFPS();
  if (fps <= 0.0) fps = 30.0;
  printf("fps = %f\n", fps);

  frameNb=-1;
  prev_frameNb=-1;

  bmcthreads = 0;

  printf("initialize bmx port %i\n", cmd_port);
    init_bimulticast_player(&bmc,cmd_port,1);
    if( bmcthreads ) bi_receive_thread_start(&bmc,50,500);

#ifdef USE_GIMP
    if( udp_init_receiver(&udp,gimp_port,"226.0.0.1") ) {
        printf("IMPOSSIBLE D'OUVRIR udp pour gimp\n");
    }
#endif

  while( !viewer.done() ) {

      timer->setStartTick();

      // printf("osgvidplayerlua: main loop\n");
    strcpy(cmdbuf,"tick");
    process_cmd_lua(cmdbuf,L,-1);
    //printf("%f : process_cmd_lua tick\n", timer->time_m());
    //  timer->setStartTick();

    //viewer->setReferenceTime(t);

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_start("main loop");
  #endif
#endif


    timeClock=timer->time_s()-timeOffset;
    timeDiff=timeClock-timeLastRefresh;
    if (timeDiff<1.0/refresh_rate) //refresh 'refresh_rate' times per second
    {
      usleep_time=1000000/refresh_rate;
      usleep_time-=1000000*timeDiff;
      usleep(usleep_time);
    }



    // if (n % 500 == 0) profiler_dump();

    if( sync ) {
        // process toutes les commandes qui attendent dans la queue...
        if( bmcthreads ) {
            message msg;
            //printf("got message?\n");
            while( bi_receive_data_poll_threaded(&bmc,&msg)>0 ) {
                //printf("got message [%s]\n",(char *)msg.data );
                process_cmd_lua((char *)msg.data,L,-1);
                //printf("processed the message\n");
                bi_done_with_message(&bmc,&msg);
                //printf("done with the message\n");
            }
        }else{
            while( bi_receive_data_poll(&bmc,(unsigned char *)blub,sizeof(blub))>0 ) {
                //printf("got message [%s]\n",blub );
                process_cmd_lua(blub,L,-1);
                //printf("processed the message\n");
            }
        }
    }
    // printf("%f : sync + lua\n", timer->time_m());
    // timer->setStartTick();

    // GIMP

#ifdef zzUSE_GIMP
    {
        char buf[1500];
        int len;
        while( (len=udp_receive_data_poll(&udp,(unsigned char *)buf,sizeof(buf)-1))>0 ) {
            // il faut encoder le contenu binaire dans un blob pour lua
            // On envoie la longueur recue...
            // Tout ce qui depasse de strlen(buf)+1 est du binaire... (donc len-(strlen(buf)+1)
            // Ce data doit etre simplement envoye sous forme d'un pointeur de data brut (void).
            process_cmd_lua(buf,L,len);
        }
    }
#endif

    // ajuste le temps le mieux possible....
    //fs = viewer.getFrameStamp();
    //fs->setReferenceTime(fs->getReferenceTime()+timeOffset);

#ifdef VERBOSE
    //printf("t=%12.6f\n",fs->getReferenceTime());
#endif

    /***
    printf("viewer.frame done\n");
    profiler_start("viewer.frame");
    printf("before frame vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
    //viewer.frame();
    viewer.advance();
    printf("***** EVENT TRAVERSAL *****\n");
    viewer.eventTraversal();
    printf("***** UPDATE TRAVERSAL *****\n");
    viewer.updateTraversal();
    printf("***** RENDERING TRAVERSALS ******\n");
    profiler_start("renderingTraversals");
    viewer.renderingTraversals();
    //profiler_stop("renderingTraversals");
    printf("after frame  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    profiler_stop("viewer.frame");
    ****/
    // timeLastRefresh=timer->time_s();

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_start("viewer.frame");
  #endif
#endif

    viewer.frame();
    ///printf("%f : viewer.frame\n\n\n", timer->time_m());
    ///timer->setStartTick();

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_stop("viewer.frame");
  #endif
#endif

    n++;
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_stop("main loop");
    if( n>0 && (n%100)==0 )  {
      printf("n=%d\n",n);
      profiler_dump();
      profiler_reset();
    }
  #endif
#endif

    }   // until viewer is done

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
  profiler_dump();
  #endif
#endif

  // fermer LUA
  lua_close(L);

  return 0;
}

