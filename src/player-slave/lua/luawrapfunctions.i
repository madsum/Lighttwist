
%module player


//
// Fonctions disponibles a l'exterieur pour appel direct d'un programme en c++
//

%{

    #include "luawrapfunctions.h"

    //void luawrappointer(lua_State* L,void *p);

%}


%wrapper %{


void luawrappointer(lua_State* L,void *p)
{
    //printf(">> lua wrap pointer 0x%08lx<<\n",p);
    SWIG_NewPointerObj(L,p,SWIGTYPE_p_void,0);
}


%}


//
// fonctions qui vont apparaitre dans le module lua 'player'
//
    #include "luawrapfunctions.h"
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
    int load_layer_capture_with_plugins(int num,int camid,char *str_filters);
    int load_layer_capture_with_blend(int num,int camid,double ratio);
	int load_layer_capture_with_inverse(int num,int camid);
	int load_layer_capture_with_distorsion(int num, int camid, char* map);
	int load_layer_capture_with_scanner(int num,int camid);
    int load_layer_capture(int num,int camid);
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

    int set_plugin_param(int num, const char* pluginName, char *param_name, char *param_value);
    int set_plugin_param(int num, int idx_plugin, char *param_name, char *param_value);
    int invoke_plugin_command(int num, const char* pluginName, char *cmd);
    int invoke_plugin_command(int num, int idx_plugin, char *cmd);

        

    // painture de gimp (le data est binaire)
    int paint(int num,int bpp,int x,int y,int w,int h,void *data);

    //int capture_scan_effect(int num,int onoff);
    //int capture_freeze_effect(int num,int onoff);
    //int capture_inverse_effect(int num,int onoff);


