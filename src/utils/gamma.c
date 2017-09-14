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

#include "gamma.h"

void read_gamma_and_color(char *path,int mindex,vector3 gamma,matrix3 color)
{
  char filename[255];
  FILE *f;
  char line[300];
  int i,k;
  double gx, gy,gz;

  sprintf(filename,"%s/gamma_%02d.txt",path,mindex);
  
  f = fopen(filename,"r");
  gx=DEFAULT_GAMMA_R;
  gy=DEFAULT_GAMMA_G;
  gz=DEFAULT_GAMMA_B;

  if (f==NULL)
  {
    printf("Warning: no gamma file\n");
    gamma[0]=gx;
    gamma[1]=gy;
    gamma[2]=gz;
  }
  else
  {
    while (1)
    {
      k = fscanf(f," %[^\n]",line);
      if (k!=1) break;
      if (strcmp(line,"gammaRGB")==0)
      {
        k = fscanf(f," %[^\n]",line);
        if (sscanf(line,"%lg",&gx)==1);
        k = fscanf(f," %[^\n]",line);
        if (sscanf(line,"%lg",&gy)==1);
        k = fscanf(f," %[^\n]",line);
        if (sscanf(line,"%lg",&gz)==1);
        gamma[0]=gx;
        gamma[1]=gy;
        gamma[2]=gz;
      } else if (strcmp(line,"maxRGB")==0)
        continue;
    }
    fclose(f);
  }

  for (i=0; i <9; i++)
  {
    color[i]=0.0f;
  }
  color[0]=1.0f;
  color[4]=1.0f;
  color[8]=1.0f;

  sprintf(filename,"%s/color_%02d.txt",path,mindex);
  
  f = fopen(filename,"r");
  if (f==NULL)
  {
    printf("Warning: no color file\n");
  }
  else
  {
    while (1)
    {
      k = fscanf(f," %[^\n]",line);
      if (k!=1) break;
      if (sscanf(line,"%lg\t%lg\t%lg",&color[0],&color[1],&color[2])==1);

      k = fscanf(f," %[^\n]",line);
      if (sscanf(line,"%lg\t%lg\t%lg",&color[3],&color[4],&color[5])==1);

      k = fscanf(f," %[^\n]",line);
      if (sscanf(line,"%lg\t%lg\t%lg",&color[6],&color[7],&color[8])==1);
    }

    fclose(f);
  }

  printf(" gamma %f %f %f\n",gamma[0],gamma[1],gamma[2]);
  printf(" color %f %f %f %f %f %f %f %f %f\n",color[0],color[1],color[2],color[3],color[4],color[5],color[6],color[7],color[8]);
}

//expects arrays of size 3
int read_gamma_and_max(double *gamma,double *max, char *filename)
{
  FILE *fd;
  char temp[255];
  float in[3];

  if (filename==NULL || gamma==NULL || max==NULL) return -1;

  fd = fopen(filename, "r");
  
  if (fd==NULL)
  {
    LT_PRINTF("An error occured while opening '%s'\n",filename);
    LT_EXIT(0);
  }

  fgets(temp,255,fd);
  fscanf(fd,"%f\n%f\n%f\n",&in[0],&in[1],&in[2]);
  gamma[0]=in[0];
  gamma[1]=in[1];
  gamma[2]=in[2];
  fgets(temp,255,fd);
  fscanf(fd,"%f\n%f\n%f\n",&in[0],&in[1],&in[2]);
  max[0]=in[0];
  max[1]=in[1];
  max[2]=in[2];

  fclose(fd);

  return 0;
}

//expects arrays of size 3
int write_gamma_and_max(double *gamma,double *max, char *filename)
{
  FILE *fd;
  int i;

  if (filename==NULL || gamma==NULL || max==NULL) return -1;

  fd = fopen(filename, "w");
  
  if (fd==NULL)
  {
    LT_PRINTF("An error occured while opening '%s'\n",filename);
    LT_EXIT(0);
  }
  fprintf(fd,"gammaRGB\n");
  for (i=0;i<3;i++)
  {
    fprintf(fd,"%f\n",gamma[i]);
  }
  fprintf(fd,"maxRGB\n");
  for (i=0;i<3;i++)
  {
    fprintf(fd,"%f\n",max[i]);
  }

  fclose(fd);

  return 0;
}



