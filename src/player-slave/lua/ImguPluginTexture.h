#ifndef IMGU_VIDEO_TEXTURE_H
#define IMGU_VIDEO_TEXTURE_H


//
// ImguPluginTexture using imgu plugin system to update a texture in real time, using texture callback
//


#include <osg/StateAttribute>
#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TexMat>

#include <cstring>

//#include <imgu/imgu8.h>
#include <imgu/imgu.h>

#define NB_MAX_PLUGINS 10

#ifdef __cplusplus
extern "C" {
#endif

//
// example of use:
//
// ImguPluginTexture* vt = new ImguPluginTexture("movie.avi");
// ...
// osg::StateSet* ss = ...
// ...
// ss->setTextureAttributeAndModes(0, vt, osg::StateAttribute::ON);
//
//

class ImguPluginTexture : public osg::Texture2D {

 public:
  ~ImguPluginTexture();
  ImguPluginTexture();

  virtual void apply(osg::State& state) const; // quand la texture est dessinee
  int addPlugin(const char *name, const char *classe, const char *in_params);
  int doneAddingPlugins();
  int setPluginParam(const char* pluginName, char *param_name, char *param_value);
  int setPluginParam(int idx_plugin, char *param_name, char *param_value);
  int invokePluginCommand(const char* pluginName, char *cmd);
  int invokePluginCommand(int idx_plugin, char *cmd);

 private:
  char _queueSuffix[20];
  rqueue* _Qrecycle; // image recycling between video decoding and texture displaying
  rqueue* _Qplugins[NB_MAX_PLUGINS];
  int _tids[NB_MAX_PLUGINS];
  int _nbPlugins;

  osg::Image* _im;
};

#ifdef __cplusplus
}
#endif



#endif

