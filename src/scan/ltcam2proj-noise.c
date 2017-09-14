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
 * ltcam2proj.c - Code for running the program 'ltcam2proj' from a shell.
 *                     This program is commonly used for structured light calibration.
 *                     Only grayscale format supported for images.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/noise.h"

#define NB_MUTEX 100 //divide Imatch in several mutexes based on row index
pthread_mutex_t match_mutex[NB_MUTEX];

typedef struct{
    int id;
    noise_match *match_vars;
    imgu *Iproj;
    imgu *Icam;
    imgu *Imatch_c2p;
    imgu *Imatch_p2c;
}match_thread_params;

void *match_thread(void *t)
{
   int i,j,index;
   int ii,jj;
   int mutex_id;
   noise_code match;

   match_thread_params *params=(match_thread_params *)(t);
   noise_match_execute(params->match_vars,params->Iproj,params->Icam);
   for (i=0;i<params->Icam->ys;i++)
   {
     for (j=0;j<params->Icam->xs;j++)
     {
        noise_match_get(params->match_vars,params->Iproj,params->Icam,j,i,&match);
        jj=match.x;
        ii=match.y;
        if (match.mx>=0)
        {
mutex_id=ii*NB_MUTEX/params->Icam->ys;
pthread_mutex_lock(&(match_mutex[mutex_id]));
          if (match.cost<PIXEL(params->Imatch_c2p,jj,ii,0))
          {
            PIXEL(params->Imatch_c2p,jj,ii,0)=(pix_t)(match.cost);
            COMPLEX(params->Imatch_c2p,jj,ii,0,0)=match.mx+0.5;
            COMPLEX(params->Imatch_c2p,jj,ii,0,1)=match.my+0.5;
          }
pthread_mutex_unlock(&(match_mutex[mutex_id]));
        }
      }
    }
    for (i=0;i<params->Iproj->ys;i++)
    {
      for (j=0;j<params->Iproj->xs;j++)
      {
        index=i*params->Iproj->xs+j;
        match.x=params->match_vars->pcodes[index].x;
        match.y=params->match_vars->pcodes[index].y;
        match.cost=params->match_vars->pcodes[index].cost;
        match.mx=params->match_vars->pcodes[index].mx;
        match.my=params->match_vars->pcodes[index].my;
        jj=match.x;
        ii=match.y;
        if (match.mx>=0)
        {
mutex_id=ii*NB_MUTEX/params->Iproj->ys;
pthread_mutex_lock(&(match_mutex[mutex_id]));
          if (match.cost<PIXEL(params->Imatch_p2c,jj,ii,0))
          {
            PIXEL(params->Imatch_p2c,jj,ii,0)=(pix_t)(match.cost);
            COMPLEX(params->Imatch_p2c,jj,ii,0,0)=match.mx+0.5;
            COMPLEX(params->Imatch_p2c,jj,ii,0,1)=match.my+0.5;
          }
pthread_mutex_unlock(&(match_mutex[mutex_id]));
        }
      }
    }

    pthread_exit(NULL);
}

int imguBlack(imgu *I,int x,int y)
{
    int c;
    unsigned char black;

    if (imguCheck(I,(float)(x),(float)(y))) return -1;

    black=1;
    for (c=0;c<I->cs;c++)
    {
      if (PIXEL(I,x,y,c)) black=0;
    }
    if (black) return -1;

    return 0;
}

int imguCheckEmpty(imgu *I,int x,int y)
{
    if (I==NULL) return -1;

    if (imguBlack(I,x,y)) return -1;
    if (imguBlack(I,x+1,y)) return -1;
    if (imguBlack(I,x+1,y+1)) return -1;
    if (imguBlack(I,x,y+1)) return -1;

    return 0;
}

int main(int argc,char *argv[])
{
    int i,j,k,p,c,index;
    int grab_nb;
    int ii,jj;
    int NB_PATTERNS;
    char buf[100];
    int cam = 0; // number of the camera
    char outPath[256];
    char inPath[256];
    char patFilename[256];
    char filename[256];
    char path[256];
    char file[256];
    int imap_xs, imap_ys; // size of the inverse mapping
    int confidence;
    char uv_filename[256];
    char uvimg_filename[256];
    int proj_xs, proj_ys;
    int nb_iter;
    imgu *I,*J;
    imgu *Iavg,*Idev;
    imgu *Icmp;
    imgu *Icam,*Jcam;
    imgu *Iproj,*Jproj;
    imgu *Imatch_c2p,*Imatch_c2p_color;
    imgu *Imatch_p2c,*Imatch_p2c_color;
    imgu *Imatch_p2c_color_p2;
    int NB_THREADS;
    pthread_t *thread;
    pthread_attr_t attr;
    int rc;
    void *status;

    I=NULL;
    J=NULL;
    Iavg=NULL;
    Idev=NULL;
    Icmp=NULL;
    Icam=NULL;
    Iproj=NULL;
    Imatch_c2p=NULL;
    Imatch_p2c=NULL;
    Imatch_c2p_color=NULL;
    Imatch_p2c_color=NULL;
    Imatch_p2c_color_p2=NULL;

    NB_PATTERNS=100;
    inPath[0]=0;
    outPath[0]=0;
    confidence = 100;
    uv_filename[0]='\0';
    proj_xs = 1280;
    proj_ys = 768;
    imap_xs=2048;
    imap_ys=1024;
    nb_iter=20;

    NB_THREADS=2;

    for(i=1;i<argc;i++) 
    {
        if( strcmp(argv[i],"-i")==0 && i+1<argc ) 
        {
            strcpy(inPath,argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-o")==0 && i+1<argc ) 
        {
            strcpy(outPath,argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-pat")==0 && i+1<argc ) 
        {
            strcpy(patFilename,argv[i+1]);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-c")==0 && i+1<argc ) 
        {
            cam=atoi(argv[i+1]); 
            LT_PRINTF("Processing camera number %d\n", cam);
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-g")==0 && i+1<argc ) 
        {
            grab_nb=atoi(argv[i+1]); 
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-imap")==0 && i+2<argc ) 
        {
            imap_xs=atoi(argv[i+1]); 
            imap_ys=atoi(argv[i+2]); 
            LT_PRINTF("Inverse map size = %d X %d\n", imap_xs, imap_ys);
            i+=2;
            continue;
        }
        if( strcmp(argv[i],"-conf")==0 && i+1<argc ) 
        {
            confidence=atoi(argv[i+1]); 
            i+=1;
            continue;
        }
        if( strcmp(argv[i],"-proj")==0 && i+2<argc ) 
        {
            proj_xs=atoi(argv[i+1]); 
            proj_ys=atoi(argv[i+2]); 
            LT_PRINTF("Projector resolution = %d X %d\n", proj_xs, proj_ys);
            i+=2;
            continue;
        }
        if( strcmp(argv[i],"-nb_patterns")==0 && i+1<argc ) 
        {
            NB_PATTERNS=atoi(argv[i+1]); 
            LT_PRINTF("Number of Patterns = %d\n", NB_PATTERNS);
            i++;
            continue;
        }
        if( strcmp(argv[i],"-n")==0 && i+1<argc ) 
        {
            nb_iter=atoi(argv[i+1]); 
            i+=1;
            continue;
        }
    }

    if( inPath[0]==0 ) 
    {
        LT_PRINTF("Please specify the path of the input images (-i)\n");
        LT_EXIT(0);
    }

    thread=(pthread_t *)(malloc(sizeof(pthread_t)*NB_THREADS));
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (i=0;i<NB_MUTEX;i++) pthread_mutex_init(&(match_mutex[i]), NULL);

    sprintf(uv_filename,"%s/uvmap.txt",outPath);
    sprintf(uvimg_filename,"%s/uvmap.png",outPath);
    LT_PRINTF("UV maps:\n%s\n%s\n",uv_filename,uvimg_filename);
    uvReadFile(uv_filename);
    uvLoadImg(uvimg_filename);

    sprintf(path,inPath,cam);
    for (i=0;i<NB_PATTERNS;i++)
    {
      sprintf(filename,"grab_noise_%02d_%05d.png",grab_nb,i);
      sprintf(file,"%s/%s",path,filename);
LT_PRINTF("Loading '%s'.\n",file);
      imguLoad(&J,file,LOAD_AS_IS);
      if (i==0)
      {
        imguAllocate(&Iavg,J->xs,J->ys,1);
        imguAllocateComplex(&Iavg,J->xs,J->ys,1);
        imguClearComplex(Iavg,0);
        imguAllocate(&Idev,J->xs,J->ys,1);
        imguAllocateComplex(&Idev,J->xs,J->ys,1);
        imguClearComplex(Idev,0);
      }
      for (j=0;j<J->xs*J->ys;j++) 
      {
        Iavg->complex[j][0]+=J->data[j];
        Idev->complex[j][0]+=J->data[j]*J->data[j];
      }
      imguAddLastMulti(&I,J);
      J=NULL;
      if ((i+1)%CHANNEL_NB_BITS==0)      
      {
        for (j=0;j<Iavg->xs*Iavg->ys;j++) 
        {
          Iavg->complex[j][1]=Iavg->complex[j][0]/(i+1);
          Idev->complex[j][1]=sqrt(Idev->complex[j][0]/(i+1)-Iavg->complex[j][1]*Iavg->complex[j][1]);
        }
        imguConvertFromComplexComponent(&Iavg,Iavg,1,COMPLEX_AS_IS);
        imguConvertFromComplexComponent(&Idev,Idev,1,COMPLEX_AS_IS);
        J=I;
        j=0;
        while(J!=NULL) 
        {
          imguSubtract(&J,J,Iavg,IMGU_MAXVAL/2);
//sprintf(filename,"%08d.png",i-CHANNEL_NB_BITS+j+1);
//imguSave(J,filename,1,SAVE_16_BITS);
          J=J->next;
          j++;
        }
LT_PRINTF("Compressing...\n");
        noise_compress(&Icmp,I);
        imguFreeMulti(&I);
        imguAddLastMulti(&Icam,Icmp);
        Icmp=NULL;
      }
    }
sprintf(filename,"%s/noise_avg.png",outPath);
imguSave(Iavg,filename,1,SAVE_8_BITS_LOW);
sprintf(filename,"%s/noise_dev.png",outPath);
imguSave(Idev,filename,1,SAVE_8_BITS_LOW);

    sprintf(file,"%s/%s",outPath,patFilename);
LT_PRINTF("Loading '%s'.\n",file);
    imguLoadMulti(&Iproj,file,LOAD_AS_IS);

    if (Icam==NULL || Iproj==NULL)
    {
      LT_PRINTF("Error loading images.\n");
      LT_EXIT(0);
    }

    Jcam=Icam;
    Jproj=Iproj;
    while(Jcam!=NULL && Jcam->next!=NULL)
    {
      Jcam=Jcam->next;
      Jproj=Jproj->next;
    }
    //make Iproj and Icam equal length
    imguFreeMulti(&(Jproj->next));
    Jproj=Iproj;
    while(Jproj!=NULL)
    {
      imguExtractRectangle(&Jproj,Jproj,0,0,proj_xs,proj_ys);
      Jproj=Jproj->next;
    }

    LT_PRINTF("Number of camera images: %d\n",imguCount(Icam)*CHANNEL_NB_BITS);
    LT_PRINTF("Number of projector images: %d\n",imguCount(Iproj)*CHANNEL_NB_BITS);

    imguAllocate(&Imatch_c2p,Icam->xs,Icam->ys,1);
    imguAllocateComplex(&Imatch_c2p,Icam->xs,Icam->ys,1);
    for (i=0;i<Imatch_c2p->xs*Imatch_c2p->ys;i++) PIXEL(Imatch_c2p,i,0,0)=NOISE_COST_INF;
    imguAllocate(&Imatch_p2c,Iproj->xs,Iproj->ys,1);
    imguAllocateComplex(&Imatch_p2c,Iproj->xs,Iproj->ys,1);
    for (i=0;i<Imatch_p2c->xs*Imatch_p2c->ys;i++) PIXEL(Imatch_p2c,i,0,0)=NOISE_COST_INF;

    noise_match *match_vars;
    match_thread_params *thread_params;
    match_vars=(noise_match *)(malloc(sizeof(noise_match)*NB_THREADS));
    thread_params=(match_thread_params *)(malloc(sizeof(match_thread_params)*NB_THREADS));

    for (k=0;k<nb_iter;k++)
    {
LT_PRINTF("Iteration %d.\n",k);
      for (p=0;p<NB_THREADS;p++) noise_match_init(&(match_vars[p]),NOISE_MATCH_SORT,-1,Iproj,Icam);
      //start matching threads
      for (p=0;p<NB_THREADS;p++)
      {
        thread_params[p].id=p;
        thread_params[p].match_vars=&(match_vars[p]);
        thread_params[p].Iproj=Iproj;
        thread_params[p].Icam=Icam;
        thread_params[p].Imatch_c2p=Imatch_c2p;
        thread_params[p].Imatch_p2c=Imatch_p2c;
        //fprintf(stderr,"Matching codes thread %d start.\n",p);
        rc = pthread_create(&thread[p], &attr, match_thread, (void *)&(thread_params[p])); 
        if (rc)
        {
          fprintf(stderr,"ERROR: return code from pthread_create() is %d\n", rc);
          exit(-1);
        }
      }
      for(p=0;p<NB_THREADS;p++) 
      {
        rc = pthread_join(thread[p], &status);
        if (rc) 
        {
          fprintf(stderr,"ERROR: return code from pthread_join() is %d\n", rc);
          exit(-1);
        }
        noise_match_free(&(match_vars[p]));
        //fprintf(stderr,"Matching thread %d is done.\n",p);
      }

      noise_match_backward_heuristic(Iproj,Icam,Imatch_c2p);
      noise_match_forward_heuristic(Iproj,Icam,Imatch_c2p);

      noise_match_backward_heuristic(Icam,Iproj,Imatch_p2c);
      noise_match_forward_heuristic(Icam,Iproj,Imatch_p2c);
    }

    for (i=0;i<Imatch_c2p->ys;i++)
    {
      for (j=0;j<Imatch_c2p->xs;j++)
      {
        if (PIXEL(Idev,j,i,0)<confidence)
        {
          COMPLEX(Imatch_c2p,j,i,0,0)=-0.1;
          COMPLEX(Imatch_c2p,j,i,0,1)=-0.1;
        }
        if(uvCheck((double)(j),(double)(i))==0)
        {
          COMPLEX(Imatch_c2p,j,i,0,0)=-0.1;
          COMPLEX(Imatch_c2p,j,i,0,1)=-0.1;
        }
      }
    }

    for (i=0;i<Imatch_p2c->ys;i++)
    {
      for (j=0;j<Imatch_p2c->xs;j++)
      {
        if(uvCheck((double)(COMPLEX(Imatch_p2c,j,i,0,0)),(double)(COMPLEX(Imatch_p2c,j,i,0,1)))==0)
        {
          COMPLEX(Imatch_p2c,j,i,0,0)=-0.1;
          COMPLEX(Imatch_p2c,j,i,0,1)=-0.1;
        }
      }
    }

    noise_match_roundrip_check(Iproj,Icam,Imatch_c2p,Imatch_p2c,10.0);

    for (i=0;i<Imatch_p2c->ys;i++)
    {
      for (j=0;j<Imatch_p2c->xs;j++)
      {
        if(COMPLEX(Imatch_p2c,j,i,0,0)>=0)
        {
          COMPLEX(Imatch_p2c,j,i,0,0)/=Icam->xs;
          COMPLEX(Imatch_p2c,j,i,0,1)/=Icam->ys;
        }
      }
    }

    imguConvertFromComplexUV(&Imatch_c2p_color,Imatch_c2p,COMPLEX_RESCALE);
    imguConvertFromComplexUV(&Imatch_p2c_color,Imatch_p2c,COMPLEX_RESCALE);

    //scale projector map to size imap_xs,imap_ys
    imguAllocate(&Imatch_p2c_color_p2,imap_xs,imap_ys,3);
    imguCopyText(Imatch_p2c_color_p2,Imatch_p2c_color);
    double x,y;
    vector4 val;
    for (i=0;i<Imatch_p2c_color_p2->ys;i++)
    {
      for (j=0;j<Imatch_p2c_color_p2->xs;j++)
      {
        x=((double)(j))*Imatch_p2c_color->xs/Imatch_p2c_color_p2->xs;
        y=((double)(i))*Imatch_p2c_color->ys/Imatch_p2c_color_p2->ys;
        if (imguCheckEmpty(Imatch_p2c_color,(int)(x),(int)(y))==0)
        {
          imguInterpolateBilinear(Imatch_p2c_color,x,y,val);
          for (c=0;c<Imatch_p2c_color_p2->cs;c++)
          {
            PIXEL(Imatch_p2c_color_p2,j,i,c)=(pix_t)(val[c]);
          }
        }
      }
    }

sprintf(filename,"%s/xyvalid_color_%02d.png",outPath,cam);
imguSave(Imatch_c2p_color,filename,1,SAVE_16_BITS);
imguConvertToGray(&Imatch_c2p_color,Imatch_c2p_color);
sprintf(filename,"%s/xyvalid_%02d.png",outPath,cam);
imguSave(Imatch_c2p_color,filename,1,SAVE_16_BITS);
sprintf(filename,"%s/imapXY_%02d.lut.png",outPath,cam);
imguSave(Imatch_p2c_color_p2,filename,1,SAVE_16_BITS);
sprintf(filename,"%s/imapXY_%02d-copy.lut.png",outPath,cam);
imguSave(Imatch_p2c_color_p2,filename,1,SAVE_16_BITS);

    imguFreeMulti(&I);
    imguFree(&Iavg);
    imguFree(&Idev);
    imguFreeMulti(&Icmp);
    imguFreeMulti(&Icam);
    imguFreeMulti(&Iproj);
    imguFree(&Imatch_c2p);
    imguFree(&Imatch_p2c);
    imguFree(&Imatch_c2p_color);
    imguFree(&Imatch_p2c_color);
    imguFree(&Imatch_p2c_color_p2);

    pthread_attr_destroy(&attr);
    for (i=0;i<NB_MUTEX;i++) pthread_mutex_destroy(&(match_mutex[i]));
    //pthread_exit(NULL);

    LT_EXIT(0);
}


