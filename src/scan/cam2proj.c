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
 * cam2proj.c  -  This program will make a deformation map based on
 *               the structured light images received in input.
 *               Only grayscale format supported for images.
 */

#include "cam2proj.h"

#include "utils/graycode.h"
#include "utils/uvmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>

int crop_active;

typedef struct
{
    unsigned long mx;	// mask x
    unsigned long my;	// mask y
    int pos,neg;		// number of positive and negative images
} pattern;

int NB_MAX_ITER;

// loading variables
imgu **Tselect; // pattern images (1 image per pattern)
imgu *Trange; // contrast of the images for the whole sequence (for each pixel)
int g_img_size_x, g_img_size_y; // size of pattern images (from the camera)
int g_type;  // Type of reconstruction
float g_mrf_beta; // Markovian beta
int g_range_min; // minimum range

// binary code for each pattern
pattern *pat;

// to save
char *g_out_prefix; // Prefix for the output data
int g_cam; //Number of the camera

// codes (created by recode_xy)
// codes are position * SUBPIX (=3 bits)
int *xcode,*ycode; // final code of the pixel
int *x_bestcode,*y_bestcode; // best cost code
int *xconf,*yconf; // Confidence. For each bit of the code, 1=can be trusted, 0=cannot be trusted
int *xyvalid; // number of valid bits in x and y

int *xy_changed_code; //flag to know if the code has changed

int nbbits_x;
int nbbits_y;

double randNumber()
{
    double rd;
#ifdef WIN32
    rd=(double)(rand())/RAND_MAX;
#else
    rd=drand48();
#endif
    return rd;
}

void code_to_img(int* data, int xsize, int ysize, char* name, int nb_bit, int offset)
{
    int i;
    imgu *i_new;

    i_new=NULL;
    int mask = pow(2, nb_bit) -1;
    imguAllocate(&i_new,xsize, ysize, 1);

    for(i=0;i< i_new->xs * i_new->ys ;i++)
    {
        i_new->data[i]=(unsigned char) ( (data[i] >> offset) & mask  ) << (8 - nb_bit);
    }

    imguSave(i_new,name,FAST_COMPRESSION,SAVE_8_BITS_LOW);
    imguFree(&i_new);

    LT_PRINTF ("Saved %s\n", name);
}

// assume that images 0..nb match to pattern pat[]
void recover_xy(int confidence_min,int nb_pattern,int XY_THRESHOLD)
{
    int i,p,ipos,ineg,d,v;
    int m;
    xcode=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    ycode=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    xconf=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    yconf=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    xyvalid=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));

    // initialize to 0
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        xcode[i] = 0;
        ycode[i] = 0;
        xconf[i] = 0;
        yconf[i] = 0;
        xyvalid[i]=0;
    }

    // for each pixel
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        // for each code
        // pat[0..9]   ==> horizontal
        // pat[10..19] ==> vertical
        for(p=0;p<nb_pattern/2;p++)
        {
            ipos=Tselect[pat[p].pos]->data[i];
            ineg=Tselect[pat[p].neg]->data[i];

            // build the binary code (with repetitive logical OR)
            // if ipos > ineg ==> white strip on the positive pattern
            if( ipos>=ineg )
            {
                xcode[i]|=pat[p].mx;
                ycode[i]|=pat[p].my;
            }

            d=ipos-ineg;
            if( d<0 )
                d=-d;

            v=d*100/(Trange->data[i]+1);

            // check if the pixel is clearly defined (compare with Trange(the maximum difference for this pixel))
            if( v>=confidence_min && Trange->data[i]> g_range_min)
            {
                xconf[i]|=pat[p].mx;
                yconf[i]|=pat[p].my;
            }
        }

        // we shift the code for sub-pixel
        // all new bits are set to 0 and 0 confidence
        // warning, it's graycode!
        xcode[i]=encode(decode(xcode[i],nbbits_x)<<SUBPIX);
        ycode[i]=encode(decode(ycode[i],nbbits_y)<<SUBPIX);
        xconf[i]=xconf[i]<<SUBPIX;
        yconf[i]=yconf[i]<<SUBPIX;
    }


    // calculate validity for each pixel -> number of trusted bits
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        for(m=getpowerof2(nbbits_x)/2<<SUBPIX;m;m>>=1)
        {
            if( xconf[i]&m ) xyvalid[i]++;
        }
        for(m=getpowerof2(nbbits_y)/2<<SUBPIX;m;m>>=1)
        {
            if( yconf[i]&m ) xyvalid[i]++;
        }
    }

    ////// filtre les xyvalide
    // un xyvalid > VALIDITY_THRESHOLD
    // entoure de non valides sera elimine...
    {
        int x,y,xx,yy,dx,dy,k;
        for(y=0;y<g_img_size_y;y++)
            for(x=0;x<g_img_size_x;x++) {
                if( xyvalid[y*g_img_size_x+x]<VALIDITY_THRESHOLD ) continue;
                k=4;
                for(dy=-1;dy<=1;dy+=2)
                    for(dx=-1;dx<=1;dx+=2) {
                        xx=x+dx;
                        yy=y+dy;
                        if( xx<0 || xx>=g_img_size_x || yy<0 || yy>=g_img_size_y) continue;
                        if( xyvalid[yy*g_img_size_x+xx]<VALIDITY_THRESHOLD ) k--;
                    }
                if( k==0 ) xyvalid[y*g_img_size_x+x]=0;
            }
    }

    // save codes
    // Note: xcode is a array of int. Each cell of the array contains a number that range from 0 to 1023.
    //       To save it in image, we must divide it in 2 part - hi and low (1023 cannot fit into a image of zsize=1)
    {
        char buf[200];
        imgu *hi,*hi_cmap,*low;
        hi=NULL;
        hi_cmap=NULL;
        low=NULL;
        imguAllocate(&hi, g_img_size_x , g_img_size_y ,1);
        imguAllocateComplex(&hi, g_img_size_x , g_img_size_y ,1);
        imguAllocate(&low, g_img_size_x , g_img_size_y ,1);
        for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        {
            hi->data[i]=decode(xcode[i],nbbits_x+SUBPIX)>>(nbbits_x-8+SUBPIX);
            low->data[i]=decode(xcode[i],nbbits_x+SUBPIX)&0xff;
        }
        sprintf(buf,"%sxcode_hi_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(hi,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        sprintf(buf,"%sxcode_low_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(low,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        {
            hi->data[i]=xconf[i]>>(nbbits_x-8+SUBPIX);
            low->data[i]=xconf[i]&0xff;
        }
        sprintf(buf,"%sxconf_hi_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(hi,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        sprintf(buf,"%sxconf_low_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(low,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);

        for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        {
            hi->data[i]=decode(ycode[i],nbbits_y+SUBPIX)>>(nbbits_y-8+SUBPIX);
            low->data[i]=decode(ycode[i],nbbits_y+SUBPIX)&0xff;
        }
        sprintf(buf,"%sycode_hi_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(hi,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        sprintf(buf,"%sycode_low_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(low,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        {
            hi->data[i]=yconf[i]>>(nbbits_y-8+SUBPIX);
            low->data[i]=yconf[i]&0xff;
        }
        sprintf(buf,"%syconf_hi_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(hi,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
        sprintf(buf,"%syconf_low_%02d.png",g_out_prefix,g_cam);
        if (C2P_DEBUG) imguSave(low,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);

        for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        {
          hi->data[i]=xyvalid[i]*10;
          if (hi->data[i]<XY_THRESHOLD)
          {
            hi->data[i]=0;    
            hi->complex[i][0]=-0.1;
            hi->complex[i][1]=-0.1;
          }
          else
          {
            hi->complex[i][0]=decode(xcode[i],nbbits_x+SUBPIX);
            hi->complex[i][1]=decode(ycode[i],nbbits_y+SUBPIX); 
          }          
        }
        imguConvertFromComplexUV(&hi_cmap,hi,COMPLEX_RESCALE);
        sprintf(buf,"%sxyvalid_color_%02d.png",g_out_prefix,g_cam);
        imguSave(hi_cmap,buf,FAST_COMPRESSION,SAVE_16_BITS);
        imguConvertToGray(&hi_cmap,hi_cmap);
        sprintf(buf,"%sxyvalid_%02d.png",g_out_prefix,g_cam);
        LT_PRINTF("saving %s\n",buf);
        imguSave(hi_cmap,buf,FAST_COMPRESSION,SAVE_16_BITS);

        imguFree(&hi);
        imguFree(&hi_cmap);
        imguFree(&low);
    }
}

// integer mean...but with SUBPIX. Sufficient.
// the problem is: how to obtain the good mean when the points
// of the window are not equally distributed around the middle?
// This is especially the case on the border of the images.
// It's an absolute neccesity to have a symetric window (in the 2 ways).
void mean(int w)
{
    int n,xsum,ysum,vsum;
    int x,y,xx,yy,i,ii,iii,xxx,yyy;
    int *xc,*yc,*val;

    LT_PRINTF("--- mean on %dx%d, sz=%d\n",1+w*2,1+w*2,(1+w*2)*(1+w*2));
    xc=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    yc=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    val=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));

    for(y=0;y< g_img_size_y ;y++)
    {
        for(x=0;x< g_img_size_x ;x++)
        {
            i=y* g_img_size_x +x;
            n=0;
            xsum=ysum=vsum=0;

            // symetric scan
            // keep only if they are 2 useable points
#ifdef SYMETRIC
            for(yy=y-w;yy<=y;yy++)
            {
                yyy=2*y-yy;
                if( yy<0 || yy>= g_img_size_y  )
                    continue;
                if( yyy<0 || yyy>= g_img_size_y  )
                    continue;

                for(xx=x-w;xx<=x;xx++)
                {
                    xxx=2*x-xx;
                    if( xx<0 || xx>= g_img_size_x  )
                        continue;
                    if( xxx<0 || xxx>= g_img_size_x  )
                        continue;
                    ii=yy* g_img_size_x +xx;
                    iii=yyy* g_img_size_x +xxx;
                    if( xyvalid[ii]<VALIDITY_THRESHOLD || xyvalid[iii]<VALIDITY_THRESHOLD )
                        continue;

                    if( ii!=iii )
                    {
                        xsum+=xcode[ii]+xcode[iii];
                        ysum+=ycode[ii]+ycode[iii];
                        vsum+=xyvalid[ii]+xyvalid[iii];
                        n+=2;
                    }
                    else
                    {
                        xsum+=xcode[ii];
                        ysum+=ycode[ii];
                        vsum+=xyvalid[ii];
                        n++;
                    }
                }
            }
#else
            for(yy=y-w;yy<=y+w;yy++)
            {
                if( yy<0 || yy>= g_img_size_y  )
                    continue;
                for(xx=x-w;xx<=x+w;xx++)
                {
                    if( xx<0 || xx>= g_img_size_x  )
                        continue;
                    ii=yy* g_img_size_x +xx;
                    if( xyvalid[ii]<VALIDITY_THRESHOLD )
                        continue;
                    xsum+=xcode[ii];
                    ysum+=ycode[ii];
                    vsum+=xyvalid[ii];
                    n++;
                }
            }
#endif
            if( n==0 )
            {
                xc[i]=xcode[i];
                yc[i]=ycode[i];
                val[i]=xyvalid[i];
                continue;
            }

            // codes are sub-pixel now
            xc[i]=((xsum*2)+n)/(n*2);
            yc[i]=((ysum*2)+n)/(n*2);
            val[i]=(vsum*2+n)/(n*2);
        }
    }

    // copy!
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        xcode[i]=xc[i];
        ycode[i]=yc[i];
    }

    free(xc);
    free(yc);
    free(val);
}


// overwrite the Gray code by the decoded binary code
void kill_gray(void)
{
    int i;
    LT_PRINTF("kill gray codes...\n");
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        xcode[i]=decode(xcode[i],nbbits_x+SUBPIX);
        ycode[i]=decode(ycode[i],nbbits_y+SUBPIX);
    }
    LT_PRINTF("kill gray codes.. done\n");
}


// return 0 if ok, -1 si inversed triangle
int check_triangle(float x1,float y1,float x2,float y2,float x3,float y3)
{
    float z;

    z= -(x2*y1) + x3*y1 + x1*y2 - x3*y2 - x1*y3 + x2*y3;
    if( z>0 )
        return(1);
    else if( z<0 )
        return(-1);
    else
        return(0);
}

void write_osg_head(FILE *F)
{
    fprintf(F,"Group {\n");
    fprintf(F,"  ClearNode {\n");
    fprintf(F,"  clearColor 0 0 0 1\n");
    fprintf(F,"  }\n");
    fprintf(F,"\n");
    fprintf(F,"Projection {\n");
    fprintf(F,"  DataVariance DYNAMIC\n");
    fprintf(F,"  nodeMask 0xffffffff\n");
    fprintf(F,"  cullingActive TRUE\n");
    fprintf(F,"  num_children 1\n");
    fprintf(F,"  MatrixTransform {\n");
    fprintf(F,"    DataVariance DYNAMIC\n");
    fprintf(F,"    nodeMask 0xffffffff\n");
    fprintf(F,"    cullingActive FALSE\n");
    fprintf(F,"    referenceFrame RELATIVE_TO_ABSOLUTE\n");
    fprintf(F,"    Matrix {\n");
    fprintf(F,"      1 0 0 0\n");
    fprintf(F,"      0 1 0 0\n");
    fprintf(F,"      0 0 1 0\n");
    fprintf(F,"      0 0 0 1\n");
    fprintf(F,"    }\n");
    fprintf(F,"    num_children 1\n");
    fprintf(F,"Geode {\n");
    fprintf(F,"  UniqueID Geode_0\n");
    fprintf(F,"  DataVariance DYNAMIC\n");
    fprintf(F,"  name \"hud_1\"\n");
    fprintf(F,"  cullingActive FALSE\n");
    fprintf(F,"  num_drawables 1\n");
    fprintf(F,"  Geometry {\n");
    fprintf(F,"    DataVariance DYNAMIC\n");

    if(!OSG_TEXTURED)
    {
        fprintf(F,"    useDisplayList FALSE\n");
        fprintf(F,"\n");
        fprintf(F,"\n");
    }
    else
    {
        fprintf(F,"    StateSet {\n");
        fprintf(F,"      UniqueID StateSet_1\n");
        fprintf(F,"      DataVariance STATIC\n");
        fprintf(F,"      rendering_hint DEFAULT_BIN\n");
        fprintf(F,"      renderBinMode INHERIT\n");
        fprintf(F,"      GL_CULL_FACE OFF\n");
        fprintf(F,"      GL_LIGHTING ON\n");
        fprintf(F,"      textureUnit 0 {\n");
        fprintf(F,"        GL_TEXTURE_2D ON\n");
        fprintf(F,"        Texture2D {\n");
        fprintf(F,"          DataVariance STATIC\n");
        fprintf(F,"          file \"target1024.jpg\"\n");
        fprintf(F,"          wrap_s REPEAT\n");
        fprintf(F,"          wrap_t REPEAT\n");
        fprintf(F,"          wrap_r CLAMP\n");
        fprintf(F,"          min_filter NEAREST_MIPMAP_LINEAR\n");
        fprintf(F,"          mag_filter LINEAR\n");
        fprintf(F,"          internalFormatMode USE_IMAGE_DATA_FORMAT\n");
        fprintf(F,"          subloadMode OFF\n");
        fprintf(F,"        }\n");
        fprintf(F,"        TexEnv {\n");
        fprintf(F,"          DataVariance STATIC\n");
        fprintf(F,"          mode DECAL\n");
        fprintf(F,"        }\n");
        fprintf(F,"      }\n");
        fprintf(F,"    }\n");
        fprintf(F,"    useDisplayList FALSE\n");
        fprintf(F,"\n");
        fprintf(F,"\n");
    }
}


void write_osg_tail(FILE *F)
{
    fprintf(F,"  }\n");
    fprintf(F,"}\n");
    fprintf(F,"\n");
    fprintf(F,"  Matrix {\n");
    fprintf(F,"    0.001953125 0 0 0\n");
    fprintf(F,"    0 0.0026041667 0 0\n");
    fprintf(F,"    0 0 -1 0\n");
    fprintf(F,"    -1 -1 -0 1\n");
    fprintf(F,"  }\n");
    fprintf(F,"}\n");
    fprintf(F,"\n");
    fprintf(F,"}\n");
    fprintf(F,"\n");
}


void save_mapping(int proj_res_y)
{
    int *index;
    float *point; // [x,y]
    FILE *F;
    int x,y,nbtri,nbpoint,j,i;
    int t1,t2,t3,t4,q1,q2,q3,q4;
    int k123,k134,z_pos_total,z_neg_total,z_good;
    char buf[200];

    index=(int *)malloc( g_img_size_x * g_img_size_y *sizeof(int));
    point=(float *)malloc( g_img_size_x * g_img_size_y *2*sizeof(int)); // [x,y]
    j=0;

    // find which points can participate at the end
    // here we use a different threshold
    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
        index[i]=-1;

    for(y=0;y< g_img_size_y ;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x ;x+=TRIANGLE_SIZE)
        {
            i=y* g_img_size_x +x;
            if( xyvalid[i]<OUTPUT_VALIDITY_THRESHOLD )
                continue; // pixel not valid

            index[i]=j;

            //#warning SUBiX OVERIDES MOYSUBIX
            point[i*2+0]=(float)xcode[i]/(1<<SUBPIX);
            point[i*2+1]= ((float)proj_res_y - 1.0) - (float)ycode[i]/(1<<SUBPIX);
            j++;
        }
    }

    // before additional validation... distribute surface normals
    z_pos_total=z_neg_total=0;
    for(y=0;y< g_img_size_y -TRIANGLE_SIZE;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x -TRIANGLE_SIZE;x+=TRIANGLE_SIZE)
        {
            q1=y* g_img_size_x +x;
            q2=y* g_img_size_x +x+TRIANGLE_SIZE;
            q3=(y+TRIANGLE_SIZE)* g_img_size_x +x+TRIANGLE_SIZE;
            q4=(y+TRIANGLE_SIZE)* g_img_size_x +x;

            t1=index[q1];
            t2=index[q2];
            t3=index[q3];
            t4=index[q4];

            // first triangle
            if( t1>=0 && t2>=0 && t3>=0 )
            {
                k123=check_triangle(point[q1*2+0],point[q1*2+1],
                        point[q2*2+0],point[q2*2+1],
                        point[q3*2+0],point[q3*2+1]);
            }
            else
                k123=0;

            if( t1>=0 && t3>=0 && t4>=0 )
            {
                k134=check_triangle(point[q1*2+0],point[q1*2+1],
                        point[q3*2+0],point[q3*2+1],
                        point[q4*2+0],point[q4*2+1]);
            }
            else
                k134=0;

            if( k123<0 )
                z_neg_total++;
            if( k123>0 )
                z_pos_total++;
            if( k134<0 )
                z_neg_total++;
            if( k134>0 )
                z_pos_total++;
        }
    }

    if( z_pos_total >= z_neg_total )
        z_good=1;
    else
        z_good=-1;

    LT_PRINTF("STATS : z_pos=%d, z_neg=%d, z_good=-1\n",z_pos_total,z_neg_total);

    // additional validation... surface normals of the triangles...
    int nb_killed = 0;
    for(y=0;y< g_img_size_y -TRIANGLE_SIZE;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x -TRIANGLE_SIZE;x+=TRIANGLE_SIZE)
        {
            q1=y* g_img_size_x +x;
            q2=y* g_img_size_x +x+TRIANGLE_SIZE;
            q3=(y+TRIANGLE_SIZE)* g_img_size_x +x+TRIANGLE_SIZE;
            q4=(y+TRIANGLE_SIZE)* g_img_size_x +x;

            t1=index[q1];
            t2=index[q2];
            t3=index[q3];
            t4=index[q4];

            // first triangle
            if( t1>=0 && t2>=0 && t3>=0 )
            {
                k123=check_triangle(point[q1*2+0],point[q1*2+1],
                        point[q2*2+0],point[q2*2+1],
                        point[q3*2+0],point[q3*2+1]);
            }
            else
                k123=z_good;

            if( t1>=0 && t3>=0 && t4>=0 )
            {
                k134=check_triangle(point[q1*2+0],point[q1*2+1],
                        point[q3*2+0],point[q3*2+1],
                        point[q4*2+0],point[q4*2+1]);
            }
            else
                k134=z_good;

            if( k123!=z_good && k134!=z_good )
            {
                // 2 bad triangles!! Kill 4 points!
                index[q1]=-1;
                index[q2]=-1;
                index[q3]=-1;
                index[q4]=-1;
                //LT_PRINTF("Kill reversed [%d,%d,%d,%d]\n",t1,t2,t3,t4);
                nb_killed+=4;
            }
            else if( k123!=z_good )
            {
                index[q2]=-1;
                //LT_PRINTF("Kill reversed [%d]\n",t2);
                nb_killed +=1;
            }
            else if( k134!=z_good )
            {
                index[q4]=-1;
                //LT_PRINTF("Kill reversed [%d]\n",t4);
                nb_killed += 1;
            }
        }
    }

    LT_PRINTF("Number of deleted triangles: %d\n", nb_killed);

    // reorganize index because of the eliminated index...
    j=0;
    for(y=0;y< g_img_size_y ;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x ;x+=TRIANGLE_SIZE)
        {
            i=y* g_img_size_x +x;
            if( index[i]<0 )
                continue;
            index[i]=j;
            j++;
        }
    }

    // count triangles
    nbtri=0;
    for(y=0;y< g_img_size_y -TRIANGLE_SIZE;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x -TRIANGLE_SIZE;x+=TRIANGLE_SIZE)
        {
            t1=index[y* g_img_size_x +x];
            t2=index[y* g_img_size_x +x+TRIANGLE_SIZE];
            t3=index[(y+TRIANGLE_SIZE)* g_img_size_x +x+TRIANGLE_SIZE];
            t4=index[(y+TRIANGLE_SIZE)* g_img_size_x +x];

            if( t1>=0 && t2>=0 && t3>=0 )
                nbtri++;
            if( t1>=0 && t3>=0 && t4>=0 )
                nbtri++;
        }
    }

    sprintf(buf,"%sproj_%02d.osg",g_out_prefix,g_cam);
    F=fopen(buf,"w");
    if( F==NULL )
    {
        LT_PRINTF("Unable to open osg\n");
        LT_EXIT(0);
    }

    write_osg_head(F);

    fprintf(F,"Primitives 1\n");
    fprintf(F,"{\n");
    fprintf(F,"DrawElementsUShort TRIANGLES %d\n{\n",nbtri*3);
    for(y=0;y< g_img_size_y -TRIANGLE_SIZE;y+=TRIANGLE_SIZE)
    {
        for(x=0;x< g_img_size_x -TRIANGLE_SIZE;x+=TRIANGLE_SIZE)
        {
            t1=index[y* g_img_size_x +x];
            t2=index[y* g_img_size_x +x+TRIANGLE_SIZE];
            t3=index[(y+TRIANGLE_SIZE)* g_img_size_x +x+TRIANGLE_SIZE];
            t4=index[(y+TRIANGLE_SIZE)* g_img_size_x +x];

            if( t1>=0 && t2>=0 && t3>=0 )
            {
                fprintf(F,"%d %d %d\n",t1,t2,t3);
            }

            if( t1>=0 && t3>=0 && t4>=0 )
            {
                fprintf(F,"%d %d %d\n",t1,t3,t4);
            }
        }
    }
    fprintf(F,"}\n}\n");

    // find the number of points
    for(i=0,nbpoint=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        if( index[i]<0 )
            continue;
        nbpoint++;
    }
    fprintf(F,"VertexArray Vec3Array %d\n{\n",nbpoint);

    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        if( index[i]<0 )
            continue;

        fprintf(F,"%f %f 0\n",point[i*2+0],point[i*2+1]);
    }

    fprintf(F,"}\n");
    fprintf(F,"NormalBinding OVERALL\nNormalArray Vec3Array 1\n{\n0 0 1\n}\n");
    fprintf(F,"ColorBinding OVERALL\nColorArray Vec4Array 1\n{\n1 1 1 1\n}\n");
    fprintf(F,"TexCoordArray 0 UniqueID ltreconID Vec2Array %d\n{\n",nbpoint);

    for(i=0;i< g_img_size_x * g_img_size_y ;i++)
    {
        if( index[i]<0 )
            continue;

        fprintf(F,"%f %f\n",(float)(i% g_img_size_x )/( g_img_size_x -1.0),(float)(i/ g_img_size_x )/( g_img_size_y -1.0));
    }

    fprintf(F,"}\n");
    // New
    fprintf(F,"TexCoordArray 1 Use ltreconID\n");
    fprintf(F,"}\n");
    // New

    write_osg_tail(F);
    fclose(F);

    free(index);
    free(point);
}

void save_code ()
{
    char buf[400];

    sprintf(buf,"%sxcode_hi_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(xcode, g_img_size_x, g_img_size_y, buf, 8, 2);

    sprintf(buf,"%sxcode_low_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(xcode, g_img_size_x, g_img_size_y, buf, 8, 0);

    sprintf(buf,"%sxcode_lowest_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(xcode, g_img_size_x, g_img_size_y, buf, 3, 0);


    sprintf(buf,"%sycode_hi_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(ycode, g_img_size_x, g_img_size_y, buf, 8, 2);

    sprintf(buf,"%sycode_low_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(ycode, g_img_size_x, g_img_size_y, buf, 8, 0);

    sprintf(buf,"%sycode_lowest_new_%02d.png",g_out_prefix,g_cam);
    if (C2P_DEBUG) code_to_img(ycode, g_img_size_x, g_img_size_y, buf, 3, 0);
}


typedef struct {
    int dx,dy;
    float weight;
} dxyinfo;

int cmpdxyinfo(dxyinfo *A,dxyinfo *B)
{
    if( A->weight < B->weight ) return(-1);
    if( A->weight > B->weight ) return(1);
    return(0);
}


void barycentric( double x1, double y1, double x2, double y2, double x3, double y3,
        double x,double y,
        double *a,double *b,double *c)
{
    double denom;
    denom=-(x2*y1) + x3*y1 + x1*y2 - x3*y2 - x1*y3 + x2*y3;
    if( denom==0.0 ) {
        *a = *b = *c = -1.0; // outside!
        return;
    }
    *a=-(x2*y - x3*y - x*y2 + x3*y2 + x*y3 - x2*y3)/denom;
    *b=-(-(x1*y) + x3*y + x*y1 - x3*y1 - x*y3 + x1*y3)/denom;
    *c=-(x1*y - x2*y - x*y1 + x2*y1 + x*y2 - x1*y2)/denom;
    if( *a>1.0 ) *a=1.0;
    if( *b>1.0 ) *b=1.0;
    if( *c>1.0 ) *c=1.0;
    return;
}

// retourne 0 si ok, -1 si pas un cercle
int circle(double x1, double y1, double x2, double y2, double x3, double y3,
        double *xc,double *yc,double *r)
{
    double denom;
    denom= 2*(x2*y1 - x3*y1 - x1*y2 + x3*y2 + x1*y3 - x2*y3);
    if( denom==0.0 ) return(-1);

    *xc = x3*x3*(-y1 + y2) + x2*x2*(y1 - y3) - (x1*x1 + (y1 - y2)*(y1 - y3))*(y2 - y3);
    *yc = x1*x1*(x2 - x3) + x2*x2*x3 + x3*(-y1*y1 + y2*y2) -
        x2*(x3*x3 - y1*y1 + y3*y3) +
        x1*(-x2*x2 + x3*x3 - y2*y2 + y3*y3);
    *r=(x1*x1 - 2*x1*x2 + x2*x2 + (y1 - y2)*(y1 - y2))*
        (x1*x1 - 2*x1*x3 + x3*x3 + (y1 - y3)*(y1 - y3))*
        (x2*x2 - 2*x2*x3 + x3*x3 + (y2 - y3)*(y2 - y3));

    *xc /= denom;
    *yc /= denom;
    *r = sqrt(*r/(denom*denom));

    return(0);
}



// fill triangle
// prend un triangle et marque les tableau imap_x,imap_y, et imap_dist
// les valeurs dont u,v,r
void scan_triangle(int x1,int y1,int x2,int y2,int x3,int y3,double u1,double u2,double u3,double v1,double v2,double v3,int imap_xs,int imap_ys,float *imap_u,float *imap_v,float *imap_r)
{
    int xmin,xmax,ymin,ymax;
    int x,y;
    double a,b,c,u,v,xc,yc,r;
    xmin=xmax=x1;
    if( x2<xmin ) xmin=x2;
    if( x2>xmax ) xmax=x2;
    if( x3<xmin ) xmin=x3;
    if( x3>xmax ) xmax=x3;
    ymin=ymax=y1;
    if( y2<ymin ) ymin=y2;
    if( y2>ymax ) ymax=y2;
    if( y3<ymin ) ymin=y3;
    if( y3>ymax ) ymax=y3;

    //k=check_triangle(x1,y1,x2,y2,x3,y3);
    circle((double)x1,(double)y1,(double)x2,(double)y2,(double)x3,(double)y3,&xc,&yc,&r);

    //LT_PRINTF("(%d,%d)-(%d,%d)-(%d,%d) u=%d,%d,%d v=%d,%d,%d bb=(%d..%d,%d..%d) c=(%f,%f) r=%f k=%d\n",x1,y1,x2,y2,x3,y3,(int)u1,(int)u2,(int)u3,(int)v1,(int)v2,(int)v3,xmin,xmax,ymin,ymax,xc,yc,r,k);
    //LT_PRINTF("{%f,%d}\n",r,k);

    int k;
    for(y=ymin;y<=ymax;y++) for(x=xmin;x<=xmax;x++) {
        barycentric((double)x1,(double)y1,(double)x2,(double)y2,(double)x3,(double)y3,(double)x,(double)y,&a,&b,&c);
        if( a<0 || b<0 || c<0 ) continue;
        u=a*u1+b*u2+c*u3;
        v=a*v1+b*v2+c*v3;
        //LT_PRINTF("(%d,%d) : (%12.6f,%12.6f,%12.6f) u=%12.6f v=%12.6f\n",x,y,a,b,c,u,v);

        k=1;
        if( crop_active )
        {
            k=uvCheck(u,v);
        }else k=1;

        if( k ) {
	      if( x>=0 && x<imap_xs && y>=0 && y<imap_ys )
          {
		    imap_u[y*imap_xs+x]=u;
		    imap_v[y*imap_xs+x]=v;
		    imap_r[y*imap_xs+x]=r;
	      }
  	    }
    }
}


void scan_all_triangles(int skip,int imap_xs,int imap_ys,float *imap_x,float *imap_y,float *imap_r,int *z_pos_total,int *z_neg_total, int proj_res_x, int proj_res_y)
{
    int x,y;
    int q1,q2,q3,q4;
    int q1x,q2x,q3x,q4x;
    int q1y,q2y,q3y,q4y;
    int k123,k134;
    double u1,u2,u3,u4,v1,v2,v3,v4;
    float scale_x,scale_y;
    int zsign;

    // si on veut que ca dessine des triangles, alors on suppose que zpos/zneg sont deja bon
    if( imap_x!=NULL ) {
        zsign = (*z_pos_total > *z_neg_total )?1:-1;
    }

    //find scaling values
    scale_x = (float)imap_xs / (float)proj_res_x;
    scale_y = (float)imap_ys / (float)proj_res_y;

    *z_pos_total= *z_neg_total =0;
    for(y=0;y<g_img_size_y-skip;y+=skip) {
        for(x=0;x<g_img_size_x-skip;x+=skip) {
            q1=y* g_img_size_x +x;
            q2=y* g_img_size_x +x+skip;
            q3=(y+skip)* g_img_size_x +x+skip;
            q4=(y+skip)* g_img_size_x +x;

            q1x=(int)(xcode[q1] * scale_x +0.5);
            q2x=(int)(xcode[q2] * scale_x +0.5);
            q3x=(int)(xcode[q3] * scale_x +0.5);
            q4x=(int)(xcode[q4] * scale_x +0.5);

            q1y=(int)(ycode[q1] * scale_y +0.5);
            q2y=(int)(ycode[q2] * scale_y +0.5);
            q3y=(int)(ycode[q3] * scale_y +0.5);
            q4y=(int)(ycode[q4] * scale_y +0.5);

            if(xyvalid[q1]<INVERSE_MAP_VALIDITY_THRESHOLD) q1=-1;
            if(xyvalid[q2]<INVERSE_MAP_VALIDITY_THRESHOLD) q2=-1;
            if(xyvalid[q3]<INVERSE_MAP_VALIDITY_THRESHOLD) q3=-1;
            if(xyvalid[q4]<INVERSE_MAP_VALIDITY_THRESHOLD) q4=-1;

            // first triangle
            if( q1>=0 && q2>=0 && q3>=0) k123=check_triangle(q1x,q1y,q2x,q2y,q3x,q3y);
            else k123=0;

            // second triangle
            if( q1>=0 && q3>=0 && q4>=0) k134=check_triangle(q1x,q1y,q3x,q3y,q4x,q4y);
            else k134=0;

            if( k123<0 ) *z_neg_total+=1;
            if( k123>0 ) *z_pos_total+=1;
            if( k134<0 ) *z_neg_total+=1;
            if( k134>0 ) *z_pos_total+=1;

            u1=x;
            u2=x+skip;
            u3=x+skip;
            u4=x;
#ifdef INVERSE_Y
            v1=(g_img_size_y - 1 - (y));
            v2=(g_img_size_y - 1 - (y));
            v3=(g_img_size_y - 1 - (y+skip));
            v4=(g_img_size_y - 1 - (y+skip));
#else
            v1=y;
            v2=y;
            v3=y+skip;
            v4=y+skip;
#endif

            if( q1>=0 && q2>=0 && q3>=0 && imap_x!=NULL && k123*zsign>0 ) scan_triangle(q1x,q1y,q2x,q2y,q3x,q3y,u1,u2,u3,v1,v2,v3,imap_xs,imap_ys,imap_x,imap_y,imap_r);
            if( q1>=0 && q3>=0 && q4>=0 && imap_x!=NULL && k134*zsign>0 ) scan_triangle(q1x,q1y,q3x,q3y,q4x,q4y,u1,u3,u4,v1,v3,v4,imap_xs,imap_ys,imap_x,imap_y,imap_r);
        }
    }

    //LT_PRINTF("STATS : skip=%d, z_pos=%d, z_neg=%d\n",skip,*z_pos_total,*z_neg_total);
    return;
}





// This function makes an inverse map of size imap_xs, imap_ys.
// xcode[i,j] and ycode[i,j] must contain a point x,y in the projector.
// We want to imap_x[..] and imap_y[..] to return i,j in camera image.
// scale_x and scale_y multiply the coordinates of the projector.
// chaque map contient 3 valeurs par pixel, les trois + proches...
void inverse_map(int imap_xs, int imap_ys, int proj_res_x, int proj_res_y)
{
    int i,x,y;
    float temp_code;
    float *imap_x,*imap_y; // [imap_xs*imap_ys], valeur de map inverse [y*xs+x]
    float *imap_r; // taille du triangle [y*xs+x]
    imgu *imap_final,*imap_final_cmap;
    imgu *temp_img; //temporary image for saving
    char buf[400],buf2[400]; //text buffer
    char cmd[400];
    int zpos[20],zneg[20],zsign[20];
    int skip;

    temp_img=NULL;
    imap_final=NULL;
    imap_final_cmap=NULL;

    LT_PRINTF("InverseMap...\n");


    /// allocate memory
    imap_x=(float *)malloc(imap_xs*imap_ys*sizeof(float));
    imap_y=(float *)malloc(imap_xs*imap_ys*sizeof(float));
    imap_r=(float *)malloc(imap_xs*imap_ys*sizeof(float));

    imguAllocateComplex(&imap_final,imap_xs,imap_ys,1);

    // init : rayon trop grand
    for(i=0;i<imap_xs*imap_ys;i++) { imap_r[i]=99999999.0; }


    // imap_xyr can be set to null for statistics only
    for( skip=1;skip<10;skip++) {
        scan_all_triangles(skip,imap_xs,imap_ys,NULL,NULL,NULL,zpos+skip,zneg+skip,proj_res_x,proj_res_y);
        zsign[skip] = (zpos[skip]>zneg[skip])?1:-1;
    }
    // output stats
    /*for( skip=1;skip<10;skip++) {
        LT_PRINTF("skip = %2d , pos=%6d  neg=%6d : %12.2f%%  %12.2f%%\n",skip,
                zpos[skip],zneg[skip],
                (float)zneg[skip]/zpos[skip]*100.0,
                (float)zpos[skip]/zneg[skip]*100.0);
    }*/
    // select best
    // On commence a 2, mais bon...
    /*for(skip=2;skip<9;skip++) {
        if( zpos[skip]>zneg[skip] && (float)zneg[skip]/zpos[skip]<0.21 ) break;
        if( zpos[skip]<zneg[skip] && (float)zpos[skip]/zneg[skip]<0.21 ) break;
    }*/
    //// a cause des discontinuites... les stats ne sont pas fiables...
    skip=2;
    LT_PRINTF("best skip is %d\n",skip);

    scan_all_triangles(skip,imap_xs,imap_ys,imap_x,imap_y,imap_r,zpos+skip,zneg+skip,proj_res_x,proj_res_y);

#define SKIP
#ifndef SKIP
    //go through each pixel of the camera image and vote for distances
    for(y=0;y<g_img_size_y;y++) {
        for(x=0;x<g_img_size_x;x++) {
            // camera says it's invalid
            if(xyvalid[y*g_img_size_x+x]<INVERSE_MAP_VALIDITY_THRESHOLD) continue;

            //LT_PRINTF("vote (%d,%d) valid=%d\n",x,y,xyvalid[y*g_img_size_x+x]);

            // read xcode and ycode, scale and round them
            imap_xcode=(int)(xcode[y * g_img_size_x + x] * scale_x +0.5);
            imap_ycode=(int)(ycode[y * g_img_size_x + x] * scale_y +0.5);

            //LT_PRINTF("(%d,%d)proj\n",imap_xcode,imap_ycode);

            // outside the projector
            if( imap_xcode<0 || imap_xcode>=imap_xs || imap_ycode<0 || imap_ycode>=imap_ys ) continue;

            // save the value as a '0' distance
            //LT_PRINTF("voting at (%d,%d)proj = (%d,%d)cam\n",imap_xcode,imap_ycode,x,y);
            imap_r[(imap_ycode*imap_xs+imap_xcode)] = 1.0;
            imap_x[(imap_ycode*imap_xs+imap_xcode)] = x;
            imap_y[(imap_ycode*imap_xs+imap_xcode)] =
#ifdef INVERSE_Y
            (g_img_size_y - 1 - y);
#else
            y;
#endif
        }
    }
#endif

    LT_PRINTF("selecting final code...\n");

#define OUT_OF_CAM -1.0
    // get imap_x and imap_y in texture mapping format (0..1)
    // and put it in imap_x_final and imap_y_final
    for(y=0;y<imap_ys;y++)
    {
        for(x=0;x<imap_xs;x++)
        {
            int adr = y * imap_xs + x; //address
            int adr_inv = (imap_ys - 1 - y) * imap_xs + x; //inverse-y address

            // prends seulement le premier pour l'instant
            if( imap_r[adr]>999.0 )
            {
                temp_code=OUT_OF_CAM;
            }
            else
            {
                temp_code = imap_x[adr] / g_img_size_x;
                if( temp_code<0 )
                    temp_code=0.0;
                if( temp_code>1 )
                    temp_code=1.0;
            }
#ifdef INVERSE_Y
            imap_final->complex[adr_inv][0]=temp_code;
#else
            imap_final->complex[adr][0]=temp_code;
#endif

            if( imap_r[adr]>999.0 )
            {
                temp_code=OUT_OF_CAM;
            }
            else
            {
                temp_code = imap_y[adr] / g_img_size_y;
                if( temp_code<0 )
                    temp_code=0.0;
                if( temp_code>1 )
                    temp_code=1.0;
            }
#ifdef INVERSE_Y
            imap_final->complex[adr_inv][1]=temp_code;
#else
            imap_final->complex[adr][1]=temp_code;
#endif
        }
    }

    LT_PRINTF("Creating images...\n");

    // save imap_x_final and imap_y_final in separate images files
    imguAllocate(&temp_img,imap_xs,imap_ys,1);
    for(i=0;i<imap_xs*imap_ys;i++) {
        if(imap_final->complex[i][0] == OUT_OF_CAM)
            temp_img->data[i]=(int)(0.5*255.0+0.5);
        else
            temp_img->data[i]= imap_final->complex[i][0]*255;
    }
    sprintf(buf,"%simapX_%02d.png",g_out_prefix, g_cam);
    //imguSave(temp_img,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
    LT_PRINTF ("Saved %s\n", buf);
    for(i=0;i<imap_xs*imap_ys;i++)
        if(imap_final->complex[i][1] == OUT_OF_CAM)
            temp_img->data[i]=(int)(0.5*255.0+0.5);
        else
            temp_img->data[i]= imap_final->complex[i][1]*255;
    sprintf(buf,"%simapY_%02d.png",g_out_prefix, g_cam);
    //imguSave(temp_img,buf,FAST_COMPRESSION,SAVE_8_BITS_LOW);
    LT_PRINTF ("Saved %s\n", buf);
    imguFree(&temp_img);

    {
        FILE *F;
        int adr;
        // save imap_x_final and imap_y_final in the same list file
        sprintf(buf,"%simapXY_%02d.lut.png",g_out_prefix, g_cam);
        sprintf(buf2,"%simapXY_%02d-copy.lut.png",g_out_prefix, g_cam);
        imguConvertFromComplexUV(&imap_final_cmap,imap_final,COMPLEX_RESCALE);
        imguSave(imap_final_cmap,buf,FAST_COMPRESSION,SAVE_16_BITS);
        sprintf(cmd,"cp %s %s\n",buf,buf2);
        system(cmd);
    }

    LT_PRINTF("done\n");
}


int main_ltcam2proj(imgu **Tselect_param, imgu *Trange_param, int img_size_x, int img_size_y, char *out_prefix, int type, float mrf_beta_param, int cam, int imap_xs, int imap_ys, int smooth_ws, int confidence_min, int range_min, char *uv_filename, char *uvimg_filename, int proj_res_x, int proj_res_y,int nb_pattern, int xy_threshold)
{
    int i,k,n;
    char buf[200];
    unsigned long mx,my; // used to setup pattern information
    char type_prefix[400];  // Prefix for the type of reconstruction

    // init le crop
    crop_active = uv_filename[0]?1:0;
    if( crop_active ) {
        uvReadFile(uv_filename);
        uvLoadImg(uvimg_filename);
    }
    
    nbbits_x=getnbbits(proj_res_x);
    nbbits_y=getnbbits(proj_res_y);

    pat=(pattern *)(malloc(sizeof(pattern)*nb_pattern));

    //// mini histo pour rire
    {
        int Hr[256],Hg[256],Hb[256],Hgg[256],maxr,maxg,maxb,v,x,y,yy,i,nr,ng,ngg,nb;
        int xx;
        for(i=0;i<256;i++) Hr[i]=Hg[i]=Hb[i]=Hgg[i]=0;
        maxr=maxg=maxb=0;

        for(y=0;y<Trange_param->ys;y++)
            for(x=0;x<Trange_param->xs;x++) {
                v=Trange_param->data[y*Trange_param->xs+x];
                if( x%2==0 && y%2==0 ) Hb[v]++;
                else if( x%2==0 && y%2==1 ) Hr[v]++;
                else if( x%2==1 && y%2==0 ) Hg[v]++;
                else if( x%2==1 && y%2==1 ) Hgg[v]++;
            }

        nr=ng=nb=ngg=0;
        for(i=0;i<256;i++) { nr+=Hr[i];ng+=Hg[i];nb+=Hb[i];ngg+=Hgg[i]; }
        LT_PRINTF("NB  G=%d GG=%d R=%d B=%d\n",ng,ngg,nr,nb);
        for(i=0;i<256;i++) {
            //LT_PRINTF("%3d : %12.2f %12.2f %12.2f\n",i,(Hg[i]*100.0)/ng,(Hr[i]*100.0)/nr,(Hb[i]*100.0)/nb);
            LT_PRINTF("%3d : %7d %7d %7d %7d\n",i,Hg[i],Hgg[i],Hr[i],Hb[i]);
        }

        imgu *IA;
        IA=NULL;
        imguAllocate(&IA,Trange_param->xs,Trange_param->ys,1);

        for(y=0;y<Trange_param->ys;y++)
            for(x=0;x<Trange_param->xs;x++) {
                v=Trange_param->data[y*Trange_param->xs+x];
                xx=x/2+((x%2==1)?Trange_param->xs/2:0);
                yy=y/2+((y%2==1)?Trange_param->ys/2:0);
                IA->data[yy*Trange_param->xs+xx]=v;
            }

        //imguSave(IA,"toto.png",FAST_COMPRESSION,SAVE_16_BITS);
    }

    // fill global variables
    Tselect = Tselect_param;
    Trange = Trange_param;
    g_img_size_x = img_size_x;
    g_img_size_y = img_size_y;
    g_out_prefix = out_prefix;
    g_type = type;
    g_cam = cam;
    if(mrf_beta_param != INVALID)
        g_mrf_beta = mrf_beta_param;
    else
        g_mrf_beta = MRF_BETA;
    g_range_min = range_min;

    init_gray_tables (nbbits_x+SUBPIX,nbbits_y+SUBPIX);

    // setup pattern information
    for(n=0;n<nbbits_x+nbbits_y;n++)
    {
        if( n<nbbits_x )
        {
            //mx=1<<(nbbits_x-1-n);
            //mx=1<<(n);
            //mx=(getpowerof2(nbbits_x)/2>>n);
            mx=1<<(nbbits_x-1-n);
            my=0;
        }
        else if( n<nbbits_x+nbbits_y )
        {
            mx=0;
	        //my=1<<(nbbits_y+nbbits_x-1-n);
  	        //my=1<<(n-nbbits_y);
            //my=(getpowerof2(nbbits_y)/2>>(n-nbbits_x));
            my=1<<(nbbits_y+nbbits_x-1-n);
        }

        pat[n].mx=mx;
        pat[n].my=my;
        pat[n].pos=n;
        pat[n].neg=n+nbbits_x+nbbits_y;
    }

    LT_PRINTF("OK1\n");
    sprintf(type_prefix, "orig");
    LT_PRINTF("OK2\n");
    recover_xy(confidence_min,nb_pattern,xy_threshold);
    LT_PRINTF("OK3\n");
    kill_gray();
    LT_PRINTF("OK8\n");
    inverse_map(imap_xs, imap_ys, proj_res_x, proj_res_y);
    LT_PRINTF("OK9\n");
    if (C2P_DEBUG) save_mapping(proj_res_y);
    LT_PRINTF("OK10\n");
    save_code();
    LT_PRINTF("OK11\n");
    
    free(xcode);
    free(ycode);
    free(xconf);
    free(yconf);
    free(xyvalid);

    free(pat);

    return(0);
}


