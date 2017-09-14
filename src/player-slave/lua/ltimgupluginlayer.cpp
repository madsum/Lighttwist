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

//#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/FragmentProgram>
#include <osg/CameraNode>
#include <osg/TexMat>

#include <osgDB/ReadFile>

#include "ltimgupluginlayer.h"
//#include <utils/gamma.h>

/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


/// sert pour les video
LTImguPluginLayer::LTImguPluginLayer(const char* shaderFile,
                                     float xTexMin, float xTexMax, float yTexMin, float yTexMax)
    : LTLayer( shaderFile, NULL, 1.0, 1.0, xTexMin, xTexMax, yTexMin, yTexMax, true ) {

    setName( "LTImguPluginLayer" );

    _imguTex = new ImguPluginTexture();

    _imguTex->setInternalFormat(GL_RGBA);
    _imguTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    _imguTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    _imguTex->setResizeNonPowerOfTwoHint(false);

    _imguTex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    _imguTex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_BORDER);//REPEAT);//

    this->setTexture(_imguTex);

    printf("load_layer_image: texture size = %i x %i\n", _imguTex->getTextureWidth() , _imguTex->getTextureHeight() );

    printf("SHADER is '%s'\n",shaderFile);

    //_imguTex->addPlugin("gige","camera","-camid 13139");
    //_imguTex->addPlugin("inverse","filter","");
    //_imguTex->doneAddingPlugins();
}


/******************************************************************************/

LTImguPluginLayer::~LTImguPluginLayer()
{
  printf("KILL IMGU PLUGIN LAYER\n");
}



/******************************************************************************/

int LTImguPluginLayer::addPlugin(const char *name, const char *classe, const char *params) {
    return _imguTex->addPlugin(name,classe,params);
}


/******************************************************************************/

int LTImguPluginLayer::doneAddingPlugins() {
    return _imguTex->doneAddingPlugins();
}


/******************************************************************************/

int LTImguPluginLayer::setPluginParam(const char* pluginName, char *param_name, char *param_value) {
    return _imguTex->setPluginParam(pluginName, param_name, param_value);
}

int LTImguPluginLayer::setPluginParam(int plugin_idx, char *param_name, char *param_value) {
    return _imguTex->setPluginParam(plugin_idx, param_name, param_value);
}


/******************************************************************************/

int LTImguPluginLayer::invokePluginCommand(const char* pluginName, char *cmd) {
    return _imguTex->invokePluginCommand(pluginName, cmd);
}

int LTImguPluginLayer::invokePluginCommand(int plugin_idx, char *cmd) {
    return _imguTex->invokePluginCommand(plugin_idx, cmd);
}



/******************************************************************************/
