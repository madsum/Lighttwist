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

#include "listitem.h"

void initItemList(list_item *list)
{
    int i;

    if (list==NULL) return; 

    for (i=0;i<MAX_NB_LIST_ITEMS;i++)
    {
        list[i].id=-1;
        list[i].value=-1;
        list[i].label[0]='\0';
    }
}

int findListItem(list_item *list,int value)
{
    int i;

    if (list==NULL) return -1; 

    for (i=0;i<MAX_NB_LIST_ITEMS;i++)
    {
        if (list[i].value==value) return list[i].id;
    }

    return -1;
}

int findListItemFromLabel(list_item *list,const char *label)
{
    int i;

    if (list==NULL) return -1;
    if (label==NULL) return -1;  

    for (i=0;i<MAX_NB_LIST_ITEMS;i++)
    {
        if (strcmp(list[i].label,label)==0) return list[i].id;
    }

    return -1;
}






