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

#ifndef LTIMGUPLUGINLAYER_H
#define LTIMGUPLUGINLAYER_H

#include <osg/Image>
#include <imgu/imgu.h>

#include "ltlayer.h"
#include "ImguPluginTexture.h"

#define NB_IMGU 20

class OSG_EXPORT LTImguPluginLayer : public LTLayer {

  public:

    LTImguPluginLayer( const char* shaderFile,
                       float xTexMin, float xTexMax, float yTexMin, float yTexMax);

    virtual ~LTImguPluginLayer();
    int addPlugin(const char *name, const char *classe, const char *params);
    int doneAddingPlugins();
    int setPluginParam(const char* pluginName, char *param_name, char *param_value);
    int setPluginParam(int plugin_idx, char *param_name, char *param_value);
    int invokePluginCommand(const char* pluginName, char *cmd);
    int invokePluginCommand(int plugin_idx, char *cmd);

  protected:

  private:
    ImguPluginTexture* _imguTex;

};


#endif // LTIMGUPLUGINLAYER_H
