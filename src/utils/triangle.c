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

#include "triangle.h"

int triMapPixel(triangle *t,double pixel[2],double puv[2])
{
    double a,b,c;
    double denom;

    if (t==NULL) return -1;

    denom=-t->x[1]*t->y[0] + t->x[2]*t->y[0] + t->x[0]*t->y[1] - t->x[2]*t->y[1] - t->x[0]*t->y[2] + t->x[1]*t->y[2];
    if( fabs(denom)<1e-8 ) {
        return -1;
    }
    a=-(t->x[1]*pixel[1] - t->x[2]*pixel[1] - pixel[0]*t->y[1] + t->x[2]*t->y[1] + pixel[0]*t->y[2] - t->x[1]*t->y[2])/denom;
    b=-(-t->x[0]*pixel[1] + t->x[2]*pixel[1] + pixel[0]*t->y[0] - t->x[2]*t->y[0] - pixel[0]*t->y[2] + t->x[0]*t->y[2])/denom;
    c=-(t->x[0]*pixel[1] - t->x[1]*pixel[1] - pixel[0]*t->y[0] + t->x[1]*t->y[0] + pixel[0]*t->y[1] - t->x[0]*t->y[1])/denom;
    if( a>1.0 ) a=1.0;
    if( b>1.0 ) b=1.0;
    if( c>1.0 ) c=1.0;
    if( a<0 || b<0 || c<0 ) return -1;

    puv[0]=a*t->u[0]+b*t->u[1]+c*t->u[2];
    puv[1]=a*t->v[0]+b*t->v[1]+c*t->v[2];

    return 0;
}

int triMapUV(triangle *t,double puv[2],double pixel[2])
{
    double a,b,c;
    double denom;

    if (t==NULL) return -1;

    denom=-t->u[1]*t->v[0] + t->u[2]*t->v[0] + t->u[0]*t->v[1] - t->u[2]*t->v[1] - t->u[0]*t->v[2] + t->u[1]*t->v[2];
    if( fabs(denom)<1e-8 ) {
        return -1;
    }
    a=-(t->u[1]*puv[1] - t->u[2]*puv[1] - puv[0]*t->v[1] + t->u[2]*t->v[1] + puv[0]*t->v[2] - t->u[1]*t->v[2])/denom;
    b=-(-t->u[0]*puv[1] + t->u[2]*puv[1] + puv[0]*t->v[0] - t->u[2]*t->v[0] - puv[0]*t->v[2] + t->u[0]*t->v[2])/denom;
    c=-(t->u[0]*puv[1] - t->u[1]*puv[1] - puv[0]*t->v[0] + t->u[1]*t->v[0] + puv[0]*t->v[1] - t->u[0]*t->v[1])/denom;
    if( a>1.0 ) a=1.0;
    if( b>1.0 ) b=1.0;
    if( c>1.0 ) c=1.0;
    if( a<0 || b<0 || c<0 ) return -1;

    pixel[0]=a*t->x[0]+b*t->x[1]+c*t->x[2];
    pixel[1]=a*t->y[0]+b*t->y[1]+c*t->y[2];

    return 0;
}


