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

#include <osg/ClearNode>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osg/Timer>
#include <osg/Texture2D>

#include "ltlayer.h"
#include "lt3dlayer.h"
#include "ltmulti3dlayer.h"
#include "ltcapturelayer.h"
#include "ltvideotexture.h"
#include "lttileinfo.h"
#include "lttiledvideolayer.h"
#include "ltvideocontroller.h"
#include "ReceiveThread.h"

// pour le test de synchro de frame()
#include <sys/time.h>
#include <time.h>


#ifdef HAVE_PROFILER
  #include <profiler.h>
#endif

//#define VERBOSE

#include "utils/gamma.h"
#include <imgu/imgu.h>


char shaderFilmPath[255];
char shader3dPath[255];
char sharePath[255];
char shaderStereoFragPath[255];
char shaderStereoVertPath[255];
osg::Group* rootNode;

int stereo_mode = STEREO_NONE;
float stereo_B=STEREO_B_DEFAULT;
float stereo_R=STEREO_R_DEFAULT;
float stereo_distortion=STEREO_DISTORTION_DEFAULT;
float stereo_slit=STEREO_SLIT_DEFAULT;

int enable_lighting=1;

// ajouter des textures pour ces rigolos...
char outPath[200];
char dataPath[200];
char lutName[200];
char blendName[200];

int machine_num;
int nb_machines;

int rttsx,rttsy;

int frame_right_delay; //frame delay for right eye (stereo mode)
double voffsetx;
double x_global_offset; //angular offset of panorama seam (old zero_loc)
double x_right_offset; //right angular offset of panorama seam relative to global offset

/// on conserve la LUT (image et texture) ici pour partager
float xTexMin,xTexMax,yTexMin,yTexMax;
osg::ref_ptr<osg::Image> lutTexImage;
osg::ref_ptr<osg::Texture2D> lutTex;
osg::ref_ptr<osg::Image> blendTexImage;
osg::ref_ptr<osg::Texture2D> blendTex;

#define NB_LAYER  5
//osg::Group* layers[NB_LAYER]; // on ajoute
osg::ref_ptr<LTLayer> layers[NB_LAYER]; // valid() dit si actif
LTVideoController* vidCtrls[NB_LAYER];

osg::Matrixd mat_tmp;

//ReceiveThread *bmc;
bimulticast bmc;
udpcast udp;
char cmd[50];

double timeOffset; // offset pour les animations OSG

/******************************************************************************/

int change_filename_right_eye(char *file)
{
    char *pt_char;
    char extension[20];

    if (file==NULL) return -1;

    //copy extension
    pt_char=strrchr(file,'.');
    pt_char[0]='\0';
    pt_char++;
    strcpy(extension,pt_char);
    //remove -left and replace by -right
    pt_char=strrchr(file,'-');
    if (pt_char!=NULL && pt_char[1]=='l' && pt_char[2]=='e' && pt_char[3]=='f' && pt_char[4]=='t')
    {
      pt_char[0]='\0';
      strcat(file,"-right");
    }
    strcat(file,".");
    strcat(file,extension);

    return 0;
}

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
    usleep(200000);
    //rootNode->removeChild(layers[num].get());
    rootNode->setChild( NB_LAYER-num-1, new osg::Node() );
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

int load_layer(int num,char *movie_name)
{
  char movie_path[200];
#ifdef VERBOSE
  printf("loading layer %d movie '%s' (machine %d)\n",num,movie_name,machine_num);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  if (dataPath[0]=='\0') strcpy(movie_path,movie_name);
  else sprintf(movie_path,"%s/%s",dataPath,movie_name);

  char fullPathShader[255];
  sprintf( fullPathShader, "%s/%s", sharePath, shaderFilmPath );

  if (stereo_mode==STEREO_P_RIGHT)
  {
    change_filename_right_eye(movie_path);
  }

  layers[num] = new LTTiledVideoLayer(movie_path, fullPathShader, rttsx, rttsy);
  vidCtrls[num] = dynamic_cast<LTVideoController*>(layers[num].get()); // pas beau.  need to rethink this

  layers[num]->setFade(0.0); // default invisible
  layers[num]->updateSlit(stereo_slit);
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
  return(0);
}


/******************************************************************************/

int load_layer_multi3d(int num,char *model_name,double vertical_angle_of_view=25.0)
{
  char model_path[200];
  //#ifdef VERBOSE
  //printf("loading layer %d model '%s' (machine %d)\n",num,model_name,machine_num);
  //#endif
  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  sprintf(model_path,"%s/%s",dataPath,model_name);

  osg::ref_ptr<osgDB::ReaderWriter::Options> opts = new osgDB::ReaderWriter::Options();
  osgDB::FilePathList fpl = opts->getDatabasePathList();
  fpl.push_back( "." );
  fpl.push_back( dataPath );
  fpl.push_back( osgDB::getFilePath(model_path) );

  osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(model_path, opts.get());
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
  char fullPath3dShader[255];
  sprintf( fullPath3dShader, "%s/%s", sharePath, shader3dPath );
  char vertfp[256];
  char fragfp[256];
  sprintf(vertfp, "%s/%s", sharePath, shaderStereoVertPath);
  sprintf(fragfp, "%s/%s", sharePath, shaderStereoFragPath);

  LTMulti3DLayer *m = new LTMulti3DLayer(fullPath3dShader, loadedModel.get(), rttsx, rttsy,vertical_angle_of_view,stereo_mode);
  if (stereo_mode!=STEREO_NONE) 
  {
      m->setSceneShaders(vertfp,fragfp,stereo_B,stereo_R,stereo_distortion);
  }
  if (enable_lighting==0) m->disableLighting();
  else m->enableLighting();

  layers[num] = m;
  layers[num]->setFade(0.0); // default invisible
  layers[num]->updateSlit(stereo_slit);
  vidCtrls[num] = NULL; // laid.  beurk.
  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );

  return(0);
}

/******************************************************************************/

int load_layer_image(int num,char *img_name)
{
  char img_path[200];

  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);

  sprintf(img_path,"%s/%s",dataPath,img_name);

  if (stereo_mode==STEREO_P_RIGHT)
  {
    change_filename_right_eye(img_path);
  }
 
  printf("loading layer %d image '%s' (machine %d)\n",num, img_path ,machine_num);

  osg::ref_ptr<osg::Texture2D> tex  = new  osg::Texture2D;
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
  tex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
  tex->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
  tex->setImage(img1.get());

  osg::ref_ptr<osg::TexMat> tm = new osg::TexMat;
  float p[16];
  for(int i=0;i<16;i++) p[i]=0.0;
  p[0]=1.0; p[5]=1.0; p[10]=1.0; p[15]=1.0;

  osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();
  tm->setMatrix(osg::Matrixd(p));
  ss->setTextureAttributeAndModes(0, tm.get() ,osg::StateAttribute::ON);
  ss->setTextureAttributeAndModes(0, tex.get() , osg::StateAttribute::ON);
  tex->setResizeNonPowerOfTwoHint(false);

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

  char fullPathShader[255];
  sprintf( fullPathShader, "%s/%s", sharePath, shaderFilmPath );


  layers[num] = new LTLayer(fullPathShader,tex.get(), 1.0, 1.0,
                                    xTexMin, xTexMax, yTexMin, yTexMax, true);

  layers[num]->setFade(0.0); // default invisible
  layers[num]->updateSlit(stereo_slit);
  vidCtrls[num] = NULL; //pas beau
  //rootNode->addChild(layers[num].get());
  rootNode->setChild(  NB_LAYER-num-1, layers[num].get() );
  return(0);
}
/******************************************************************************/

//
// Voir luawrapfunctions.cpp pour savoir comment re-activer cette option
//
/*
int load_capture_layer(int num,char *capStr) {

#ifdef VERBOSE
// printf("loading layer %d movie '%s' (machine %d)\n",num,movie,machine_num);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);

  unload_layer(num);
  char fullPathShader[255];
  sprintf( fullPathShader, "%s/%s", sharePath, shaderFilmPath );

  layers[num] = new LTCaptureLayer(capStr, fullPathShader);
  vidCtrls[num] = NULL;
  layers[num]->setFade(0.0); // default invisible
  layers[num]->updateSlit(stereo_slit);

  rootNode->setChild( NB_LAYER-num-1, layers[num].get() );
  return(0);
}
*/

/******************************************************************************/

int frame(int num,int fr)
{
  int ret;
  ret=0;
#ifdef VERBOSE
  printf("frame layer %d, frame=%d\n",num,fr);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  if (vidCtrls[num] != NULL)
  {
    if (stereo_mode==STEREO_P_RIGHT) vidCtrls[num]->setNextFrame(fr+frame_right_delay);
    else vidCtrls[num]->setNextFrame(fr);
  }

  //dynamic_cast<LTTiledVideoLayer*>(layers[0].get())->go(fr);

  return(ret);
}

/******************************************************************************/

int fade(int num,float fd)
{
#ifdef VERBOSE  
  printf("fade layer %d, fade=%f\n",num,fd);
#endif
  if( num<0 || num>=NB_LAYER ) return(-1);
  if( !layers[num].valid() ) { /*printf("layer %d not active\n",num);*/return(-1); }

  layers[num]->setFade(fd);
  return(0);
}

/******************************************************************************/

void time(float t,osgViewer::Viewer* viewer)
{
  osg::FrameStamp * fs;
  fs = viewer->getFrameStamp();
  timeOffset=t - fs->getReferenceTime();
  //printf("time=%12.6f %12.6f  offset=%12.6f\n",t,fs->getReferenceTime(),timeOffset);
  //fs->setReferenceTime(t);
}

/// load <layer> <movie>
/// unload <layer>
/// frame <layer> <frame_num>
/// fade <layer> <fade_val>

/******************************************************************************/

void process_cmd(char *buf,osgViewer::Viewer* viewer)
{
  char cmd[50];
  int i,num,fr;
  float fd,t;
  char media[256];
  float motion, f1, f2, f3, f4;
  float tmp_stereo_B,tmp_stereo_R,tmp_stereo_distortion,tmp_stereo_slit;
  int vtoggle;
  vector3 gamma;
  double offset;

  if( buf==NULL ) return;

  // trouve 1er espace, pour la commande
  sscanf(buf," %s",cmd);

#ifdef VERBOSE
  printf("CMD_ID: %s\n",cmd);
  printf("CMD_FULL: %s\n",buf);
  fflush(stdout);
#endif

  if (strcmp(cmd,"model")==0 ) {
      sscanf(buf," model %d %s",&num,media);
      load_layer_multi3d(num,media);
  } else if(strcmp(cmd,"load")==0 ) {
      sscanf(buf," load %d %s",&num,media);
      load_layer(num,media);
  //
  // Voir luawrapfunctions.cpp pour savoir comment re-activer cette option
  /*
  } else if(strcmp(cmd,"capture")==0 ) {
    sscanf(buf," capture %d %s",&num, movie);
    load_capture_layer(num, movie);
  */
  } else if( strcmp(cmd,"tile")==0 ) {
      sscanf(buf," tile %d %s",&num,media);
      load_layer(num,media);

  } else if(strcmp(cmd,"unload")==0 ) {
      sscanf(buf," unload %d",&num);
      unload_layer(num);

  } else if(strcmp(cmd,"frame")==0 ) {
      sscanf(buf," frame %d %d",&num,&fr);
      frame(num,fr);
  } else if (strcmp(cmd,"image")==0 ) {
      sscanf(buf," image %d %s",&num, media);
      load_layer_image(num,media);
  } else if (strcmp(cmd,"scaletrans")==0 ) {
    sscanf(buf," scaletrans %d %f %f %f %f",&num, &f1, &f2, &f3, &f4);
    if (layers[num].valid() && strcmp(layers[num]->getName().c_str(),"LTLayer")==0)
    {
      layers[num]->setScaleTrans(f1, f2, f3 , f4);
    }
  } else if (strcmp(cmd,"tx")==0 ) {
      sscanf(buf," tx %d %f",&num, &motion);
      layers[num]->tx = motion;

  } else if (strcmp(cmd,"ty")==0 ) {
      sscanf(buf," ty %d %f",&num, &motion);
      layers[num]->ty = motion;
  } else if (strcmp(cmd,"rot")==0 ) {
      sscanf(buf," rot %d %f",&num, &motion);
      layers[num]->rot_angle = motion;
  } else if (strcmp(cmd,"scale")==0 ) {
      sscanf(buf," scale %d %f",&num, &motion);
      layers[num]->scale = motion;
  } else if (strcmp(cmd,"fade")==0 ) {
      sscanf(buf," fade %d %f",&num,&fd);
      fade(num,fd);
  } else if (strcmp(cmd,"time")==0 ) {
      sscanf(buf," time %f",&t);
      time(t,viewer);
  } else if (strcmp(cmd,"delay")==0 ) {
      sscanf(buf," delay %d",&frame_right_delay);
  } else if (strcmp(cmd,"offset")==0 ) {
      sscanf(buf," offset %lg %lg",&x_global_offset,&x_right_offset);
      x_global_offset=fmod(x_global_offset,360.0);
      x_right_offset=fmod(x_right_offset,360.0);
      if (x_global_offset<0) x_global_offset+=360.0;
      if (x_right_offset<0) x_right_offset+=360.0;
      x_global_offset/=360.0;
      x_right_offset/=360.0;
      offset=voffsetx+x_global_offset;
      if (stereo_mode==STEREO_P_RIGHT) offset+=x_right_offset;
      lutTexImage = LTLayer::loadLUT( lutName, &xTexMin, &xTexMax, &yTexMin, &yTexMax,offset);
  } else if (cmd[0] == '!'  ) {
      char* animCmd = (char*)malloc(sizeof(char)*strlen(buf));
      strcpy(animCmd, buf);
      animCmd++;
      //printf( "Animation command = %s\n", animCmd );

  } else if (strcmp(cmd,"stereo")==0 ) {
      sscanf(buf," stereo %f %f %f %f",&tmp_stereo_B,&tmp_stereo_R,&tmp_stereo_distortion,&tmp_stereo_slit);
      if (tmp_stereo_B>=0) stereo_B=tmp_stereo_B;
      if (tmp_stereo_R>=0) stereo_R=tmp_stereo_R;
      if (tmp_stereo_distortion>=0) stereo_distortion=tmp_stereo_distortion;
      if (tmp_stereo_slit>=0) stereo_slit=tmp_stereo_slit;
      for (i=0;i<NB_LAYER;i++)
      {
        if (layers[i]!=NULL)
        {
          layers[i].get()->updateSlit(stereo_slit);
          if (strcmp(layers[i]->getName().c_str(),"LTMulti3DLayer")==0)
          {
            dynamic_cast<LTMulti3DLayer*>(layers[i].get())->updateStereoConfiguration(stereo_B,stereo_R,stereo_distortion);
          }
        }
      }
  } else if (strcmp(cmd,"lighting")==0 ) {
      sscanf(buf," lighting %d",&enable_lighting);
      for (i=0;i<NB_LAYER;i++)
      {
        if (layers[i]!=NULL)
        {
          layers[i].get()->updateSlit(stereo_slit);
          if (strcmp(layers[i]->getName().c_str(),"LTMulti3DLayer")==0)
          {
            if (enable_lighting==0) dynamic_cast<LTMulti3DLayer*>(layers[i].get())->disableLighting();
            else dynamic_cast<LTMulti3DLayer*>(layers[i].get())->enableLighting();
          }
        }
      }
  } else if (strcmp(cmd,"vtoggle")==0 ) {
      sscanf(buf," vtoggle %d",&vtoggle);
      for (i=0;i<NB_LAYER;i++)
      {
        if (layers[i]!=NULL)
        {
          if (strcmp(layers[i]->getName().c_str(),"LTMulti3DLayer")==0)
          {
            dynamic_cast<LTMulti3DLayer*>(layers[i].get())->updateEyeSelection(vtoggle);
          }
        }
      }
  } else if (strcmp(cmd,"gamma")==0 ) {
      sscanf(buf," gamma %lg %lg %lg",&(gamma[0]),&(gamma[1]),&(gamma[2]));
      LTLayer::setGamma(gamma);
      for (i=0;i<NB_LAYER;i++)
      {
        if (layers[i]!=NULL)
        {
          layers[i].get()->updateGamma(gamma);
        }
      }
  } else if (strcmp(cmd,"pain")==0 ) {
		// commande 'paint' de gimp
      num=0;
      if (layers[num].valid() && strcmp(layers[num]->getName().c_str(),"LTLayer")==0)
      {
        layers[num]->paintTexture((unsigned char *)(buf));
      }
  }else if (strcmp(cmd,"viewmatrix")==0 ) {
    double p[16];
    //printf("got '%s'\n",buf);
    //int n =
    sscanf( buf," viewmatrix %lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf_%lf",
                    p+0,p+1,p+2,p+3,p+4,p+5,p+6,p+7,
                    p+8,p+9,p+10,p+11,p+12,p+13,p+14,p+15 );


       //printf("GOT A %12.6f %12.6f %12.6f %12.6f\n",p[0],p[1],p[2],p[3]);
       //printf("GOT B %12.6f %12.6f %12.6f %12.6f\n",p[4],p[5],p[6],p[7]);
       //printf("GOT C %12.6f %12.6f %12.6f %12.6f\n",p[8],p[9],p[10],p[11]);
       //printf("GOT D %12.6f %12.6f %12.6f %12.6f\n",p[12],p[13],p[14],p[15]);
       //printf("END\n");

    osg::Matrixd m = osg::Matrixd(p);
    for (i=0;i<NB_LAYER;i++)
    {
      if (layers[i]!=NULL)
      {
        if (strcmp(layers[i]->getName().c_str(),"LTMulti3DLayer")==0)
        {
          dynamic_cast<LTMulti3DLayer*>(layers[i].get())->updateViewMatrix(m);
        }
      }
    }
  } else if( strcmp(cmd,"quit")==0 ) {
      exit(0);
  } else {
#ifdef VERBOSE
      printf("Commande inconnue: '%s'\n",buf);
#endif
  }
}

/******************************************************************************/

int main( int argc, char **argv )
{
  int i,j,k,n,b,ret;
  char *ctemp;
  int zero_loc;
  int dlutsx,dlutsy;
  char inMovieFilename[256];
  char in3DFilename[256];
  double view_matrix[16],view_matrixT[16],view_angle;
  int sync;
  int threading;
  imgu *img;
  //char *buf;
  int port;
  int vpxmin, vpymin, vpwidth, vpheight;
  float leftborder, rightborder;
  struct timeval tv;
  int refresh_rate,frame_time,usleep_time; // frames par second
  int frameNb,prev_frameNb;

#ifdef USE_PROFILER
  profiler_init();
#endif

  img=NULL;

  mat4Identity(view_matrix);
  view_angle=25.0;

  /*for (i=0;i<NB_LAYER;i++)
  {
    multi_textures[i]=NULL;
    }*/

  sync=0;
  threading=0;
  inMovieFilename[0]='\0';
  in3DFilename[0]='\0';
  lutName[0]='\0';
  dataPath[0]='\0';
  outPath[0]='\0';
  blendName[0]='\0';
  machine_num=0;
  nb_machines=1;
  shaderFilmPath[0]='\0';
  shader3dPath[0]='\0';
  rttsx=2048;
  rttsy=2048;
  //strcpy(shaderFilmPath,"./deform_video.glsl");
  //strcpy(shader3dPath,"./deform_3d.glsl");
  dlutsx=1024;
  dlutsy=1024;
  vpxmin = 0;
  vpymin = 0;
  vpwidth = 512; // si 0, ca plante
  vpheight = 512; // si 0, ca plante
  leftborder = 0.0;
  rightborder = 0.0;
  port=PLAYER_PORT;
  x_global_offset=0;
  x_right_offset=0;
  frame_right_delay=0;

  for(i=1;i<argc;i++) {
    if( strcmp(argv[i],"-i")==0 && i+1<argc ) {
      strcpy(inMovieFilename,argv[i+1]);
      i++;continue;
    }
    if( strcmp(argv[i],"-3dfile")==0 && i+1<argc ) {
      strcpy(in3DFilename,argv[i+1]);
      i++;continue;
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
    if( strcmp(argv[i],"-vpxmin")==0 && i+1<argc) {
      vpxmin=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpymin")==0 && i+1<argc) {
      vpymin=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpwidth")==0 && i+1<argc) {
      vpwidth=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-vpheight")==0 && i+1<argc) {
      vpheight=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-leftborder")==0 && i+1<argc) {
      leftborder=atof(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-rightborder")==0 && i+1<argc) {
      rightborder=atof(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-threading")==0 && i+1<argc) {
      threading=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-view_matrix")==0 && i+16<argc) {
      for (j=0;j<16;j++) view_matrix[j]=atof(argv[i+j+1]);
      i+=16;
      continue;
    }
    if( strcmp(argv[i],"-view_angle")==0 && i+1<argc) {
      view_angle=atof(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-rttsx")==0 && i+1<argc) {
      rttsx=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-rttsy")==0 && i+1<argc) {
      rttsy=atoi(argv[i+1]);
      i++;
      continue;
    }
    if( strcmp(argv[i],"-port")==0 && i+1<argc) {
      port=atoi(argv[i+1]);
      i++;
      continue;
    }
  }

  if (sync==0 && inMovieFilename[0]=='\0' && in3DFilename[0]=='\0') {fprintf(stderr,"Usage: osgvideoplayer -i <movie filename>\n");fprintf(stderr,"Usage: osgvideoplayer -3dfile <3d scene filename>\n");fprintf(stderr,"Usage: osgvideoplayer -sync\n");exit(0);}

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

  voffsetx=zero_loc/360.0;
  if (voffsetx<0.0) voffsetx=0.0;
  if (voffsetx>1.0) voffsetx=0.0;

  lutTexImage = LTLayer::loadLUT( lutName, &xTexMin, &xTexMax, &yTexMin, &yTexMax,voffsetx);

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

  printf("BLEND...\n");
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
  //activate FSAA, doesn't seem to work
  //osg::DisplaySettings::instance()->setNumMultiSamples( 4 ); 

  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

  traits->x = vpxmin;
  traits->y = vpymin;
  if (vpwidth > 0) traits->width = vpwidth;
  if (vpheight > 0) traits->height = vpheight;
  traits->windowDecoration = false;
  //traits->samples=4;
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
 
  if (threading)
  {
    //viewer.setThreadingModel( osgViewer::Viewer::CullDrawThreadPerContext  );
    viewer.setThreadingModel( osgViewer::Viewer::ThreadPerContext    );
    //viewer.setThreadingModel( osgViewer::Viewer::DrawThreadPerContext );
    //viewer.setThreadingModel( osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext   );
    //viewer.setThreadingModel( osgViewer::Viewer::ThreadPerCamera   );
    //viewer.setThreadingModel( osgViewer::Viewer::AutomaticSelection  );
  }
  else
  {
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

  osg::ClearNode* cn = new osg::ClearNode();
  cn->setClearColor(osg::Vec4(0.0,0.0,0.0,1.0));
  cn->setName( "clear" );

  rootOfRoot->addChild(cn);
  rootOfRoot->addChild(rootNode);

  // le layer 0 sera dessine en premier

  for(i=0;i<NB_LAYER;i++) {
    layers[i] = NULL;
    vidCtrls[i] = NULL;
  }

  // pass the loaded scene graph to the viewer.
  viewer.setSceneData(rootOfRoot);

  // create the windows and run the threads.
  viewer.realize();
  //viewer.setEndBarrierPosition( osgViewer::Viewer::BeforeSwapBuffers );

  // Force la dimension et la position de la fenetre
  // Les preferences de fenetre de l'OS semblent avoir preseance
  // sur le GraphicsContext, et donc la barre de menu de GNOME empeche
  // la fenetre d'etre en (0,0).
  osgViewer::Viewer::Windows windows;
  viewer.getWindows( windows, true );
  if ( windows[0] ) {
    windows[0]->setWindowRectangle( vpxmin, vpymin, vpwidth, vpheight );
    windows[0]->setCursor( osgViewer::GraphicsWindow::NoCursor );
  }

  //viewer.setView(osg::Matrix::identity());
  //viewer.setView(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), 1.0f, 0.0f, 0.0f));
  viewer.getCamera()->setViewMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), 1.0f, 0.0f, 0.0f));

  n=0;
  b=0;

  if (inMovieFilename[0]!='\0') //play movie as normal movie player
  {
    printf("loading movie...\n");
    sync=0;
    load_layer(0,inMovieFilename);
    fade(0,1.0);
  }
  if (in3DFilename[0]!='\0')
  {
    printf("loading 3d...\n");
    sync=0;
    load_layer_multi3d(0,in3DFilename,view_angle);
    printf("View Matrix:\n");
    mat4Print(view_matrix);
    mat4Transpose(view_matrix,view_matrixT);
    osg::Matrixd m = osg::Matrixd(view_matrixT);
    dynamic_cast<LTMulti3DLayer*>(layers[0].get())->updateViewMatrix(m);
    fade(0,1.0);
  }

  refresh_rate=60;

  double fps = 0;
  if (vidCtrls[0] != NULL) fps = vidCtrls[0]->getFPS();
  if (fps <= 0.0) fps = 30.0;
  printf("fps = %f\n", fps);

  frameNb=-1;
  prev_frameNb=-1;

  //bmc=NULL;
  if (sync) {
    init_bimulticast_player(&bmc,port,1);
    bi_receive_thread_start(&bmc,50,500); 
    //bmc=new ReceiveThread( port );
  }
  udp_init_receiver(&udp,15000,NULL); // pour GIMP

  Entry entry;
  unsigned char refresh_cmd;
  char *buf;
  buf=(char *)(malloc(sizeof(char)*1100));
  while( !viewer.done() ) {
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_start("main loop");
  #endif
#endif

    // if (n % 500 == 0) profiler_dump();

    if( sync ) {
        /*if( bmc!=NULL ) 
        {
          i=0;
          while(1)
          {
            if (bmc->GetNextCommand(&entry)==0)
            {
              if (strcmp(entry.key,"refresh")==0) refresh_cmd=1;
              else
              {
                process_cmd(entry.value,&viewer);
                refresh_cmd=0;
              }
#ifdef VERBOSE
	  		  printf("PROCESSED: '%s' (%d)\n",entry.value,refresh_cmd);
#endif
              if (refresh_cmd && bmc->ContainsRefresh()==0) break;
            }
            else
            {
              usleep(1000);
            }
          }
        }*/

        message msg;
        k=0;
        while( bi_receive_data_poll_threaded(&bmc,&msg)>0 ) 
        {
            process_cmd((char *)msg.data,&viewer);
#ifdef VERBOSE
	    printf("PROCESSED: '%s'\n",msg.data);
#endif
            bi_done_with_message(&bmc,&msg);
            k++;
break;
        }

        while( udp_receive_data_poll(&udp,(unsigned char *)buf,1100)>0 )
        {
	    process_cmd(buf,&viewer);
        }
    }else{
      // pas de sync... on auto calcule les frame num
	
      //prev_frameNb=frameNb;
      //frameNb = timeClock * fps;
      //if (vidCtrls[0] != NULL) vidCtrls[0]->setNextFrame(frameNb);
    }

    /*printf("viewer.frame done\n");
    profiler_start("viewer.frame");
    //printf("before frame vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
    //viewer.frame();
    viewer.advance();
    //printf("***** EVENT TRAVERSAL *****\n");
    viewer.eventTraversal();
    //printf("***** UPDATE TRAVERSAL *****\n");
    viewer.updateTraversal();
    //printf("***** RENDERING TRAVERSALS ******\n");
    profiler_start("renderingTraversals");
    viewer.renderingTraversals();
    //profiler_stop("renderingTraversals");
    //printf("after frame  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    profiler_stop("viewer.frame");*/

	frame_time=1000000/refresh_rate; // en usec
	gettimeofday(&tv,NULL);
	usleep_time=frame_time-tv.tv_usec%frame_time;
    //if (usleep_time>100 ) usleep(usleep_time);
#ifdef VERBOSE
	printf("time=%d.%06d sleep=%d\n",tv.tv_sec,tv.tv_usec,usleep_time);
#endif

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_start("viewer.frame");
  #endif
#endif
    viewer.frame();
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_stop("viewer.frame");
  #endif
#endif

    n++;
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_stop("main loop");
  #endif
#endif
  }
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
  profiler_dump();
  #endif
#endif

  return 0;
}


