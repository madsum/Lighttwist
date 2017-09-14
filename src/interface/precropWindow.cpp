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

#include "precropWindow.h"

void precropWindow::createMenuWindow()
{
    int posx;
    LINEPOSY=4*OFFSET;

    machine=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_MACHINES));
    setChoice(machine,&LINEPOSY);    

    auto_playlist_flag=new Fl_Check_Button(STANDARD_WIDGET,langGetLabel(LABEL_AUTO_PLAYLIST));
    setCheckButton(auto_playlist_flag,&LINEPOSY);

    LINEPOSY+=LINESPACE;

    tile_header=new Fl_Box(STANDARD_WIDGET,langGetLabel(LABEL_PRECROP_HEADER));
    setBox(tile_header,&LINEPOSY);
    tile_header->align(FL_ALIGN_LEFT);
    tile_header->labelfont(FL_HELVETICA_BOLD);
    tile_width=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_PRECROP_WIDTH));
    setChoice(tile_width,&LINEPOSY);
    tile_width->add("128");
    tile_width->add("256");
    tile_width->add("512");
    tile_width->add("1024");
    tile_height=new Fl_Choice(STANDARD_WIDGET,langGetLabel(LABEL_PRECROP_HEIGHT));
    setChoice(tile_height,&LINEPOSY);
    tile_height->add("128");
    tile_height->add("256");
    tile_height->add("512");
    tile_height->add("1024");
    
    auto_soundtrack_flag=new Fl_Check_Button(STANDARD_WIDGET, langGetLabel(LABEL_ADD_SOUNDTRACK));
    setCheckButton(auto_soundtrack_flag,&LINEPOSY);

    LINEPOSY+=LINESPACE;
    posx=0;
    cancel=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_CANCEL));
    cancel->callback(cb_cancel);
    posx+=-cancel->w()-OFFSET;
    ok=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_OK));
    ok->callback(cb_ok);
    posx+=-ok->w()-OFFSET;
    reset=new Fl_Button(posx+SMALL_WIDGET,langGetLabel(LABEL_RESET));
    reset->callback(cb_reset);
}

void precropWindow::loadDefaultValues()
{
  tile_width->value(1);
  tile_height->value(2);
}

void precropWindow::cb_ok(Fl_Widget *w, void*)
{
  precropWindow *win;

  win=((precropWindow *)(w->parent()));
  win->hide();

  cursorWait();
  selectMediaAndExecute(win->mediatree,do_precrop);
  cursorDefault();
  updateImportedMedia();
  updateLocalPlaylists();
}
void precropWindow::cb_cancel(Fl_Widget *w, void*)
{
  ((precropWindow *)(w->parent()))->hide();
}
void precropWindow::cb_reset(Fl_Widget *w, void*)
{
  ((precropWindow *)(w->parent()))->loadDefaultValues();
}
void precropWindow::cb_undo(Fl_Widget *w, void*)
{
}





