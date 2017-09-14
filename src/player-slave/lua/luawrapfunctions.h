#ifndef LUAWRAPFUNCTIONS_H
#define LUAWRAPFUNCTIONS_H

//
// un certain nombre de define et variables pour les fonctions
// qui gerent les layers, dans le module 'play' d'interface lua
//
//

#include <osg/ClearNode>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osg/PositionAttitudeTransform>
#include <osgViewer/Viewer>
#include <osg/Timer>
#include <osg/Image>
#include <osg/Texture2D>

#include "ltlayer.h"
#include "lt3dlayer.h"
#include "ltmulti3dlayer.h"
#include "ltimgupluginlayer.h"
#include "ltcapturelayer.h"
#include "ltvideotexture.h"
#include "lttileinfo.h"
#include "lttiledvideolayer.h"
#include "ltvideocontroller.h"
#include "ltvideolayer.h"
#include "ltcolorlayer.h"

#include <sys/time.h>



//
// nb de layer en meme temps
//
#define NB_LAYER        32

//
// les variables globales sont declarees dans play.cpp
//

extern osg::ref_ptr<LTLayer> layers[NB_LAYER]; // valid() dit si actif
extern LTVideoController* vidCtrls[NB_LAYER];

extern char dataPath[200];

extern osg::Group* rootNode;
extern osg::ClearNode* clearNode;

extern int machine_num;

extern char SHADER_FILM_PATH[255];
extern char SHADER_3D_PATH[255];
extern char SHADER_STEREO_VERT_PATH[255];
extern char SHADER_STEREO_FRAG_PATH[255];
extern int STEREO_MODE;

extern float xTexMin,xTexMax,yTexMin,yTexMax;

extern double timeOffset; // reference_time = local_time() + timeOffset

extern osg::ref_ptr<osg::Image> lutTexImage;
extern osg::ref_ptr<osg::Texture2D> lutTex;
extern osg::ref_ptr<osg::Image> blendTexImage;
extern osg::ref_ptr<osg::Texture2D> blendTex;
extern float voffsetx;
//
// les fonctions utilisables de l'exterieur
// ces fonctions sont utilisables directement en lua
//

//void luawrappointer(lua_State* L,void *p);
int quit();
int set_alpha_mask(int num, const char* file);
int enableLUT( int num, bool b );
int load_lut_blend(const char* lutName, const char* blendName);

void *sebcreate(int a,int b);
int sebprocess(void *z);

int unload_layer(int num);
int unload_all();
int load_layer_image(int num,char *img_name, int swrap = 0, int twrap = 0);
int load_layer_tiled_movie(int num,char *movie_name);
int load_layer_movie(int num,char *movie_name, int swrap = 0, int twrap = 0);
int load_layer_multi3d(int num,char *model_name);
int load_layer_color(int num, float r, float g, float b, float a, int mode);
int load_layer_camera_crayon(int num);
int load_layer_stream_capture(int num,int port, int mtu);
int load_layer_capture(int num,int camid);
int load_layer_capture_with_blend(int num,int camid,double ratio);
int load_layer_capture_with_inverse(int num,int camid);
int load_layer_capture_with_distorsion(int num, int camid, char* map);
int load_layer_capture_with_scanner(int layer,int camid);
int load_layer_capture_with_plugins(int num,int camid,char *str_plugins);
int load_layer_crayon(int num);
int load_layer_plugin(int num, char* pname, char* pclass, char* param);
int load_layer_add_plugin(int num, char* pname, char* pclass, char* param);
int load_layer_ultime(int num, int camid);
int load_layer_dl1track(int num);

int set_color(int num, float r, float g, float b, float a);
int frame(int num,int fr);
double getFPS(int num);
int fade_out_all(float f);
int fade_in_all(float f);
int fade(int num,float fd);
int set_clear_node(double r, double g, double b);
int scaletrans(int num, float xs, float ys, float xy, float yt);
int queuepoint(double x, double y, double z);
int newline();
int newpage();
double getTime();
void setTime(double t);

// painture de gimp (le data est binaire)
int paint(int num,int bpp,int x,int y,int w,int h,void *data);

int set_plugin_param(int num, const char* pluginName, char *param_name, char *param_value);
int set_plugin_param(int num, int idx_plugin, char *param_name, char *param_value);
int invoke_plugin_command(int num, const char* pluginName, char *cmd);
int invoke_plugin_command(int num, int idx_plugin, char *cmd);

/*
int capture_scan_effect(int num,int onoff);
int capture_freeze_effect(int num,int onoff);
int capture_inverse_effect(int num,int onoff);
*/


//double getLocalTime();





#endif // LUAWRAPFUNCTIONS_H
