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
 * ltselect.c - Code for running the program 'ltselect' from a shell.
 *                        This program is commonly used for structured light calibration. 
 *                        Only grayscale format supported for images.                
 */

#include <imgu/imgu.h>
#include "select.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char *argv[])
{
    int i,j;
    // to save
    char out_prefix[200];	
    char in_files[200];
    int cam = 0; // number of the data camera
    imgu *Imin;		//image containing the minimum value for each pixel through the whole sequence
    imgu *Imax;		//image containing the maximum value for each pixel through the whole sequence
    imgu *Irange;		//Tmax - Tmin (for each pixel)


    // images
    int no_img;
    imgu *T[MAX_IMG];

    for (i=0;i<MAX_IMG;i++) T[i]=NULL;
    Imin=NULL;
    Imax=NULL;
    Irange=NULL;

    float contrast_factor = 1.0;

    int k;
    char buf[200];
    int scale = 1; // scale images by 1/scale
    int nb_patterns;
    in_files[0]=0;
    out_prefix[0]=0;

    //LT_PRINTF("%s\n",VERSION);

    // check parameters
    for(i=1;i<argc;i++) 
    {
        if( strcmp(argv[i],"-i")==0 && i+1<argc ) 
        {
            strcpy(in_files,argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-o")==0 && i+1<argc ) 
        {
            strcpy(out_prefix,argv[i+1]);
            LT_PRINTF ("OutPrefix: %s\n", out_prefix);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-c")==0 && i+1<argc ) 
        {
            cam=atoi(argv[i+1]); 
            LT_PRINTF("Processing camera number %d\n", cam);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-s")==0 && i+1<argc ) 
        {
            scale=atoi(argv[i+1]);
            i+=1;
            LT_PRINTF("Scaling images 1/%d\n", scale);
            continue;
        }
        if( strcmp(argv[i],"-nb_patterns")==0 && i+1<argc ) 
        {
            nb_patterns=atoi(argv[i+1]);
            i+=1;
            LT_PRINTF("Nb patterns%d\n", nb_patterns);
            continue;
        }
        if( strcmp(argv[i],"-d")==0 && i+1<argc ) 
        {
            contrast_factor=atof(argv[i+1]);
            LT_PRINTF ("Contrast divided by %f, CONFIDENCE_MINIMUM scaled to %d \n", contrast_factor, CONFIDENCE_MINIMUM );	
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0 ) 
        {
            LT_PRINTF("%s\n\n",VERSION);
            exit(0);
        }
        if( strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0 ) 
        {
            LT_PRINTF("\nUsage: ./shell_exe_ltselect [OPTIONS]\n\n");
            LT_PRINTF("Options: \n");
            LT_PRINTF("   -i path\n");
            LT_PRINTF("         Specify the path to find the input images.\n\n");
            LT_PRINTF("   -o path\n");
            LT_PRINTF("         Specify the path to find the output images.\n\n");
            LT_PRINTF("   -c int\n");
            LT_PRINTF("         Specify the camera id. 'int' stands for a integer.\n\n");
            LT_PRINTF("   -s int\n");
            LT_PRINTF("         Specify the scaling of the images. 'int' stands for a integer.\n\n");
            LT_PRINTF("   -d float\n");
            LT_PRINTF("         Specify the contrast division factor. 'float' stands for a floating number.\n\n");
            LT_PRINTF("   -nb_patterns int \n");
            LT_PRINTF("         Specify the desired number of patterns. Default number is %d.\n", NB_PATTERNS_DEFAULT);
            LT_PRINTF("   -V, --version\n");
            LT_PRINTF("         Show the version of the program.\n\n");
            LT_PRINTF("   -h, --help\n");
            LT_PRINTF("         Show the help screen.\n\n");
            exit(0);
        }
    }

    // no input images names ==> exit
    if( in_files[0]==0 ) 
    {
        LT_PRINTF("Please specify the path of the input images (-i)\n");
        LT_EXIT(1);
    }

    // loading images
    for(no_img=0;no_img<nb_patterns;no_img++) 
    {
        LT_PRINTF(".");
        sprintf(buf,in_files,cam,no_img);

        LT_PRINTF("trying '%s'\n",buf);

        //k=imguLoad(&T[no_img],buf,LOAD_16_BITS);
        k=imguLoad(&T[0],buf,LOAD_16_BITS); //only use first image

        if( k ) 
        { 
            LT_PRINTF("Fatal error loading images\n");
            LT_EXIT(1);
        }
        else
        {
            //if (T[no_img]->cs != 1)
            if (T[0]->cs != 1)
            {
                LT_PRINTF("Fatal error: only grayscale format supported for images\n");
                LT_EXIT(1); 
            }
        }
        if (no_img==0) //first image, do allocation
        {
            imguAllocate(&Imin,T[0]->xs, T[0]->ys, 1);
            imguAllocate(&Imax,T[0]->xs, T[0]->ys, 1);
            for (j = 0; j < T[0]->xs*T[0]->ys; j++)
            {
                Imin->data[j] = Imax->data[j] = T[0]->data[j];
            }
        }
        else //other images
        {
            for (j = 0; j < T[0]->xs*T[0]->ys; j++)
            {
                if (T[0]->data[j] < Imin->data[j]) Imin->data[j] = T[0]->data[j];
                if (T[0]->data[j] > Imax->data[j]) Imax->data[j] = T[0]->data[j];
            }
        }
    }

    LT_PRINTF("\nLoaded %i images\n\n",no_img);

    imguAllocate(&Irange,T[0]->xs, T[0]->ys, 1);
    for (j = 0; j < T[0]->xs*T[0]->ys; j++)
    {
        Irange->data[j] = Imax->data[j] - Imin->data[j];
    }

    sprintf (buf, "%sTmin_%02d.png", out_prefix, cam);
    imguSave(Imin,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    sprintf (buf, "%sTmax_%02d.png", out_prefix, cam);
    imguSave(Imax,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    sprintf (buf, "%sTrange_%02d.png", out_prefix, cam);
    imguSave(Irange,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);

    imguFree(&Imin);
    imguFree(&Imax);
    imguFree(&Irange);

    //if (no_img==0) LT_EXIT(1);
    //main_ltselect(T,out_prefix,no_img,cam,scale,contrast_factor,nb_patterns);

    LT_EXIT(0);
}


