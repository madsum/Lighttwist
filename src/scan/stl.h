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
 * ltstl.h  -  Main library for the display of the patterns.
 */ 

#ifndef LTSTL_H
#define LTSTL_H

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/Notify>
#include <osg/TextureRectangle>
#include <osg/Texture>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/Group>
#include <osg/TexMat>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osgText/Text>
#include <osgDB/Registry>
#include <osg/Camera>
#include <osg/io_utils>
#include <osg/GraphicsContext>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <imgu/imgu.h>
#include <bmc/bmc.h>

#include "utils/ltsys.h"
#include "remote/ltports.h"

#define VERSION "v1.0"

#define FALSE 0
#define TRUE 1

#define MODE_PATTERNS 0
#define MODE_NOISE 1
#define MODE_GAMMA 2
#define MODE_COLOR 3
#define MODE_IMAGE 4

#define BS 255

void patterns_schedule(osgViewer::Viewer *viewer,osg::Image* buf,imgu *I,int XSize, int YSize, int start_sleep, int interval_sleep,int end_sleep, int verbose, int nb_patterns,int sync,int mode,int port,double freq);

#endif

