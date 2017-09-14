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

#include "DnDBox.h"

void DnDBox::updateSize()
{
  int x_temp,size_temp,end_pos;
  x_temp=convertTimeToPixelAbsolute(start_x);
  if (convert_width) size_temp=convertTimeToPixelRelative(len_x);
  else size_temp=(int)(len_x+0.5); 
  end_pos=x_temp+size_temp;
  if (x_temp>pixel_max || end_pos<pixel_min)
  {
    size_temp=0; //do not draw if starts after displayed timeline
    x_temp=DND_MAX_POS;
  }
  else
  {
    if (end_pos>pixel_max)
    {
      size_temp=pixel_max-x_temp;
    }
    if (x_temp<pixel_min)
    {
      size_temp-=(pixel_min-x_temp);
      x_temp=pixel_min;
    }
  }
  if (center_flag) x_temp-=size_temp/2;
  if (opts_type!=DND_STANDARD) resize(x_temp,y(),size_temp,h());
}

void DnDBox::draw()
{
  updateSize();

//fprintf(stderr,"DRAW %d %d %d %d %f %f %d %d %f %f %d %d\n",x(),y(),w(),h(),start_x,len_x,fps,offset,frames_to_pixels,disp_start,pixel_min,pixel_max);
  Fl_Box::draw();
}

int DnDBox::handle(int e)
{
    double temp;
    int ret = Fl_Box::handle(e);

    //fprintf(stderr,"DNDBOX %d %d %d\n",Fl::event_x(),Fl::event_y(),e);

    if (e==FL_DRAG)
    {
        if (Fl::event_button() == 1) //drag only if left click
        {
          if (opts_drag==DRAG_HORIZONTAL || opts_drag==DRAG_BOTH)
          {  
            temp=start_x;
            if (opts_type==DND_STANDARD) position((int)(Fl::event_x()-offsets[0]),y());
            else start_x=convertPixelToTimeAbsolute(Fl::event_x())-offsets[0];
            //start_x_relative+=(start_x-temp);
          }
          if (opts_drag==DRAG_VERTICAL || opts_drag==DRAG_BOTH)
          {
            if (opts_type==DND_STANDARD) position(x(),(int)(Fl::event_y()-offsets[1]));
            else setPosition(convertTimeToPixelAbsolute(start_x), (int)(Fl::event_y()-offsets[1]+0.5));
          }
          if (opts_type==DND_STANDARD)
          {
            if (x()<xmin) setPosition(xmin,y());
            if (x()+w()>xmax) setPosition(xmax-w(),y());
          }
          else
          {
            if (start_x<xmin)
            {
              start_x=xmin;
            }
            if (start_x+len_x>xmax)
            {
              start_x=xmax-len_x;
            }
          }
          if (y()<ymin)
          {
            position(x(),(int)(ymin+0.5));
          }
          if (y()+h()>ymax)
          {
            position(x(),(int)(ymax-h()+0.5));
          }
          if (win!=NULL && opts_drag!=DRAG_NONE) {win->redraw();}
          return(1);
        }
    }
    else if (e==FL_PUSH)
    {
        update_offsets=0;
        if (Fl::event_button() == 1) { if (callback_fcn_left!=NULL) callback_fcn_left(this,NULL); }
        if (Fl::event_button() > 1) { if (callback_fcn_right!=NULL) callback_fcn_right(this,NULL); }
        return(1);
    }
    else if (e==FL_RELEASE)
    {
        update_offsets=1;
    }
    else
    {
      if (update_offsets)
      {
        if (opts_type==DND_STANDARD)
        {
          offsets[0] = (double)(Fl::event_x()-x());
          offsets[1] = (double)(Fl::event_y()-y());
          offsets[2] = offsets[0] - w();
          offsets[3] = offsets[1] - h();
        }
        else
        {
          offsets[0] = convertPixelToTimeAbsolute(Fl::event_x()) - start_x;    // save where user clicked for dragging
          offsets[1] = (double)(Fl::event_y()-y());
          offsets[2] = offsets[0] - len_x;
          offsets[3] = offsets[1] - h();
        }
      }
    }
    return(ret);
}

void DnDBox::setPosition(int x,int y)
{
  start_x=convertPixelToTimeAbsolute(x);
  position(x,y);
}

void DnDBox::setSize(int w,int h)
{
  len_x=convertPixelToTimeRelative(w);
  size(w,h);
}

void DnDBox::setResize(int x,int y,int w,int h)
{
  start_x=convertPixelToTimeAbsolute(x);
  len_x=convertPixelToTimeRelative(w);
  resize(x,y,w,h);
}

int DnDBox::setImage(imgu *I)
{
    int i,j,k,index,isize;
    if (I==NULL)
    {
      if (img!=NULL) delete img;
      img=NULL;
      free(imgdata);
      imgdata=NULL;
      //box(FL_THIN_UP_BOX);
      box(FL_THIN_DOWN_BOX);
      //box(FL_NO_BOX);   
      setSize(DEFAULT_SIZE,DEFAULT_SIZE);
      image(NULL);
      return -1;
    }

    isize=I->xs*I->ys*I->cs;
    if (img!=NULL) 
    {
      delete img;
      img=NULL;
    }
    if (ds<isize) 
    {
      if (imgdata!=NULL)
      {
        free(imgdata);
        imgdata=NULL;
      }
    }
    if (imgdata==NULL) imgdata=(unsigned char *)(malloc(sizeof(unsigned char)*isize));
    if (imgdata==NULL) return -1;
    //assume I is 16 bits
    index=0;
    for (i=0;i<I->ys;i++)
    {
      for (j=0;j<I->xs;j++)
      {
        for (k=0;k<I->cs;k++)
        {
          //horrible screen door transparency on x11
          //true alpha blending is only available in fltk 1.1.8 
          //for now, clamp alpha to either 0 or 255  
          if (0 && I->cs%2==0 && k==I->cs-1)
          {
            if (I->data[(i*I->xs+j)*I->cs+k]>IMGU_MAXVAL/2) imgdata[index]=(unsigned char)(255);
            else imgdata[index]=(unsigned char)(0);
          }
          else imgdata[index]=(unsigned char)(I->data[index]>>8);
          index++;
        }
      }
    }
    img=new Fl_RGB_Image(imgdata,I->xs,I->ys,I->cs);
    if (img==NULL) return -1;
    setSize(img->w(),img->h());
    image(img);

    box(FL_NO_BOX);
    return  0;
}

int DnDBox::convertTimeToPixelAbsolute(double time)
{
  return (int)((time*fps-disp_start)*frames_to_pixels+offset);
}

double DnDBox::convertPixelToTimeAbsolute(int pixel)
{
  return ((pixel-offset)/frames_to_pixels+disp_start)/fps;
}

int DnDBox::convertTimeToPixelRelative(double time)
{
  return (int)(time*frames_to_pixels*fps);
}

double DnDBox::convertPixelToTimeRelative(int pixel)
{
  return pixel/frames_to_pixels/fps;
}






