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
 * ltcam2proj.c - Code for running the program 'ltcam2proj' from a shell.
 *                     This program is commonly used for structured light calibration.
 *                     Only grayscale format supported for images.
 * 
 */

#include "cam2proj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define VERSION	"HUF_LOW - V.04.09.13"
#define VERSION	"LTCAM2PROJ - V.01"

void help(int cam)
{
    LT_PRINTF("\nUsage: ./ltcam2proj [OPTIONS]\n\n");
    LT_PRINTF("Options: \n");
    LT_PRINTF("   -i path\n");
    LT_PRINTF("         Specify the path to find the input images.\n\n");
    LT_PRINTF("   -o path\n");
    LT_PRINTF("         Specify the path to save the output images.\n\n");
    LT_PRINTF("   -t int\n");
    LT_PRINTF("         Specify the type of reconstruction.'int' stands for a integer.\n");
    LT_PRINTF("         Possibles types: ORIG or 0 = Original (default)\n");
    LT_PRINTF("                          SMOOTH or 1 = Smooth\n"); 
    LT_PRINTF("                          MARKOV_SIMPLE or 2 = Markov simple\n");
    LT_PRINTF("                          MARKOV_CONT or 3 = Markov cont.\n");
    LT_PRINTF("                          MARKOV_JP or 4 = Markov JP\n");
    LT_PRINTF("   -B float\n");
    LT_PRINTF("         Specify the Markovian BETA (used in MARKOV_SIMPLE, MARKOV_CONT and\n");
    LT_PRINTF("         MARKOV_JP reconstruction type only). 'float' stands for a integer.\n");
    LT_PRINTF("         Default = %g\n\n", MRF_BETA);
    LT_PRINTF("   -c int\n");
    LT_PRINTF("         Specify the camera id. 'int' stands for a integer.\n");
    LT_PRINTF("         Default = %i\n\n", cam);
    LT_PRINTF("   -imap width height\n");
    LT_PRINTF("         Specify the inverse map size. 'width'(size in X direction)\n");
    LT_PRINTF("         and height(size in Y direction) must be integer numbers.\n");
    LT_PRINTF("         Default values: width= %i , height= %i\n\n", IMAP_XS_DEFAULT, IMAP_YS_DEFAULT);
    LT_PRINTF("   -w int\n");
    LT_PRINTF("         Specify the window size (only used when the reconstruction type is SMOOTH).\n");
    LT_PRINTF("         'int' stands for a integer.\n");
    LT_PRINTF("         Default value: %i\n\n", SMOOTH_WINDOW_SIZE_DEFAULT);
    LT_PRINTF("   -conf int\n");
    LT_PRINTF("         Specify the minimum confidence in %% of the range of the pixel.\n");
    LT_PRINTF("         Normally = 50%%, but if the intensity is low, lower this value a bit.\n");
    LT_PRINTF("         'int' stands for a integer.\n");
    LT_PRINTF("         Default value: %i\n\n", CONFIDENCE_MINIMUM_DEFAULT);
    LT_PRINTF("   -r int\n");
    LT_PRINTF("         Specify the minimum range value. 'int' stands for a integer.\n");
    LT_PRINTF("         This value must be adapted to the exposure time.\n");
    LT_PRINTF("         It's equal to the black image noise level X2.\n");
    LT_PRINTF("         Default value: %i\n\n", RANGE_MIN_DEFAULT);
    LT_PRINTF("   -target path\n");
    LT_PRINTF("         Specify the path to find 'target.pgm'.\n\n");
    LT_PRINTF("   -proj width height\n");
    LT_PRINTF("         Specify the projector resolution. 'width'(resolution in X direction)\n");
    LT_PRINTF("         and height(resolution in Y direction) must be integer numbers.\n");
    LT_PRINTF("         Default values: width= %i , height= %i\n\n", PROJ_RES_X_DEFAULT, PROJ_RES_Y_DEFAULT);
    LT_PRINTF("   -V, -version\n");
    LT_PRINTF("         Show the version of the program.\n\n");
    LT_PRINTF("   -h, -help\n");
    LT_PRINTF("         Show the help screen.\n\n");
    exit(0);
}


int main(int argc,char *argv[])
{
    int i,k;
    int NB_PATTERN;
    char buf[100];
    imgu **Tselect; // pattern images (1 image per pattern)
    imgu *Trange; // contrast of the images for the whole sequence (for each pixel)
    int img_size_x, img_size_y; // size of input images
    int imap_xs, imap_ys; // size of the inverse mapping
    int type; // type of resolution
    int cam = 0; // number of the camera
    char out_prefix[200];	// Prefix for the output data
    char in_prefix[200];	// Prefix for the input data
    float mrf_beta_user; // Markovian beta
    int smooth_ws;
    int confidence_min;
    int range_min;
    char uv_filename[200];
    char uvimg_filename[200];
    int proj_res_x, proj_res_y;
    int xy_threshold;

    NB_PATTERN=40;
    // reset variables
    in_prefix[0]=0;
    out_prefix[0]=0;
    type = 0;
    imap_xs = IMAP_XS_DEFAULT;
    imap_ys = IMAP_YS_DEFAULT;
    mrf_beta_user = INVALID;
    smooth_ws = SMOOTH_WINDOW_SIZE_DEFAULT;
    confidence_min = CONFIDENCE_MINIMUM_DEFAULT;
    range_min = RANGE_MIN_DEFAULT;
    xy_threshold=XY_THRESHOLD_DEFAULT;
    uv_filename[0]='\0';
    proj_res_x = PROJ_RES_X_DEFAULT;
    proj_res_y = PROJ_RES_Y_DEFAULT;

    // check parameters
    if(argc == 1)
        help(cam);

    for(i=1;i<argc;i++) 
    {
        if( strcmp(argv[i],"-i")==0 && i+1<argc ) 
        {
            strcpy(in_prefix,argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-o")==0 && i+1<argc ) 
        {
            strcpy(out_prefix,argv[i+1]);
            LT_PRINTF ("OutPrefix : %s\n", out_prefix);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-t")==0 && i+1<argc ) 
        {
            if(strcmp(argv[i+1],"ORIG")==0 || strcmp(argv[i+1],"0")==0)
                type = ORIG;
            else
                if(strcmp(argv[i+1],"SMOOTH")==0 || strcmp(argv[i+1],"1")==0)
                    type = SMOOTH;
                else
                    if(strcmp(argv[i+1],"MARKOV_SIMPLE")==0 || strcmp(argv[i+1],"2")==0)
                        type = MARKOV_SIMPLE;
                    else
                        if(strcmp(argv[i+1],"MARKOV_CONT")==0 || strcmp(argv[i+1],"3")==0)
                            type = MARKOV_CONT;
                        else
                            if(strcmp(argv[i+1],"MARKOV_JP")==0 || strcmp(argv[i+1],"4")==0)
                                type = MARKOV_JP;
                            else
                            {
                                LT_PRINTF ("Invalid type, quitting...\n");
                                LT_EXIT (1);
                            }
            LT_PRINTF ("Type of resolution : %s\n", argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-B")==0 && i+1<argc ) 
        {
            mrf_beta_user=atof(argv[i+1]);
            LT_PRINTF ("Markovian BETA is : %f\n", mrf_beta_user);
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
        if( strcmp(argv[i],"-imap")==0 && i+2<argc ) 
        {
            imap_xs=atoi(argv[i+1]); 
            imap_ys=atoi(argv[i+2]); 
            LT_PRINTF("Inverse map size = %d X %d\n", imap_xs, imap_ys);
            i+=2;
            continue;
        }
        if( strcmp(argv[i],"-w")==0 && i+1<argc ) 
        {
            smooth_ws=atoi(argv[i+1]); 
            LT_PRINTF("Window size (SMOOTH only) = %d\n", smooth_ws);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-conf")==0 && i+1<argc ) 
        {
            confidence_min=atoi(argv[i+1]); 
            LT_PRINTF("Minimum confidence = %d%%\n", confidence_min);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-r")==0 && i+1<argc ) 
        {
            range_min=atoi(argv[i+1]); 
            LT_PRINTF("Minimum range = %d\n", range_min);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-proj")==0 && i+2<argc ) 
        {
            proj_res_x=atoi(argv[i+1]); 
            proj_res_y=atoi(argv[i+2]); 
            LT_PRINTF("Projector resolution = %d X %d\n", proj_res_x, proj_res_y);
            i+=2;
            continue;
        }
        if( strcmp(argv[i],"-nb_patterns")==0 && i+1<argc ) 
        {
            NB_PATTERN=atoi(argv[i+1]); 
            LT_PRINTF("Number of Patterns = %d\n", NB_PATTERN);
            i++;
            continue;
        }
        if( strcmp(argv[i],"-V")==0 || strcmp(argv[i],"-version")==0 ) 
        {
            LT_PRINTF("%s\n\n",VERSION);
            exit(0);
        }
        if( strcmp(argv[i],"-h")==0 || strcmp(argv[i],"-help")==0 ) 
        {
            help(cam);
        }
        if( strcmp(argv[i],"-threshold")==0 && i+1<argc ) 
        {
            xy_threshold=atoi(argv[i+1]); 
            continue;
        }
    }

    if( in_prefix[0]==0 ) 
    {
        LT_PRINTF("Please specify the path of the input images (-i)\n");
        LT_EXIT(0);
    }

    sprintf(uv_filename,"%s/uvmap.txt",out_prefix);
    sprintf(uvimg_filename,"%s/uvmap.png",out_prefix);
    LT_PRINTF("UV maps:\n%s\n%s\n",uv_filename,uvimg_filename);

    Tselect=(imgu **)(malloc(sizeof(imgu *)*NB_PATTERN));
    for (i=0;i<NB_PATTERN;i++) Tselect[i]=NULL;
    Trange=NULL;

    // load input images
    LT_PRINTF("Loading %d images from %s, format: %s\n", NB_PATTERN ,in_prefix,"<in_prefix>grab_\%02d_\%05d.png" );
    fflush(stdout);
    for(i=0; i<NB_PATTERN; i++)
    {
        LT_PRINTF(".");
        fflush(stdout);
        sprintf(buf,"%sgrab_\%02d_\%05d.png",in_prefix,cam,i);
        //LT_PRINTF("loading %s\n",buf);
        k=imguLoad(&Tselect[i],buf,LOAD_16_BITS);    
        if( k ) 
        { 
            LT_PRINTF("Fatal error loading images\n");
            LT_EXIT(0); 
        }
        if (Tselect[i]->cs != 1)
        {
            LT_PRINTF("Fatal error: only grayscale format supported for images\n");
            LT_EXIT(0); 
        }   
        imguConvert16bitTo8bit(&Tselect[i],Tselect[i]);
    } 
    LT_PRINTF("done\n");
    LT_PRINTF("Loading Trange image from %s, format: %s\n", out_prefix,"\%s_Trange_\%02d.png" );
    fflush(stdout);
    sprintf(buf,"%sTrange_%02d.png",out_prefix,cam);
    k=imguLoad(&Trange,buf,LOAD_AS_IS);    
    if( k ) 
    { 
        LT_PRINTF("Fatal error loading Trange image\n");
        LT_EXIT(0); 
    }
    if (Trange->cs != 1)
    {
        LT_PRINTF("Fatal error: only grayscale format supported for Trange image\n");
        LT_EXIT(0); 
    }    

    // take the size of images (all images has same sizes)
    img_size_x =Tselect[0]->xs;
    img_size_y =Tselect[0]->ys;  
    LT_PRINTF ("image size %d %d\n", img_size_x , img_size_y);


    // *********
    // Main call
    // *********
    main_ltcam2proj(Tselect, Trange, img_size_x, img_size_y, out_prefix, type, mrf_beta_user, cam, imap_xs, imap_ys, smooth_ws, confidence_min, range_min, uv_filename, uvimg_filename, proj_res_x, proj_res_y,NB_PATTERN,xy_threshold);

    free(Tselect);

    LT_EXIT(0);
}


