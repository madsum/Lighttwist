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

/*
 * stl.cpp - Code for running the program 'ltstl' from a shell.
 *                     This program is commonly used for structured light calibration. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stl.h"


// sync reseau????
int sync_network=0;

// *****************************************************************
// Other global variables
int start_sleep = 0; //waiting time (in seconds) before the 1st showed pattern (option on the command line)
int end_sleep = 0; //waiting time (in seconds) after the last showed pattern (option on the command line)
int interval_sleep = 1; //waiting time (in seconds) after the last showed pattern (option on the command line)
int verbose = FALSE; //show the details on each pattern (option on the command line)
int XSize = 1024, YSize = 768; //size of the screen
int nb_patterns;
int mode=MODE_PATTERNS;
// *****************************************************************


void start_patterns_schedule(osgViewer::Viewer *viewer,osg::Image* buf,imgu *I,int port,double freq)
{
  // call to the patterns schedule function in ltstl.c
  patterns_schedule(viewer,buf,I,XSize, YSize, start_sleep, interval_sleep, end_sleep, verbose, nb_patterns,sync_network,mode,port,freq);
}


int main(int argc, char **argv) 
{
  int i;
  char inFilename[255];
  int xpos,ypos,port;
  imgu *I;
  long int seed;
  double freq;

  port=STL_PORT;
  xpos=0;
  ypos=0;
  seed=0;

  //Option parameters
  for(i=1;i<argc;i++) 
  {
    if( strcmp(argv[i],"-i")==0 && i+1<argc ) 
    {
      strcpy(inFilename,argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-x")==0 || strcmp(argv[i],"-XSize")==0) && i+1<argc ) 
    {
      XSize = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-y")==0 || strcmp(argv[i],"-YSize")==0) && i+1<argc ) 
    {
      YSize = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-XPos")==0 && i+1<argc ) 
    {
      xpos = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-YPos")==0 && i+1<argc ) 
    {
      ypos = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-port")==0 && i+1<argc ) 
    {
      port = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"--nb_patterns")==0 && i+1<argc ) 
    {
      nb_patterns = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"--mode")==0 ) {
      if (strcmp(argv[i+1],"gamma")==0) mode=MODE_GAMMA;
      else if (strcmp(argv[i+1],"noise")==0) mode=MODE_NOISE;
      else if (strcmp(argv[i+1],"color")==0) mode=MODE_COLOR;
      else if (strcmp(argv[i+1],"image")==0) mode=MODE_IMAGE;
      else mode=MODE_PATTERNS;
      i+=1;
	  continue;
    }
    /// synchro frame par frame
    if( strcmp(argv[i],"--sync")==0 ) {
  	  sync_network=1;
	  continue;
    }
    if( strcmp(argv[i],"-seed")==0 && i+1<argc ) {
  	  seed=atoi(argv[i+1]);
          i+=1;
          continue;
    }
    if( strcmp(argv[i],"-freq")==0 && i+1<argc ) {
  	  freq=atof(argv[i+1]);
          i+=1;
          continue;
    }
    if( (strcmp(argv[i],"-s")==0 || strcmp(argv[i],"--start_sleep")==0) && i+1<argc ) 
    {
      start_sleep = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( (strcmp(argv[i],"-e")==0 || strcmp(argv[i],"--end_sleep")==0) && i+1<argc ) 
    {
      end_sleep = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-interval")==0 && i+1<argc ) 
    {
      interval_sleep = atoi(argv[i+1]);
      i+=1;
      continue;
    }
    if( strcmp(argv[i],"-v")==0 || strcmp(argv[i],"--verbose")==0 ) 
    {
      verbose = TRUE;
      continue;
    }
    if( strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0 ) 
    {
      LT_PRINTF("shell_exe_ltstl %s\n\n",VERSION);
      LT_EXIT(0);
    }
    if( strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0 ) 
    {
      LT_PRINTF("\nUsage: ./shell_exe_ltstl [OPTIONS]\n\n");
      LT_PRINTF("Options: \n");
      LT_PRINTF("   -x int, --XSize int\n");
      LT_PRINTF("         Specify the width of the screen (default = 1024). 'int' stands for an integer.\n\n");
      LT_PRINTF("   -y int, --YSize int\n");
      LT_PRINTF("         Specify the height of the screen (default = 768). 'int' stands for an integer.\n\n");
      LT_PRINTF("   -s int, --start_sleep int\n");
      LT_PRINTF("         Specify 'int' seconds of waiting with the black screen at the start of the patterns sequence (default = 1).\n");
      LT_PRINTF("         'int' stands for an integer.\n\n");
      LT_PRINTF("   -e int, --end_sleep int\n");
      LT_PRINTF("         Specify 'int' seconds of waiting with the black screen at the end of the sequence (default = 1).\n");
      LT_PRINTF("         'int' stands for an integer.\n\n");
      LT_PRINTF("   --sync\n");
      LT_PRINTF("         synchronize avec le reseau\n");
      LT_PRINTF("   -v, --verbose\n");
      LT_PRINTF("         Show the details of each pattern on the shell.\n\n");
      LT_PRINTF("   -V, --version\n");
      LT_PRINTF("         Show the version of the program.\n\n");
      LT_PRINTF("   -h, --help\n");
      LT_PRINTF("         Show the help screen.\n\n");
      exit(0);
    }
  }

    matRandSeed(seed);

    I=NULL;
    if (mode==MODE_COLOR || mode==MODE_GAMMA)
    {
      imguAllocate(&I,XSize,YSize,3);
    }
    else
    {
      imguAllocate(&I,XSize,YSize,1);
    }

    // construct the viewer.
    osgViewer::Viewer viewerProj;

	// to remove a bug because we update the image almost without sync
    viewerProj.setThreadingModel( osgViewer::Viewer::SingleThreaded);

    double xmin=0.0;
    double ymin=0.0;
    double xmax=1.0;
    double ymax=1.0;

    osg::Vec3 top_left(    xmin,ymax,0.0);
    osg::Vec3 bottom_left( xmin,ymin,0.0);
    osg::Vec3 bottom_right(xmax,ymin,0.0);
    osg::Vec3 top_right(   xmax,ymax,0.0);

    // create geometry
    osg::Geometry* geom = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);

    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f, 0.0f);
    (*texcoords)[1].set(0.0f, 1.0f);
    (*texcoords)[2].set(1.0f, 1.0f);
    (*texcoords)[3].set(1.0f, 0.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,0.0f,1.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

    // disable display list so our modified tex coordinates show up
    geom->setUseDisplayList(false);

    osg::Image* img = new osg::Image;
    osg::TextureRectangle* texture;
    texture = new osg::TextureRectangle(img);

    // pour voir les pixels
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);

    texture->setBorderColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture::CLAMP_TO_BORDER);

    osg::TexMat* texmat; // texture matrix for scale/offset/aspect
    texmat = new osg::TexMat;
    texmat->setScaleByTextureRectangleSize(true);

	// scale l'image et translate (en pixels de l'image)
    //vv->texmat->setMatrix(osg::Matrix::scale(1,1,1)*osg::Matrix::translate(0,0,0.0));

    // setup state
    osg::StateSet* state;
    state = geom->getOrCreateStateSet();
    state->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
    state->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

    // to see under the current image
    state->setMode(GL_BLEND,osg::StateAttribute::ON);
    state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


    // turn off lighting
    state->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    // install 'update' callback
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    //geode->setUpdateCallback(new TexturePanCallback(texmat));

    osg::Group* root = new osg::Group;

    osg::ClearNode* clearNode = new osg::ClearNode;
    clearNode->setRequiresClear(false); // we've got base and sky to do it.

    root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    root->addChild(clearNode);

    osg::Camera* camera = new osg::Camera;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(0);
    camera->setAllowEventFocus(false);

    camera->addChild(geode);
    root->addChild(camera);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = xpos;
    traits->y = ypos;
    traits->width = XSize;
    traits->height = YSize;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

    gc->setClearColor(osg::Vec4f(0.0f,0.0f,0.0f,0.0f));
    gc->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //gc->setClearMask(GL_DEPTH_BUFFER_BIT);
    //gc->setClearMask(0);

    viewerProj.getCamera()->setGraphicsContext(gc.get());
    viewerProj.getCamera()->setViewport(new osg::Viewport(0,0, traits->width, traits->height));

    viewerProj.setSceneData(root);

    viewerProj.realize();

	// Force la dimension et la position de la fenetre
	// Les preferences de fenetre de l'OS semblent avoir preseance
	// sur le GraphicsContext, et donc la barre de menu de GNOME empeche
	// la fenetre d'etre en (0,0).
	osgViewer::Viewer::Windows windows;
	viewerProj.getWindows(windows,true);
	if (windows[0]) {
		windows[0]->setWindowRectangle(xpos,ypos,XSize,YSize);
		windows[0]->setCursor( osgViewer::GraphicsWindow::NoCursor );
	}

    if (mode!=MODE_IMAGE) start_patterns_schedule(&viewerProj,img,I,port,freq);
    else
    {
      imguLoad(&I,inFilename,LOAD_16_BITS);
      imguScale(&I,I,((double)(XSize))/I->xs,((double)(YSize))/I->ys);
      //imguSave(I,"test.png",1,SAVE_16_BITS);
      if( I->cs==1 ) {  
        img->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
      }else if( I->cs==3 ) {
        img->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
      }else if( I->cs==4 ) {
        img->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
      }
      viewerProj.frame();
      while(1) sleep(1);
    }

    imguFree(&I);
 
    LT_EXIT(0);
}

