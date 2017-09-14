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
 * select.c  -  This program will select, among all images received in input,
 *                only one image per pattern.  
 *                This program is commonly used for structured light calibration. 
 *                Only grayscale format supported for images.
 */

#include <imgu/imgu.h>
#include <utils/graycode.h>
#include "select.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// ***************
// Global variables
// ***************

//FILE *LogFile = NULL;

// to save
char *g_out_prefix;
int g_cam = 0;			// number of the data camera

// images
int g_nb_img;
int g_img_size_x, g_img_size_y;
imgu **T;

// global stats
imgu *Tmin;		//image containing the minimum value for each pixel through the whole sequence
imgu *Tmax;		//image containing the maximum value for each pixel through the whole sequence
imgu *Trange;		//Tmax - Tmin (for each pixel)

float g_contrast_factor;

#define QUICK

// *******************************
void divide_contrast (imgu  *img, float factor)
{
    int x, y;

    for (y = 0; y < img->ys; y++)
    {
        for (x = 0; x < img->xs; x++)
        {
            int val = img->data[y * img->xs + x];

            val /= factor;
            val += (255. - 255. / factor) / 2.;
            val += .5;

            if (val > 255)
                val = 255;
            if (val < 0)
                val = 0;

            img->data[y * img->xs + x] = (unsigned char) val;
        }
    }
}


// Calculate the contrast for each pixel (on the whole sequence)
// Tmin contains the minimum value for each pixel
// Tmax contains the maximum value for each pixel
// Trange = Tmax - Tmin
void compute_min_max (void)
{
    int i, j;
    imguAllocate(&Tmin,g_img_size_x, g_img_size_y, 1);
    imguAllocate(&Tmax,g_img_size_x, g_img_size_y, 1);
    for (j = 0; j < g_img_size_x * g_img_size_y; j++)
    {
        Tmin->data[j] = Tmax->data[j] = T[0]->data[j];
    }

    for (i = 1; i < g_nb_img; i++)
    {
        for (j = 0; j < g_img_size_x * g_img_size_y; j++)
        {
            if (T[i]->data[j] < Tmin->data[j])
                Tmin->data[j] = T[i]->data[j];
            if (T[i]->data[j] > Tmax->data[j])
                Tmax->data[j] = T[i]->data[j];
        }
    }

    imguAllocate(&Trange,g_img_size_x, g_img_size_y, 1);
    for (j = 0; j < g_img_size_x * g_img_size_y; j++)
    {
        Trange->data[j] = Tmax->data[j] - Tmin->data[j];
    }

    char buf[200];
    sprintf (buf, "%sTmin_%02d.png", g_out_prefix, g_cam);
    imguSave(Tmin,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    sprintf (buf, "%sTmax_%02d.png", g_out_prefix, g_cam);
    imguSave(Tmax,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    sprintf (buf, "%sTrange_%02d.png", g_out_prefix, g_cam);
    imguSave(Trange,buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
}




// Find one image for each pattern. 
// We start with all the images and we put away images that are 
// too similar with their neighboring images (variable 'limit').
// 'limit' is then leveling up and we restart the whole process
// until we have only one image per pattern.
    void
find_patterns (int nb_patterns)
{
    int i, j, sum, d, n, nb_pat;
    int *dif, *pat;
    int min, limit;
    int d1, d2, d3;
    int *p_start, *p_end, *p_select;
    char buf[200];

    dif = (int *) malloc (g_nb_img * sizeof (int));
    pat = (int *) malloc (g_nb_img * sizeof (int));
    dif[0] = 0;
    for (i = 1; i < g_nb_img; i++)
    {
        sum = 0;
        n = 0;
        for (j = 0; j < g_img_size_x * g_img_size_y; j++)
        {
            d = (T[i]->data[j] - T[i - 1]->data[j]);	// diff (pixel image) and (pixel image-1)
            if (d < 0)
                d = -d;
            sum += d;
            n += Trange->data[j];
        }
        dif[i] = (sum * 10) / (n / 100);	// fill dif (mean of diff of the whole image (in comparison with image-1))
    }
    dif[0] = dif[1];

    // find min dif of the sequence
    min = dif[0];
    for (i = 0; i < g_nb_img; i++)
    {
        if (dif[i] < min)
            min = dif[i];
    }
    LT_PRINTF ("min=%d\n", min);

    limit = min;			// instead of putting limit to 0 

    for (i = 0; i < g_nb_img; i++)
        pat[i] = i;			// take all image as differents patterns at start

    for (;;)			//until 1 image per pattern ==> break
    {
        for (i = 1; i < g_nb_img - 1; i++)
        {
            //Note: limit++ at the end

            if (dif[i] < limit && dif[i + 1] < limit)	// dif ==> comparison with image-1. if(TRUE) ==> i-1 == i == i+1
            {
                // connect [i] with [i-1] and [i+1]
                d1 = pat[i - 1];
                d2 = pat[i];
                d3 = pat[i + 1];
                if (d2 < d1)
                    d1 = d2;
                if (d3 < d1)
                    d1 = d3;
                pat[i] = pat[i - 1] = pat[i + 1] = d1;
            }
            else
            {
                if (dif[i] < dif[i + 1])	//To work, it must have at least 2 images per pattern
                {
                    // connect [i] with [i-1]
                    d1 = pat[i - 1];
                    d2 = pat[i];
                    if (d1 <= d2)
                        pat[i] = pat[i - 1] = d1;
                    else
                        pat[i] = pat[i - 1] = d2;
                }
                else
                {
                    // connect [i] with [i+1]
                    d1 = pat[i + 1];
                    d2 = pat[i];
                    if (d1 <= d2)
                        pat[i] = pat[i + 1] = d1;
                    else
                        pat[i] = pat[i + 1] = d2;
                }
            }
        }
        // count patterns... must have =42 at the end
        for (i = 1, j = 1; i < g_nb_img; i++)
            if (pat[i] != pat[i - 1])
                j++;

        //LT_PRINTF("j=%d\n",j);
        if (j == nb_patterns + 2)
            break;
        if (j < nb_patterns + 2)
            break;

        // if we have more than 42 patterns, retry with a higher 'limit'
        limit++;
    }

    // count patterns
    for (i = 1, j = 1; i < g_nb_img; i++)
        if (pat[i] != pat[i - 1])
            j++;

    LT_PRINTF ("Got %d pattern\n", j);
    nb_pat = j;

    // rename from the end 
    j = nb_pat - 1;
    for (i = g_nb_img - 1; i >= 1; i--)
    {
        if (pat[i] != pat[i - 1])
            pat[i] = j--;
        else
            pat[i] = j;
    }

    // begin/end of pattern
    p_start = (int *) malloc (nb_pat * sizeof (int));
    p_end = (int *) malloc (nb_pat * sizeof (int));
    p_select = (int *) malloc (nb_pat * sizeof (int));

    j = 0;
    for (i = 0; i < g_nb_img; i++)	//find all p_start and all p_end for the whole sequence
    {
        if (i == 0 || pat[i] != pat[i - 1])
        {
            p_start[j] = i;
        }
        if (i == g_nb_img - 1 || pat[i] != pat[i + 1])
        {
            p_end[j] = i;
            j++;
        }
    }

    // scan. For each "first" pattern, use dif[i]
    // scan. For each "last" pattern, use dif[i+1]
    // Take only one image per pattern
    for (j = 0; j < nb_pat; j++)
    {
        if (p_end[j] - p_start[j] + 1 >= 3 || p_start[j] == 0
                || p_end[j] == g_nb_img - 1)
        {
            // take the middle
            p_select[j] = (p_start[j] + p_end[j]) / 2;
        }
        else if (dif[p_start[j]] > dif[p_end[j]])	//look for the most dif (Pstart or Pend)
        {
            p_select[j] = p_start[j];
        }
        else
            p_select[j] = p_end[j];
    }

    /**
      for (i = 0; i < g_nb_img; i++)
      {
      LT_PRINTF ("i=%d dif=%4d: [%3d]-[%3d] : %8d p=%d delta=%d\n", i, dif[i],
      i - 1, i, dif[i], pat[i], dif[i + 1] - dif[i]);
      }
      for (i = 0; i < nb_pat; i++)
      {
      LT_PRINTF ("Pattern[%3d]: %3d .. %3d -> select %3d\n", i, p_start[i],
      p_end[i], p_select[i]);
      }
     **/

    // free all images (except p_select)
    for (i = 0, j = 0; j < nb_pat; j++)
    {
        while (i < p_select[j])
        {
            //LT_PRINTF("free %d\n",i);
            imguFree(&T[i]);
            i++;
        }

        // i==select -> pass
        i++;
    }

    // keep only the good images
    // i is the image to keep
    for (i = 0, j = 1; j < nb_pat - 1; j++)
    {
        if (p_select[j] == i)
            continue;		// already on the right place

        T[i] = T[p_select[j]];
        //LT_PRINTF("Deplace image [%d] -> [%d]\n",p_select[j],i);
        i++;
    }

    if (nb_pat != nb_patterns + 2)
    {
        // extraction of patterns failed

        LT_PRINTF ("ERR!!!! nb_pat should be %d!!!!\n", nb_patterns + 2);
        // done
        sprintf (buf, "touch %sdone", g_out_prefix);
        system (buf);
        LT_EXIT (1);
    }

    g_nb_img = nb_patterns;	// must be 40 or 104

    // un-shuffle the low-res patterns
    for (i = 0; i < 40; i++)
    {
        imgu *z;
        z=NULL;
        j = (i / 10) * 10 + shuffle (i % 10, 10);	// /10*10 delete the last digit (replace by 0) because integer division
        if (i == j || j < i)
            continue;
        //LT_PRINTF ("Shuffle %d -> %d\n", i, j);
        z = T[i];
        T[i] = T[j];
        T[j] = z;
    }

    //----

    // finish
    free (dif);
    free (pat);
    free (p_start);
    free (p_end);
    free (p_select);
}


int main_ltselect (imgu **T_param, char *out_prefix, int nb_img, int cam, int scale, float contrast_factor, int nb_patterns)
{
    Tmin=NULL;
    Tmax=NULL;
    Trange=NULL;

    // fill global variables
    T = T_param;
    g_out_prefix = out_prefix;
    g_nb_img = nb_img;
    g_cam = cam;
    g_contrast_factor = contrast_factor;

    int i;
    char buf[100];

    //LogFile = fopen ("_log", "w");

    // resize images
    for (i = 0; i < g_nb_img; i++)
    {
        if (T[i]->cs != 1)
        {
            LT_PRINTF("Fatal error: only grayscale format supported for images\n");
            LT_EXIT (0);
        }
        if (scale > 1)
            imguScale(&T[i],T[i],scale,scale);
    }
    LT_PRINTF("done\n");

    // take the size of images (all images has same sizes)
    g_img_size_x = T[0]->xs;
    g_img_size_y = T[0]->ys;
    LT_PRINTF ("image size %d %d\n", g_img_size_x, g_img_size_y);

    // calculate the contrast for each pixel (on the whole sequence)
    compute_min_max ();

    // find patterns and delete unnessary images
    //find_patterns (nb_patterns);
    LT_PRINTF ("Pattern identification succesful\n");

    // modify contrast of images
    if (g_contrast_factor != 1.0)
    {
        for (i = 0; i < g_nb_img; i++)
        {
            divide_contrast (T[i], g_contrast_factor);
        }
    }

    // save images
#ifndef QUICK
    for (i = 0; i < g_nb_img; i++)	//g_nb_img = 40 after the call of find_patterns
    {
        sprintf (buf, "%spat%03d_%02d.png", g_out_prefix, i, g_cam);
        imguSave(T[i],buf,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    }
#endif

    // close program
    //fclose (LogFile);

    // done
    sprintf (buf, "touch %sdone", g_out_prefix);
    system (buf);

    return (0);
}

