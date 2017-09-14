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
 * ltgammafit.c  -  Finds gamma curves from the grabbed intensity patterns
 */

#include <stdio.h>
#include <string.h>
#include <imgu/imgu.h>
#include "utils/uvmap.h"
#include "utils/gamma.h"

#define STR_LEN 256

#define EPSILON 0.0001

#define DISCARD 1000.0

//O=\alpha*\beta*I^\gamma
//where \alpha is bulb intensity factor
//where \beta is screen specularity factor
//
//solve log(O)=\gamma*log(I)+K
int get_gamma(int n,double *I,double *O,double *gamma,double *max)
{
  int i,j,count;
  matrix *A,*b,*res;
  A=NULL;
  b=NULL;
  res=NULL;

  if (gamma==NULL || max==NULL) return -1;

  count=0;
  for (i=0;i<n;i++)
  {
    if (I[i]!=DISCARD) count++;
  }

  matAllocate(&A,count,2);
  matAllocate(&b,count,1);

  j=0;
  for (i=0;i<n;i++)
  {
    if (I[i]!=DISCARD)
    {
      matSet(A,j,0,I[i]);
      matSet(A,j,1,1.0);
      matSet(b,j,0,O[i]);
      j++;
    }
  }

  matSolveAb(&res,A,b);

//printf("COUNT %d (%d)\n",count,n);
//matPrint(A);
//matPrint(b);
//matPrint(res);

  (*gamma)=res->values[0];
  (*max)=exp(res->values[1]);

  matFree(&A);
  matFree(&b);
  matFree(&res);

  return 0;
}

int computeLogMean(imgu *img,double *mean)
{
  int i,imgsize;
  int count;

  if (img==NULL || img->data==NULL) return -1;
  if (mean==NULL) return -1;

  imgsize=img->xs*img->ys;

  count=0;
  (*mean)=0;
  for(i=0;i<imgsize;i++)
  {
    if (img->data[i]!=0)
    {
      (*mean)+=log(img->data[i]/255.0);
      count++;
    }
  }

  if (count!=0) (*mean)/=count;

  return count;
}

int main (int argc,char *argv[])
{
  int i,j,k,l,index;
  char inPath[STR_LEN];
  char outPath[STR_LEN];
  char filename[STR_LEN];
  char temp[STR_LEN];
  char uvFilename[STR_LEN];
  int xsize,ysize,zsize;
  int NbImages;
  int ProjectorIndex;
  imgu *image,*ilist;
  imgu *stddev;
  imgu **image_rgb;
  double **imgmeans,**imgmeanslog;
  double **trueint,**trueintlog;
  int csize;
  double gamma[3];
  double max[3];
  int count,nb_pixels;
  int bayer;
  double gamma_camera[3];
  int IntensityThreshold=55;//lower values are discarded

  bayer=0;

  image=NULL;
  ilist=NULL;
  stddev=NULL;

  for(i=1;i<argc;i++) {
    if( strcmp("-ipath",argv[i])==0 && i+1<argc ) {
	  strcpy(inPath,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-opath",argv[i])==0 && i+1<argc ) {
      strcpy(outPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp("-nbimgs",argv[i])==0 && i+1<argc ) {
	  NbImages=atoi(argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-pindex",argv[i])==0 && i+1<argc ) {
	  ProjectorIndex=atoi(argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ithres",argv[i])==0 && i+1<argc ) {
	  IntensityThreshold=atoi(argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-bayer",argv[i])==0) {
	  bayer=1;continue;
    }
    if( strcmp("-gamma_cam",argv[i])==0 && i+3<argc ) {
	  gamma_camera[0]=atof(argv[i+1]);
	  gamma_camera[1]=atof(argv[i+2]);
	  gamma_camera[2]=atof(argv[i+3]);
	  i++;continue;
    }
  }

  if (NbImages<1) { LT_PRINTF("Number of images should be bigger than 1\n"); LT_EXIT(-1); }

  sprintf(uvFilename,"%s/uvmap.txt",outPath);

  sprintf(temp,"xyvalid_%02d.png",ProjectorIndex);
  strcpy(filename,outPath);
  strcat(filename,temp);
  k=imguLoad(&stddev,filename,LOAD_16_BITS);
  if( k ) { LT_PRINTF("Unable to load %s\n",filename); LT_EXIT(-1); }

  xsize=stddev->xs;
  ysize=stddev->ys;
  zsize=3;

  //get info for uv cropping
  strcpy(filename,uvFilename);
  if (uvReadFile(filename)==0)
  {
    if (uvCropImg(stddev))
    {
      LT_PRINTF("Error: unable to identify crop type\n");
      LT_EXIT(0);
    }
  }

//imguSave(stddev,"test.png",1,SAVE_16_BITS);
//LT_EXIT(0);

  image_rgb=(imgu **)(malloc(sizeof(imgu *)*3));
  for(i=0;i<3;i++) image_rgb[i]=NULL;
  imgmeans=(double **)(malloc(sizeof(double *)*3));  
  trueint=(double **)(malloc(sizeof(double *)*3));  
  imgmeanslog=(double **)(malloc(sizeof(double *)*3));  
  trueintlog=(double **)(malloc(sizeof(double *)*3));  

  NbImages/=3;

  for(i=0;i<3;i++)
  {
    imgmeans[i]=(double *)(malloc(sizeof(double)*NbImages));
    trueint[i]=(double *)(malloc(sizeof(double)*NbImages));
    imgmeanslog[i]=(double *)(malloc(sizeof(double)*NbImages));
    trueintlog[i]=(double *)(malloc(sizeof(double)*NbImages));

    if (bayer) imguAllocate(&image_rgb[i],xsize/2,ysize/2,1);
    else imguAllocate(&image_rgb[i],xsize,ysize,1); 
  }

  for (i=NbImages-1;i>=0;i--)
  {
    for(j=0;j<3;j++)
    {
      //images were captured grayscale
      //make rgb images out of 3 images, offset is NbImages (which is divided by 3, see above)
      sprintf(temp,"grab_gamma_%02d_%05d.png",ProjectorIndex,i+j*NbImages);
      sprintf(filename,inPath,ProjectorIndex);
      strcat(filename,temp);
      k=imguLoad(&image,filename,LOAD_16_BITS);
      if( k ) { LT_PRINTF("Unable to load image %s\n",filename); LT_EXIT(-1); }
      imguConvert16bitTo8bit(&image,image);
      //imguConvertToRGB(&image,image); 
      //make list of 3 grayscale images, and concat afterwards
      imguAddLastMulti(&ilist,image);
      image=NULL;
    }
    imguConcat(&image,ilist,0,3);
    imguFreeMulti(&ilist);
    if (bayer)
    {
      index=0;
      for(j=0;j<ysize/2;j++)
      {
        for(k=0;k<xsize/2;k++)
        {
          image_rgb[0]->data[index]=0; //r image
          image_rgb[1]->data[index]=0; //g image
  	      image_rgb[2]->data[index]=0; //b image
          l=(2*j)*xsize+(2*k);
          if (stddev->data[l]!=0) image_rgb[0]->data[index]=image->data[l]; //r image
  	      l=(2*j)*xsize+(2*k+1);
          if (stddev->data[l]!=0) image_rgb[1]->data[index]=image->data[l]; //g image
          l=(2*j+1)*xsize+(2*k+1);
  	      if (stddev->data[l]!=0) image_rgb[2]->data[index]=image->data[l]; //b image
	      index++;
        }
      }
    }
    else
    {
      index=0;
      for(j=0;j<ysize;j++)
      {
        for(k=0;k<xsize;k++)
        {
          image_rgb[0]->data[index]=0; //r image
          image_rgb[1]->data[index]=0; //g image
  	      image_rgb[2]->data[index]=0; //b image
          if (stddev->data[index]!=0) image_rgb[0]->data[index]=image->data[index*zsize]; //r image
          if (stddev->data[index]!=0) image_rgb[1]->data[index]=image->data[index*zsize+1]; //g image
  	      if (stddev->data[index]!=0) image_rgb[2]->data[index]=image->data[index*zsize+2]; //b image
	      index++;
        }
      }
    }    
    //find max mean value
    for(j=0;j<3;j++)
    {
      //compute mean of log intensities
      count=computeLogMean(image_rgb[j],&(imgmeans[j][i]))/255.0;
      if (i==NbImages-1 && j==0) nb_pixels=count;
      //discard if some pixels are black (log is undefined)
      if (count<nb_pixels)
      {
        LT_PRINTF("DISCARDING %d %d (%d<%d)\n",i,j,count,nb_pixels);
        imgmeans[j][i]=DISCARD;
      }
    }
    //LT_PRINTF("%f\n",imgmeans[0][i]); 
  }

  //take log intensities
  for(i=0;i<NbImages;i++)
  {
    for(j=0;j<3;j++)
    {
      imgmeanslog[j][i]=imgmeans[j][i];
      //if (imgmeanslog[j][i]<EPSILON) imgmeanslog[j][i]+=EPSILON;
      //imgmeanslog[j][i]=log(imgmeanslog[j][i]);

      //take log of true intensities
  	  trueint[j][i]=i/(float)(NbImages-1);     
      trueintlog[j][i]=trueint[j][i];
      if (trueintlog[j][i]<EPSILON) trueintlog[j][i]+=EPSILON;
      trueintlog[j][i]=log(trueintlog[j][i]);

      if (trueint[j][i]<IntensityThreshold/255.0)
      {
        imgmeanslog[j][i]=DISCARD;
        trueintlog[j][i]=DISCARD;
      }
	}
    //LT_PRINTF("%f\n",imgmeans[0][i]); 
  }
 
  for(i=0;i<3;i++)
  {
    get_gamma(NbImages,trueintlog[i],imgmeanslog[i],&gamma[i],&max[i]);
    LT_PRINTF("%f %f\n",gamma[i],max[i]);

    for (j=0;j<NbImages;j++)
    {
      //LT_PRINTF("%f\t%f\t%f\n",trueintlog[i][j],imgmeanslog[i][j],trueintlog[i][j]*gamma[i]);
      //LT_PRINTF("%f\t%f\t%f\n",trueint[i][j],imgmeans[i][j],pow(trueint[i][j],gamma[i]));
    }
    gamma[i]/=gamma_camera[i];
  }

  sprintf(temp,"gamma_%02d.txt",ProjectorIndex);
  strcpy(filename,outPath);
  strcat(filename,temp);
  write_gamma_and_max(gamma,max,filename);
   
  i=NbImages-1;
  sprintf(filename,"%smask_gamma_%02d.png",outPath,ProjectorIndex);
  //imguSave(images_rgb[0][i],filename,FAST_COMPRESSION,SAVE_16_BITS); 
  sprintf(filename,"%smask_gamma_%02d_g.png",outPath,ProjectorIndex);
  //imguSave(images_rgb[1][i],filename,FAST_COMPRESSION,SAVE_16_BITS); 
  sprintf(filename,"%smask_gamma_%02d_b.png",outPath,ProjectorIndex);
  //imguSave(images_rgb[2][i],filename,FAST_COMPRESSION,SAVE_16_BITS);

  LT_PRINTF("Saving mean and stddev images...");
  //sprintf(filename,"%smean_gamma_%02d.png",outPath,ProjectorIndex);
  //imguSave(&mean,filename,FAST_COMPRESSION,SAVE_16_BITS);
  //sprintf(filename,"%sstddev_gamma_%02d.pgm",outPath,ProjectorIndex);
  //sprintf(filename,"%smask_gamma_%02d.png",outPath,ProjectorIndex);
  //imguSave(stddev,filename,FAST_COMPRESSION,SAVE_8_BITS_LOW);
  LT_PRINTF("done\n");

  imguFree(&image);
  imguFree(&stddev);

  for(i=0;i<3;i++)
  {
     imguFree(&image_rgb[i]);
     free(imgmeans[i]);
     free(trueint[i]);
  }
  free(image_rgb);
  free(imgmeans);
  free(trueint);

  LT_EXIT(0);
}

