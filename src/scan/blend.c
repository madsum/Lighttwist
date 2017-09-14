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

#include "blend.h"

int compareProjectors(const void *a, const void *b){
  const projector *a_proj = (projector *)a;
  const projector *b_proj = (projector *)b;
  return a_proj->sortkey < b_proj->sortkey ? -1 : (a_proj->sortkey == b_proj->sortkey ? 0 : 1);
}

int computeMaps(imgu **I,int NbProjs,imgu **mapall,imgu **maxall)
{
  int i,j,k;
  int xsize,ysize;
  int index;
  projector *projectors;

  if (NbProjs<1) return -1;

  projectors=(projector *)(malloc(sizeof(projector)*NbProjs));

  xsize=I[0]->xs;
  ysize=I[0]->ys;

  imguAllocate(mapall,xsize,ysize,1);
  imguAllocate(maxall,xsize,ysize,1);

  //find intersections
  index=0;
  for(i=0;i<ysize;i++)
  {
    for(j=0;j<xsize;j++) 
    {
      for(k=0;k<NbProjs;k++) 
      {
        projectors[k].sortkey=(double)(I[k]->data[index]);
        projectors[k].index=k;                      
      }
      //sort projectors according to their maximum intensity at the current pixel
      qsort(projectors, NbProjs, sizeof(projector), compareProjectors);
      (*maxall)->data[index]=(int)(projectors[NbProjs-1].sortkey);

      //identify regions in map image (MAP_INTERSECTION,MAP_SINGLE_PROJECTOR,MAP_NO_PROJECTOR)
      if (NbProjs-2>=0 && projectors[NbProjs-2].sortkey!=0) //intersection between 2 projectors!
      {
        (*mapall)->data[index]=MAP_INTERSECTION;
      }
      else if (projectors[NbProjs-1].sortkey!=0)
      {
        (*mapall)->data[index]=MAP_SINGLE_PROJECTOR;
      }
      else
      {
        (*mapall)->data[index]=MAP_NO_PROJECTOR;
      }
      index++;
    }
  }

  free(projectors);

  return 0;
}


