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

#include "playlistMedia.h"

void playlistMedia::draw()
{
    int i;
    int x1,x2,y1,y2;
    int tempx,tempy;
    double dx,dy;

    //fprintf(stderr,"MEDIA\n");

    //draw lines between fade ctrls
    removePrimitives();
    for (i=0;i<3;i++)
    {
      if (fade_ctrls[i]!=NULL && fade_ctrls[i+1]!=NULL)
      {
        if (1 || fade_ctrls[i]->w()!=0 && fade_ctrls[i+1]->w()!=0)
        {
          x1=convertTimeToPixelAbsolute(fade_ctrls[i]->start_x);
          x2=convertTimeToPixelAbsolute(fade_ctrls[i+1]->start_x);
          y1=fade_ctrls[i]->y()+fade_ctrls[i]->h()/2;
          y2=fade_ctrls[i+1]->y()+fade_ctrls[i+1]->h()/2;
          if (x1>x2) //put left point in x1
          {
            tempx=x1;
            tempy=y1;
            x1=x2;
            y1=y2;
            x2=tempx;
            y2=tempy;
          }
          //fprintf(stderr,"LINES %d %d %d %d\n",x1,y1,x2,y2);
          if (x1<pixel_min && x2<pixel_min) continue;
          if (x1>pixel_max && x2>pixel_max) continue;
          if (x1<pixel_min) 
          {
            dx=(double)(x2-x1);
            dy=(double)(y2-y1);
            if (dx>0.0001)
            {
              dy/=dx;
              x1=pixel_min;
              y1=(int)(y2-dy*(x2-x1)+0.5);
            }
          }
          if (x2>pixel_max) 
          {
            dx=(double)(x2-x1);
            dy=(double)(y2-y1);
            if (dx>0.0001)
            {
              dy/=dx;
              x2=pixel_max;
              y2=(int)(y1+dy*(x2-x1)+0.5);
            }
          }
          addLine(FL_RED,FL_SOLID,2,x1,y1,x2,y2);
        }
      }
    }  
    canvasIcon::draw();
    for (i=0;i<4;i++)
    {
      if (fade_ctrls[i]!=NULL)
      {
          fade_ctrls[i]->resize(offset,fade_ctrls[i]->y(),fade_ctrls[i]->getImgWidth(),fade_ctrls[i]->getImgHeight());
          fade_ctrls[i]->redraw();
      }
    } 
}
int playlistMedia::handle(int e) 
{
	int i;
	int ret,used_event;
	double prevx,prevw;
	double snap_x,snap_diff_x, curr_len_x;
	
	prevx=start_x;
	prevw=len_x;
	
	//perform possible drag of underlying DnDBox
	ret=canvasIcon::handle(e);
	
	used_event=0;
	if (win!=NULL)
	{
		if (e==FL_LEAVE)
		{
			win->cursor(FL_CURSOR_DEFAULT);
			used_event=1;
		}
		else
		{
			/*if (MEDIA_ENABLE_RESIZE && abs(Fl::event_x() - x()) < MEDIA_MARGIN) //enable changing of start time
			 {
			 used_event=1;
			 win->cursor(FL_CURSOR_WE);
			 if (e == FL_DRAG)
			 {
			 //'start_absolute' is the pixel position of the start of the media (i.e. the frame 0)
			 //we want to snap to this position if the cursor is near
			 snap_x=x()-start_absolute;
			 snap_diff_x=snap_x;
			 while (snap_diff_x<0) snap_diff_x+=skin_cpy->xs;
			 snap_diff_x=snap_diff_x%skin_cpy->xs;
			 //'snap_diff_x' now has non-negative modulo difference with 'start_absolute'
			 //cursor approaches from right
			 if (snap_diff_x<MEDIA_SNAP) snap_x-=snap_diff_x;
			 //cursor approaches from left
			 if (snap_diff_x>skin_cpy->xs-MEDIA_SNAP) snap_x-=(snap_diff_x-skin_cpy->xs);
			 snap_x+=start_absolute;
			 setPosition(snap_x,y());
			 setSize(w()+prevx-x(),h());
			 start_x-=convertPixelToTimeRelative(prevx-x());
			 //make sure fade ctrls are not outside DnDBox
			 //if (fade_ctrls[0]!=NULL) fade_ctrls[0]->setPosition(fade_ctrls[0]->x()-prevx+x(),fade_ctrls[0]->y());
			 //if (fade_ctrls[1]!=NULL) fade_ctrls[1]->setPosition(fade_ctrls[1]->x()-prevx+x(),fade_ctrls[1]->y());
			 }
			 }*/
			//else if (fabs(convertTimeToPixelRelative(convertPixelToTimeAbsolute(Fl::event_x()) - start_x - len_x)) < MEDIA_MARGIN) //enable changing of end time
			if (convertTimeToPixelRelative(fabs(offsets[2])) < MEDIA_MARGIN) //enable changing of end time
			{
				win->cursor(FL_CURSOR_WE);
				used_event=1;
				if (e == FL_DRAG)
				{
					//the width changes, but the start position remains fixed
					start_x=prevx;
					curr_len_x=convertPixelToTimeAbsolute(Fl::event_x())-prevx-offsets[2];
					if (curr_len_x > 1.0) //don't allow the length of media == 0
					{
						len_x = curr_len_x;
						snap_x=len_x;
						snap_diff_x=fmod(snap_x,len_x_orig);
						//snapping from right direction
						if (convertTimeToPixelRelative(snap_diff_x)<MEDIA_SNAP) snap_x-=snap_diff_x;
						if (snap_x == 0) snap_x = 1.0; //don't allow length of media == 0
						//snapping from left doesn't work...
						//if (snap_diff_x>len_x_orig-MEDIA_SNAP) snap_x-=(snap_diff_x-len_x_orig);
						len_x=snap_x;
						offsets[0]=convertPixelToTimeAbsolute(Fl::event_x()) - start_x;
						//make sure fade ctrls are not outside DnDBox
						if (fade_ctrls[2]!=NULL) fade_ctrls[2]->start_x-=prevw-len_x;
						if (fade_ctrls[3]!=NULL) fade_ctrls[3]->start_x-=prevw-len_x;
					}
				}
			}
			else //normal drag
			{
				win->cursor(FL_CURSOR_DEFAULT);
				start_absolute-=prevx-start_x;
				for (i=0;i<4;i++)
				{
					if (fade_ctrls[i]!=NULL)
					{  
						fade_ctrls[i]->start_x-=prevx-start_x;
					}
				}
				//fprintf(stderr,"%f\n",convertPixelToTimeRelative(prevx-x()));
			}
		}
		//update skin image
		if (skin_cpy!=NULL)
		{
			imguExtractRectangle(&skin_part,skin_cpy,x()-convertTimeToPixelRelative(start_absolute),0,w(),h());
			setImage(skin_part);
		}
	}
	
	if (len_x<0) len_x=1.0;
	
	for (i=0;i<4;i++)
	{
		if (fade_ctrls[i]!=NULL)
		{
			fade_ctrls[i]->xmin=start_x;
			fade_ctrls[i]->xmax=start_x+len_x+fade_ctrls[i]->len_x;
			fade_ctrls[i]->ymin=y()-fade_ctrls[i]->h()/2;
			fade_ctrls[i]->ymax=y()+h()+fade_ctrls[i]->h()/2;
		}
	}
	
	//make sure upper-left fade ctrl is located after x()+w()
	if (fade_ctrls[1]!=NULL) 
	{
		if (fade_ctrls[1]->start_x>start_x+len_x) fade_ctrls[1]->start_x=start_x+len_x;
	}
	//make sure upper-right fade ctrl is located after x()
	if (fade_ctrls[2]!=NULL) 
	{
		if (fade_ctrls[2]->start_x<start_x) fade_ctrls[2]->start_x=start_x;
	}
	
	if (fade_ctrls[3]!=NULL) 
	{
		if (fade_ctrls[3]->start_x<start_x+len_x) fade_ctrls[3]->start_x=start_x+len_x;
	}
	
	
	if (e==FL_PUSH)
	{
		//if (Fl::event_button() == 1) { cb_media_left_click(this,NULL); }
		if (Fl::event_button() > 1)
		{
			//fprintf(stderr,"CLICKMEDIA\n");
			cb_media_right_click(this,NULL);
			used_event=1;
		}
	}
	
	if (used_event) return 1;
	else return ret;
	
	return 0;
}


//FL_CURSOR_DEFAULT - the default cursor, usually an arrow (0)
//FL_CURSOR_ARROW - an arrow pointer
//FL_CURSOR_CROSS - crosshair
//FL_CURSOR_WAIT - watch or hourglass
//FL_CURSOR_INSERT - I-beam
//FL_CURSOR_HAND - hand (uparrow on MSWindows)
//FL_CURSOR_HELP - question mark
//FL_CURSOR_MOVE - 4-pointed arrow
//FL_CURSOR_NS - up/down arrow
//FL_CURSOR_WE - left/right arrow
//FL_CURSOR_NWSE - diagonal arrow
//FL_CURSOR_NESW - diagonal arrow
//FL_CURSOR_NONE - invisible

int playlistMedia::setControl(int i,double offset,double fade)
{
    if (i<0 || i>4) return -1;
    fade_ctrls[i]->len_x=(double)(fade_ctrls[i]->w());
    fade_ctrls[i]->position(0,(int)(y()+(1.0-fade)*h()-fade_ctrls[i]->h()/2.0));
    if (i<=1) fade_ctrls[i]->start_x=start_x+offset;
    else fade_ctrls[i]->start_x=start_x+len_x+offset;
    fade_ctrls[i]->convert_width=0;
    fade_ctrls[i]->center_flag=1;
    return 0;
}

void playlistMedia::resetMenuFiles()
{
  menu_right_click->clear();
}

int playlistMedia::addMenuFile(const char *filename,Fl_Callback *fcn,void *data)
{
  if (filename==NULL) return -1;
  menu_right_click->add(filename,NULL,fcn,data);
  menu_right_click->hide();
  return 0;
}

void playlistMedia::showMenuFile(Fl_Widget *w)
{
  //menu_right_click->hide();
  menu_right_click->position(Fl::event_x(), Fl::event_y());
  const Fl_Menu_Item *m = menu_right_click->popup();
  if ( m ) m->do_callback(w, m->user_data());
}

