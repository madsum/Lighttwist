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

#ifndef DND_BOX_H
#define DND_BOX_H

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Image.H>
#include <FL/fl_ask.H>
#include <stdio.h>
#include <imgu/imgu.h>
#include "watchCommand.h"

#define DRAG_NONE 0
#define DRAG_HORIZONTAL 1
#define DRAG_VERTICAL 2
#define DRAG_BOTH 3

#define DND_MINUS_INFINITY -1000000
#define DND_PLUS_INFINITY 1000000

#define DND_MAX_SIZE 4096
#define DND_MAX_POS 4096

#define DND_STANDARD 0
#define DND_XSCALABLE 1

#define DEFAULT_SIZE 200

class DnDBox : public Fl_Box {
protected:
    Fl_Window *win;
    void draw();
    int handle(int e);
    void (*callback_fcn_left)(Fl_Widget *w, void*v); //callback functions for left and right clicks
    void (*callback_fcn_right)(Fl_Widget *w, void*v);
    unsigned char opts_drag;
    unsigned char opts_type;
    Fl_Image *img;
    unsigned char* imgdata;
    int ds; //size of internal img buffer
    double offsets[4];
    unsigned char update_offsets;

public:
    DnDBox(Fl_Window *w,unsigned char t,void (*cb_fcn_left)(Fl_Widget *w, void*v),void (*cb_fcn_right)(Fl_Widget *w, void*v),unsigned char d,int X, int Y, imgu *I) : Fl_Box(X,Y,10,10,NULL) {
        win=w;
        opts_type=t;
        opts_drag=d;
        callback_fcn_left=cb_fcn_left;
        callback_fcn_right=cb_fcn_right;
        img=NULL;
        imgdata=NULL;
        ds=0;
        setImage(I);
        color(FL_GRAY);
        xmin=DND_MINUS_INFINITY;
        xmax=DND_PLUS_INFINITY;
        ymin=DND_MINUS_INFINITY;
        ymax=DND_PLUS_INFINITY;
        pixel_min=DND_MINUS_INFINITY;
        pixel_max=DND_PLUS_INFINITY;
        update_offsets=1;
        fps=1;
        frames_to_pixels=1.0;
        disp_start=0;
        offset=0;
        start_x=(double)(X);
        convert_width=1;
        center_flag=0;
        if (I!=NULL) len_x=(double)(I->xs);
        else len_x=(double)(DEFAULT_SIZE);
    }
    DnDBox(Fl_Window *w,unsigned char t,void (*cb_fcn_left)(Fl_Widget *w, void*v),void (*cb_fcn_right)(Fl_Widget *w, void*v),unsigned char d,int X, int Y, int width,int height,imgu *I=NULL) : Fl_Box(X,Y,width,height,NULL) {
        win=w;
        opts_type=t;
        opts_drag=d;
        callback_fcn_left=cb_fcn_left;
        callback_fcn_right=cb_fcn_right;
        img=NULL;
        imgdata=NULL;
        ds=0;
        setImage(I);
        color(FL_GRAY);
        xmin=DND_MINUS_INFINITY;
        xmax=DND_PLUS_INFINITY;
        ymin=DND_MINUS_INFINITY;
        ymax=DND_PLUS_INFINITY;
        pixel_min=DND_MINUS_INFINITY;
        pixel_max=DND_PLUS_INFINITY;
        update_offsets=1;
        fps=1;
        frames_to_pixels=1.0;
        disp_start=0;
        offset=0;
        start_x=(double)(X);
        convert_width=1;
        center_flag=0;
        if (I!=NULL) len_x=(double)(I->xs);
        else len_x=(double)(DEFAULT_SIZE);
    }

    //bounding box limits, cannot be dragged outside of these limits
    double xmin,xmax;
    double ymin,ymax;  

    unsigned char convert_width;
    unsigned char center_flag; //non-zero if position represents center (not the top left corner)

    int setImage(imgu *I);
    void setPosition(int x,int y);
    void setSize(int w,int h);
    void setResize(int x,int y,int w,int h);
    void updateSize();
    int getImgWidth(){if (img==NULL) return DEFAULT_SIZE; else return img->w();};
    int getImgHeight(){if (img==NULL) return DEFAULT_SIZE; else return img->h();};

    int convertTimeToPixelAbsolute(double time);
    double convertPixelToTimeAbsolute(int pixel);
    int convertTimeToPixelRelative(double time);
    double convertPixelToTimeRelative(int pixel);

    //variables for time to pixel conversion
    int fps;
    double frames_to_pixels;
    double disp_start; //first pixel of layer to be displayed
    int offset; //pixel position of start of layer
    int pixel_min;
    int pixel_max;

    //position in time
    double start_x;
    double len_x,len_x_orig;
    friend class FileReaderThread;
};

#endif
