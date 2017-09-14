/*
 *  ltdistanceblend.c
 *
 *  Created by yulia on 7/07/09.
 *
 *	Blends any number of Iareas together.
 *  Two modes of operation:
 *  - full blending - does not take into account information about location and direction
 *					  of the Iareas and intersections. Distance transform is computed for 
 *					  each image and then normalized to get sum of the projector intensities
 *					  in the intersection area equal to 1.
 *	- directional blending - best works for intersections of 2 projectors but can be applied
 *					  to more general cases as well. When computing, masks information about 
 *					  the boundary is taken into account, which allows for creating directional
 *					  gradients.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <imgu/imgu.h>
#include "blend.h"

#define STR_LEN 256
#define EPSILON 0.0001
#define MAX_NB_PROJS 8

#define BLEND_INFINITY 1E20

//EDT copyrights
//Distance Transforms of Sampled Functions
//Pedro F. Felzenszwalb and Daniel P. Huttenlocher
//Cornell Computing and Information Science TR2004-1963 

//compute euclidean distance transform 
void EDT(float *f, float *result, int size)
{
    int k, q;
    float s;
    int *v;
    float *z;

    v=(int *)(malloc(sizeof(int)*size));
    z=(float *)(malloc(sizeof(float)*(size+1)));

    k = 0;
    v[0] = 0;
    z[0] = -BLEND_INFINITY;
    z[1] = BLEND_INFINITY;
    for (q = 1; q < size; q++)
    {
        s  = ((f[q]+q*q)-(f[v[k]]+v[k]*v[k]))/(2*q-2*v[k]);
        while(s <= z[k])
        {
            k--;
            s = ((f[q]+q*q) - (f[v[k]]+v[k]*v[k]))/(2*q-2*v[k]);
        }
        k++;
        v[k] = q;
        z[k] = s;
        z[k+1] = BLEND_INFINITY;
    }

    k = 0;
    for (q = 0; q < size; q++)
    {
        while(z[k+1] < q) k++;
        result[q] = (q-v[k])*(q-v[k]) + f[v[k]];		
    }

    free(z);
    free(v);
}

//compute distance transform for each row of the image
//and save it in the array
void compute_partial_transform_row(float **image,int xsize,int ysize)
{
    int i, j;
    float *copy;
    copy=(float *)(malloc(sizeof(float)*xsize));
    for (i = 0; i < ysize; i++)
    {
        for (j = 0; j < xsize; j++)
        {
            copy[j] = image[i][j];
        }	

        EDT(copy, image[i], xsize);
    }

    free(copy);
}

//compute distance transform for each column of the image
//and save it in the array
void compute_partial_transform_column(float **image,int xsize,int ysize)
{
    int i, j;
    float *copy,*column;

    copy=(float *)(malloc(sizeof(float)*ysize));
    column=(float *)(malloc(sizeof(float)*ysize));
    for (i = 0; i < xsize; i++)
    {
        for (j = 0; j < ysize; j++)
        {
            copy[j] = image[j][i];
        }	
        EDT(copy, column, ysize);

        //copy column back to temp image array
        for (j = 0; j < ysize; j++)
        {
            image[j][i] = column[j];
        }
    }

    free(copy);
    free(column);
}

int imguEDT(imgu **I)
{
  int i,j;
  float **temp;
  double maxdist;

  if (I==NULL || (*I)==NULL || (*I)->data==NULL) return -1;

  maxdist=(*I)->xs*(*I)->xs+(*I)->ys*(*I)->ys;

  temp = (float **)(malloc(sizeof(float *)*(*I)->ys));
  for (i=0;i<(*I)->ys;i++)  
  { 
    temp[i] = (float *)(malloc(sizeof(float)*(*I)->xs));
    for (j=0;j<(*I)->xs;j++)  
    { 
      temp[i][j]=PIXEL((*I),j,i,0);
      //non zero pixels are maximum distance squared
      if (temp[i][j]!=0) temp[i][j]=(float)(maxdist);
    }
  }

  compute_partial_transform_column(temp,(*I)->xs,(*I)->ys);	
  compute_partial_transform_row(temp,(*I)->xs,(*I)->ys);	

  imguAllocateComplex(I,(*I)->xs,(*I)->ys,(*I)->cs);
  for (i=0;i<(*I)->ys;i++)  
  { 
    for (j=0;j<(*I)->xs;j++)  
    { 
      COMPLEX((*I),j,i,0,0)=sqrt(temp[i][j]);
    }
  }

  for (i=0;i<(*I)->ys;i++) free(temp[i]);
  free(temp);

  return 0;
}

int main (int argc, char *argv[])
{
    int i,j,k,n,m,index;
    int xsize, ysize, NbProjs;
    char Path[STR_LEN];
    char inFilename[STR_LEN];
    char outFilename[STR_LEN];
    char filename[STR_LEN];
    char maxFilename[STR_LEN];
    char tempfilename[STR_LEN];
    char line[STR_LEN];
    imgu **Iareas,**Iblends, **Iedges;
    imgu *mapall,*maxall;
    double sum;
    imgu *Isum;
    vector3 max, gamma;
    float d, dist;
    int istart=0;
    int maxNumIntersectingProjs=0;
    int directional = 0;

    mapall=NULL;
    maxall=NULL;

    for(i=1;i<argc;i++) {
        if( strcmp("-path",argv[i])==0 && i+1<argc ) {
            strcpy(Path,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-ifname",argv[i])==0 && i+1<argc ) {
            strcpy(inFilename,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-ofname",argv[i])==0 && i+1<argc ) {
            strcpy(outFilename,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-nbprojs",argv[i])==0 && i+1<argc ) {
            NbProjs=atoi(argv[i+1]);
            i++;continue;
        }
        if( strcmp("-start",argv[i])==0 && i+1<argc ) {
            istart=atoi(argv[i+1]);
            i++;continue;
        }
        if( strcmp("-fmax",argv[i])==0 && i+1<argc ) {
            strcpy(maxFilename,argv[i+1]);
            i++;continue;
        }
        //option for directional blending
        //is available for blending no more than 2 projectors
        //in the intersection area
        if( strcmp("-directional",argv[i])==0) {
            directional = 1;
            i++;continue;
        }
    }

    if (NbProjs<1) { LT_PRINTF("Number of projectors should be bigger than 0\n"); LT_EXIT(-1); }

    float gamma_values[NbProjs];
    float gamma_max;
    int count[NbProjs];

    //allocate space for Iareas
    Iareas = (imgu **)(malloc(sizeof(imgu *) *NbProjs));
    Iedges = (imgu **)(malloc(sizeof(imgu *)*NbProjs)); 
    Iblends = (imgu **)(malloc(sizeof(imgu *)*NbProjs));

    //load the xyvalid Iareas
    for (i=0; i<NbProjs; i++) 
    {
        Iareas[i] = NULL;
        sprintf(tempfilename, inFilename, i);
        strcpy(filename, Path);
        strcat(filename, tempfilename);
        k = imguLoad(&Iareas[i], filename, LOAD_AS_IS);
        //imguScale(&Iareas[i],Iareas[i],0.25,0.25);
        if (k) {LT_PRINTF("Unable to load image %s\n", filename); LT_EXIT(-1); }
        Iblends[i]=NULL;

        sprintf(tempfilename,maxFilename,istart+i);
        strcpy(filename,Path);
        strcat(filename,tempfilename);

        //read maximum intensity of each projector
        read_gamma_and_max(gamma,max,filename);
        gamma_values[i] = gamma[0]; //use first value only (red...pattern were grayscale anyways)
        if (i==0 || gamma_max<gamma_values[i]) gamma_max=gamma_values[i];
    }

    xsize = Iareas[0]->xs; //get dimensions of Iareas
    ysize = Iareas[0]->ys;

    for (i=0;i<NbProjs;i++)
    { 
      index=0;
      for(j=0;j<ysize;j++)
      {
        for(k=0;k<xsize;k++) 
        {
          if (Iareas[i]->data[index]!=0) 
          {
            Iareas[i]->data[index]=(pix_t)(gamma_values[i]/gamma_max*BLEND_MAXVAL);
          }
          index++;
        }
      }
      sprintf(filename,"%s/blend_mask_%02d.png",Path,i);
      imguSave(Iareas[i],filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    }

    computeMaps(Iareas,NbProjs,&mapall,&maxall);

    sprintf(filename,"%s/blend_map_all.png",Path);
    imguSave(mapall,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    sprintf(filename,"%s/blend_max_all.png",Path);
    imguSave(maxall,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);

    printf("Preprocessing...");

    //////////////////////////////////////////////////
    //allocating space for Iareas, temp arrays etc.

    Isum = NULL;
    imguAllocate(&Isum, xsize, ysize, 1); 

    printf("xsize %i ysize %i\n", xsize, ysize);

    for (i = 0; i < NbProjs; i++)
    {
        //allocate space for alpha masks
        Iblends[i]=NULL;
        imguAllocate(&Iblends[i], xsize, ysize, 1);
        Iedges[i]=NULL;
        imguCopy(&Iedges[i],Iareas[i]);
        imguCannyEdgeDetection(&Iedges[i],Iedges[i],0.0);
        if (directional)
        {
          //remove some edges here...
        }
        for (j=0; j < xsize*ysize; j++) 
        {
          //create binary image with 0 for edge, 1 otherwise
          if (Iedges[i]->data[j]!=0) Iedges[i]->data[j]=0;
          else Iedges[i]->data[j]=1;
        }
        imguEDT(&Iedges[i]);
        imguConvertFromComplexComponent(&Iedges[i],Iedges[i],0,COMPLEX_RESCALE);
        sprintf(filename, "%s/edges_%02d.png",Path,i);
        imguSave(Iedges[i], filename, FAST_COMPRESSION, SAVE_8_BITS_HIGH);
    }


    index = 0;
    for (i=0; i < ysize; i++) 
    {
      for (j=0; j < xsize; j++) 
      {
        sum=0;
        for (k=0; k < NbProjs; k++) 
        {
          if (Iareas[k]->data[index]) 
          {
            sum+=Iedges[k]->complex[index][0];
          } 
        }
        for (k=0; k < NbProjs; k++) 
        {
          if (Iareas[k]->data[index] && fabs(sum)>1e-8)
          {
            Iblends[k]->data[index]=Iedges[k]->complex[index][0]*IMGU_MAXVAL/sum;
            if (Iblends[k]->data[index]==0) Iblends[k]->data[index]++; //convertion for cam2proj: 0 means no blending, it doesn't mean blending with intensity 0. Thus 0->1
          }
          else Iblends[k]->data[index]=0;
        }        
        index++;
      } 
    }								

    imguAllocate(&Isum,Iareas[0]->xs,Iareas[0]->ys,1);
    //set Isum as sum of blendproj images
    index=0;
    for (i=0;i<Isum->ys;i++)
    {
      for (j=0;j<Isum->xs;j++)
      {
        sum=0;
        for (k=0;k<NbProjs;k++)
        {
          sum+=Iblends[k]->data[index];		  
        }
        if (sum>BLEND_MAXVAL) sum=(double)(BLEND_MAXVAL);
        Isum->data[index]=(pix_t)(sum);
        index++;
      }
    }

    sprintf(filename,"%s/blend_smoothmaxsum.png",Path);
    imguSave(Isum,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);

    for (i=0; i<NbProjs; i++) 
    {
        sprintf(tempfilename, outFilename, i);
        strcpy(filename, Path);
        strcat(filename, tempfilename);
        LT_PRINTF("%s\n", filename);
        imguSave(Iblends[i], filename, FAST_COMPRESSION, SAVE_16_BITS);

        imguFree(&Iblends[i]);
        imguFree(&Iareas[i]);
        imguFree(&Iedges[i]);
    }

    //free memory
    imguFree(&Isum);
    free(Iblends);
    free(Iareas);	
    free(Iedges);
    LT_EXIT(0);
}


