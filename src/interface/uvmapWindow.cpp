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

#include "uvmapWindow.h"

void uvmapWindow::draw()
{
    float xc,yc,r;
    float x1,y1;
    float x2,y2;
    float x3,y3;
    float x4,y4;

    background->removePrimitives();

    if (configuration==CYCLORAMA_CONFIGURATION)
    {
      x1=controls_inner[0]->x()+controls_inner[0]->w()/2;
      y1=controls_inner[0]->y()+controls_inner[0]->h()/2;
      x2=controls_inner[1]->x()+controls_inner[1]->w()/2;
      y2=controls_inner[1]->y()+controls_inner[1]->h()/2;
      x3=controls_inner[2]->x()+controls_inner[2]->w()/2;
      y3=controls_inner[2]->y()+controls_inner[2]->h()/2;
      uvEllipseCenter(x1,y1,x2,y2,x3,y3,&xc,&yc,&r);
      background->addEllipse(FL_BLUE,FL_SOLID,2,(int)(xc-r/scale_inner->value()),(int)(yc-r),(int)(r/scale_inner->value()*2),(int)(r*2));

      x1=controls_outer[0]->x()+controls_outer[0]->w()/2;
      y1=controls_outer[0]->y()+controls_outer[0]->h()/2;
      x2=controls_outer[1]->x()+controls_outer[1]->w()/2;
      y2=controls_outer[1]->y()+controls_outer[1]->h()/2;
      x3=controls_outer[2]->x()+controls_outer[2]->w()/2;
      y3=controls_outer[2]->y()+controls_outer[2]->h()/2;
      uvEllipseCenter(x1,y1,x2,y2,x3,y3,&xc,&yc,&r);
      background->addEllipse(FL_RED,FL_SOLID,2,(int)(xc-r/scale_outer->value()),(int)(yc-r),(int)(r/scale_outer->value()*2),(int)(r*2));
    }
    else if (configuration==PLANAR_CONFIGURATION)
    {
      x1=controls_inner[0]->x()+controls_inner[0]->w()/2;
      y1=controls_inner[0]->y()+controls_inner[0]->h()/2;
      x2=controls_inner[1]->x()+controls_inner[1]->w()/2;
      y2=controls_inner[1]->y()+controls_inner[1]->h()/2;
      background->addRectangle(FL_RED,FL_SOLID,2,(int)(x1),(int)(y1),(int)(x2-x1),(int)(y2-y1));
    }
    else if (configuration==BEZIER_CONFIGURATION)
    {
      x1=controls_inner[0]->x()+controls_inner[0]->w()/2;
      y1=controls_inner[0]->y()+controls_inner[0]->h()/2;
      x2=controls_inner[1]->x()+controls_inner[1]->w()/2;
      y2=controls_inner[1]->y()+controls_inner[1]->h()/2;
      x3=controls_inner[2]->x()+controls_inner[2]->w()/2;
      y3=controls_inner[2]->y()+controls_inner[2]->h()/2;
      x4=controls_inner[3]->x()+controls_inner[3]->w()/2;
      y4=controls_inner[3]->y()+controls_inner[3]->h()/2;
      background->addCurve(FL_BLUE,FL_SOLID,2,(int)(x1),(int)(y1),(int)(x2),(int)(y2),(int)(x3),(int)(y3),(int)(x4),(int)(y4));

      x1=controls_outer[0]->x()+controls_outer[0]->w()/2;
      y1=controls_outer[0]->y()+controls_outer[0]->h()/2;
      x2=controls_outer[1]->x()+controls_outer[1]->w()/2;
      y2=controls_outer[1]->y()+controls_outer[1]->h()/2;
      x3=controls_outer[2]->x()+controls_outer[2]->w()/2;
      y3=controls_outer[2]->y()+controls_outer[2]->h()/2;
      x4=controls_outer[3]->x()+controls_outer[3]->w()/2;
      y4=controls_outer[3]->y()+controls_outer[3]->h()/2;
      background->addCurve(FL_RED,FL_SOLID,2,(int)(x1),(int)(y1),(int)(x2),(int)(y2),(int)(x3),(int)(y3),(int)(x4),(int)(y4));
    }
    else if (configuration==HOMOGRAPHY_CONFIGURATION)
    {
      x1=controls_inner[0]->x()+controls_inner[0]->w()/2;
      y1=controls_inner[0]->y()+controls_inner[0]->h()/2;
      x2=controls_inner[1]->x()+controls_inner[1]->w()/2;
      y2=controls_inner[1]->y()+controls_inner[1]->h()/2;
      x3=controls_inner[2]->x()+controls_inner[2]->w()/2;
      y3=controls_inner[2]->y()+controls_inner[2]->h()/2;
      x4=controls_inner[3]->x()+controls_inner[3]->w()/2;
      y4=controls_inner[3]->y()+controls_inner[3]->h()/2;

      background->addLine(FL_BLUE,FL_SOLID,2,x1,y1,x2,y2);
      background->addLine(FL_BLUE,FL_SOLID,2,x2,y2,x3,y3);
      background->addLine(FL_BLUE,FL_SOLID,2,x3,y3,x4,y4);
      background->addLine(FL_BLUE,FL_SOLID,2,x4,y4,x1,y1);
    }
    else
    {
    } 

    Fl_Double_Window::draw();
}

void uvmapWindow::createMenuWindow()
{
    int i,posx;
    LINEPOSY=h()-2*LINESPACE;

    posx=-300;
    cancel=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_CANCEL));
    cancel->callback(cb_cancel);
    posx+=-cancel->w()-OFFSET;
    cancel->hide();
    ok=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_OK));
    ok->callback(cb_ok);
    posx+=-ok->w()-OFFSET;
    reset=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_RESET));
    reset->callback(cb_reset);

    LINEPOSY+=LINESPACE;

    posx=0;
    vcenter=new Fl_Spinner(SMALL_WIDGET,langGetLabel(LABEL_UVMAP_VMIDDLE));
    setFloatSpinner(vcenter,0.0,1.0,&LINEPOSY);
    LINEPOSY-=LINESPACE;
    posx-=135;
    scale_inner=new Fl_Spinner(posx+SMALL_WIDGET,langGetLabel(LABEL_UVMAP_INNER_SCALE));
    setFloatSpinner(scale_inner,0.5,2.0,&LINEPOSY);
    scale_inner->callback(cb_update_ellipse);
    LINEPOSY-=LINESPACE;
    posx-=200;
    scale_outer=new Fl_Spinner(posx+SMALL_WIDGET,langGetLabel(LABEL_UVMAP_OUTER_SCALE));
    setFloatSpinner(scale_outer,0.5,2.0,&LINEPOSY);
    scale_outer->callback(cb_update_ellipse);

    scroll=new Fl_Scroll(0,0,w(),h()-2*LINESPACE-OFFSET,NULL);
    scroll->box(FL_NO_BOX);

    background=new canvasIcon(this,DND_STANDARD,NULL,DRAG_NONE,0,0,NULL);
    for (i=0;i<MAX_NB_UV_CTRLS;i++)
    {
      controls_inner[i]=new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_BOTH,0,0,NULL);
      controls_outer[i]=new DnDBox(this,DND_STANDARD,NULL,NULL,DRAG_BOTH,0,0,NULL);
    }

    scroll->end();
}

void uvmapWindow::loadDefaultValues()
{
  int centerx,centery;
  int inner_radius,outer_radius;
  int side_inner_radius,side_outer_radius;

  scale_inner->value(1.0);
  scale_outer->value(1.0);
  vcenter->value(0.5);

  centerx=background->x()+background->w()/2;
  centery=background->y()+background->h()/2;
  inner_radius=(int)(0.3*background->w()/2);
  outer_radius=(int)(0.6*background->w()/2);
  side_inner_radius=(int)(sqrt(0.5*inner_radius*inner_radius));
  side_outer_radius=(int)(sqrt(0.5*outer_radius*outer_radius));

  if (configuration==CYCLORAMA_CONFIGURATION)
  {
    controls_inner[0]->setPosition(centerx,centery+inner_radius);
    controls_inner[1]->setPosition(centerx-side_inner_radius,centery-side_inner_radius);
    controls_inner[2]->setPosition(centerx+side_inner_radius,centery-side_inner_radius);
    controls_outer[0]->setPosition(centerx,centery+outer_radius);
    controls_outer[1]->setPosition(centerx-side_outer_radius,centery-side_outer_radius);
    controls_outer[2]->setPosition(centerx+side_outer_radius,centery-side_outer_radius);
  }
  else if (configuration==PLANAR_CONFIGURATION)
  {
    controls_inner[0]->setPosition(centerx-side_inner_radius,centery-side_inner_radius);
    controls_inner[1]->setPosition(centerx+side_inner_radius,centery+side_inner_radius);
  }
  else if (configuration==HOMOGRAPHY_CONFIGURATION)
  {
    controls_inner[0]->setPosition(centerx-side_inner_radius,centery-side_inner_radius);
    controls_inner[1]->setPosition(centerx+side_inner_radius,centery-side_inner_radius);
    controls_inner[2]->setPosition(centerx+side_inner_radius,centery+side_inner_radius);
    controls_inner[3]->setPosition(centerx-side_inner_radius,centery+side_inner_radius);
  }
  else if (configuration==BEZIER_CONFIGURATION)
  {
    controls_inner[0]->setPosition(centerx-side_inner_radius,centery-side_inner_radius);
    controls_inner[1]->setPosition(centerx-side_inner_radius/2,centery-side_inner_radius);
    controls_inner[2]->setPosition(centerx+side_inner_radius/2,centery-side_inner_radius);
    controls_inner[3]->setPosition(centerx+side_inner_radius,centery-side_inner_radius);
    controls_outer[0]->setPosition(centerx-side_inner_radius,centery+side_inner_radius);
    controls_outer[1]->setPosition(centerx-side_inner_radius/2,centery+side_inner_radius);
    controls_outer[2]->setPosition(centerx+side_inner_radius/2,centery+side_inner_radius);
    controls_outer[3]->setPosition(centerx+side_inner_radius,centery+side_inner_radius);
  }
  else
  {
    fprintf(stderr,"Warning: configuration not supported in uvmap window\n");
  }
}

void uvmapWindow::cb_update_ellipse(Fl_Widget *w, void*)
{
  uvmapWindow *win;
  win=(uvmapWindow *)(w->parent());
  win->redraw();
}

int uvmapWindow::setUVConfiguration(int c)
{
  configuration=c;

  updateLayout();

  return 0;
}

void uvmapWindow::updateLayout()
{
  if (configuration==CYCLORAMA_CONFIGURATION)
  {
    scale_inner->show();
    scale_outer->show();
    vcenter->show();

    controls_inner[0]->show();
    controls_inner[1]->show();
    controls_inner[2]->show();
    controls_inner[3]->hide();
    controls_outer[0]->show();
    controls_outer[1]->show();
    controls_outer[2]->show();
    controls_outer[3]->hide();
  }
  else if (configuration==PLANAR_CONFIGURATION)
  {
    scale_inner->hide();
    scale_outer->hide();
    vcenter->hide();

    controls_inner[0]->show();
    controls_inner[1]->show();
    controls_inner[2]->hide();
    controls_inner[3]->hide();
    controls_outer[0]->hide();
    controls_outer[1]->hide();
    controls_outer[2]->hide();
    controls_outer[3]->hide();
  }
  else if (configuration==HOMOGRAPHY_CONFIGURATION)
  {
    scale_inner->hide();
    scale_outer->hide();
    vcenter->hide();

    controls_inner[0]->show();
    controls_inner[1]->show();
    controls_inner[2]->show();
    controls_inner[3]->show();
    controls_outer[0]->hide();
    controls_outer[1]->hide();
    controls_outer[2]->hide();
    controls_outer[3]->hide();
  }
  else if (configuration==BEZIER_CONFIGURATION)
  {
    scale_inner->hide();
    scale_outer->hide();
    vcenter->hide();

    controls_inner[0]->show();
    controls_inner[1]->show();
    controls_inner[2]->show();
    controls_inner[3]->show();
    controls_outer[0]->show();
    controls_outer[1]->show();
    controls_outer[2]->show();
    controls_outer[3]->show();
  }
  else
  {
    //fprintf(stderr,"Warning: configuration not supported in uvmap window\n");
    //return;
  }
}

void uvmapWindow::cb_ok(Fl_Widget *w, void*)
{
  ((uvmapWindow *)(w->parent()))->hide();

  cursorWait();
  Fl::check();
  saveCrop();
  cursorDefault();
}
void uvmapWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((uvmapWindow *)(w->parent()))->hide();
}
void uvmapWindow::cb_reset(Fl_Widget *w, void*)
{
  uvmapWindow *win;
  win=(uvmapWindow *)(w->parent());
  win->loadDefaultValues();
  win->redraw();
}


