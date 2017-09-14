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
 * ltstl.c  -  Main library for the display of the patterns.
 */ 

#include "stl.h"

#include <utils/graycode.h>
#include <unistd.h>     // Header File for sleeping.
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "utils/noise.h"

#define HORIZONTAL 0
#define VERTICAL 1
#define BLACK 0
#define NO_PATTERN -1

void wait_for_go(tcpcast *tcp)
{
    char buf[BS];
    LT_PRINTF("Waiting for go...\n");
    for(;;) {
        tcp_receive_data(tcp,(unsigned char *)(buf),BS);
        //LT_PRINTF("[STL] received message '%s'\n",buf);
        if( strcmp(buf,"go")==0 ) break;
        usleep(1000);
    }
}

int get_color(int position, int pattern_id, int nb_bits)
{
  unsigned int mask,xxor;
  int pos_gray;
  int color;

  if( pattern_id < nb_bits)
  {
    //mask=((1<<(nb_bits-1))>>shuffle(pattern_id, nb_bits));
    mask=(1<<(nb_bits-1))>>pattern_id;
    //mask=1<<(pattern_id);
    xxor=0;
  }
  else
  {
    //mask=((1<<(nb_bits-1))>>shuffle(pattern_id - nb_bits, nb_bits));
    mask=(1<<(nb_bits-1))>>(pattern_id - nb_bits);
    //mask=1<<(pattern_id - nb_bits);
    xxor=IMGU_MAXVAL;
  }

  pos_gray = encode(position);
  if((pos_gray & mask))
    color = IMGU_MAXVAL;
  else
    color = 0;

  return( color ^ xxor );
}





// The main drawing function. 
void draw_pattern(imgu *I,int pattern_id, int orientation, int nb_bits)
{
    int x, y, c, color;
    //int correct_pattern_id;
    int bit=pattern_id%nb_bits;

    printf("orientation %d, pattern %d , bit %d\n",orientation,pattern_id,bit);

    if(pattern_id != NO_PATTERN)
    {
        if(orientation == VERTICAL) //vertical pattern
        {
    		///?????????? pourquoi shuffle ici? le shuffle va etre elimine dans get_color [???]
            //correct_pattern_id = (pattern_id / nb_bits) * nb_bits + shuffle (pattern_id % nb_bits, nb_bits);	// /10*10 delete the last digit (replace by 0) because integer division
            for(x=0; x<I->xs; x++)
            {
                color = get_color(x, /*correct_pattern_id*/pattern_id, nb_bits);
                for(y=0;y<I->ys;y++) for(c=0;c<I->cs;c++) PIXEL(I,x,y,c)=color;
            }
        }
        else //horizontal pattern
        {
            //correct_pattern_id = (pattern_id / nb_bits) * nb_bits + shuffle (pattern_id % nb_bits, nb_bits);
            for(y=0; y<I->ys; y++)
            {
                color = get_color(y, /*correct_pattern_id*/pattern_id, nb_bits);
                for (x=0;x<I->xs;x++) for (c=0;c<I->cs;c++) PIXEL(I,x,y,c)=color;
            }
        }
    }
}

void draw_gamma(imgu *I,int pattern_id,int nb_patterns)
{
    int x,y,c;
    unsigned short graylevel[3];

    if(pattern_id != NO_PATTERN)
    {
        //send pattern like 1.0 0.4 0.9 0.3 0.8 0.2 0.7 0.1 0.6 0.0 0.5 to facilitate Select
        //if (pattern_id%2==0) graylevel=(nb_patterns-1-pattern_id/2)/(float)(nb_patterns-1);
        //else graylevel=(nb_patterns/2-1-pattern_id/2)/(float)(nb_patterns-1);
        graylevel[0]=0;
        graylevel[1]=0;
        graylevel[2]=0;
        c=pattern_id/(nb_patterns/3);
        pattern_id=pattern_id%(nb_patterns/3);
        graylevel[c]=(unsigned short)(IMGU_MAXVAL*(pattern_id/(nb_patterns/3-1.0)));
        for(y=0; y<I->ys; y++)
        {
           for (x=0;x<I->xs;x++)
           {
             for (c=0;c<I->cs;c++)
             {
                I->data[(y*I->xs+x)*I->cs+c]=graylevel[c];
             }
           }
        }
    }
}

void draw_color(imgu *I,int pattern_id,int nb_patterns)
{
    int x,y;
    unsigned short r,g,b;
    //int n;

    if(pattern_id != NO_PATTERN)
    {
        //n=(int)floor(cbrt(nb_patterns));

        //r=(float)((pattern_id/(n*n))%n)/(float)(n-1);
        //g=(float)((pattern_id/(n))%n)/(float)(n-1);
        //b=(float)((pattern_id)%n)/(float)(n-1);

        if (nb_patterns==24)
        {
            //For now, display the NTSC color checkers
            //http://www.brucelindbloom.com/index.html?ColorCheckerRGB.html
            //see also http://scien.stanford.edu/class/psych221/projects/00/shibumi/index.html
            if (pattern_id==0) { r=115;g=82;b=68;}
            else if (pattern_id==1) { r=194;g=150;b=130;}
            else if (pattern_id==2) { r=98;g=122;b=157;}
            else if (pattern_id==3) { r=87;g=108;b=67;}
            else if (pattern_id==4) { r=133;g=128;b=177;}
            else if (pattern_id==5) { r=103;g=189;b=170;}
            else if (pattern_id==6) { r=214;g=126;b=44;}
            else if (pattern_id==7) { r=80;g=91;b=166;}
            else if (pattern_id==8) { r=193;g=90;b=99;}
            else if (pattern_id==9) { r=94;g=60;b=108;}
            else if (pattern_id==10) { r=157;g=188;b=64;}
            else if (pattern_id==11) { r=224;g=163;b=46;}
            else if (pattern_id==12) { r=56;g=61;b=150;}
            else if (pattern_id==13) { r=70;g=148;b=73;}
            else if (pattern_id==14) { r=175;g=54;b=60;}
            else if (pattern_id==15) { r=231;g=199;b=31;}
            else if (pattern_id==16) { r=187;g=86;b=149;}
            else if (pattern_id==17) { r=8;g=133;b=161;}
            else if (pattern_id==18) { r=243;g=243;b=242;}
            else if (pattern_id==19) { r=200;g=200;b=200;}
            else if (pattern_id==20) { r=160;g=160;b=160;}
            else if (pattern_id==21) { r=122;g=122;b=121;}
            else if (pattern_id==22) { r=85;g=85;b=85;}
            else if (pattern_id==23) { r=52;g=52;b=52;}
        }
        //scale to get unsigned short
        r*=257;
        g*=257;
        b*=257;
        for(y=0; y<I->ys; y++)
        {
           for (x=0;x<I->xs;x++)
           {
                I->data[(y*I->xs+x)*I->cs]=r;
                I->data[(y*I->xs+x)*I->cs+1]=g;
                I->data[(y*I->xs+x)*I->cs+2]=b;
           }
        }
    }
}

void draw_frame(imgu *I,int pattern_id, int nb_patterns, int orientation, int XSize, int YSize, int nb_bits, int verbose, int *nb_frames, int *pattern_id_txt, int mode,double freq)
{
    imgu *Inoise;

    Inoise=NULL;

    // Draw each frame
    if (mode==MODE_PATTERNS)
    {
      draw_pattern(I,pattern_id, orientation, nb_bits);
    }
    else if (mode==MODE_GAMMA)
    {
      draw_gamma(I,pattern_id, nb_patterns);
    }
    else if (mode==MODE_NOISE)
    {
      noise_texture_2d(&Inoise,XSize,YSize,1.0,freq,2*freq,1,1);
      imguCopy(&I,Inoise);
    }
    else //color
    {
      draw_color(I,pattern_id, nb_patterns);
    }

    /*char filename[100];
    printf("%d\n",*pattern_id_txt);
    sprintf(filename,"%08d.png",*pattern_id_txt);
    imguSave(I,filename,1,SAVE_AS_IS);*/

    *pattern_id_txt = *pattern_id_txt + 1;

    imguFree(&Inoise);
}

void display_frame(osgViewer::Viewer *viewer,osg::Image* buf,imgu *I)
{
    if( I->cs==1 ) {
      buf->setImage(I->xs,I->ys, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==3 ) {
      buf->setImage(I->xs,I->ys, 1, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }else if( I->cs==4 ) {
      buf->setImage(I->xs,I->ys, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,(unsigned char *) I->data, osg::Image::NO_DELETE, 1);
    }

    viewer->frame(); usleep(1000000/30); //wait 1/30 sec, larger time than refresh rate
    viewer->frame(); usleep(1000000/30); //wait 1/30 sec, larger time than refresh rate
}

// Here we determine the pattern to sent. VSync is checking here.
void patterns_schedule(osgViewer::Viewer *viewer,osg::Image* buf,imgu *I,int XSize, int YSize, int start_sleep, int interval_sleep,int end_sleep, int verbose, int nb_patterns,int sync,int mode,int port,double freq)
{
    imgu *noise_patterns;
    char msg[BS];
    // synchro reseau
    tcpcast tcp;

    noise_patterns=NULL;

    tcp_server_wait(&tcp);
    sprintf(msg,"done");

    LT_PRINTF("\nStart.\n");
    if( sync )
    {
      tcp_server_init(&tcp, port);
      tcp_server_wait(&tcp);
    }

    // START
    LT_PRINTF("\nInit.\n");
    LT_PRINTF("Waiting...\n");

    sleep(start_sleep);

    //Variables initialization
    int nb_frames = 0;
    //int nb_bits_X = (int)(ceil(log10((double)(XSize)) / log10(2.0)));
    //int nb_bits_Y = (int)(ceil(log10((double)(YSize)) / log10(2.0)));
    int nb_bits_X = getnbbits(XSize);
    int nb_bits_Y = getnbbits(YSize);
    int pattern_id; //real pattern_id (for the algorithm)
    int pattern_id_txt = 0; //pattern_id based ont the order on the screen (for printf purposes)

    // Draw each pattern
    if (mode==MODE_PATTERNS)
    {
      // Vertical 1st pass
      for(pattern_id = 0; pattern_id < nb_bits_X; pattern_id++) {
        draw_frame(I,pattern_id, nb_patterns, VERTICAL, XSize, YSize, nb_bits_X, verbose, &nb_frames, &pattern_id_txt, mode,freq);
        if( sync ) wait_for_go(&tcp);
        else sleep(interval_sleep); //when displaying patterns without synchonization

        display_frame(viewer,buf,I);
        //sprintf(name,"%03d_X.png",pattern_id); imguSave(I,name,1,SAVE_16_BITS);
        if( sync ) tcp_send_string(&tcp,(unsigned char *)(msg));
      }

      // Horizontal 1st pass
      for(pattern_id = 0; pattern_id < nb_bits_Y; pattern_id++) {
        draw_frame(I,pattern_id, nb_patterns, HORIZONTAL, XSize, YSize, nb_bits_Y, verbose, &nb_frames, &pattern_id_txt, mode,freq);
        if( sync ) wait_for_go(&tcp);
        else sleep(interval_sleep);

        display_frame(viewer,buf,I);
        if( sync ) tcp_send_string(&tcp,(unsigned char *)(msg));
      }

      // Vertical 2nd pass
      for(pattern_id = nb_bits_X; pattern_id < nb_bits_X * 2; pattern_id++) {
        draw_frame(I,pattern_id, nb_patterns, VERTICAL, XSize, YSize, nb_bits_X, verbose, &nb_frames, &pattern_id_txt, mode,freq);
        if( sync ) wait_for_go(&tcp);
        else sleep(interval_sleep);

        display_frame(viewer,buf,I);
        if( sync ) tcp_send_string(&tcp,(unsigned char *)(msg));
      }

      // Horizontal 2nd pass
      for(pattern_id = nb_bits_Y; pattern_id < nb_bits_Y * 2; pattern_id++) {
        draw_frame(I,pattern_id, nb_patterns, HORIZONTAL, XSize, YSize, nb_bits_Y, verbose, &nb_frames, &pattern_id_txt, mode,freq);
        if( sync ) wait_for_go(&tcp);
        else sleep(interval_sleep);

        display_frame(viewer,buf,I);
        if( sync ) tcp_send_string(&tcp,(unsigned char *)(msg));
      }
    }//end of patterns
    else if (mode==MODE_GAMMA || mode==MODE_COLOR || mode==MODE_NOISE)
    {
      for(pattern_id = 0; pattern_id < nb_patterns; pattern_id++)
      {
        draw_frame(I,pattern_id,nb_patterns, -1,XSize, YSize, -1, verbose, &nb_frames, &pattern_id_txt, mode,freq);
        if( sync ) wait_for_go(&tcp);
        else sleep(interval_sleep);

        display_frame(viewer,buf,I);
        if( sync ) tcp_send_string(&tcp,(unsigned char *)(msg));
      }
    }
    else
    {
      LT_PRINTF("Error: supported modes are patterns,gamma,color\n");
      LT_EXIT(0);
    }

    if( sync ) tcp_receive_data(&tcp,(unsigned char *)(buf),BS);
    else sleep(interval_sleep);

    // END (black screen)
    sleep(end_sleep);

    if( sync )
    {
      tcp_server_close_connection(&tcp);
      tcp_server_close(&tcp);
    }

    imguFreeMulti(&noise_patterns);
}


