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

// classe qui change la copie d'image...

#include <osg/Texture>
#include <osg/Texture2D>
#include <osg/TexMat>

#include <errno.h>
#include <string>
#include <math.h>

//#define USE_PROFILER
#ifdef HAVE_PROFILER
  #include <profiler.h>
#endif

#include "ltvideotexture.h"

#include <osgDB/WriteFile>

#define THREAD_SLOW_COUNT 10


/************************************************************************************/

extern "C" void* threadStartup(void*);

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

//// Destructeur de video
LTVideoTexture2D::~LTVideoTexture2D()
{

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_dump();
  #endif
#endif

  _threadRunning = false;

  // queue d'images
  pthread_mutex_destroy(&_imgQueueAccess);

}

/************************************************************************************/

//// constructeur de video
LTVideoTexture2D::LTVideoTexture2D(char *movieName,int clamp_to_border, int rescale,int verbose)
{
  AVCodec         *_pCodec;
    //printf("init videotex %s\n",movieName);
  int k;
    //on a rien joue encore
    _frame=-1;

    _rescale=rescale;
    _threadSlowDown = false;
    _threadSlowCount = THREAD_SLOW_COUNT;
    // create a texture matrix
    _tm = new osg::TexMat();
    _tm->setDataVariance( osg::Object::DYNAMIC );

    av_register_all();

    setDataVariance( osg::Object::DYNAMIC );

    k = av_open_input_file(&_pFormatCtx, movieName, NULL, 0, NULL);

    if (k != 0) {
      fprintf(stderr,"Can't open %s. error %i (%i, %i, %i)\n" , movieName, k,
              AVERROR_IO, AVERROR_NOFMT,AVERROR_NUMEXPECTED );
      exit(1);
    }


    /*! Retrieve streams information */
    if(av_find_stream_info(_pFormatCtx)<0){
        fprintf(stderr,"Can't find a stream.\n");
        exit(1);
    }

    /*!\brief Dump information about file onto standard error
     *
     *
     */

    if (verbose==MSG_VERBOSE)
    {
      fprintf(stdout,"ID_LENGTH=%f\n",getDuration());
      dump_format(_pFormatCtx, 0, movieName, 0);
    }

    /*!\briefFind a stream
     *
     *
     */
    _videoStream=-1;
    for(unsigned int i=0; i<_pFormatCtx->nb_streams; i++)
    {
        if(_pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
        {
            _videoStream=i;
            break;
        }
    }


    if(_videoStream==-1){
        fprintf(stderr,"Could not find a video stream.\n");
        exit(1);
    }

    /*!\brief Get a pointer to the codec context for the video stream
     *
     */

    _pCodecCtx = _pFormatCtx->streams[_videoStream]->codec;
    _dec_width = _pCodecCtx->width;
    _dec_height = _pCodecCtx->height;
    if (verbose==MSG_VERBOSE)
    {
      fprintf(stdout,"ID_VIDEO_WIDTH=%d\n",getWidth());
      fprintf(stdout,"ID_VIDEO_HEIGHT=%d\n",getHeight());
      fprintf(stdout,"ID_VIDEO_FPS=%f\n",getFPS());
    }

    /*!\brief Find the decoder for the video stream
     *
     *
     */

    _pCodec = avcodec_find_decoder(_pCodecCtx->codec_id);
    if(_pCodec==NULL){
        fprintf(stderr,"No suitable video codec found.\n");
        exit(1);
    }

    /*!\brief Open codec
     *
     *
     */
    if(avcodec_open(_pCodecCtx, _pCodec)<0){
        fprintf(stderr,"Can't open codec.\n");
        exit(1);
    }

    _pFrame = avcodec_alloc_frame();

    /*!\brief Allocate an AVFrame structure
     *
     *
     */

    /*_pFrameRGB = avcodec_alloc_frame();
    if(_pFrameRGB==NULL){
        fprintf(stderr,"Can't allocate an AVFrame.\n");
        exit(1);
        }*/

    /*!\brief Calculate the nearest pow 2 of image dimensions
     *
     *
     */

    _tex_width=next_pow2(_pCodecCtx->width);
    _tex_height=next_pow2(_pCodecCtx->height);

    /*!
     *Portion de l'image par rapport a la texture de pow2
     */
    _width2 = _height2=1;
    while(_width2< _pCodecCtx->width) _width2 *=2;
    while(_height2< _pCodecCtx->height) _height2 *=2;

    float xtcoord,ytcoord;

    xtcoord = (float)_dec_width/_tex_width;
    ytcoord = (float)_dec_height/_tex_height;
    //printf("Movie Name: %s \n" , movieName);
    //printf("Movie Size: %d %d \n" , pCodecCtx->width , pCodecCtx->height );
    //printf("Texture Size: %d %d \n" , _width2 , _height2 );

    // arrange la texture
    _tm->setMatrix(osg::Matrix::scale(xtcoord,ytcoord,1.0));


    for ( int i = 0; i < IMAGE_QUEUE_SIZE; i++ ) {

        _padded_buffer[i] = (unsigned char*) calloc( sizeof(unsigned char) , _tex_width*_tex_height*3 );

      _padded_image[i] = new osg::Image();
      _padded_image[i]->setImage( _tex_width, _tex_height, 1, 3,
                                 GL_RGB, GL_UNSIGNED_BYTE,
                                 _padded_buffer[i], osg::Image::USE_MALLOC_FREE, 1 );
      _padded_image[i]->setDataVariance( osg::Object::DYNAMIC );

      _pFrameRGB[i] = avcodec_alloc_frame();
      if(_pFrameRGB[i]==NULL){
        fprintf(stderr,"Can't allocate an AVFrame.\n");
        exit(1);
      }

      avpicture_fill((AVPicture *)_pFrameRGB[i], _padded_buffer[i],
                     PIX_FMT_RGB24, _tex_width, _tex_height);

    }
    // ajuste l'image de cette texture...
    this->setImage(_padded_image[0].get());


#ifdef HAVE_LIBSWSCALE
    _dec_convert_ctx = sws_getContext(_dec_width, _dec_height,
                                      _pCodecCtx->pix_fmt,
                                      _dec_width, _dec_height,
                                      PIX_FMT_RGB24, SWS_FAST_BILINEAR,
                                      NULL, NULL, NULL);
    if(_dec_convert_ctx == NULL)
    {
        fprintf(stderr, "Cannot initialize the conversion context!\n");
        exit(1);
    }
#endif

    this->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    this->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    if (clamp_to_border)
    {
      this->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
      this->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    }
    else
    {
      this->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
      this->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);
    }

    // init la queue de frames
        // nb images in queue is always IdxIn-IdxOut
    _imgQueueIdxIn = 0; // next freespace to read an image
    _imgQueueIdxOut = 0; // next available loaded image, when > IdxIn
    pthread_mutex_init(&_imgQueueAccess,NULL);

    k = pthread_create( &_thread, NULL,
                        threadStartup, this );
    //printf("LTVideoTexture2D: pthread got %d\n",k);

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
    profiler_init();
  #endif
#endif

}


/************************************************************************************/
/************************************************************************************/
/************************************************************************************/


#define MAX_VIDEOQ_SIZE 10

int usec_sleep(int usec_delay)
{
#ifdef HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec  =  usec_delay / 1000000;
    ts.tv_nsec = (usec_delay % 1000000) * 1000;
    return nanosleep(&ts, NULL);
#else
    return usleep(usec_delay);
#endif
}

/************************************************************************************/

extern "C" void* threadStartup(void* ptr) {
  LTVideoTexture2D* obj = (LTVideoTexture2D*) ptr;
  obj->decodeThread();
  return NULL;
}

/************************************************************************************/

void LTVideoTexture2D::decodeThread() {

  int frameFinished;
  int qidx;

  printf("Starting decodeThread...\n");
  _threadRunning = true;

  _waitForKeyFrame = false;
  _dropNextFrame = false;
  _nbDroppedFrames = 0;

  seek(0);

  while (_threadRunning) {
    usec_sleep(1000);

    _threadSlowDown = false;
    if ( _threadSlowDown ) {
      if ( _threadSlowDown > 0 ) {
        _threadSlowCount--;
        usec_sleep(5000);
        continue;
      }
      else _threadSlowCount = THREAD_SLOW_COUNT;
    }

    // Is the queue filled enough? If so, loop.

    //pthread_mutex_lock(&_imgQueueAccess);
    int k= ( _imgQueueIdxIn - _imgQueueIdxOut >= IMAGE_QUEUE_DIFF );
    //pthread_mutex_unlock(&_imgQueueAccess);

    if ( k ) {
      /*if (!i_said_i_was_sleeping) {
         printf("read %Ld (%Ld), displayed %Ld (%Ld)... sleeping\n",
               _imgQueueIdxIn, _imgQueueIdxIn % IMAGE_QUEUE_SIZE,
               _imgQueueIdxOut, _imgQueueIdxOut % IMAGE_QUEUE_SIZE);
         i_said_i_was_sleeping = true;
         }*/
      continue;
    }

    if (!_threadRunning) break;

    //Ask ffmpeg for a new frame
    if( av_read_frame(_pFormatCtx, &_packet) >= 0 ) {

      if ( _dropNextFrame ) {
        _nbDroppedFrames++;
        _waitForKeyFrame = true;
      }

      if ( _waitForKeyFrame && _packet.flags != PKT_FLAG_KEY ) {
        //printf("_waitForKeyFrame... drop=%Ld\n", _nbDroppedFrames);
        av_free_packet( &_packet );
        continue;
      }

      _dropNextFrame = false;
      _waitForKeyFrame = false;

      if ( _packet.stream_index == _videoStream ) {

        //printf("TIME_BASE: %i / %i\n",
        //       _pFormatCtx->streams[_videoStream]->time_base.num,
        //       _pFormatCtx->streams[_videoStream]->time_base.den );

        //printf("packet: %Ld %Ld %i\n", _packet.pts, _packet.dts, _packet.pos);
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
        profiler_start("full decode");
        //Decode video frame
        profiler_start("avcodec_decode_video");
  #endif
#endif
        avcodec_decode_video( _pCodecCtx, _pFrame, &frameFinished,
                              _packet.data, _packet.size );
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
        profiler_stop("avcodec_decode_video");
  #endif
#endif

        if (!_threadRunning) break;

        //Did we get a video frame?
        if( frameFinished ) {

            // instead of locking the buffer for a long time,
            // we take the spot and mark it as busy, then free the lock immediately.
            // This means that the reader must check the busy status

            //pthread_mutex_lock(&_imgQueueAccess);
            qidx = _imgQueueIdxIn % IMAGE_QUEUE_SIZE;

            _padded_image[qidx]->dirty(); ///setModifiedCount(qidx);
            //#ifdef SKIP
            //   printf("decodeThread: reading frame %Ld [%i]... displayed %Ld\n", _imgQueueIdxIn,
            //   /*_padded_image[qidx]->getName().c_str(), */
            //     _padded_image[qidx]->getModifiedCount(),
            //  _imgQueueIdxOut);
            //#endif

          //Convert the image from its native format to RGB
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
          profiler_start("scale or convert");
  #endif
#endif

#ifdef HAVE_LIBSWSCALE
          //profiler_start("sws_scale");
          sws_scale(_dec_convert_ctx, _pFrame->data,
                    _pFrame->linesize, 0,
                    _pCodecCtx->height,
                    _pFrameRGB[qidx]->data, _pFrameRGB[qidx]->linesize);
          // profiler_stop("sws_scale");
#else
          //printf("img_convert\n");
          img_convert((AVPicture *)_pFrameRGB[qidx], PIX_FMT_RGB24,
                      (AVPicture*)_pFrame, _pCodecCtx->pix_fmt, _pCodecCtx->width,
                      _pCodecCtx->height);
          //printf("img_convert done\n");
#endif

          //sprintf(str, "image %04Ld.png", _imgQueueIdxIn);
          //std::string stdstr(str);
          //osgDB::writeImageFile(*_padded_image[qidx], stdstr);

#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
          profiler_stop("scale or convert");
  #endif
#endif
          _imgQueueIdxIn++;
          pthread_mutex_unlock(&_imgQueueAccess);
        }
#ifdef HAVE_PROFILER
  #ifdef USE_PROFILER
        profiler_stop("full decode");
  #endif
#endif
      }
      /*! Free the packet that was allocated by av_read_frame*/
      av_free_packet(&_packet);

    } else { /*if(av_read_frame... */

      /*Si le movie a fini...on reboucle*/
      if (_nb_frame_total<0) _nb_frame_total = _frame;

      //printf("decodeThread SEEEEEEEEEEEEEEEEEEEEEEEEEEEK\n");
      av_seek_frame(_pFormatCtx , _videoStream, 0, AVSEEK_FLAG_ANY);
    }
  }

  printf("videotexture decodeThread exiting.\n");

  // ffmpeg
  av_free(_pFrame);
  avcodec_close(_pCodecCtx);
  av_close_input_file(_pFormatCtx);

  for ( int i = 0; i < IMAGE_QUEUE_SIZE; i++ ) {
      ///printf("freeing padded buffer[%i]\n", i);
      ///if (_padded_buffer[i]) free(_padded_buffer[i]);
      /// printf("free done\n");
      ///if (_padded_image[i]) _padded_image[i]->unref();
      av_free(_pFrameRGB[i]);
  }

  // queue d'images
  //pthread_mutex_destroy(&_imgQueueAccess);

  pthread_exit(0);

}
/************************************************************************************/

//
// nb is the number of frames to display
// if 0, we are in sync. do nothing
// if <0, we are ahead... do nothing
// if >0, skip in the queue... at least you can empty the In/Out queue.
//
int LTVideoTexture2D::next(int nb) {

    if( nb<=0 || !_threadRunning ) return(0);

    // is the image queu empty? if so, we can't do anything but wait.
    if( _imgQueueIdxOut == _imgQueueIdxIn ) return(0);

    // we want to advance more images than the queue size? adjust to queuesize
    int qsize=_imgQueueIdxIn-_imgQueueIdxOut;
    if( nb > qsize ) nb=qsize;

    //pthread_mutex_lock(&_imgQueueAccess);
    int qidx = (_imgQueueIdxOut+(nb-1)) % IMAGE_QUEUE_SIZE;

    //printf("next: displaying frame %Ld (%i)... using [%i]\n",  _imgQueueIdxOut,
    //    qidx,  _padded_image[qidx]->getModifiedCount() );

  this->setImage( _padded_image[qidx].get() );
  //this->setName( _padded_image[_imgQueueIdxOut%IMAGE_QUEUE_SIZE]->getName() );

  _imgQueueIdxOut+=nb;
  _frame+=nb;

    qsize=_imgQueueIdxIn-_imgQueueIdxOut;
    //if( nb>1 ) printf("queue is now %d images\n",qsize);

  //pthread_mutex_unlock(&_imgQueueAccess);
  return 0;

}


/************************************************************************************/

int LTVideoTexture2D::go(int64_t framenum)
{

  _threadSlowDown = false;

  //if (framenum == 0)
  //  seek(0);

  int diff = framenum-_frame;

/***
    //if( (diff>1) || (framenum%1000==0) ) {
          printf("go: framenum = %Ld; frame = %Ld, IdxIn=%lld IdxOut = %lld buf=%d; diff = %d\n", framenum, _frame, _imgQueueIdxIn,_imgQueueIdxOut, (int)(_imgQueueIdxIn-_imgQueueIdxOut), diff);
    //}
****/


  next(diff); // this is the number of frames we have to display to be in sync

  usec_sleep(100);
  return 0;
}

/************************************************************************************/

int LTVideoTexture2D::next_img(imgu **I)
{
    int i,j,k;
    int frameFinished;

    if (I==NULL) return -1;

    //!Ask ffmpeg for a new frame
    if(av_read_frame(_pFormatCtx, &_packet)<0) return -1;
    else
    {
        if(_packet.stream_index!=_videoStream) return 1;
        else
        {

          //!Decode video frame
            avcodec_decode_video(_pCodecCtx, _pFrame, &frameFinished,
                    _packet.data, _packet.size);

            //Did we get a video frame?
            if(!frameFinished) return 1;
            else
            {
              //Convert the image from its native format to RGB
#ifdef HAVE_LIBSWSCALE
              //profiler_start("sws_scale");
              sws_scale(_dec_convert_ctx, _pFrame->data,
                    _pFrame->linesize, 0,
                    _pCodecCtx->height,
                    _pFrameRGB[0]->data, _pFrameRGB[0]->linesize);
              // profiler_stop("sws_scale");
#else
              //printf("img_convert\n");
              img_convert((AVPicture *)_pFrameRGB[0], PIX_FMT_RGB24,
                      (AVPicture*)_pFrame, _pCodecCtx->pix_fmt, _pCodecCtx->width,
                      _pCodecCtx->height);
              //printf("img_convert done\n");
#endif

              /***
                img_convert((AVPicture *)_pFrameRGB[0], PIX_FMT_RGB24,
                        (AVPicture*)_pFrame, _pCodecCtx->pix_fmt, _pCodecCtx->width,
                        _pCodecCtx->height);
              ***/


              imguAllocate(I,_pCodecCtx->width,_pCodecCtx->height,3);

              //imguUnpack8bit((*I),_pFrameRGB[0]->data[0],MAKE_16_BITS);
              for (i=0;i<_dec_height;i++)
              {
                for (j=0;j<_dec_width;j++)
                {
                  for (k=0;k<3;k++)
                  {
                    (*I)->data[(i*_dec_width+j)*3+k]=(unsigned short)(_pFrameRGB[0]->data[0][(i*_tex_width+j)*3+k]*257);
                  }
                }
              }

              _frame++;
              //_tex_image->dirty();   FIIIIXXXX
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&_packet);
    }

    return 0;
}

/************************************************************************************/

int LTVideoTexture2D::seek(int64_t framenum)
{
  int ret;
  //printf("SEEEEEEEEEEEEEEEEEEEEEEEEKKKKKKKKKKKKKK %ld\n",(long)framenum);
  if (!_threadRunning) return -1;

  // on dirait que c'est appelle un peu trop souvent...
  ret=av_seek_frame(_pFormatCtx , _videoStream, framenum, AVSEEK_FLAG_ANY);
  // ret=0;
  _frame = framenum;
  return ret;
}

/************************************************************************************/

// le apply de la texture doit appliquer la texmat
void LTVideoTexture2D::apply(osg::State& state) const
{

  // les coordonnees textures
  if( _rescale ) _tm->apply(state);

  // la texture elle-meme
  osg::Texture2D::apply(state);
}

/************************************************************************************/

int LTVideoTexture2D::next_pow2(int val)
{
  int pow = 1;
  while(pow<val) pow*=2;
  return pow;
}


/************************************************************************************/


























