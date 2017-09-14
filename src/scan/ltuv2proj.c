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
 * ltuv2proj.c  -  This program will make a calibrated (and cropped) inverse map
 *                lookup table from the normal inverse map lookup table (normally obtained
 *                by the ltcam2proj program).
 *                This program is commonly used for structured light calibration. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <imgu/imgu.h>
#include "utils/uvmap.h"

#include "cam2proj.h"

#define VERSION	"LTCAMERA - v1.0"

#define MINIMUM_DIFFERENCE_VOISIN_ZERO	0.3
#define ANGLE_PROJ_MAX_DEFAULT 144

//#define CAM_DEBUG

/*! LUT DATA*/
int xs,ys;

/*![xs*ys] coord u,v -> x,y*/
imgu *UV,*UV_cmap; 

/*!temp U and temp V for columns filling*/
float *tmpU,*tmpV; 

/** \brief columns or obstacle coordinates
 *
 *is done after the cropping
 */

typedef struct 
{
    int outer_x1, outer_y1, 
        inner_x2, inner_y2,
        mid_x1, mid_y1,
        mid_x2, mid_y2,
        inv_x1, inv_y1,
        inv_x2, inv_y2,
        valid;
} col_coord_struct;


int load_LUT(char *Name)
{
    int x,y;
    float xmin,ymin,xmax,ymax;

    UV=NULL;
    UV_cmap=NULL;

    imguLoad(&UV_cmap,Name,LOAD_16_BITS);
    imguConvertToComplexUV(&UV,UV_cmap);

    xs=UV->xs;
    ys=UV->ys;

    xmin=ymin=999999;
    xmax=ymax=-99999;
    for(y=0;y<ys;y++)
    { 
        for(x=0;x<xs;x++) 
        {
            if( UV->complex[y*xs+x][0]<xmin && UV->complex[y*xs+x][0]>=0.0 ) xmin=UV->complex[y*xs+x][0];
            if( UV->complex[y*xs+x][0]>xmax ) xmax=UV->complex[y*xs+x][0];
            if( UV->complex[y*xs+x][1]<ymin && UV->complex[y*xs+x][1]>=0.0 ) ymin=UV->complex[y*xs+x][1];
            if( UV->complex[y*xs+x][1]>ymax ) ymax=UV->complex[y*xs+x][1];
        }
    }

    LT_PRINTF("X : (%f - %f)  Y: (%f - %f)\n",xmin,xmax,ymin,ymax);

    return(0);
}

int save_LUT(char *Name)
{
    int x,y;

    xs=UV->xs;
    ys=UV->ys;

    imguConvertFromComplexUV(&UV_cmap,UV,COMPLEX_RESCALE);
    imguSave(UV_cmap,Name,FAST_COMPRESSION,SAVE_16_BITS);

    return(0);
}

//// fast! simple regle: on trouve le max,
// et on a un angle max sous la forme d'une fraction de 1
// par exemple: 0.5 signifie 180 degre... et ca doit representer
// l'angle max d'un projecteur
int angle_unwrap_fast(int angle_proj_max)
{
    int x,y;
    double min,max;
    float angle_proj_max_norm;

    angle_proj_max_norm = angle_proj_max / 360.0;

    min=9999;
    max=-9999;
    for(y=0;y<ys;y++) for(x=0;x<xs;x++) 
    {
        if( UV->complex[y*xs+x][0]<0.0 ) continue;
        if( UV->complex[y*xs+x][0]<min ) min=UV->complex[y*xs+x][0];
        if( UV->complex[y*xs+x][0]>max ) max=UV->complex[y*xs+x][0];
    }
    LT_PRINTF("Difference is %f ... %f\n",min,max);
    LT_PRINTF("Max angle is: %f\n",angle_proj_max_norm);
    if( max-min < angle_proj_max_norm ) return(0);

    for(y=0;y<ys;y++)
    {   
      for(x=0;x<xs;x++) 
      {
        if( UV->complex[y*xs+x][0]<0.0 ) continue;
        if( max-UV->complex[y*xs+x][0] > angle_proj_max_norm ) {
            UV->complex[y*xs+x][0]+=1.0;
        }
      }
    }

    min=9999;
    max=-9999;
    for(y=0;y<ys;y++) for(x=0;x<xs;x++) 
    {
        if( UV->complex[y*xs+x][0]<0.0 ) continue;
        if( UV->complex[y*xs+x][0]<min ) min=UV->complex[y*xs+x][0];
        if( UV->complex[y*xs+x][0]>max ) max=UV->complex[y*xs+x][0];
    }
    LT_PRINTF("Difference after uv mapping is %f ... %f\n",min,max);

    return(0);
}

int imguVerifyInterpolation(imgu *img,int x,int y)
{
  int k,l,index,count;
  int not_valid;

  if (img==NULL) return -1;
  if (imguCheck(img,(float)(x),(float)(y))!=0) return -1;

  not_valid=0;
  for(l=0;l<4;l++)
  {
    if (l==0) index=y*img->xs+x;
    else if (l==1)
    {
      if (x==img->xs-1) continue;
      index=y*img->xs+x+1;
    }
    else if (l==2)
    {
      if (y==img->ys-1) continue;
      index=(y+1)*img->xs+x;
    }
    else
    {
      if (y==img->ys-1) continue;
      index=(y+1)*img->xs+x+1;
    }
    //skip pixels with intensity 0
    count=0;
    for (k=0;k<img->cs;k++)
    {      
      if (img->data[index*img->cs+k]==0) count++;
    }
    if (count==img->cs)
    {
      not_valid=1;
    }
  }

  return not_valid;
}

void inverse(imgu *IA,imgu **IB)
{
    int i,j,k,index;
    int x,y;
    int count;
    double temp;
    double xx,yy,v,u;
    int ix,iy;
    int ixk,iyk;
    int cvals[4];
    int vi; 
    unsigned short default_val;

    imguAllocate(IB,xs,ys,1);

    //default_val=IMGU_MAXVAL/2;
    default_val=0;

    for(y=0;y<ys;y++) 
    {
        for(x=0;x<xs;x++) 
        {
            /*! U[] et V[] are the coordinates of camera texture (0..1) -> pixel camera*/
            u=UV->complex[y*xs+x][0];
            v=UV->complex[y*xs+x][1];

            if(u<0 || v<0)
            {
                (*IB)->data[y*xs+x]=default_val;
            }
            else
            {
                /*! xx et yy are the coordinates in camera image*/
                xx=u*(IA->xs);
                yy=v*(IA->ys);
                ix=(int)(xx);
                iy=(int)(yy);
                if (ix>=IA->xs) ix=IA->xs-1;
                if (iy>=IA->ys) iy=IA->ys-1;
                
                index=y*xs+x;

                /*!inverse y*/
                //yy=IA->ys-1.0-yy;

                if(imguCheck(IA,xx,yy)==0 && imguCheck(IA,xx+1.0,yy+1.0)==0) 
                {
                    if (uvCheck(xx,yy)) 
                    {
                      if ((imguVerifyInterpolation(IA,(int)(xx),(int)(yy))==0 || IA->data[iy*IA->xs+ix]==1) && imguInterpolateBilinear(IA,xx,yy,&v)==0)
                      {
                        vi=(int)(v+0.5);
                        //vi=IA->data[iy*IA->xs+ix];
                        if( vi>IMGU_MAXVAL ) vi=IMGU_MAXVAL;
                        if( vi<0 ) vi=0;
                        (*IB)->data[index]=vi; 
                      }
                      /*else if (IA->data[iy*IA->xs+ix]==1 && && imguInterpolateBilinear(IA,xx,yy,&v)==0)
                      {
                        (*IB)->data[index]=IA->data[iy*IA->xs+ix];
                      }*/
                      else
                      {
                          (*IB)->data[index]=default_val;
                      }
                    } //cropCheck
                    else
                    {
                      (*IB)->data[index]=default_val; 
                    }
                }
                else //imguCheck
                {
                    (*IB)->data[index]=default_val;
                }
            }
        }
    }
}

void lut_uvmap(int camxs,int camys,unsigned char vmirror,int angle_proj_max)
{
    int x,y,index;
    double u,v;
    double range;
    vector3 uv_cam;
    float angle_proj_max_norm;

    angle_proj_max_norm = angle_proj_max / 360.0;

    for(y=0;y<ys;y++) 
    {
        for(x=0;x<xs;x++) 
        {
            /*! coord texture camera -> pixel camera*/
            u=UV->complex[y*xs+x][0]*camxs;
            v=UV->complex[y*xs+x][1]*camys;
            if (uvCrop(u,v,uv_cam) || uv_cam[0]<0.0 || uv_cam[1]<0.0)
            {
              UV->complex[y*xs+x][0]=-1.0;
              UV->complex[y*xs+x][1]=-1.0;
            }
            else
            {
              if (vmirror) uv_cam[1]=1.0-uv_cam[1];
              UV->complex[y*xs+x][0]=uv_cam[0];
              UV->complex[y*xs+x][1]=uv_cam[1];
            }
        }
    }
}

int fill_blend(imgu *IA,imgu *IB)
{
  int k,index;
  int i,j;
  int x,y;
  double xx,yy,u,v;
  int ix,iy;
  double temp;
  int count;
  int ixk,iyk;
  imgu *IBcpy;
  IBcpy=NULL;

  imguCopy(&IBcpy,IB);

  for (y=0;y<IB->ys;y++)
  {
    for (x=0;x<IB->xs;x++)
    {
       index=y*xs+x;
       u=UV->complex[index][0];
       v=UV->complex[index][1];

       if(u<0 || v<0)
       {
         IB->data[index]=0;
       }
       else
       {
         xx=u*(IA->xs);
         yy=v*(IA->ys);

         if (uvCheck(xx,yy) && IB->data[index]==0)  
         {
             ix=x;
             iy=y;
             k=1;
             while(1) //look in region of size k around ix,iy
             {
                 temp=0;
                 count=0;
                 for (j=0;j<4;j++) //2 rows and 2 columns are added each time k is increased
                 {
                     for (i=-k;i<=k;i++)
                     {
                         if (j==0) //top row
                         {
                             ixk=ix-k;
                             iyk=iy-i;
                         }
                         else if (j==1) //bottom row
                         {
                             ixk=ix+k;
                             iyk=iy+i;
                         }
                         else if (j==2) //left column
                         {
                             ixk=ix-i;
                             iyk=iy-k;
                         }
                         else //right column
                         {
                             ixk=ix+i;
                             iyk=iy+k;
                         }
                         if (imguCheck(IB,(double)(ixk),(double)(iyk))==0)
                         {
                             if (IBcpy->data[iyk*IB->xs+ixk]!=0)
                             {
                                 temp+=IBcpy->data[iyk*IB->xs+ixk];
                                 count++;
                             }
                         }
                     }
                 }
                 if (temp!=0) break;
                 k++;
             }
             IB->data[index]=temp/=count;
         }
       }
    }
  }


  imguFree(&IBcpy);

  return 0;
}

int slim_blend(imgu *IB)
{
  int i,j,k,l;
  unsigned char found_end;

  if (IB==NULL) return -1;
  if (UV==NULL) return -1;
  if (IB->xs!=UV->xs) return -1;
  if (IB->ys!=UV->ys) return -1;

  for (i=1;i<IB->ys-1;i++)
  {
    for (j=1;j<IB->xs-1;j++)
    {
      found_end=0;
      for (k=-1;k<=1;k++)
      {
        for (l=-1;l<=1;l++)
        {
          if (UV->complex[(i+k)*UV->xs+(j+l)][0]<0) found_end=1;
        }
      }
      if (found_end) IB->data[i*IB->xs+j]=0;
    }
  }

  return 0;
}


void help()
{
    LT_PRINTF("\nUsage: ./ltuv2proj [OPTIONS]\n\n");
    LT_PRINTF("Options: \n");
    LT_PRINTF("   -lut filename\n");
    LT_PRINTF("         Specify the filename of the input lookup table.\n\n");
    LT_PRINTF("   -olut filename\n");
    LT_PRINTF("         Specify the filename of the output lookup table.\n\n");
    LT_PRINTF("   -i filename\n");
    LT_PRINTF("         Specify the filename of the input blend map (support).\n\n");
    LT_PRINTF("   -o filename\n");
    LT_PRINTF("         Specify the filename of the output blend map (support).\n\n");
    LT_PRINTF("   -crop filename\n");
    LT_PRINTF("         Specify the filename that contain the configuration of the crop.\n\n");
    LT_PRINTF("   -xcode filename\n");
    LT_PRINTF("         Specify the filename .math for x.\n\n");
    LT_PRINTF("   -ycode filename\n");
    LT_PRINTF("         Specify the filename .math for y.\n\n");
    LT_PRINTF("   -proj width height\n");
    LT_PRINTF("         Specify the projector resolution. 'width'(resolution in X direction)\n");
    LT_PRINTF("         and height(resolution in Y direction) must be integer numbers.\n");
    LT_PRINTF("         Default values: width= %i , height= %i\n\n", PROJ_RES_X_DEFAULT, PROJ_RES_Y_DEFAULT);
    LT_PRINTF("   -V, --version\n");
    LT_PRINTF("         Show the version of the program.\n\n");
    LT_PRINTF("   -h, --help\n");
    LT_PRINTF("         Show the help screen.\n\n");
}


int main(int argc,char *argv[])
{
    char LutName[100];
    char OutLUTName[100];
    char InName[100];
    char OutName[100];
    char uv_filename[100];
    char uvimg_filename[100];
    imgu *IA,*IB;
    int i,k;
    int uv_type;
    int vmirror;
    int angle_proj_max;
    int extend_blend;  
    int proj_res_x, proj_res_y;

    IA=NULL;
    IB=NULL;

    LutName[0]=0;
    OutLUTName[0]=0;
    InName[0]=0;
    OutName[0]=0;
    uv_filename[0]=0;
    uv_type=NONE;
    uvimg_filename[0]=0;
    vmirror = 0;
    extend_blend = 0;
    angle_proj_max = ANGLE_PROJ_MAX_DEFAULT;
    proj_res_x = PROJ_RES_X_DEFAULT;
    proj_res_y = PROJ_RES_Y_DEFAULT;

    LT_PRINTF("\n-- %s --\n",VERSION);
    for(i=1;i<argc;i++) {
        if( strcmp("-lut",argv[i])==0 && i+1<argc ) {
            strcpy(LutName,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-olut",argv[i])==0 && i+1<argc ) {
            strcpy(OutLUTName,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-i",argv[i])==0 && i+1<argc ) {
            strcpy(InName,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-o",argv[i])==0 && i+1<argc ) {
            strcpy(OutName,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-uv",argv[i])==0 && i+1<argc ) {
            strcpy(uv_filename,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-uvimg",argv[i])==0 && i+1<argc ) {
            strcpy(uvimg_filename,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-angle_proj_max",argv[i])==0 && i+1<argc ) {
            angle_proj_max = atoi(argv[i+1]);
            i++;continue;
        }
        if( strcmp("-vmirror",argv[i])==0) {
            vmirror = 1;
            continue;
        }
        if( strcmp("-extend_blend",argv[i])==0) {
            extend_blend = 1;
            continue;
        }
        if( strcmp(argv[i],"-proj")==0 && i+2<argc ) 
        {
            proj_res_x=atoi(argv[i+1]); 
            proj_res_y=atoi(argv[i+2]); 
            i+=2;
            continue;
        }
        if( (strcmp("-V",argv[i])==0) || (strcmp("--version",argv[i])==0)  ) {
            LT_PRINTF("\n%s\n\n",VERSION);
            LT_EXIT(0);
        }
        if( (strcmp("-h",argv[i])==0) || (strcmp("--help",argv[i])==0)  ) {
            help();
            LT_EXIT(0);
        }
    }

    if( LutName[0]==0 ) 
    {
        LT_PRINTF("Error: No LutName!\n");
        LT_EXIT(-1);
    }

    if( InName[0]==0 ) {
        LT_PRINTF("Blend image needed to get camera image size!\n");LT_EXIT(0);
    }
    k=imguLoad(&IA,InName,LOAD_AS_IS);
    if( k ) { LT_PRINTF("Unable to load blend.\n");LT_EXIT(-1); }

    k=load_LUT(LutName);
    if( k ) { LT_PRINTF("Unable to load lut.\n");LT_EXIT(-1); }

    uvReadFile(uv_filename);
    uvLoadImg(uvimg_filename);
    uvGetType(&uv_type);    

    /*!we must reverse BEFORE	
     * while the luts points toward the real camera
     *because after, the LUT points toward a virtual camera*/

    inverse(IA,&IB);

    if (extend_blend) fill_blend(IA,IB);

    lut_uvmap(IA->xs,IA->ys,vmirror,angle_proj_max);

    slim_blend(IB);

    /// resoudre le 0
    if( uv_type==UV_DUAL_CIRCLES )  {
      if (angle_proj_max<360)
      {
        while( angle_unwrap_fast(angle_proj_max) ) ;
      }
    }

    if( OutName[0] ) {
        imguSave(IB,OutName,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    }

    if( OutLUTName[0] ) {
        save_LUT(OutLUTName);
    }

    imguFree(&UV);
    imguFree(&UV_cmap);

    LT_EXIT(0);
}



