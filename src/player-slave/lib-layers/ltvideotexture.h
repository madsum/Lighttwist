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

#ifndef LTVIDEOTEXTURE_H
#define LTVIDEOTEXTURE_H


// classe qui change la copie d'image...

#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TexMat>

#ifdef __cplusplus
extern "C" {
#endif

#include <avcodec.h>
#include <avformat.h>

//#define SWS_SCALE
//#ifdef SWS_SCALE
#include <swscale.h>
//#endif

#ifdef __cplusplus
}
#endif

#include <pthread.h>
#include <imgu/imgu.h>
#include "utils/ltsys.h"

#define MSG_SILENT 0
#define MSG_VERBOSE 1

#define IMAGE_QUEUE_SIZE 20
#define IMAGE_QUEUE_DIFF 10

class LTVideoTexture2D : public osg::Texture2D {

 public:
  ~LTVideoTexture2D();
  LTVideoTexture2D(char *movieName,int clamp_to_border,int rescale=1,int verbose=MSG_SILENT);

  void decodeThread();
  int next(int nb);
  int next_img(imgu **I);
  int seek(int64_t framenum);
  int go(int64_t framenum);
  void apply(osg::State& state) const; // quand la texture change...

 public:

  //osg::ref_ptr<osg::Image> tex_image;
  osg::ref_ptr<osg::Image> _padded_image[IMAGE_QUEUE_SIZE];
  unsigned char *_padded_buffer[IMAGE_QUEUE_SIZE]; // malloc
  AVFrame         *_pFrameRGB[IMAGE_QUEUE_SIZE];

  inline float getXRatio() { return (float)_dec_width/_tex_width; };
  inline float getYRatio() { return (float)_dec_height/_tex_height; };

  inline double getFPS() { if (_videoStream==-1 ||
                               _pFormatCtx->streams[_videoStream]->time_base.num==0) return 0.0;
                           else return (((double)(_pFormatCtx->streams[_videoStream]->time_base.den))/_pFormatCtx->streams[_videoStream]->time_base.num); };

  inline double getDuration() { return ((double)(_pFormatCtx->duration))/AV_TIME_BASE; };
  inline int getWidth() { return _dec_width; };
  inline int getHeight() { return _dec_height; };

  int64_t getFrame() { return _frame; }

 protected:
  void stream_construct(char *movieName, int rescale);
  osg::ref_ptr<osg::TexMat> _tm; // pour ajuster le img/img2

  AVFormatContext *_pFormatCtx;
  int             _videoStream;
  AVCodecContext  *_pCodecCtx;
  AVFrame         *_pFrame;

  AVPacket        _packet;

  SwsContext *_dec_convert_ctx;


  bool _threadRunning;
  int64_t _imgQueueIdxIn;
  int64_t _imgQueueIdxOut;
  pthread_mutex_t _imgQueueAccess; // acces a la queue imgQueue

  pthread_t _thread;
  bool _dropNextFrame;
  int64_t _nbDroppedFrames;
  bool _waitForKeyFrame;
  int64_t _frame;
  int64_t _nb_frame_total;

  int  p_len , n_lg;
  int             _width2 , _height2;
  int _img_width,_img_height; // taille originale. ne sert a rien dans operator


  int _dec_width,_dec_height;
  int _tex_width,_tex_height;

  int _rescale; // 0=leave texture alone. 1=rescale coord to fill 0..1
  int next_pow2(int val);

  bool _threadSlowDown;
  int _threadSlowCount;

 private:
};




#endif //LTVIDEOTEXTURE_H
