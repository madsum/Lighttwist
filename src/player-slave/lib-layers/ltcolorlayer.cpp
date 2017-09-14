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

#include <osg/CameraNode>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Node>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>

#include <cstdlib>
#include <ctime>

#include "ltcolorlayer.h"

/******************************************************************************/

std::string COLOR_SHADER =
    "uniform sampler2D blend_tex;                                  \n" // 1
    "uniform vec4 color;                                           \n" // 2
    "uniform float gamma0;                                         \n" // 3
    "uniform float gamma1;                                         \n" // 4
    "uniform float gamma2;                                         \n" // 5
    "uniform float fade;                                           \n"
    "void main(void)                                               \n" // 6
    "{                                                             \n" // 7
    "vec3 outColor = color.rgb;                                    \n" // 8
    "float alpha = texture2D( blend_tex, gl_TexCoord[0].xy ).r;    \n" // 9
    "vec4 alphagamma;                                              \n" // 10
    "alphagamma.x = pow( alpha, 1.0 / gamma0 );                    \n" // 11
    "alphagamma.y = pow( alpha, 1.0 / gamma1 );                    \n" // 12
    "alphagamma.z = pow( alpha, 1.0 / gamma2 );                    \n" // 13
    "outColor *= vec3( alphagamma.x, alphagamma.y, alphagamma.z ); \n" // 14
    "gl_FragColor.rgb = outColor;                           \n"
     "if (alpha == 0) gl_FragColor.a = fade;                         \n"
    "else gl_FragColor.a = color.a * fade;                         \n"

    //"gl_FragColor.rgb = color.rgb;                           \n"
    //"gl_FragColor.a = color.a * fade;                           \n"
    "}\n";

/******************************************************************************/


LTColorLayer::LTColorLayer( float r, float g, float b, float a, int mode )
    : LTLayer( "", createTexture(32, 32) )
{
    _color.set( r, g, b, a );

    osg::StateSet* ss = _hud->getOrCreateStateSet();

    printf("new LTColorLayer: %f %f %f %f mode = %i\n", r, g, b, a, mode);
    osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
    switch (mode) {
    case 0:
        bf->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA); // LTLAYER DEFAULT!!
        break;
    case 1:
        bf->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE);
        break;
    case 2:
        bf->setFunction(osg::BlendFunc::DST_COLOR, osg::BlendFunc::SRC_COLOR);
    }

    //bf->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    //bf->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::ONE);
    //bf->setFunction(osg::BlendFunc::DST_COLOR, osg::BlendFunc::SRC_COLOR);
    //bf->setFunction(osg::BlendFunc::SRC_COLOR, osg::BlendFunc::ONE);
    ss->setAttributeAndModes(bf.get(),osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
    ss->setMode(GL_BLEND,osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);

    enableShader( false );
    loadGLSLShaderSource( COLOR_SHADER.c_str(), ss );
    _colorUF = new osg::Uniform( "color", _color );
    ss->addUniform( _colorUF );
    enableShader( true );
}

/******************************************************************************/

LTColorLayer::~LTColorLayer()
{
    printf("KILL Cyclo 3DLayer\n");
}


/******************************************************************************/

void LTColorLayer::setColor( float r, float g, float b, float a ) {
    _color.set( r, g, b, a );
    _colorUF->set( _color );
}

/******************************************************************************/

void LTColorLayer::setColor( osg::Vec4& col ) {
    setColor( col.r(), col.g(), col.b(), col.a() );
}

/******************************************************************************/

void LTColorLayer::setColor( osg::Vec3& col ) {
    setColor( col.x(), col.y(), col.z(), 1.0 );
}

/******************************************************************************/

