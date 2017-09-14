#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <imgu/imgu.h>
#include <math.h>

#define STR_LEN 255

//#define IN_NAME "/usr/local/share/imgu/imapXY_%02d_crop.lut.png"
//#define OUT_NAME "out_%02d"

//#define IN_NAME "/home/roys/imapXY_%02d_crop.lut.png"
//#define OUT_NAME "/home/roys/imapXY_%02d_crop"


typedef struct {
	double umin,umax;
	double vmin,vmax;
    double umean,udev;
    double vmean,vdev;
} zone;

//
// trouve la zone uv couverte par cette image LUT
//
int find_covered_zone(imgu *I,zone *z)
{
int y,x;
double umin,umax,vmin,vmax;
	if (I==NULL || I->complex==NULL) return -1;

	umin=vmin=9999.0;
	umax=vmax=-9999.0;

	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		if( COMPLEX(I,x,y,0,0) < 0 ) continue; // empty space
		if( COMPLEX(I,x,y,0,0) < umin ) umin=COMPLEX(I,x,y,0,0);
		if( COMPLEX(I,x,y,0,0) > umax ) umax=COMPLEX(I,x,y,0,0);
		if( COMPLEX(I,x,y,0,1) < vmin ) vmin=COMPLEX(I,x,y,0,1);
		if( COMPLEX(I,x,y,0,1) > vmax ) vmax=COMPLEX(I,x,y,0,1);
	}

	z->umin=umin;
	z->umax=umax;
	z->vmin=vmin;
	z->vmax=vmax;

	LT_PRINTF("zone is (%f...%f, %f...%f)\n",umin,umax,vmin,vmax);
	return(0);
}

//
// get a pixels/uv matrix out of the LUT
//
// nb : nb image
// img : this image number
// Zn : list of zones, we should intersect one of the zones, execpt zone[img]
//
int extract_uv(imgu *I,matrix **pixels,matrix **uv,int step,int nb,int img,zone *Zn)
{
  int i,j,k;
  int nb_points;
  double u,v;

  if (I==NULL || I->complex==NULL) return -1;

  nb_points=0;
  for (i=0;i<I->ys;i+=step)
  {
    for (j=0;j<I->xs;j+=step)
    { 
        u=COMPLEX(I,j,i,0,0);
        v=COMPLEX(I,j,i,0,1);
		if( u<0 || v<0 ) continue;
		if( Zn ) {
			// check if we intersect another image
			for(k=0;k<nb;k++) {
				if( k==img ) continue; // our own image.
				if( Zn[k].umin<=u && u<=Zn[k].umax 
				 && Zn[k].vmin<=v && v<=Zn[k].vmax ) break; // we have a winner!
			}
			if( k==nb ) continue; // no intersection. :-(
		}
		nb_points++;
    }
  }

  //LT_PRINTF("nbpoints=%d\n",nb_points);

  matAllocate(pixels,nb_points,2);
  matAllocate(uv,nb_points,2);

  int nn=0;
  for (i=0;i<I->ys;i+=step)
  {
    for (j=0;j<I->xs;j+=step)
    { 
        u=COMPLEX(I,j,i,0,0);
        v=COMPLEX(I,j,i,0,1);
		if( u<0 || v<0 ) continue;
		if( Zn ) {
			// check if we intersect another image
			for(k=0;k<nb;k++) {
				if( k==img ) continue; // our own image.
				if( Zn[k].umin<=u && u<=Zn[k].umax 
				 && Zn[k].vmin<=v && v<=Zn[k].vmax ) break; // we have a winner!
			}
			if( k==nb ) continue; // no intersection. :-(
		}
        matSet(*pixels,nn,0,(double)(j));
        matSet(*pixels,nn,1,(double)(i));
        matSet(*uv,nn,0,u);
        matSet(*uv,nn,1,v);
        nn++;
     }
  }

  return 0; 
}

int compute_uv(imgu *I,matrix *homography,imgu **Idiff)
{
  int i,j,k;
  vector3 pixel,uv;

  if (I==NULL || I->complex==NULL) return -1;

  imguAllocateComplex(Idiff,I->xs,I->ys,I->cs);
  imguClearComplex(*Idiff,0);
  imguClearComplex(*Idiff,1);

  for (i=0;i<I->ys;i++)
  {
    for (j=0;j<I->xs;j++)
    { 
      if (I->complex[i*I->xs+j][0]>=0)
      {
        pixel[0]=(double)(j);
        pixel[1]=(double)(i);
        pixel[2]=1.0;
        mat3MultiplyVector(homography->values,pixel,uv);
		vect3Homogenize2D(uv);
        if (uv[0]<0 || uv[1]<0 || uv[1]>1.0  ||uv[1]>1.0)
        {
          COMPLEX((*Idiff),j,i,0,0)=0.0;
          COMPLEX((*Idiff),j,i,0,1)=0.0;
          COMPLEX(I,j,i,0,0)=-1.0;
          COMPLEX(I,j,i,0,1)=-1.0;
        } 
        else
        {
          COMPLEX((*Idiff),j,i,0,0)=COMPLEX(I,j,i,0,0)-uv[0];
          COMPLEX((*Idiff),j,i,0,1)=COMPLEX(I,j,i,0,1)-uv[1];
          COMPLEX(I,j,i,0,0)=uv[0];
          COMPLEX(I,j,i,0,1)=uv[1];
        }
      }
    }
  }

  return 0; 
}

int compute_uv_from_patches(imgu *I,matrix **homography,int nb_patches_x,int nb_patches_y,double psize_x,double psize_y,imgu **Idiff)
{
  int i,j,k,l,index;
  vector3 pixel,pixeltemp;
  vector3 uv,uvtemp;
  zone *patches_area; //area of patch in I, more like xmin,ymin,xmax,ymax
  double weight,total_weight;
  int nb_patches;

  if (I==NULL || I->complex==NULL) return -1;

  nb_patches=nb_patches_x*nb_patches_y;
  patches_area=(zone *)(malloc(sizeof(zone)*nb_patches));

  index=0;
  for (i=0;i<nb_patches_y;i++)
  {
    for (j=0;j<nb_patches_x;j++)
    {
      patches_area[index].umin=j*psize_x/2.0;
      patches_area[index].vmin=i*psize_y/2.0;
      patches_area[index].umax=patches_area[index].umin+psize_x;
      patches_area[index].vmax=patches_area[index].vmin+psize_y;
      patches_area[index].umean=(patches_area[index].umax+patches_area[index].umin)/2.0;
      patches_area[index].vmean=(patches_area[index].vmax+patches_area[index].vmin)/2.0;
      patches_area[index].udev=(patches_area[index].umax-patches_area[index].umin)/6.0;
      patches_area[index].vdev=(patches_area[index].vmax-patches_area[index].vmin)/6.0;

      //LT_PRINTF("Patch areas: (%f,%f)->(%f,%f)\n",patches_area[index].umin,patches_area[index].vmin,patches_area[index].umax,patches_area[index].vmax);

      index++;
    }
  }

  imguAllocateComplex(Idiff,I->xs,I->ys,I->cs);
  imguClearComplex(*Idiff,0);
  imguClearComplex(*Idiff,1);

  for (i=0;i<I->ys;i++)
  {
    LT_PRINTF("Progress: %0.02f%%\r",100.0*i/(I->ys-1));
    for (j=0;j<I->xs;j++)
    { 
      pixel[0]=(double)(j);
      pixel[1]=(double)(i);
      pixel[2]=1.0;
      uv[0]=0;
      uv[1]=0;
      uv[2]=0;
      total_weight=0.0;
      for (k=0;k<nb_patches;k++)
      {
        if (homography[k]!=NULL && 
            pixel[0]>=patches_area[k].umin &&
            pixel[0]<=patches_area[k].umax &&
            pixel[1]>=patches_area[k].vmin &&
            pixel[1]<=patches_area[k].vmax)
        {
          pixeltemp[0]=pixel[0]-patches_area[k].umin;
          pixeltemp[1]=pixel[1]-patches_area[k].vmin;
          pixeltemp[2]=pixel[2];
          mat3MultiplyVector(homography[k]->values,pixeltemp,uvtemp);
          vect3Homogenize2D(uvtemp);
          //weight is 2d gaussian around center of patch
          weight=exp(-0.5*(j-patches_area[k].umean)*(j-patches_area[k].umean)/(patches_area[k].udev*patches_area[k].udev));
          weight*=exp(-0.5*(i-patches_area[k].vmean)*(i-patches_area[k].vmean)/(patches_area[k].vdev*patches_area[k].vdev));
/*if (i==1060 && j==596)
{
  printf("%d %f %f %f %f\n",k,patches_area[k].umin,patches_area[k].umax,patches_area[k].vmin,patches_area[k].vmax);
  printf("%d %f %f %f\n",k,uvtemp[0],uvtemp[1],uvtemp[2]);
}*/
          if (uvtemp[0]>=0 && uvtemp[1]>=0 && uvtemp[0]<=1.0 && uvtemp[1]<=1.0)  
          {
            total_weight+=weight;
            uv[0]+=weight*uvtemp[0];
            uv[1]+=weight*uvtemp[1];
            uv[2]+=uvtemp[2];
          }
        }
      }
      if (uv[2]<0.5)
      {
        //fprintf(stderr,"Warning: no patches correspond to pixel (%d,%d).\n",j,i);
      }
      else
      {
        //vect3Homogenize2D(uv); //average out all uvs
        uv[0]/=total_weight;
        uv[1]/=total_weight;
//if (i==1060 && j==596) printf("%f %f\n",uv[0],uv[1]);
        if (uv[0]<0 || uv[1]<0 || uv[0]>1.0 || uv[1]>1.0)
        {
          COMPLEX((*Idiff),j,i,0,0)=0.0;
          COMPLEX((*Idiff),j,i,0,1)=0.0;
          COMPLEX(I,j,i,0,0)=-1.0;
          COMPLEX(I,j,i,0,1)=-1.0;
        } 
        else
        {
          COMPLEX((*Idiff),j,i,0,0)=COMPLEX(I,j,i,0,0)-uv[0];
          COMPLEX((*Idiff),j,i,0,1)=COMPLEX(I,j,i,0,1)-uv[1];
          COMPLEX(I,j,i,0,0)=uv[0];
          COMPLEX(I,j,i,0,1)=uv[1];
        }
      }
    }
  }

  LT_PRINTF("\n");

  free(patches_area);

  return 0; 
}

int uv_crop(imgu *I,int camxs,int camys)
{
    int x,y,index;
    double u,v;
    double range;
    vector3 uv_cam;

    if (I==NULL || I->complex==NULL) return -1;

    for(y=0;y<I->ys;y++) 
    {
        for(x=0;x<I->xs;x++) 
        {
            /*! coord texture camera -> pixel camera*/
            u=I->complex[y*I->xs+x][0]*camxs;
            v=I->complex[y*I->xs+x][1]*camys;
            if (uvCrop(u,v,uv_cam) || uv_cam[0]<0.0 || uv_cam[1]<0.0)
            {
              I->complex[y*I->xs+x][0]=-1.0;
              I->complex[y*I->xs+x][1]=-1.0;
            }
        }
    }
}

//
// process all images at the same time. Adjust for overlap
//
int execute_all(char *inFilePattern,char *outFilePattern,int n)
{
imgu *I[n];
imgu *I_cmap[n];
zone Zn[n]; // uv coverage of each image
imgu *Idiff[n];
imgu *Idiff_cmap[n];
matrix *pixels,*uv;
matrix *homography,*herrors;
int i;
char filename[STR_LEN];

	for(i=0;i<n;i++) {
		sprintf(filename,inFilePattern,i);
		I_cmap[i]=NULL;
		LT_PRINTF("loading %s\n",filename);
		if (imguLoad(I_cmap+i,filename,LOAD_AS_IS)) { LT_PRINTF("Error loading input file %d.\n",i);return(-1);}

		I[i]=NULL;
		imguConvertToComplexUV(I+i, I_cmap[i]);

		find_covered_zone(I[i],Zn+i);
	}

	//
	// now solve but only use pixels in an overlap with some other image
	//

	pixels=NULL;
	uv=NULL;
	homography=NULL;
	herrors=NULL;

	for(i=0;i<n;i++) {
		extract_uv(I[i],&pixels,&uv,8,n,i,Zn);
		//extract_uv(I[i],&pixels,&uv,32,0,0,NULL);
 
		//matPrint(pixels);
		//matPrint(uv);

		matAllocate(&herrors,pixels->cs,1);
		matHomographySolve(pixels,uv,&homography);

		matPrint(homography);
		//matPrint(herrors);

		matHomographyError(pixels,uv,homography,herrors->values);

		Idiff[i]=NULL;
		compute_uv(I[i],homography,Idiff+i);

		imguConvertFromComplexUV(I_cmap+i,I[i],COMPLEX_RESCALE);
		sprintf(filename,outFilePattern,i);
		imguSave(I_cmap[i],filename,1,SAVE_AS_IS);

		Idiff_cmap[i]=NULL;
		imguConvertFromComplexUV(Idiff_cmap+i, Idiff[i],COMPLEX_RESCALE);
		sprintf(filename,"diff_%02d.png",i);
		imguSave(Idiff_cmap[i],filename,1,SAVE_AS_IS);
	}

	for(i=0;i<n;i++) 
    {
      imguFree(&I[i]);
      imguFree(&I_cmap[i]);
      imguFree(&Idiff[i]);
      imguFree(&Idiff_cmap[i]);
    }

	return(0);
}



int main(int argc,char *argv[])
{
  int i,j,index,nbpoints;
  imgu *I_cmap,*I,*Idiff,*Idiff_cmap;
  imgu *Ipatch;
  char inFilePattern[STR_LEN];
  char outFilePattern[STR_LEN];
  char inFilename[STR_LEN];
  char outFilename[STR_LEN];
  char filename[STR_LEN];
  char uvFilename[STR_LEN];
  char uvimgFilename[STR_LEN];
  int n;
  imgu *camImg;
  char camFilename[STR_LEN];
  int doAll; // should we do the 0..n-1 projectors?
  int nb_patches_x=8;
  int nb_patches_y=8;

  I=NULL;
  Ipatch=NULL;
  Idiff=NULL;
  I_cmap=NULL;
  Idiff_cmap=NULL;

  doAll=0;
  n=4; // nb d'images

  camImg=NULL;
	
  inFilePattern[0]='\0';
  outFilePattern[0]='\0';
  camFilename[0]='\0';
  uvFilename[0]='\0';
  uvimgFilename[0]='\0';
  for(i=1;i<argc;i++) {
      if( strcmp("-i",argv[i])==0 ) {
          strcpy(inFilePattern,argv[i+1]);
          n=-1;
          i++;
          continue;
      }
      if( strcmp("-n",argv[i])==0 ) {
		  n=atoi(argv[i+1]);
          i++;
          continue;
      }
      if( strcmp("-o",argv[i])==0 ) {
          strcpy(outFilePattern,argv[i+1]);
          i++;
          continue;
      }
      if( strcmp("-all",argv[i])==0 ) {
          doAll=1;
	  }
      if( strcmp("-camfile",argv[i])==0 && i+1<argc ) {
          strcpy(camFilename,argv[i+1]);
          i++;
          continue;
      }
      if( strcmp("-uv",argv[i])==0 && i+1<argc ) {
          strcpy(uvFilename,argv[i+1]);
          i++;continue;
      }
      if( strcmp("-uvimg",argv[i])==0 && i+1<argc ) {
          strcpy(uvimgFilename,argv[i+1]);
          i++;continue;
      }
      if( strcmp("-nb_patches_x",argv[i])==0 && i+1<argc ) {
          nb_patches_x=atoi(argv[i+1]);
          i++;continue;
      }
      if( strcmp("-nb_patches_y",argv[i])==0 && i+1<argc ) {
          nb_patches_y=atoi(argv[i+1]);
          i++;continue;
      }
  }

  if (inFilePattern[0]=='\0' || outFilePattern[0]=='\0') {fprintf(stderr,"Syntax: \n");LT_EXIT(0);}

  if( doAll ) {
    execute_all(inFilePattern,outFilePattern,n);
    LT_EXIT(0);
  }

  if( n>=0 ) {
    sprintf(inFilename,inFilePattern,n);
    sprintf(outFilename,outFilePattern,n);
  }

  if (imguLoad(&camImg,camFilename,LOAD_AS_IS)) {fprintf(stderr,"Error loading camera file.\n");LT_EXIT(-1);}

  if (imguLoad(&I_cmap,inFilename,LOAD_AS_IS)) {fprintf(stderr,"Error loading input file.\n");LT_EXIT(-1);}
  imguConvertToComplexUV(&I, I_cmap);

  ////////////// testing homography by patches

  double psize_x,psize_y;
  psize_x=2.0*I->xs/(nb_patches_x+1);
  psize_y=2.0*I->ys/(nb_patches_y+1);

  LT_PRINTF("Number of patches: %dx%d\n",nb_patches_x,nb_patches_y);
  LT_PRINTF("Patch size: %fx%f\n",psize_x,psize_y);

  matrix **pixels,**uv;
  matrix **homography,**herrors;

  pixels=(matrix **)(malloc(sizeof(matrix *)*nb_patches_x*nb_patches_y));
  uv=(matrix **)(malloc(sizeof(matrix *)*nb_patches_x*nb_patches_y));
  homography=(matrix **)(malloc(sizeof(matrix *)*nb_patches_x*nb_patches_y));
  herrors=(matrix **)(malloc(sizeof(matrix *)*nb_patches_x*nb_patches_y));
  for (i=0;i<nb_patches_x*nb_patches_y;i++)
  {
    pixels[i]=NULL;
    uv[i]=NULL;
    homography[i]=NULL;
    herrors[i]=NULL;
  }

  //strcpy(filename,outFilename);strcat(filename,"_original.png");
  //imguSave(I_cmap,filename,1,SAVE_AS_IS);

  index=0;
  int subsampling=8;
  for (i=0;i<nb_patches_y;i++)
  {
    for (j=0;j<nb_patches_x;j++)
    {
      //LT_PRINTF("Patch: (%d,%d)->(%d,%d)\n",(int)(j*psize_x/2),(int)(i*psize_y/2),(int)(j*psize_x/2+psize_x),(int)(i*psize_y/2+psize_y));

      imguExtractRectangle(&Ipatch,I,(int)(j*psize_x/2),(int)(i*psize_y/2),(int)(psize_x),(int)(psize_y));
      extract_uv(Ipatch,&pixels[index],&uv[index],subsampling,0,-1,NULL);

      if (pixels[index]->cs<(psize_x/subsampling)*(psize_y/subsampling)/10) //10% of patch has to be covered
      //if (pixels[index]->cs<4)
      {
        matFree(&pixels[index]);
        fprintf(stderr,"Warning: not enough samples in patch %d: (%d,%d)->(%d,%d).\n",index,(int)(j*psize_x/2),(int)(i*psize_y/2),(int)(j*psize_x/2+psize_x),(int)(i*psize_y/2+psize_y));
      }
      else
      {
        matAllocate(&herrors[index],pixels[index]->cs,1);
        matHomographySolve(pixels[index],uv[index],&homography[index]);
        matHomographyError(pixels[index],uv[index],homography[index],herrors[index]->values);
        //matPrint(homography[index]);
        //matPrint(herrors[index]);
      }

      index++;
    }
  }

  compute_uv_from_patches(I,homography,nb_patches_x,nb_patches_y,psize_x,psize_y,&Idiff);
  uvReadFile(uvFilename);
  uvLoadImg(uvimgFilename);
  uv_crop(I,camImg->xs,camImg->ys);

  imguConvertFromComplexUV(&I_cmap, I,COMPLEX_RESCALE);
  strcpy(filename,outFilename);
  //strcat(filename,"_corrected.png");
  imguSave(I_cmap,filename,1,SAVE_AS_IS);

  //imguConvertFromComplexUV(&Idiff_cmap, Idiff,COMPLEX_RESCALE);
  //strcpy(filename,outFilename);strcat(filename,"_diff.png");
  //imguSave(Idiff_cmap,filename,1,SAVE_AS_IS);

  imguFree(&I);
  imguFree(&Ipatch);
  imguFree(&Idiff);
  imguFree(&Idiff_cmap);
  imguFree(&I_cmap);

  for (i=0;i<nb_patches_x*nb_patches_y;i++)
  {
    matFree(&pixels[i]);
    matFree(&uv[i]);
    matFree(&homography[i]);
    matFree(&herrors[i]);
  }
  free(pixels);
  free(uv);
  free(homography);
  free(herrors);

  LT_EXIT(0);

  return 0;	
}



