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
#include "utils/gamma.h"

#define STR_LEN 256

#define EPSILON 1e-8

void get_gamma_scale(int n,double *x,double *y,double *gamma,double *scale)
{
  int i;
  matrix *A,*b,*res;
  A=NULL;
  b=NULL;
  res=NULL;

  matAllocate(&A,n/2,1);
  matAllocate(&b,n/2,1);

  for (i=0;i<n/2;i++)
  {
    A->values[i]=y[2*i+1]-y[2*i];
    b->values[i]=y[2*i]*x[2*i+1]-y[2*i+1]*x[2*i];
  }

  matSolveAb(&res,A,b);
matPrint(A);
matPrint(b);
matPrint(res);
  (*scale)=exp(res->values[0]);

  for (i=0;i<n/2;i++)
  {
    A->values[i]=res->values[0]+x[2*i];
    b->values[i]=y[2*i];
  }

  matSolveAb(&res,A,b);
matPrint(A);
matPrint(b);
matPrint(res);
  (*gamma)=res->values[0];

  matFree(&A);
  matFree(&b);
  matFree(&res);
}

double computeMean(imgu *img)
{
  int i,imgsize;
  int count;
  double var;

  if (img==NULL || img->data==NULL) return 0.0;

  imgsize=img->xs*img->ys;

  count=0;
  var=0;
  for(i=0;i<imgsize;i++)
  {
    if (img->data[i]!=0)
    {
      var+=img->data[i];
      count++;
    }
  }

  if (count!=0) var/=count;

  return var;
}

int main (int argc,char *argv[])
{
  int i,j,k,l,index;
  char inPath[STR_LEN];
  char inFilename[STR_LEN];
  char outFilename[STR_LEN];
  char outPath[STR_LEN];
  char filename[STR_LEN];
  char temp[STR_LEN];
  int xsize,ysize,zsize;
  int NbImages;
  int ProjectorIndex;
  imgu *image,*ilist;
  imgu **image_rgb;
  double **imgmeans,**imgmeanslog;
  double **trueint,**trueintlog;
  int csize;
  double gamma[3];
  double scale[3];
  double max[3],min[3];;
  int bayer;

  bayer=0;
  ProjectorIndex=0;

  image=NULL;
  ilist=NULL;

  for(i=1;i<argc;i++) {
    if( strcmp("-ipath",argv[i])==0 && i+1<argc ) {
	  strcpy(inPath,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-opath",argv[i])==0 && i+1<argc ) {
      strcpy(outPath,argv[i+1]);
      i++;continue;
    }
    if( strcmp("-ifname",argv[i])==0 && i+1<argc ) {
	  strcpy(inFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-nbimgs",argv[i])==0 && i+1<argc ) {
	  NbImages=atoi(argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ofname",argv[i])==0 && i+1<argc ) {
      strcpy(outFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-bayer",argv[i])==0) {
	  bayer=1;continue;
    }
  }

  if (NbImages!=18) { LT_PRINTF("Number of images must be equal to 18.\n"); LT_EXIT(-1); }

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
  }

  for (i=0;i<NbImages;i++)
  {
    for(j=0;j<3;j++)
    {
      //images were captured grayscale
      //make rgb images out of 3 images, offset is NbImages (which is divided by 3, see above)
      sprintf(temp,inFilename,ProjectorIndex,i+j*NbImages);
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

    xsize=image->xs;
    ysize=image->ys;
    zsize=3;
    for(j=0;j<3;j++)
    {
      if (bayer) imguAllocate(&image_rgb[j],xsize/2,ysize/2,1);
      else imguAllocate(&image_rgb[j],xsize,ysize,1); 
    }

    if (bayer)
    {
      index=0;
      for(j=0;j<ysize/2;j++)
      {
        for(k=0;k<xsize/2;k++)
        {
          l=(2*j)*xsize+(2*k);
          image_rgb[0]->data[index]=image->data[l]; //r image
  	      l=(2*j)*xsize+(2*k+1);
          image_rgb[1]->data[index]=image->data[l]; //g image
          l=(2*j+1)*xsize+(2*k+1);
  	      image_rgb[2]->data[index]=image->data[l]; //b image
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
          image_rgb[0]->data[index]=image->data[index*zsize]; //r image
          image_rgb[1]->data[index]=image->data[index*zsize+1]; //g image
  	      image_rgb[2]->data[index]=image->data[index*zsize+2]; //b image
	      index++;
        }
      }
    }    

    //find max mean value
    for(j=0;j<3;j++)
    {
      imgmeans[j][i]=computeMean(image_rgb[j])/255.0;
      if (i==0 || max[j]<imgmeans[j][i]) max[j]=imgmeans[j][i];
      if (i==0 || min[j]>imgmeans[j][i]) min[j]=imgmeans[j][i];
    }
    //LT_PRINTF("%f\n",imgmeans[0][i]); 
  }

  //take log intensities
  for(i=0;i<NbImages;i++)
  {
    for(j=0;j<3;j++)
    {
  	  if (i==0) trueint[j][i]=52.00/255.0;
  	  else if (i==1) trueint[j][i]=85.00/255.0;
  	  else if (i==2) trueint[j][i]=122.00/255.0;
  	  else if (i==3) trueint[j][i]=160.00/255.0;
  	  else if (i==4) trueint[j][i]=200.00/255.0;
  	  else trueint[j][i]=243.00/255.0;

      trueintlog[j][i]=trueint[j][i];
      if (trueintlog[j][i]<EPSILON) trueintlog[j][i]+=EPSILON;
      trueintlog[j][i]=log(trueintlog[j][i]);

      imgmeanslog[j][i]=imgmeans[j][i];
      if (imgmeanslog[j][i]<EPSILON) imgmeanslog[j][i]+=EPSILON;
      imgmeanslog[j][i]=log(imgmeanslog[j][i]);
	}
    //LT_PRINTF("%f\n",imgmeans[0][i]*255); 
  }
 
  for(i=0;i<3;i++)
  {
    get_gamma_scale(NbImages,trueintlog[i],imgmeanslog[i],&gamma[i],&scale[i]);
    LT_PRINTF("%f %f\n",gamma[i],scale[i]);
    for (j=0;j<NbImages;j++)
    {
      //LT_PRINTF("%f\t%f\t%f\n",trueintlog[i][j],imgmeanslog[i][j],trueintlog[i][j]*gamma[i]);
      //LT_PRINTF("%f\t%f\t%f\n",trueint[i][j],imgmeans[i][j],pow(trueint[i][j],gamma[i]));
    }
  }
  strcpy(temp,outFilename);
  strcpy(filename,outPath);
  strcat(filename,temp);
  //write output in filename...
   
  imguFree(&image);

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

