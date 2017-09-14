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

#ifndef LTLAYER_H
#define LTLAYER_H

#include <osg/Group>
#include <osg/CameraNode>
#include <osg/Texture2D>
#include <osg/FragmentProgram>

#include <string.h>

#include <imgu/imgu.h>
#include "utils/ltsys.h"

#define STEREO_NONE 0
#define STEREO_P_LEFT 1
#define STEREO_P_RIGHT 2
#define STEREO_RC 3

#define TEX_ID 0
#define LUT_ID 1
#define BLEND_ID 2
#define ALPHA_ID 3

class OSG_EXPORT LTLayer : public osg::Group {

  public:

    LTLayer( const char* shaderFile,
             osg::Texture2D* tex, // image or RTT
             float xScale=1.0f,float yScale=1.0f,
             float xTexMin=0.0,float xTexMax=1.0,
             float yTexMin=0.0,float yTexMax=1.0,
             bool restrictContentToLUT = false,float slit=180.0 );


    LTLayer();
    ~LTLayer();

    virtual void reset();

    void setTexture(osg::Texture2D* tex);

    inline osg::Texture* getTexture() { return _texture.get(); }

    void enableShader( bool b = true );
    void disableShader() { enableShader(false); }

    inline void setFade(float f) {
		_fade=f;
		_fadeUF->set(_fade);
		// on elimine ce layer du rendu si fade=0
		if( f<1e-3 ) {
			this->setNodeMask(0x0);
		} else {
			this->setNodeMask(0x1);
		}
	}
    inline float getFade(void) { return _fade; }

    void setAlphaMask( osg::Image* img );
    void setAlphaMask( osg::Texture2D* tex );
    void enableAlphaMask( bool b );

    void reshapeHud( double left, double right,
                     double bottom, double top, double zoff,
                     double xRTTMin, double xRTTMax,
                     double yRTTMin, double yRTTMax );

    void updateSlit( float slit );
    void updateGamma( vector3 gamma );
    virtual void setScaleTrans( float xs, float ys,
                                float xt, float yt );

    void paintTexture( unsigned char *buf );
    void paintTexture( int bpp,int x,int y,int width,int height,unsigned char *bin );


    static osg::Texture2D* createTexture(unsigned int tex_width, unsigned int tex_height);

    static osg::Image* loadLUT( const char* fileName,
                                float* xMin, float* xMax,
                                float* yMin, float* yMax, float voffset=0.0);

    static osg::Image* generateLUT(int machine_num,int nb_machines,int height,int width,
                                   float* xMin, float* xMax,
                                   float* yMin, float* yMax,
                                   float leftborder, float rightborder);

    static osg::Texture2D* createLutTexFromImg( osg::Image* img );
    static osg::Texture2D* createBlendTexFromImg( osg::Image* img );

    static osg::Texture2D* getLutTex() { return _lutTex; }
    static osg::Image* getLutTexImage() { return _lutTexImage; }
    static vector4* getLutBB() { return &_lutBB; }

    static void setGamma( vector3 v ) { _gamma[0]=v[0];_gamma[1]=v[1];_gamma[2]=v[2]; }
    static void setColor( matrix3 m ) { for(int i=0;i<9;i++) {_color[i]=m[i];} }

    // utiliser ceci pour normaliser l'image de ls LUT
    static void  normalizeLUT(osg::Image* img, float xmin,float xmax,
                              float ymin,float ymax,float xscale,float yscale);

    void enableLUT( bool b );

    //Animation parameters... protected/private and leading underscore please..
    float tx , ty;
    float rot_angle,rot_cx , rot_cy;
    float aspect_x , aspect_y;
    float scale;

    //0:clamp texture , 1:repeat texture
    int is_tex_repeat;
    float mat[16];


    bool isBroken() { return _broken; }

  protected:

    bool _broken;

    osg::ref_ptr<osg::Program> _shaderProgram;

    int _precrop; // 0 = LUT originale, 1=LUT resize entre 0 et 1

    osg::ref_ptr<osg::Texture2D> _texture; // on a besoin, mais on ne devrait pas creer
    osg::ref_ptr<osg::CameraNode> _hud;
    osg::ref_ptr<osg::Geode> _hudQuad;

    osg::ref_ptr<osg::Uniform> _enableLUTUF;
    osg::ref_ptr<osg::Uniform> _gammaUF0,_gammaUF1,_gammaUF2; // gamma uniform
    osg::ref_ptr<osg::Uniform> _fadeUF; // fade uniform

    //  int _texWidth; // eventuellement inutile. sert seulement en 3d
    //  int _texHeight;

    // max tex coord of the image in the texture... usually 1.0,1.0
    static float _xLutScale;
    static float _yLutScale;

    osg::ref_ptr<osg::Uniform> _xLutScaleUF; // scale pour le film
    osg::ref_ptr<osg::Uniform> _yLutScaleUF;

    osg::ref_ptr<osg::Uniform> _slit;

    osg::Matrix3 _homography; // homographie qui remplace les scale/trans
    osg::ref_ptr<osg::Uniform> _homographyUF;

    // fade value for transparency
    float _fade; // normalement 1.0 (opaque)

    osg::Texture2D* _alphaTex;
    osg::Image* _alphaTexImage;
    osg::ref_ptr<osg::Uniform> _useAlphaUF;

    // vient de la LUT (utile seulement pour 3dlayer)
    static float _xLutMin;
    static float _xLutMax;
    static float _yLutMin;
    static float _yLutMax;
    osg::ref_ptr<osg::Uniform> _xLutMinUF;
    osg::ref_ptr<osg::Uniform> _xLutMaxUF;

    osg::ref_ptr<osg::Uniform> _xRttTexMinUF;
    osg::ref_ptr<osg::Uniform> _xRttTexMaxUF;
    osg::ref_ptr<osg::Uniform> _yRttTexMinUF;
    osg::ref_ptr<osg::Uniform> _yRttTexMaxUF;

    osg::CameraNode* createHUD( float xRTTMin,float xRTTMax,
                                float yRTTMin,float yRTTMax );

    osg::CameraNode* createHUDCamera(osg::Node* hudModel);

    osg::Geode* createQuadGeode(float xoff, float yoff, float zoff,
                                float xRTTMin, float xRTTMax,
                                float yRTTMin, float yRTTMax);

    osg::Geode* createQuadGeode(float xmin, float xmax,
                                float ymin, float ymax, float zoff,
                                float xRttTexMin, float xRttTexMax,
                                float yRttTexMin, float yRttTexMax);

    int loadGLSLShader( const char* fileName, osg::StateSet* stateset );
    int loadGLSLShaderSource( const char* source, osg::StateSet* stateset );

    static osg::Texture2D* _lutTex;
    static osg::Image* _lutTexImage;
    static vector4 _lutBB; // vector4 from imgu [xmin, xmax, ymin, ymax]
    static bool _lutNormalized;

    static osg::Texture2D* _blendTex;
    static osg::Image* _blendTexImage;
    static vector3 _gamma;
    static matrix3 _color;

  private:
    bool _restrictContentToLUT;

};



#endif // LTLAYER_H


