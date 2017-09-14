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
 * graycode.c  -  Library for the calculation of the Gray code.
 */ 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "graycode.h"

//#define MAIN


int encode(int n)
{
  int g;
	
  g=n ^ (n>>1);
  return(g);
}


int decode(int g, int nb)
{
  int n,m;
  int i;
  
  n=0;
  for(i=nb,m=1<<nb;i>=0;i--,m>>=1) 
  {
    n|=((n>>1)&m)^(g&m);
  }

  return(n);
}

int getnbbits(int w)
{
  int nb=1;
  while( (1<<nb) < w ) nb++;
  return(nb);
}

int getpowerof2(int e)
{
  return (1<<e);
}

//////////////////////////////////////////
// shuffle of data....
// 0,1,2,3,4,5   n=6   ==>   0,5,2,3,4,1
// 0,1,2,3,4     n=5   ==>   0,3,2,1,4
int shuffle(int i, int n)
{
  if(i%2 == 0) 
  {
    return(i);
  }
  else
  {
    if (n%2 == 0)
    {
      return(n-i);
    }
    else
    {
      return(n-i-1);
    }
  }
}


/****
int get_color(int position, int pattern_id, int nb_bits)
{
  unsigned int mask,xor;
  int pos_gray;
  int color;

  if( pattern_id < nb_bits)    
  { 
    //mask=((1<<(nb_bits-1))>>shuffle(pattern_id, nb_bits));
    //mask=(1<<(nb_bits-1))>>pattern_id;
    mask=1<<(pattern_id);
    xor=0; 
  }
  else		
  {
    //mask=((1<<(nb_bits-1))>>shuffle(pattern_id - nb_bits, nb_bits));
    //mask=(1<<(nb_bits-1))>>(pattern_id - nb_bits);
    mask=1<<(pattern_id - nb_bits);
    xor=65535; 
  }

  pos_gray = encode(position);
  if((pos_gray & mask)) 
    color = 65535; 
  else 
    color = 0;

  return( color ^ xor );
}
****/


/*****
int get_position(int r, int nb_bits)
{
  int mask, mp, pattern_id, res, v, xor;
	 
  mp = 1;
  res = 0;
  
  for(pattern_id = 0; pattern_id < (2*nb_bits); pattern_id++, mp*=2) 
  {
    if(pattern_id < nb_bits)    
    { 
      mask=((1<<(nb_bits-1))>>shuffle(pattern_id, nb_bits));
      xor=0; 
    }
    else		
    { 
      mask=((1<<(nb_bits-1))>>shuffle(pattern_id - nb_bits, nb_bits));
      xor=65535; 
    }
    
    v=(r & mp);
    if( (v!=0 && xor==0) || (v==0 && xor==65535) ) 
    {
      res |= mask;
    }
  }
	
  res = decode(res, nb_bits);
  return(res);
}
*****/


/*******************************************/
#define SUBPIX 3
static int* gray_table_x;//[1024<<SUBPIX];
static int* gray_table_y;//[1024<<SUBPIX];
static int NB_BIT_X;
static int NB_ETIQ_X;
static int NB_BIT_Y;
static int NB_ETIQ_Y;

int decode_table_x(int g,int nb)
{
  assert (g < NB_ETIQ_X);
  assert (nb == NB_BIT_X);

  return gray_table_x[g]; 
}

int decode_table_y(int g,int nb)
{
  assert (g < NB_ETIQ_X);
  assert (nb == NB_BIT_Y);

  return gray_table_y[g]; 
}

void init_gray_tables (int nb_x,int nb_y)
{
  NB_BIT_X = nb_x;
  NB_BIT_Y = nb_y;
  NB_ETIQ_X = 0x1 << nb_x;
  NB_ETIQ_Y = 0x1 << nb_y;
  gray_table_x = (int*) malloc  (NB_ETIQ_X * sizeof (int));
  gray_table_y = (int*) malloc  (NB_ETIQ_Y * sizeof (int));
  
  printf ("Created Gray table X for  %d bit -> %d etiq\n", NB_BIT_X, NB_ETIQ_Y);
  printf ("Created Gray table Y for  %d bit -> %d etiq\n", NB_BIT_X, NB_ETIQ_Y);

  int i;
  for (i = 0; i < NB_ETIQ_X; i++)
    gray_table_x[i] = decode(i, nb_x+SUBPIX);
  for (i = 0; i < NB_ETIQ_Y; i++)
    gray_table_y[i] = decode(i, nb_y+SUBPIX);
}


/*******************************************/
#ifdef MAIN
int main(int argc,char *argv[])
{
  int i, g, n, nb_bits;
  int x,c[200], pattern_id, res, position, mask;
  
  // **
  //    for(i=0;i<1024;i++) {
  //    g=encode(i);
  //    n=decode(g,10);
  //    printf("%4d -> %4d -> %4d\n",i,g,n);
  //    }
  // **
  
  nb_bits = 5;

  for(x=0;x<(1<<nb_bits);x++) //1<<nb --> 2^nb
  {
    for(pattern_id = 0; pattern_id < (2*nb_bits); pattern_id++) 
      c[pattern_id]= get_color(x, pattern_id, nb_bits);

    printf("x=%4d : ",x);
    
    for(pattern_id = 0; pattern_id < (2*nb_bits); pattern_id++) 
      printf("%c",c[pattern_id]==0?'.':'X');

    // test le decodage...
    mask=1;
    res=0;
    for(i=0;i<(2*nb_bits);i++,mask*=2) 
    {
      if( c[i]==65535 ) 
      { 
        res|=mask; 
      }
    }

    position = get_position(res, nb_bits);
    printf(" res=%8d -> %4d \n", res, position);
  }
  
  return(0);
}

#endif


