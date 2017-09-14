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
 * ltcolor.c  -  Measures the colors observed from the grabbed color patterns
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <imgu/imgu.h>
#include "utils/uvmap.h"
#include "utils/gamma.h"

#define STR_LEN 256

#define EPSILON 0.0001

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

//expects 3xn arrays
int writeColorCube(int n,double **imgmeans, double **imgmeans_correct,double **trueints, char *filename)
{
  FILE *fd;
  int i;

  if (filename==NULL) return -1;

  fd = fopen(filename, "w");
  
  if (fd==NULL)
  {
    LT_PRINTF("An error occured while opening '%s'\n",filename);
    LT_EXIT(0);
  }

  fprintf(fd,"%d\n",n);
  fprintf(fd,"projectedRGB correctedRGB trueRGB\n");
  for (i=0;i<n;i++)
  {
    fprintf(fd,"%f\t",imgmeans[0][i]);
    fprintf(fd,"%f\t",imgmeans[1][i]);
    fprintf(fd,"%f\t",imgmeans[2][i]);
    fprintf(fd,"%f\t",imgmeans_correct[0][i]);
    fprintf(fd,"%f\t",imgmeans_correct[1][i]);
    fprintf(fd,"%f\t",imgmeans_correct[2][i]);
    fprintf(fd,"%f\t",trueints[0][i]);
    fprintf(fd,"%f\t",trueints[1][i]);
    fprintf(fd,"%f\n",trueints[2][i]);
  }

  fclose(fd);

  return 0;
}

int main (int argc,char *argv[])
{
  int i,j,k,l,index;
  char inPath[STR_LEN];
  char inFilename[STR_LEN];
  char outPath[STR_LEN];
  char filename[STR_LEN];
  char temp[STR_LEN];
  char uvFilename[STR_LEN];
  char maxFilename[STR_LEN];
  int xsize,ysize,zsize;
  int NbImages;
  int ProjectorIndex;
  imgu *image;
  imgu *stddev;
  imgu **image_rgb;
  double **imgmeans,**imgmeans_correct;
  double **trueints;
  int color_res;
  vector3 rgb1,rgb2;
  vector3 hls1,hls2;
  int csize;
  double max[3];
  double gamma[3];
  int bayer;

  image=NULL;
  stddev=NULL;

  bayer=0;

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
    if( strcmp("-pindex",argv[i])==0 && i+1<argc ) {
	  ProjectorIndex=atoi(argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-uv",argv[i])==0 && i+1<argc ) {
      strcpy(uvFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-fmax",argv[i])==0 && i+1<argc ) {
      strcpy(maxFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-bayer",argv[i])==0) {
	  bayer=1;continue;
    }
  }

  if (NbImages<1) { LT_PRINTF("Number of images should be bigger than 1\n"); LT_EXIT(-1); }

  sprintf(temp,"xyvalid_%02d.png",ProjectorIndex);
  strcpy(filename,outPath);
  strcat(filename,temp);
  k=imguLoad(&stddev,filename,LOAD_AS_IS);
  if( k ) { LT_PRINTF("Unable to load %s\n",filename); LT_EXIT(-1); }

  xsize=stddev->xs;
  ysize=stddev->ys;
  zsize=3;

  sprintf(filename,maxFilename,ProjectorIndex);
  read_gamma_and_max(gamma,max,filename);
  //LT_PRINTF("%f %f %f\n",gamma[0],gamma[1],gamma[2]);
  //LT_PRINTF("%f %f %f\n",max[0],max[1],max[2]);

  //get info for cropping the floor and ceiling
  strcpy(filename,uvFilename);
  if (uvReadFile(filename))
  {
    LT_PRINTF("Unable to load the crop file %s\n",filename); LT_EXIT(-1); 
  }
  if (uvCropImg(stddev))
  {
    LT_PRINTF("Error: unable to identify crop type\n");
    LT_EXIT(0);
  }

  image_rgb=(imgu **)(malloc(sizeof(imgu *)*3));
  for (i=0;i<3;i++) image_rgb[i]=NULL;
  imgmeans=(double **)(malloc(sizeof(double *)*3));
  imgmeans_correct=(double **)(malloc(sizeof(double *)*3));
  trueints=(double **)(malloc(sizeof(double *)*3));
  
  for(i=0;i<3;i++)
  {
    imgmeans[i]=(double *)(malloc(sizeof(double)*NbImages));
    imgmeans_correct[i]=(double *)(malloc(sizeof(double)*NbImages));
    trueints[i]=(double *)(malloc(sizeof(double)*NbImages));
    
    if (bayer) imguAllocate(&image_rgb[i],xsize/2,ysize/2,1);
    else imguAllocate(&image_rgb[i],xsize,ysize,1); 
  }

  for (i=0;i<NbImages;i++)
  {
    sprintf(temp,inFilename,ProjectorIndex,i);
    sprintf(filename,inPath,ProjectorIndex);
    strcat(filename,temp);
    k=imguLoad(&image,filename,LOAD_16_BITS);
    if( k ) { LT_PRINTF("Unable to load image %s\n",filename); LT_EXIT(-1); }
    imguConvert16bitTo8bit(&image,image);

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

    color_res=(int)floor(cbrt(NbImages));
   
    //take log intensities
    for(j=0;j<3;j++)
    {
      imgmeans[j][i]=computeMean(image_rgb[j]);
	}
    if (NbImages==24)
    {
      if (i==0){trueints[0][i]=115.0;trueints[1][i]=82.0;trueints[2][i]=68.0;}
      else if (i==1){ trueints[0][i]=194.0;trueints[1][i]=150.0;trueints[2][i]=130.0;}
      else if (i==2){ trueints[0][i]=98.0;trueints[1][i]=122.0;trueints[2][i]=157.0;}
      else if (i==3){ trueints[0][i]=87.0;trueints[1][i]=108.0;trueints[2][i]=67.0;}
      else if (i==4){ trueints[0][i]=133.0;trueints[1][i]=128.0;trueints[2][i]=177.0;}
      else if (i==5){ trueints[0][i]=103.0;trueints[1][i]=189.0;trueints[2][i]=170.0;}
      else if (i==6){ trueints[0][i]=214.0;trueints[1][i]=126.0;trueints[2][i]=44.0;}
      else if (i==7){ trueints[0][i]=80.0;trueints[1][i]=91.0;trueints[2][i]=166.0;}
      else if (i==8){ trueints[0][i]=193.0;trueints[1][i]=90.0;trueints[2][i]=99.0;}
      else if (i==9){ trueints[0][i]=94.0;trueints[1][i]=60.0;trueints[2][i]=108.0;}
      else if (i==10){ trueints[0][i]=157.0;trueints[1][i]=188.0;trueints[2][i]=64.0;}
      else if (i==11){ trueints[0][i]=224.0;trueints[1][i]=163.0;trueints[2][i]=46.0;}
      else if (i==12){ trueints[0][i]=56.0;trueints[1][i]=61.0;trueints[2][i]=150.0;}
      else if (i==13){ trueints[0][i]=70.0;trueints[1][i]=148.0;trueints[2][i]=73.0;}
      else if (i==14){ trueints[0][i]=175.0;trueints[1][i]=54.0;trueints[2][i]=60.0;}
      else if (i==15){ trueints[0][i]=231.0;trueints[1][i]=199.0;trueints[2][i]=31.0;}
      else if (i==16){ trueints[0][i]=187.0;trueints[1][i]=86.0;trueints[2][i]=149.0;}
      else if (i==17){ trueints[0][i]=8.0;trueints[1][i]=133.0;trueints[2][i]=161.0;}
      else if (i==18){ trueints[0][i]=243.0;trueints[1][i]=243.0;trueints[2][i]=242.0;}
      else if (i==19){ trueints[0][i]=200.0;trueints[1][i]=200.0;trueints[2][i]=200.0;}
      else if (i==20){ trueints[0][i]=160.0;trueints[1][i]=160.0;trueints[2][i]=160.0;}
      else if (i==21){ trueints[0][i]=122.0;trueints[1][i]=122.0;trueints[2][i]=121.0;}
      else if (i==22){ trueints[0][i]=85.0;trueints[1][i]=85.0;trueints[2][i]=85.0;}
      else { trueints[0][i]=52.0;trueints[1][i]=52.0;trueints[2][i]=52.0;}
    }
    else
    {
      trueints[0][i]=(float)((i/(color_res*color_res))%color_res)/(float)(color_res-1);
  	  trueints[1][i]=(float)((i/(color_res))%color_res)/(float)(color_res-1);
  	  trueints[2][i]=(float)((i)%color_res)/(float)(color_res-1);
    }

    //set saturation of measured color to be the true saturation
    trueints[0][i]/=255;
    imgmeans[0][i]/=255;
    imgmeans_correct[0][i]=imgmeans[0][i];
    if (max[0]>0.0001) imgmeans_correct[0][i]/=max[0];
    imgmeans_correct[0][i]=pow(imgmeans_correct[0][i],1.0/gamma[0]);
    trueints[1][i]/=255;
    imgmeans[1][i]/=255;
    imgmeans_correct[1][i]=imgmeans[1][i];
    if (max[1]>0.0001) imgmeans_correct[1][i]/=max[1];
    imgmeans_correct[1][i]=pow(imgmeans_correct[1][i],1.0/gamma[1]);
    trueints[2][i]/=255;
    imgmeans[2][i]/=255;
    imgmeans_correct[2][i]=imgmeans[2][i];
    if (max[2]>0.0001) imgmeans_correct[2][i]/=max[2];
    imgmeans_correct[2][i]=pow(imgmeans_correct[2][i],1.0/gamma[2]);

    rgb1[0]=trueints[0][i]*255;
    rgb1[1]=trueints[1][i]*255;
    rgb1[2]=trueints[2][i]*255;
    imguRGBtoHLS(rgb1,hls1);

    rgb2[0]=imgmeans_correct[0][i]*255;
    rgb2[1]=imgmeans_correct[1][i]*255;
    rgb2[2]=imgmeans_correct[2][i]*255;
    imguRGBtoHLS(rgb2,hls2);

    hls2[2]=hls1[2];

    imguHLStoRGB(hls2,rgb2);

    imgmeans_correct[0][i]=rgb2[0]/255;
    imgmeans_correct[1][i]=rgb2[1]/255;
    imgmeans_correct[2][i]=rgb2[2]/255;
  }

  sprintf(filename,"%scolor_%02d.txt",outPath,ProjectorIndex);
  writeColorCube(NbImages,imgmeans, imgmeans_correct, trueints, filename);

  LT_PRINTF("Saving mean and stddev images...");
  sprintf(filename,"%smean_%02d.pgm",outPath,ProjectorIndex);
  //lt_save_imginfo(filename,&mean);
  sprintf(filename,"%sstddev_%02d.pgm",outPath,ProjectorIndex);
  //lt_save_imginfo(filename,&stddev);
  LT_PRINTF("done\n");

  imguFree(&image);
  imguFree(&stddev);

  for(i=0;i<3;i++)
  {
     imguFree(&image_rgb[i]);
     free(imgmeans[i]);
     free(imgmeans_correct[i]);
     free(trueints[i]);
  }
  free(image_rgb);
  free(imgmeans);
  free(imgmeans_correct);
  free(trueints);

  LT_EXIT(0);
}



