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
 * ltdiffuseblend.c  -  From the maximum intensity measured for each projector,
 *                  make a smooth transition between each neighbor projectors
 *                  and blend the overlapping regions 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <imgu/imgu.h>
#include "blend.h"
#include "utils/triangle.h"

#define NB_SCALES 4

int imguConstrainedDiffusion(imgu **J,imgu **I,imgu **Imax,int nb_imgs,imgu *Imap,imgu *Isum, int ***lookup)
{
  int i,j,k,l;
  int index;
  int updated;
  int count;
  double val,sum;
  int change,maxchange;

  if (nb_imgs<1) return -1;
  if (J==I) return -1;
  for (i=0;i<nb_imgs;i++) if (I[i]==NULL|| Imax[i]==NULL) return -1;
  if (Imap==NULL) return -1;

  updated=0;
  maxchange=-1;

  for (l=0;l<nb_imgs;l++)
  {
    imguAllocate(&J[l],I[l]->xs,I[l]->ys,I[l]->cs);
  }

  //constrained diffusion first blurs an image, and then enforces different constraints (like the maximum blending intensity or a maximum on the sum)
  //ignore 1-pixel border for now
  for (i=1;i<I[0]->ys-1;i++)
  {
    for (j=1;j<I[0]->xs-1;j++)
    {
      if (Imap->data[i*I[0]->xs+j]==MAP_NO_PROJECTOR) continue;

      sum=0;
      for (l=0;l<nb_imgs;l++)
      {
        val=0;
        count=0;
        for (k=0;k<5;k++)
        {
          //diffusion filter is:
          // 0.0 0.2 0.0
          // 0.2 0.2 0.2
          // 0.0 0.2 0.0
          //however, the filter is adapted to avoid regions with no projector
          index=lookup[i][j][k];
          if (Imap->data[index]!=MAP_NO_PROJECTOR)
          {
            val+=I[l]->data[index];
            count++;
          }
        }
        if (count==0) val=0;
        else val/=count;

        //this should never happen...
        if (val>BLEND_MAXVAL) val=(double)(BLEND_MAXVAL);

        if (Isum==NULL) //this is for smoothing the sum (1st step)
        {
          if (Imap->data[index]==MAP_SINGLE_PROJECTOR && val!=Imax[l]->data[index]) val=(double)(Imax[l]->data[index]);
          //use this line instead of the above to allow decreasing the maximum intensity of a projector outside of intersections
          //if (Imap->data[index]==MAP_SINGLE_PROJECTOR && val>Imax[l]->data[index]) val=(double)(Imax[l]->data[index]);
        }
        else //this is for blending the projectors (2nd step)
        {
          if (val>Imax[l]->data[index]) val=(double)(Imax[l]->data[index]); //no blending intensity can be larger than the max of projector
        }
  
        J[l]->data[index]=(pix_t)(val);
        sum+=val;
      }
      //rescale the blending intensities s.t. sum is equal to Isum
      if (Isum!=NULL && sum!=0) //this is for blending the projectors (2nd step)
      {
        for (l=0;l<nb_imgs;l++)
        {
          J[l]->data[index]=(pix_t)(J[l]->data[index]/sum*Isum->data[index]);
        }
      }
      for (l=0;l<nb_imgs;l++)
      {
        change=abs(J[l]->data[index]-I[l]->data[index]);
        if (maxchange<change)
        {
          maxchange=change;
          //printf("change: %d (%d,%d): %d->%d\n",change,j,i,I[l]->data[index],J[l]->data[index]);
        }
      }
    }
  }

  //if (maxchange>0) updated=1;
  if (maxchange>257.0/8) updated=1;

  return updated;
}

//fill black pixels surrounded by at least one non-black pixel
int imgu_slim(imgu *I,imgu *mapall)
{
  int i,j,k,l;
  int index;
  imgu *Itemp;
  unsigned char black,no_proj;

  Itemp=NULL;
  imguCopy(&Itemp,I);

  for (i=2;i<I->ys-2;i++)
  {
    for (j=2;j<I->xs-2;j++)
    {
      black=0;
      no_proj=0;
      for (k=-2;k<=2;k++)
      {
        for (l=-2;l<=2;l++)
        {
          if (Itemp->data[(i+k)*I->xs+j+l]==0) black=1;
          if (mapall->data[(i+k)*I->xs+j+l]==MAP_NO_PROJECTOR) no_proj=1;
        }
      }
      index=i*I->xs+j;
      //if at least one pixel is black in neighborhood, and it is not near a non-overlapping border.
      if (black && !no_proj) I->data[index]=0;
    }
  }

  imguFree(&Itemp);

  return 0;
}

int main (int argc,char *argv[])
{
  int i,j,l,k,s,index;
  int x,y;
  int xmin,ymin,xmax,ymax;
  int istart;
  char Path[STR_LEN];
  char inFilename[STR_LEN];
  char uv_filename[STR_LEN];
  char uvimg_filename[STR_LEN];
  char inLutFilename[STR_LEN];
  char outFilename[STR_LEN];
  char filename[STR_LEN];
  char maxFilename[STR_LEN];
  char temp[STR_LEN];
  int xsize,ysize;
  int NbProjs;
  imgu **Iareas,**Iareas_scaled;
  imgu **Iareas_slim;
  imgu *mapall,*maxall;
  imgu *smoothmaxsum;
  imgu **Itemp,**Iblends;
  imgu *Iuv,*Iuv_cmap;
  double *avgmax;
  vector3 max,gamma;
  double maxmax,sum;
  int ***index_lookup;
  int count;
  double scale;

  istart=0;

  for(i=1;i<argc;i++) {
    if( strcmp("-path",argv[i])==0 && i+1<argc ) {
	  strcpy(Path,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ifname",argv[i])==0 && i+1<argc ) {
	  strcpy(inFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-lut",argv[i])==0 && i+1<argc ) {
	  strcpy(inLutFilename,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ofname",argv[i])==0 && i+1<argc ) {
	  strcpy(outFilename,argv[i+1]);
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
  }

  if (NbProjs<1) { LT_PRINTF("Number of projectors should be bigger than 0\n"); LT_EXIT(-1); }

/*
  if (NbProjs<2)
  {
    LT_PRINTF("Intersection regions should involve at least 2 projectors\n");
    LT_EXIT(-1);
  } 
*/

  uvReadFile(uv_filename);
  uvLoadImg(uvimg_filename);

  Iareas=(imgu **)(malloc(sizeof(imgu *)*NbProjs));
  for (i=0;i<NbProjs;i++) Iareas[i]=NULL;
  Iareas_scaled=(imgu **)(malloc(sizeof(imgu *)*NbProjs));
  for (i=0;i<NbProjs;i++) Iareas_scaled[i]=NULL;
  Iareas_slim=(imgu **)(malloc(sizeof(imgu *)*NbProjs));
  for (i=0;i<NbProjs;i++) Iareas_slim[i]=NULL;
  avgmax=(double *)(malloc(sizeof(double)*NbProjs));

  mapall=NULL;
  maxall=NULL;
  smoothmaxsum=NULL;
  Iblends=(imgu **)(malloc(sizeof(imgu *)*NbProjs));
  Itemp=(imgu **)(malloc(sizeof(imgu *)*NbProjs));
  for (i=0;i<NbProjs;i++)
  {
    Iblends[i]=NULL;
    Itemp[i]=NULL;
  }

  Iuv=NULL;
  Iuv_cmap=NULL;
  LT_PRINTF("Loading images...");
  //load xyvalid images
  for (i=0;i<NbProjs;i++)
  {
    //images are camera pixels covered by projectors
    sprintf(temp,inFilename,istart+i);
    strcpy(filename,Path);
    strcat(filename,temp);
    k=imguLoad(&Iareas[i],filename,LOAD_16_BITS);
    if( k ) { LT_PRINTF("Unable to load image %s\n",filename); LT_EXIT(-1); }

    //images are camera pixels covered by projectors
    imguClear(Iareas[i]);
    sprintf(temp,inLutFilename,istart+i);
    strcpy(filename,Path);
    strcat(filename,temp);
    k=imguLoad(&Iuv_cmap,filename,LOAD_AS_IS);
    if( k ) { LT_PRINTF("Unable to load uv image %s\n",filename); LT_EXIT(-1); }
    imguConvertToComplexUV(&Iuv,Iuv_cmap);
    double pixel[2];
    double puv[2];
    triangle t;
    for(y=1;y<Iuv->ys;y++) 
    {
        for(x=1;x<Iuv->xs;x++) 
        {
            for (j=0;j<2;j++)
            {
              t.x[0]=Iuv->complex[(y-1)*Iuv->xs+(x-1)][0]*(Iareas[i]->xs);
              t.y[0]=Iuv->complex[(y-1)*Iuv->xs+(x-1)][1]*(Iareas[i]->ys);
              if (t.x[0]<0) continue;
              xmin=(int)(t.x[0]);
              ymin=(int)(t.y[0]);
              xmax=(int)(t.x[0]);
              ymax=(int)(t.y[0]);
              if (j==0)
              {
                t.x[1]=Iuv->complex[(y-1)*Iuv->xs+(x)][0]*(Iareas[i]->xs);
                t.y[1]=Iuv->complex[(y-1)*Iuv->xs+(x)][1]*(Iareas[i]->ys);
              }
              else
              {
                t.x[1]=Iuv->complex[(y)*Iuv->xs+(x-1)][0]*(Iareas[i]->xs);
                t.y[1]=Iuv->complex[(y)*Iuv->xs+(x-1)][1]*(Iareas[i]->ys);
              }
              if (t.x[1]<0) continue;
              if (t.x[1]<xmin) xmin=(int)(t.x[1]);
              if (t.y[1]<ymin) ymin=(int)(t.y[1]);
              if (t.x[1]>xmax) xmax=(int)(t.x[1]);
              if (t.y[1]>ymax) ymax=(int)(t.y[1]);
              t.x[2]=Iuv->complex[(y)*Iuv->xs+(x)][0]*(Iareas[i]->xs);
              t.y[2]=Iuv->complex[(y)*Iuv->xs+(x)][1]*(Iareas[i]->ys);
              if (t.x[2]<0) continue;
              if (t.x[2]<xmin) xmin=(int)(t.x[2]);
              if (t.y[2]<ymin) ymin=(int)(t.y[2]);
              if (t.x[2]>xmax) xmax=(int)(t.x[2]);
              if (t.y[2]>ymax) ymax=(int)(t.y[2]);
              for (l=ymin;l<=ymax;l++)
              {
                for (k=xmin;k<=xmax;k++)
                {
                  pixel[0]=k;
                  pixel[1]=l;
                  if (triMapPixel(&t,pixel,puv)>=0) Iareas[i]->data[l*Iareas[i]->xs+k]=BLEND_MAXVAL;
                }
              }
            }
        }
    }
    uvCropImg(Iareas[i]);
    sprintf(temp,maxFilename,istart+i);
    strcpy(filename,Path);
    strcat(filename,temp);

    //read maximum intensity of each projector
    read_gamma_and_max(gamma,max,filename);

    avgmax[i]=max[1]; //use second value (green)
    if (i==0 || maxmax<avgmax[i]) maxmax=avgmax[i];
  }
  imguFree(&Iuv);
  imguFree(&Iuv_cmap);
  LT_PRINTF("done\n");

  xsize=Iareas[0]->xs;
  ysize=Iareas[0]->ys;

  //max images are projectors represented by their maximum intensity
  //TODO: we might want to use the intensities of a white projected pattern instead of a uniform maximum
  for (i=0;i<NbProjs;i++)
  {
    index=0;
    for(j=0;j<ysize;j++)
    {
      for(k=0;k<xsize;k++) 
      {
        if (Iareas[i]->data[index]!=0) Iareas[i]->data[index]=(pix_t)(avgmax[i]/maxmax*BLEND_MAXVAL);
        index++;
      }
    }
    //sprintf(filename,"%s/blend_mask_%02d.png",Path,istart+i);
    //imguSave(Iareas[i],filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
  }

  for (s=0;s<NB_SCALES;s++)
  {
    scale=1.0/pow(2.0,NB_SCALES-1-s);
    LT_PRINTF("Scale: %f\n",scale);
  
    for (i=0;i<NbProjs;i++)
    {
      imguScale(&Iareas_scaled[i],Iareas[i],scale,scale);
    }
    computeMaps(Iareas_scaled,NbProjs,&mapall,&maxall);
    for (i=0;i<NbProjs;i++)
    {
      imguCopy(&Iareas_slim[i],Iareas_scaled[i]);
      imgu_slim(Iareas_slim[i],mapall);
      sprintf(filename,"%s/blend_mask_%02d.png",Path,istart+i);
      imguSave(Iareas[i],filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    }
    computeMaps(Iareas_slim,NbProjs,&mapall,&maxall);

    sprintf(filename,"%s/blend_map_all_%d.png",Path,s);
    imguSave(mapall,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    //sprintf(filename,"%s/blend_max_all_%d.png",Path,s);
    //imguSave(maxall,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);   

    if (s==0) imguCopy(&smoothmaxsum,maxall);
    else
    {
      imguScale(&smoothmaxsum,smoothmaxsum,Iareas[0]->xs/(double)(smoothmaxsum->xs)*scale,Iareas[0]->ys/(double)(smoothmaxsum->ys)*scale); 
      for (i=0;i<smoothmaxsum->xs*smoothmaxsum->ys;i++)
      {
        if (mapall->data[i]==MAP_NO_PROJECTOR) smoothmaxsum->data[i]=0;
      }
    }

    xsize=Iareas_scaled[0]->xs;
    ysize=Iareas_scaled[0]->ys;

    index_lookup = (int ***)malloc(ysize * sizeof(int **)); 
    for (i = 0; i < ysize; i++)
    {
      index_lookup[i] = (int **)malloc(xsize*sizeof(int *));
      for (j = 0; j < xsize; j++)
      {
        index_lookup[i][j] = (int *)malloc(5*sizeof(int));
      }
    }
	
    for (i = 1; i < ysize-1; i++)
    {
      for (j = 1; j < xsize-1; j++)
      {	
        for (k=0;k<5;k++)
        {
	  	  //diffusion filter is:
		  // 0.0 0.2 0.0
		  // 0.2 0.2 0.2
		  // 0.0 0.2 0.0
				  
		  if (k==0) index_lookup[i][j][k]=(i-1)*xsize+j;
		  else if (k==1) index_lookup[i][j][k]=(i+1)*xsize+j;
		  else if (k==2) index_lookup[i][j][k]=i*xsize+j-1;
		  else if (k==3) index_lookup[i][j][k]=i*xsize+j+1;
		  else if (k==4) index_lookup[i][j][k]=i*xsize+j;
        }
      }
    }

    //diffuse smoothmaxsum to get smooth desired sum of projectors
    //is this step necessary? can we compute blend of images while enforcing a smooth sum at the same time?

    i=0;
    int updated=1;
    while(updated && i<2500)
    {
      if (i%100==0) 
      {
		LT_PRINTF("Smoothing maximum intensities: iteration %d\n",i);		
      }
	  if (i%2==0) updated=imguConstrainedDiffusion(Itemp,&smoothmaxsum,&maxall,1,mapall,NULL,index_lookup); 
      else updated=imguConstrainedDiffusion(&smoothmaxsum,Itemp,&maxall,1,mapall,NULL,index_lookup);
      i++;
    }

    //sprintf(filename,"%s/blend_smoothmax_%d.png",Path,s);
    //imguSave(smoothmaxsum,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);

    //initialize blending of projectors
    for (k=0;k<NbProjs;k++)
    {
      if (s==0) imguCopy(&Iblends[k],Iareas_scaled[k]);
      else
      {
        imguScale(&Iblends[k],Iblends[k],Iareas[0]->xs/(double)(Iblends[k]->xs)*scale,Iareas[0]->ys/(double)(Iblends[k]->ys)*scale); 
        for (i=0;i<Iblends[k]->xs*Iblends[k]->ys;i++)
        {
          if (mapall->data[i]==MAP_NO_PROJECTOR) Iblends[k]->data[i]=0;
        }
      }
    }

    //blend projectors
    i=0;
    updated=1;
    while(updated && i<2500)
    { 
      if (i%100==0) LT_PRINTF("Blending projectors: iteration %d\n",i);
      if (i%2==0) updated=imguConstrainedDiffusion(Itemp,Iblends,Iareas_slim,NbProjs,mapall,smoothmaxsum,index_lookup); 
      else updated=imguConstrainedDiffusion(Iblends,Itemp,Iareas_slim,NbProjs,mapall,smoothmaxsum,index_lookup);
      i++;
    }

    for (k=0;k<NbProjs;k++)
    {
      //sprintf(filename,"%s/blend_%02d_%d.png",Path,k,s);
      //imguSave(Iblends[k],filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);
    }

    for (i=0;i<ysize;i++) 
    {
      for (j=0;j<xsize;j++)
      {
        free(index_lookup[i][j]);
      }
    }
    for (i=0;i<ysize;i++) free(index_lookup[i]);
    free(index_lookup);
  } //end of scale loop

  //set smoothmaxsum as sum of blendproj images
  index=0;
  for (i=0;i<smoothmaxsum->ys;i++)
  {
    for (j=0;j<smoothmaxsum->xs;j++)
    {
      sum=0;
      for (k=0;k<NbProjs;k++)
      {
        sum+=Iblends[k]->data[index];		  
      }
      if (sum>BLEND_MAXVAL) sum=(double)(BLEND_MAXVAL);
      smoothmaxsum->data[index]=(pix_t)(sum);
      index++;
    }
  }

  sprintf(filename,"%s/blend_smoothmaxsum.png",Path);
  imguSave(smoothmaxsum,filename,FAST_COMPRESSION,SAVE_8_BITS_HIGH);

  for (k=0;k<NbProjs;k++)
  {
    imguMapRange(&Iblends[k],Iblends[k],0,IMGU_MAXVAL);
//    imguScale(&Iblends[k],Iblends[k],1.0/SCALE_FACTOR,1.0/SCALE_FACTOR);
    sprintf(temp,outFilename,istart+k);
    strcpy(filename,Path);
    strcat(filename,temp);
    for (i=0;i<Iblends[k]->ys;i++)
    {
      for (j=0;j<Iblends[k]->xs;j++)
      {
        index=i*Iblends[k]->xs+j;
        if (Iareas[k]->data[index]!=0)
        {
           if (Iblends[k]->data[index]==0) Iblends[k]->data[index]++; //convertion for cam2proj: 0 means no blending, it doesn't mean blending with intensity 0. Thus 0->1
        }    
      }
    }
    imguSave(Iblends[k],filename,1,SAVE_16_BITS);
  }

  imguFree(&mapall);
  imguFree(&maxall);
  imguFree(&smoothmaxsum);
  for (i=0;i<NbProjs;i++)
  {
    imguFree(&Iblends[i]);
    imguFree(&Itemp[i]);
    imguFree(&Iareas[i]);
    imguFree(&Iareas_scaled[i]);
    imguFree(&Iareas_slim[i]);
  }
  free(Iblends);
  free(Itemp);
  free(avgmax);
  free(Iareas);
  free(Iareas_scaled);
  free(Iareas_slim);

  LT_EXIT(0);
}



