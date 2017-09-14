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
 * imgSum.c  -  This program will add all the Trange images into a single one.
 *             Typically runned before drawing manually the crop borders.
 */

#define VERSION	" V.01"

#include <imgu/imgu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/*! input images (Trange)*/
int g_nb_img; 

/*!array of input images (Trange)*/
imgu **img; 
/*!size of input images (Trange)*/
int g_xs,g_ys; 

/*!\brief sum of the images
 *
 *data (integer format) for images (full precision)
 */
int *sum_data; 

/*!\brief path to save
 *
 * prefix for the output data
 */
char g_out[200]; 
char g_in_prefix[200]; 

void sum_Trange() {
    int i,j;
    imgu *img_sum_Trange;

    img_sum_Trange=NULL;

    // min et max des images
    imguAllocate(&img_sum_Trange,g_xs,g_ys,1);
    imguClear(img_sum_Trange);

    // compute max
    for(i=0;i<g_nb_img;i++)
    { 
        if (img[i]!=NULL)
        {
          for(j=0;j<g_xs*g_ys;j++) 
          {
            if( img[i]->data[j] > img_sum_Trange->data[j] ) img_sum_Trange->data[j]=img[i]->data[j];
          }  
        }
    }

    // save
    imguSave(img_sum_Trange,g_out,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    imguFree(&img_sum_Trange); 
}

void help()
{
    LT_PRINTF("\nUsage: ./ltsum [OPTIONS]\n\n");
    LT_PRINTF("Options: \n");
    LT_PRINTF("   -n int\n");
    LT_PRINTF("         Specify the number of input (Trange) images. 'int' stands for a integer.\n\n");
    LT_PRINTF("   -i path\n");
    LT_PRINTF("         Specify the path to find the input (Trange) images.\n\n");
    LT_PRINTF("   -o path\n");
    LT_PRINTF("         Specify the path to save the output images.\n\n");
    LT_PRINTF("   -V, --version\n");
    LT_PRINTF("         Show the version of the program.\n\n");
    LT_PRINTF("   -h, --help\n");
    LT_PRINTF("         Show the help screen.\n\n");  
}


int main(int argc, char **argv)
{
    int i,k;
    int istart;

    istart=0;
    for(i=1;i<argc;i++) {
        if( strcmp(argv[i],"-start")==0 && i+1<argc ) {
            istart=atoi(argv[i+1]);
            i++;continue;
        }		        
        if( strcmp(argv[i],"-n")==0 && i+1<argc ) {
            g_nb_img=atof(argv[i+1]);
            i++;continue;
        }		        
        if( strcmp(argv[i],"-o")==0 && i+1<argc ) {
            strcpy(g_out,argv[i+1]);
            i++;continue;
        }
        if( strcmp(argv[i],"-i")==0 && i+1<argc ) {
            strcpy(g_in_prefix,argv[i+1]);
            i++;continue;
        }
        if( strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
            help();
            exit(0);
        }
        if( strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0) {
            LT_PRINTF("\nltsum version %s\n\n",VERSION);
            exit(0);
        }
    }  

    //Load all Trange
    if(g_nb_img==0)
    {
        LT_PRINTF("\nError: Please specify the number of input (Trange) images (option -n).\n\n");
        LT_EXIT(1);
    }

    char filename[200];
    img = (imgu **)(malloc(g_nb_img*sizeof(imgu *)));
    for (i=0;i<g_nb_img;i++) img[i]=NULL;
    for(i=0; i<g_nb_img; i++)
    {
        sprintf(filename,"%sTrange_%02i.png",g_in_prefix,istart+i);
        LT_PRINTF("Loading '%s' as image %d\n",filename,istart+i);
        k=imguLoad(&img[i],filename,LOAD_16_BITS);
        if( i==0 ) 
        {
            g_xs=img[i]->xs;
            g_ys=img[i]->ys;
        }
        else if (img[i]!=NULL)
        {
            if( img[i]->xs!=g_xs || img[i]->ys!=g_ys ) 
            {
                LT_PRINTF("err xy\n");
                LT_EXIT(1);
            }
        }
    }

    sum_Trange();

    LT_EXIT(0);  
}

