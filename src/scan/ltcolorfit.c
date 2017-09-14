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
 * ltcolorfit.c  -  Finds the 3x3 transformation from the measured colors to the true colors
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <imgu/matrixmath.h>

#define STR_LEN 256

#define EPSILON 0.0001

void get3x3Transformation(int n,double **x,double **y,double **transformation)
{
  int i,j;
  matrix *A;
  matrix *res;
  matrix *b;

  A=NULL;
  res=NULL;
  b=NULL;

  matAllocate(&A,3*n,9);
  matAllocate(&b,3*n,1);
   
  for (i=0;i<n;i++)
  {
    matSet(A,3*i,0,x[0][i]);
	matSet(A,3*i,1,x[1][i]);
	matSet(A,3*i,2,x[2][i]);

	matSet(A,3*i+1,3,x[0][i]);
	matSet(A,3*i+1,4,x[1][i]);
	matSet(A,3*i+1,5,x[2][i]);

	matSet(A,3*i+2,6,x[0][i]);
	matSet(A,3*i+2,7,x[1][i]);
	matSet(A,3*i+2,8,x[2][i]);	
  }

  for (i=0;i<n;i++)
  {
	matSet(b,3*i,0,y[0][i]);
	matSet(b,3*i+1,0,y[1][i]);
	matSet(b,3*i+2,0,y[2][i]);
  }
	
  matSolveAb(&res,A,b);

  for (i=0;i<3;i++)
  {
    for (j=0;j<3;j++)
    {
      transformation[i][j]=res->values[i*3+j];
    }
  }

  matFree(&A);
  matFree(&b);
  matFree(&res);
}

int readColorCube(int *n,double ***projectedRGB, double ***correctedRGB,double ***trueRGB, char *filename)
{
  FILE *fd;
  int i;
  float temp;
  char buf[STR_LEN];

  if (filename==NULL) return -1;

  fd = fopen(filename, "r");
  
  if (fd==NULL)
  {
    LT_PRINTF("An error occured while opening '%s'\n",filename);
    LT_EXIT(0);
  }

  fscanf(fd,"%d\n",n);

  (*projectedRGB)=(double **)(malloc(sizeof(double *)*3));
  (*correctedRGB)=(double **)(malloc(sizeof(double *)*3));
  (*trueRGB)=(double **)(malloc(sizeof(double *)*3));

  for(i=0;i<3;i++)
  {
    (*projectedRGB)[i]=(double *)(malloc(sizeof(double)*(*n)));
    (*correctedRGB)[i]=(double *)(malloc(sizeof(double)*(*n)));
    (*trueRGB)[i]=(double *)(malloc(sizeof(double)*(*n)));
  }

  fgets(buf,STR_LEN,fd); //skip first line

  for (i=0;i<(*n);i++)
  {
    fscanf(fd,"%f\t",&temp);
    (*projectedRGB)[0][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*projectedRGB)[1][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*projectedRGB)[2][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*correctedRGB)[0][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*correctedRGB)[1][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*correctedRGB)[2][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*trueRGB)[0][i]=temp;
    fscanf(fd,"%f\t",&temp);
    (*trueRGB)[1][i]=temp;
    fscanf(fd,"%f\n",&temp);
    (*trueRGB)[2][i]=temp;
  }
  
  fclose(fd);

  return 0;
}

//expects 3x3 array
int writeTransformation(double **transformation, char *filename)
{
  FILE *fd;
  int i,j;

  if (filename==NULL || transformation==NULL) return -1;

  fd = fopen(filename, "w");
  
  if (fd==NULL)
  {
    LT_PRINTF("An error occured while opening '%s'\n",filename);
    LT_EXIT(0);
  }

  for (i=0;i<3;i++)
  {
    for (j=0;j<3;j++)
    {
      fprintf(fd,"%f\t",transformation[i][j]);
    }
    fprintf(fd,"\n");
  }

  fclose(fd);

  return 0;
}


int main (int argc,char *argv[])
{
  int i;
  int nbsamples1,nbsamples2;
  double **projectedRGB1,**projectedRGB2;
  double **correctedRGB1,**correctedRGB2;
  double **trueRGB1,**trueRGB2;
  double **transformation;
  char inFilename1[STR_LEN],inFilename2[STR_LEN];
  char inPath[STR_LEN];
  char outFilename[STR_LEN];
  char filename[STR_LEN];
  
  for(i=1;i<argc;i++) {
    if( strcmp("-ifname1",argv[i])==0 && i+1<argc ) {
	  strcpy(inFilename1,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ifname2",argv[i])==0 && i+1<argc ) {
	  strcpy(inFilename2,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ipath",argv[i])==0 && i+1<argc ) {
	  strcpy(inPath,argv[i+1]);
	  i++;continue;
    }
    if( strcmp("-ofname",argv[i])==0 && i+1<argc ) {
	  strcpy(outFilename,argv[i+1]);
	  i++;continue;
    }
  }

  strcpy(filename,inPath);
  strcat(filename,inFilename1);
  if (readColorCube(&nbsamples1, &projectedRGB1, &correctedRGB1, &trueRGB1, filename)) {LT_PRINTF("Error reading input file %s\n",filename); LT_EXIT(-1);}
  strcpy(filename,inPath);
  strcat(filename,inFilename2);
  if (readColorCube(&nbsamples2, &projectedRGB2, &correctedRGB2, &trueRGB2, filename)) {LT_PRINTF("Error reading input file %s\n",filename); LT_EXIT(-1);}

  if (nbsamples1!=nbsamples2) {LT_PRINTF("Error: number of samples is not the same"); LT_EXIT(-1);}

  transformation=(double **)(malloc(sizeof(double *)*3));    
  for(i=0;i<3;i++)
  {
    transformation[i]=(double *)(malloc(sizeof(double)*3));
  }

  //get3x3Transformation(nbsamples1,projectedRGB1,projectedRGB2,transformation);
  get3x3Transformation(nbsamples1,correctedRGB1,trueRGB1,transformation);

  strcpy(filename,inPath);
  strcat(filename,outFilename);

  writeTransformation(transformation, filename);

  LT_EXIT(0);
}
