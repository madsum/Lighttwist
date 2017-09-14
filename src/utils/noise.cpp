#include "noise.h"

int noise_texture_2d(imgu **I,int sx,int sy,double alpha,double min_freq,double max_freq,unsigned char binarize,unsigned char blur)
{
    int i, j, index;
    unsigned int the_val;    
    double fx, fy, m, min_squared, max_squared;
    double favg,pattern_width,pattern_dev;
    double ktk, v;
    int msx,msy;
    imgu *J,*Iblurx;
    GPU *gpu_convolvex;
    GPU *gpu_convolvey;

    Iblurx=NULL;

    favg=(min_freq+max_freq)/2.0;
    pattern_width=sx/favg;
    pattern_dev=pattern_width/6.0;

    //to avoid border artifacts due to modulo condition in FFT, generate larger texture and then cut extra margins
    msx=(int)(sx*1.10);
    msy=(int)(sy*1.10);
    min_freq*=1.10;
    max_freq*=1.10;
    pattern_dev*=1.10;

    gpu_convolvex = new GPU(msx,msy,10,10,GPU::NO_WINDOW);
    gpu_convolvex->setupConvolve(Convolve::gaussianx_dev,pattern_dev);
    gpu_convolvey = new GPU(msx,msy,10,10,GPU::NO_WINDOW);
    gpu_convolvey->setupConvolve(Convolve::gaussiany_dev,pattern_dev);

    imguAllocateComplex(I,msx,msy,1);
    J=(*I);

    min_squared = min_freq*min_freq;
    max_squared = max_freq*max_freq;

    imguClearComplex(J,0);
    imguClearComplex(J,1);
    for(i=1;i<msy/2;i++)
    {
        fy = (i-msy/2)*((double)(msx))/msy;
        for(j=1;j<msx/2;j++)
        {
            fx = j-msx/2;
            m = fx*fx + fy*fy;
            if (m >= min_squared && m <= max_squared)
            {
                //the following is just a trick to compute ktk as pow(sqrt(m),alpha) without using the sqrt operation
                ktk = pow(2, alpha/2 * log((double)(m))/log(2.0));
                //COMPLEX(J,j,i,0,0)=1.0/ktk;
                COMPLEX(J,j,i,0,0)=1.0; //white noise
                COMPLEX(J,j,i,0,1)=matRandNumber()*2*M_PI;
                //conjugate value
                COMPLEX(J,msx-j,msy-i,0,0)=COMPLEX(J,j,i,0,0);
                COMPLEX(J,msx-j,msy-i,0,1)=-COMPLEX(J,j,i,0,1);

                COMPLEX(J,msx-j,i,0,0)=COMPLEX(J,j,i,0,0);
                COMPLEX(J,msx-j,i,0,1)=matRandNumber()*2*M_PI;
                //conjugate value
                COMPLEX(J,j,msy-i,0,0)=COMPLEX(J,msx-j,i,0,0);
                COMPLEX(J,j,msy-i,0,1)=-COMPLEX(J,msx-j,i,0,1);
            }
        }
    }

    imguAdjustQuadrants(I,J);
    imguRealImag(I,J); //transform magn,phase into real,imag
    imguFFTInverse(I,J);

    imguAllocate(I,msx,msy,1);
    //threshold at 0
    if( binarize )
    {
	  for(i=0;i<msy;i++) 
      {
        for(j=0;j<msx;j++) 
        {
		  index=INDEX(J,j,i,0);
		  if (J->complex[index][0]<0) J->data[index]=0;
		  else J->data[index]=IMGU_MAXVAL;
        } 
      }
    }else{
	  imguConvertFromComplexComponent(I,J,0,COMPLEX_RESCALE);
    }

    if (blur)
    {
      //fprintf(stderr,"Blur kernel (std dev): %f\n",pattern_dev);
      //imguBlur(I,J,pattern_dev,pattern_dev,CONVOLVE_KEEP_MARGIN);
      gpu_convolvex->setInputImage(0,J);
      gpu_convolvex->setOutputImage(1,&Iblurx);
      gpu_convolvex->process();
      gpu_convolvey->setInputImage(0,Iblurx);
      gpu_convolvey->setOutputImage(1,&J);
      gpu_convolvey->process();
    }

    delete gpu_convolvex;
    delete gpu_convolvey;

    imguExtractRectangle(I,J,(msx-sx)/2,(msy-sy)/2,sx,sy);

    imguFree(&Iblurx);

    return 0;
}

int noise_texture_1d(imgu **I,int sx,int sy,double alpha,double min_freq,double max_freq,unsigned char binarize,unsigned char blur,int dim)
{
    int i, j, index;
    unsigned int the_val;
    double f, m, min_squared, max_squared;
    double favg,pattern_width,pattern_dev;
    double ktk, v;
    int msx,msy;
    imgu *Ifreq;
    imgu *J;

    favg=(min_freq+max_freq)/2.0;
    pattern_width=sx/favg;
    pattern_dev=pattern_width/6.0;

    //to avoid border artifacts due to modulo condition in FFT, generate larger texture and then cut extra margins
    msx=(int)(sx*1.10);
    msy=(int)(sy*1.10);
    min_freq*=1.10;
    max_freq*=1.10;

    Ifreq=NULL;
    if (dim==NOISE_X) imguAllocateComplex(&Ifreq,msx,1,1);
    else imguAllocateComplex(&Ifreq,msy,1,1);

    min_squared = min_freq*min_freq;
    max_squared = max_freq*max_freq;

    imguClearComplex(Ifreq,0);
    imguClearComplex(Ifreq,1);
    for(i=1;i<Ifreq->xs;i++)
    {
        if (dim==NOISE_Y) f = (i-msy/2)*((double)(msx))/msy;
        else f = i-msx/2;
        m = f*f;
        if (m >= min_squared && m <= max_squared)
        {
            //the following is just a trick to compute ktk as pow(sqrt(m),alpha) without using the sqrt operation
            ktk = pow(2, alpha/2 * log((double)(m))/log(2.0));
            //COMPLEX(Ifreq,i,0,0,0)=1.0/ktk;
            COMPLEX(Ifreq,i,0,0,0)=1.0; //white noise
            COMPLEX(Ifreq,i,0,0,1)=matRandNumber()*2*M_PI;
            //conjugate value
            COMPLEX(Ifreq,Ifreq->xs-i,0,0,0)=COMPLEX(Ifreq,i,0,0,0);
            COMPLEX(Ifreq,Ifreq->xs-i,0,0,1)=-COMPLEX(Ifreq,i,0,0,1);
        }
    }
    imguAdjustQuadrants(&Ifreq,Ifreq);
    imguRealImag(&Ifreq,Ifreq); //transform magn,phase into real,imag
    imguFFTInverse(&Ifreq,Ifreq);

    imguAllocate(I,msx,msy,1);
    J=(*I);
    //threshold at 0
    if(binarize) 
    {
      for(i=0;i<msy;i++)
      {
        for(j=0;j<msx;j++)
        {
          if (dim==NOISE_X) index=j;
          else index=i;
		  if (Ifreq->complex[index][0]<0) PIXEL(J,j,i,0)=0;
		  else PIXEL(J,j,i,0)=IMGU_MAXVAL;          
		}
	  }
    }else{
	  imguConvertFromComplexComponent(&Ifreq,Ifreq,0,COMPLEX_RESCALE);
      for(i=0;i<msy;i++)
      {
        for(j=0;j<msx;j++)
        {
          if (dim==NOISE_X) index=j;
          else index=i;
		  PIXEL(J,j,i,0)=PIXEL(Ifreq,index,0,0);
		}
	  }
    }

    if (blur) imguBlur(I,J,pattern_dev,pattern_dev,CONVOLVE_KEEP_MARGIN);

    imguExtractRectangle(I,J,(msx-sx)/2,(msy-sy)/2,sx,sy);

    imguFree(&Ifreq);

    return 0;
}

//I is a list of images where the number of images should be a multiple of CHANNEL_NB_BITS
int noise_compress(imgu **Icmp,imgu *I)
{
  int i,j;
  int nbframes, nbchannels, nbimgs;
  pix_t mask;
  imgu *J,*Jcmp;

  if (I==NULL) return -1;
  if ((*Icmp)==I) return -1;

  nbframes=imguCount(I);
  nbchannels=nbframes/CHANNEL_NB_BITS;
  if (nbframes%CHANNEL_NB_BITS!=0) nbchannels++;

  J=NULL;

  imguFreeMulti(Icmp);
  for (i=0;i<nbchannels;i++)
  {
    imguAllocate(&J,I->xs,I->ys,1);
    imguClear(J);
    imguAddLastMulti(Icmp,J);
    J=NULL;
  }

  Jcmp=*Icmp;
  J=I;  
  i=0;
  while(J!=NULL)
  {
    mask=1;
    mask<<=(i%CHANNEL_NB_BITS);
    for (j=0;j<J->xs*J->ys;j++)
    {
      //if pixel is IMGU_MAXVAL/2, flip a coin and choose value randomly
      if (PIXEL(J,j,0,0)<IMGU_MAXVAL/2 || (PIXEL(J,j,0,0)==IMGU_MAXVAL/2 && matRandNumber()<0.5))
      {
        mask=~mask;
        PIXEL(Jcmp,j,0,0)&=mask;
        mask=~mask;
      }
      else
      {
        PIXEL(Jcmp,j,0,0)|=mask;
      }
    }
    i++;
    if (i%CHANNEL_NB_BITS==0) Jcmp=Jcmp->next;
    J=J->next;
  }

  return 0;
}

int noise_compress_pattern(imgu **Icmp,imgu *I,int index)
{
  int i,j,k;
  int nbframes, nbchannels, nbimgs;
  pix_t mask;
  imgu *J,*Jcmp;

  if (I==NULL) return -1;
  if ((*Icmp)==I) return -1;
  if (index<0 || index>=CHANNEL_NB_BITS) return -1;

  if ((*Icmp)==NULL)
  {
    imguAllocate(Icmp,I->xs,I->ys,1);
    imguClear((*Icmp));
  }

  Jcmp=*Icmp;
  J=I;  
  mask=1<<index;
  for (j=0;j<J->xs*J->ys;j++)
  {
    //if pixel is IMGU_MAXVAL/2, flip a coin and choose value randomly
    if (PIXEL(J,j,0,0)<IMGU_MAXVAL/2 || (PIXEL(J,j,0,0)==IMGU_MAXVAL/2 && matRandNumber()<0.5))
    {
      mask=~mask;
      PIXEL(Jcmp,j,0,0)&=mask;
      mask=~mask;
    }
    else
    {
      PIXEL(Jcmp,j,0,0)|=mask;
    }
  }

  return 0;
}

int noise_decompress_pattern_at(imgu *Icmp,int pat,int x,int y)
{
  int i,j,k;
  int frame;
  pix_t mask;
  imgu *Jcmp;

  if (Icmp==NULL) return -1;

  frame=pat/CHANNEL_NB_BITS;
  if (frame>=imguCount(Icmp)) return -1;

  Jcmp=Icmp;
  for (i=0;i<frame;i++)
  {
    Jcmp=Jcmp->next;
  }
  
  mask=1;
  mask<<=(pat%CHANNEL_NB_BITS);
  if (PIXEL(Jcmp,x,y,0)&mask)
  {
    return IMGU_MAXVAL;
  }
  else
  {
    return 0;
  }
}

int noise_decompress_pattern(imgu **I,imgu *Icmp,int index)
{
  int i,j,k;
  int frame;

  if (Icmp==NULL) return -1;
  if ((*I)==Icmp) return -1;

  frame=index/CHANNEL_NB_BITS;
  if (frame>=imguCount(Icmp)) return -1;

  imguAllocate(I,Icmp->xs,Icmp->ys,1);
  
  for (j=0;j<Icmp->xs*Icmp->ys;j++)
  {
    (*I)->data[j]=noise_decompress_pattern_at(Icmp,index,j,0);
  }

  return 0;
}

int noise_decompress_patterns(imgu **I,imgu *Icmp)
{
  int i;
  int nbframes;
  imgu *J;

  if (Icmp==NULL) return -1;
  if ((*I)==Icmp) return -1;

  J=NULL;

  nbframes=imguCount(Icmp)*CHANNEL_NB_BITS;

  for (i=0;i<nbframes;i++)
  {
    noise_decompress_pattern(&J,Icmp,i);
    imguAddLastMulti(I,J);
    J=NULL;
  }

  return 0;
}

int noise_print(imgu *Icmp,int x,int y)
{
  int k;
  int nbpatterns;

  if (Icmp==NULL) return -1;
  nbpatterns=imguCount(Icmp)*CHANNEL_NB_BITS;
  if (imguCheck(Icmp,(float)(x),(float)(y))!=0) return -1;
  if (Icmp->cs!=1) return -1;

  for (k=0;k<nbpatterns;k++)
  {
    printf("%f\t",noise_decompress_pattern_at(Icmp,k,x,y)/(double)(IMGU_MAXVAL));
  }
  printf("\n");

  return 0;
}

static int noise_compressed_to_uchar(imgu *I,noise_code **cnoise)
{
  int i,j,k,index;
  int nbchars;
  int nbcodes;

  if (cnoise==NULL) return -1;
  if (I->cs!=1) return -1;

  nbcodes=I->xs*I->ys;
  nbchars=imguCount(I)*CHANNEL_NB_BITS;

//printf("%d %d\n",nbcodes,(nbchars+1));
  (*cnoise)=(noise_code *)(malloc(sizeof(noise_code)*nbcodes));
  for (i=0;i<nbcodes;i++) (*cnoise)[i].code=(char *)(malloc(sizeof(char)*(nbchars+1)));

  index=0;
  for (i=0;i<I->ys;i++)
  {
    for (j=0;j<I->xs;j++)
    {
      (*cnoise)[index].x=j;
      (*cnoise)[index].y=i;
      for (k=0;k<nbchars;k++)
      {
        if (noise_decompress_pattern_at(I,k,j,i)<IMGU_MAXVAL/2) (*cnoise)[index].code[k]='0';
        else (*cnoise)[index].code[k]='1';
      }
      (*cnoise)[index].code[k]='\0';
      index++;
    }
  }

  return 0;
}

static int compare_noise_uchar( const void *a, const void *b )
{
    noise_code *na=(noise_code *)(a);
    noise_code *nb=(noise_code *)(b);
    return( strcmp(na->code, nb->code) );
}

int noise_count_total(imgu *Icmp)
{
  int i,j,k,index;
  int count;
  imgu *J;
  noise_code *cnoise;
  int nbcodes;
  int dx,dy;
  double dist,maxdist;
  int maxx1,maxy1,maxx2,maxy2;

  if (Icmp==NULL) return -1;

  noise_compressed_to_uchar(Icmp,&cnoise);
  nbcodes=Icmp->xs*Icmp->ys;

  qsort(cnoise,nbcodes,sizeof(noise_code),compare_noise_uchar);

  count=1;
  maxdist=-1.0;
  for (i=1;i<nbcodes;i++)
  {
    if (strcmp(cnoise[i].code,cnoise[i-1].code)!=0) count++;
    else
    {
      //find max distance between duplicates
      /*for (k=0;k<i;k++)
      {
        if (strcmp(cnoise[i].code,cnoise[i-1-k].code)!=0) break;
        dx=cnoise[i].x-cnoise[i-1-k].x;
        dy=cnoise[i].y-cnoise[i-1-k].y;
        dist=sqrt(dx*dx+dy*dy);
        if (maxdist<dist)
        {
          maxdist=dist;
          maxx1=cnoise[i].x;
          maxy1=cnoise[i].y;
          maxx2=cnoise[i-1-k].x;
          maxy2=cnoise[i-1-k].y;
        }
      }*/
    }
  }

  /*printf("%d %d %d %d\n",maxx1,maxy1,maxx2,maxy2);
  noise_print(Icmp,maxx1,maxy1);
  noise_print(Icmp,maxx2,maxy2);

  printf("Maximum distance between duplicates was: %f\n",maxdist);
  fflush(stdout);
  fprintf(stderr,"Maximum distance between duplicates was: %f\n",maxdist);
  fflush(stderr);*/

  for (i=0;i<nbcodes;i++) free(cnoise[i].code);
  free(cnoise);

  return count;
}

static unsigned char bit_one_count_table[IMGU_MAXVAL+1];

//counts the number bits of value 1 in an unsigned integer
//this runs in O(number of bits of value 1)
static int bit_count (unsigned int n)  {
   int count = 0 ;
   while (n)  {
      count++ ;
      n &= (n - 1) ;
   }
   return count;
}

int noise_bit_count_init()
{
  int i;
  for(i=0;i<=IMGU_MAXVAL;i++)
  {
    bit_one_count_table[i]=bit_count(i);
  } 

  return 0;
}

static int noise_bit_count_get(unsigned int n)
{
  return bit_one_count_table[n];
}

int noise_cost(imgu *Iproj,int x1,int y1,imgu *Icam,int x2,int y2)
{
  imgu *Jproj,*Jcam;
  int cost;
  int diff;

  if (Iproj==NULL) return NOISE_COST_INF;
  if (Icam==NULL) return NOISE_COST_INF;
  //if (imguCheck(Iproj,(float)(x1),(float)(y1))!=0) return NOISE_COST_INF;
  //if (imguCheck(Icam,(float)(x2),(float)(y2))!=0) return NOISE_COST_INF;

  Jproj=Iproj;
  Jcam=Icam;
  cost=0;
  while(Jproj!=NULL && Jcam!=NULL)
  {
    diff=PIXEL(Jproj,x1,y1,0)^PIXEL(Jcam,x2,y2,0);
    cost+=noise_bit_count_get(diff);
    Jproj=Jproj->next;
    Jcam=Jcam->next;
  }

  return cost;
}

//choose k bits out of N
//assmes that mask has N+1 characters
static int noise_generate_mask(char *mask,int k,int N)
{
  int i,pos;

  for (i=0;i<N;i++) mask[i]='0';
  mask[i]='\0';

  if (k>=N)
  {
    for (i=0;i<N;i++) mask[i]='1';
  }
  else
  {
    for (i=0;i<k;i++)
    {
      pos=(int)(matRandNumber()*N);
      if (mask[pos]=='0') mask[pos]='1';
      else i--;
    }
  }

  return 0;
}

typedef struct {
	int pos;
	double key;
} shuffle_key;

static int compare_shuffle_keys(const void *a,const void *b) {
    shuffle_key *sa=(shuffle_key *)(a);
    shuffle_key *sb=(shuffle_key *)(b);
	if( sa->key < sb->key ) return -1;
	if( sa->key > sb->key ) return 1;
	return 0;
}

//assmes that shuffle has N-1 double values
static int noise_generate_shuffle(int *shuffle,int N)
{
  int i;
  shuffle_key *keys;

  keys=(shuffle_key *)(malloc(sizeof(shuffle_key)*N));

  for (i=0;i<N;i++)
  {
    keys[i].pos=i;
    keys[i].key=matRandNumber();
  }

  qsort(keys,N,sizeof(shuffle_key),compare_shuffle_keys);

  for (i=0;i<N;i++)
  {
    shuffle[i]=keys[i].pos;
  }

  free(keys);

  return 0;
}

static int noise_shuffle(imgu **Ishuffle,imgu *Icmp,int *shuffle,int N)
{
   int i,j,k;
   int pos;
   int smask,gmask; //set mask and get mask
   imgu *Jcmp;
   imgu *Jshuffle;

   if (Icmp==NULL) return -1;
   if (Ishuffle==NULL) return -1;

   //imguFreeMulti(Ishuffle);
   imguCopyMulti(Ishuffle,Icmp);

   Jshuffle=(*Ishuffle);
   imguClear(Jshuffle);
   smask=1;
   for (k=0;k<N;k++)
   {
     pos=shuffle[k];
     Jcmp=Icmp;
     while(pos>=CHANNEL_NB_BITS)
     {
       Jcmp=Jcmp->next;
       pos-=CHANNEL_NB_BITS;
     }
     gmask=1<<pos;

     for (i=0;i<Icmp->ys;i++)
     {
       for (j=0;j<Icmp->xs;j++)
       {
         //bit at 'pos' is 1
         if (PIXEL(Jcmp,j,i,0)&gmask)
         {
           PIXEL(Jshuffle,j,i,0)|=smask;
         }
       }
     }
 
     if ((k+1)%CHANNEL_NB_BITS==0)
     {
       Jshuffle=Jshuffle->next;
       imguClear(Jshuffle);
       smask=1;
     }
     else
     {
       smask<<=1;
     }
   }

   return 0;
}

int noise_compare( const void *a, const void *b )
{
    int diff;
    noise_code *na=(noise_code *)(a);
    noise_code *nb=(noise_code *)(b);
    imgu *Ia=na->I;
    imgu *Ib=nb->I;
    while(Ia!=NULL && Ib!=NULL)
    {
      diff=PIXEL(Ia,na->x,na->y,0)-PIXEL(Ib,nb->x,nb->y,0);
      if (diff!=0) return diff;
      Ia=Ia->next;
      Ib=Ib->next;
    }

    return 0;
}

static int noise_sort_match_list(noise_code *proj,int Nproj,noise_code *cam,int Ncam)
{
    int p,c;
    int cost;
    int diff;

    //initialize all matches to infinity
    c=0;
    while(c<Ncam)
    {
      cam[c].cost=NOISE_COST_INF;
      c++;
    }

    p=0;
    while(p<Nproj)
    {
      proj[p].cost=NOISE_COST_INF;
      p++;
    }

    p=0;
    c=0;
    while(c<Ncam && p<Nproj)
    {
      cost=noise_cost(proj[p].I,proj[p].x,proj[p].y,cam[c].I,cam[c].x,cam[c].y);
      if (cost<cam[c].cost)
      {
        cam[c].mx=proj[p].x;
        cam[c].my=proj[p].y;
        cam[c].cost=cost;
      }
      if (cost<proj[p].cost)
      {
        proj[p].mx=cam[c].x;
        proj[p].my=cam[c].y;
        proj[p].cost=cost;
      }
      diff=noise_compare(&(cam[c]),&(proj[p])); 
      if (p==Nproj-1 || diff<0) //proj>cam
      {
        c++;
      }
      else if (c==Ncam-1 || diff>0)
      {
        p++;
      }
      else //diff=0, check next positions
      {
        //the following should not overflow because of the previous conditions
        diff=noise_compare(&(cam[c+1]),&(proj[p+1]));  
        if (diff==0)
        {
          c++;
          p++;
        }
        else if (diff<0)
        {
          c++;
        }
        else
        {
          p++;
        }
      }
    }

    return 0;
}

static int noise_sort_match(noise_code *proj,int Nproj,noise_code *cam,int Ncam)
{
    struct timeval tv;
    double tick1,tick2;

#ifdef NOISE_VERBOSE
    gettimeofday(&tv,NULL);
    tick1=tv.tv_sec+tv.tv_usec/1000000.0;
#endif
    qsort(proj,Nproj,sizeof(noise_code),noise_compare);
#ifdef NOISE_VERBOSE
    gettimeofday(&tv,NULL);
    tick2=tv.tv_sec+tv.tv_usec/1000000.0;
    fprintf(stderr,"Time to sort %d codes: %f\n",Nproj,tick2-tick1);
#endif
    qsort(cam,Ncam,sizeof(noise_code),noise_compare);
#ifdef NOISE_VERBOSE
    gettimeofday(&tv,NULL);
    tick1=tv.tv_sec+tv.tv_usec/1000000.0;
    fprintf(stderr,"Time to sort %d codes: %f\n",Ncam,tick1-tick2);
#endif

    noise_sort_match_list(proj,Nproj,cam,Ncam);
    
    return 0;     
}

static int noise_hash(imgu *Icmp,int x,int y,char *mask,int N)
{
  int i,j,mi;
  int nbchannels,channel;
  int val;
  imgu *J;

  if (Icmp==NULL) return -1;
  if (imguCount(Icmp)*CHANNEL_NB_BITS<N) return -1;
  if (mask==NULL) return -1;

  nbchannels=N/CHANNEL_NB_BITS;

  val=0;
  J=Icmp;
  mi=0;
  for (i=0;i<nbchannels;i++)
  {
    channel=PIXEL(J,x,y,0);
    for (j=0;j<CHANNEL_NB_BITS;j++)
    {
      if (mask[mi]=='1')
      {
        val<<=1;
        val+=channel&1;
      }  
      channel>>=1;
      mi++;
    }
    J=J->next;
  }

  return val;
}


static void noise_bins_print(noise_code **bins,int bin)
{
  noise_code *code;
  if (bins==NULL) return;

  printf("Codes hashed into bin %d\n:",bin);
  code=bins[bin];
  while(code!=NULL)
  {
    printf("%d %d\n",code->x,code->y);
    code=code->next;
  }
}

static int noise_bins_count(noise_code **bins,int bin)
{
  int count;
  noise_code *code;
  if (bins==NULL) return -1;

  count=0;

  code=bins[bin];
  while(code!=NULL)
  {
    count++;
    code=code->next;
  }

  return count;
}

void noise_bins_stats(noise_code **bins,int bsize)
{
  int i;
  matrix *count;
  int hsize,hindex;
  matrix *hist;
  double avg,stdev,min,max;

  noise_code *code;
  if (bins==NULL) return;

  hsize=10000;
  hist=NULL;
  matAllocate(&hist,hsize,1);

  count=NULL;

  matAllocate(&count,bsize,1);

  for (i=0;i<bsize;i++)
  {
    count->values[i]=(double)(noise_bins_count(bins,i));
    hindex=noise_bins_count(bins,i);
    if (hindex>=hsize) hindex=hsize-1;
    hist->values[hindex]+=1.0;
  }
  avg=vectxAvg(count->values,bsize);
  stdev=vectxStdev(count->values,bsize);
  min=vectxMin(count->values,bsize);
  max=vectxMax(count->values,bsize);

  fprintf(stderr,"hstats={%f,%f,%f,%f};\n",avg,stdev,min,max);
  fprintf(stderr,"hist={");
  for (i=0;i<hsize;i++)
  {
    //fprintf(stderr,"%d",(int)(matGet(hist,i,0)+0.5));
    //if (i!=hsize-1) fprintf(stderr,",");
  }
  fprintf(stderr,"};\n");
  //printf("Stats of hash table:\n");
  //printf("Number of bins: %d\n",bsize);
  //printf("Average number of elements in each bin: %f (%f)\n",avg,stdev);
  //printf("Minimum and maximum number of elements in each bin: %f, %f\n",min,max);

  matFree(&hist);
  matFree(&count);
}

static int noise_bins_min(noise_code **bins,int bsize,int bin,int maxlen,imgu *Iproj,imgu *Icam,int x,int y,noise_code *match)
{
  int i;
  noise_code *code;
  int cost;
  int diff;

  if (bins==NULL) return -1;
  if (match==NULL) return -1;
  //if (bin<0) return -1;
  //if (bin>=bsize) return -1;

  code=bins[bin];
  match->x=x;
  match->y=y;
  //random match for initialization
  match->mx=(int)(matRandNumber()*Iproj->xs);
  match->my=(int)(matRandNumber()*Iproj->ys);
  match->cost=noise_cost(Iproj,match->mx,match->my,Icam,x,y);
  i=0;
  while(code!=NULL && (maxlen<0 || i<maxlen))
  {
    cost=noise_cost(Iproj,code->x,code->y,Icam,x,y);
    //we have found a better match!
    if (cost<match->cost)
    {
      match->mx=code->x;
      match->my=code->y;
      match->cost=cost;
    }
    i++;
    code=code->next;
  }

  return 0;
}

int noise_match_init(noise_match *mvars,unsigned char moption,int maxbinsize,imgu *Iproj,imgu *Icam)
{
  int i,j;
  int index;

  if (mvars==NULL) return -1;
  if (Iproj==NULL) return -1;
  if (Icam==NULL) return -1;

  noise_bit_count_init();
  
  mvars->nbframes=imguCount(Icam)*CHANNEL_NB_BITS;

  mvars->option=moption;
  mvars->maxbinsize=maxbinsize;

  mvars->pcodes=(noise_code *)(malloc(sizeof(noise_code)*Iproj->xs*Iproj->ys));
  mvars->ccodes=(noise_code *)(malloc(sizeof(noise_code)*Icam->xs*Icam->ys));
 
  if (mvars->option&NOISE_MATCH_HASH)
  {
      mvars->bin_mask=(char *)(malloc(sizeof(char)*(mvars->nbframes+1)));
#ifdef NOISE_VERBOSE
      fprintf(stderr,"Initializing hash table:\n");
#endif
      //number of bits to cover projector images
      mvars->psize=Iproj->xs*Iproj->ys;
      mvars->nbbits=log((double)(mvars->psize))/log(2.0)+1;
#ifdef NOISE_VERBOSE
      fprintf(stderr,"%d projector pixels.\n",mvars->psize);
      fprintf(stderr,"%d bits.\n",mvars->nbbits);
#endif
      mvars->hsize=1;
      //hash table size must be big enough to cover 2^nbbits
      mvars->hsize<<=mvars->nbbits;
#ifdef NOISE_VERBOSE
      fprintf(stderr,"hash table size %d.\n",mvars->hsize);
#endif
      mvars->hash_table=(noise_code **)(malloc(sizeof(noise_code *)*mvars->hsize));
      index=0;
      for (i=0;i<Iproj->ys;i++)
      {
          for (j=0;j<Iproj->xs;j++)
          {
              mvars->pcodes[index].x=j;
              mvars->pcodes[index].y=i;
              index++;
          }
      }
#ifdef NOISE_VERBOSE
      fprintf(stderr,"done.\n");
#endif
  }
  else
  {
#ifdef NOISE_VERBOSE
      fprintf(stderr,"Initializing sorting algorithm:\n");
#endif
      mvars->shuffle=(int *)(malloc(sizeof(int)*mvars->nbframes));
      mvars->Iproj_shuffle=NULL;
      mvars->Icam_shuffle=NULL;
#ifdef NOISE_VERBOSE
      fprintf(stderr,"done.\n");
#endif
  }

  return 0;
}

int noise_match_execute(noise_match *mvars,imgu *Iproj,imgu *Icam)
{
  int i,j,index;
  int bin_hash;
  noise_code match;
  struct timeval tv;
  double tick1,tick2;

  if (mvars==NULL) return -1;

  if (mvars->option&NOISE_MATCH_HASH)
  {
#ifdef NOISE_VERBOSE
      fprintf(stderr,"Filling hash table...");
#endif
      noise_generate_mask(mvars->bin_mask,mvars->nbbits,mvars->nbframes);
      //fprintf(stderr,"%s\n",bin_mask);
      for (i=0;i<mvars->hsize;i++) mvars->hash_table[i]=NULL;
      index=0;
      for (i=0;i<Iproj->ys;i++)
      {
          for (j=0;j<Iproj->xs;j++)
          {
              bin_hash=noise_hash(Iproj,j,i,mvars->bin_mask,mvars->nbframes);
              //add code to the front of list
              mvars->pcodes[index].next=mvars->hash_table[bin_hash];
              if (mvars->pcodes[index].next==NULL) mvars->pcodes[index].listlen=0;
              else mvars->pcodes[index].listlen=mvars->pcodes[index].next->listlen+1;
              mvars->hash_table[bin_hash]=&(mvars->pcodes[index]);
              index++;
          }
      }
      index=0;
      for (i=0;i<Icam->ys;i++)
      {
          for (j=0;j<Icam->xs;j++)
          {
              bin_hash=noise_hash(Icam,j,i,mvars->bin_mask,mvars->nbframes);
              mvars->ccodes[index].hash=bin_hash;
              index++;
          }
      }
#ifdef NOISE_VERBOSE
      noise_bins_stats(mvars->hash_table,mvars->hsize);
      fprintf(stderr,"done\n");
#endif
  }
  else
  {
#ifdef NOISE_VERBOSE
      fprintf(stderr,"Shuffling codes...");
#endif
      noise_generate_shuffle(mvars->shuffle,mvars->nbframes);
#ifdef NOISE_VERBOSE
      gettimeofday(&tv,NULL);
      tick1=tv.tv_sec+tv.tv_usec/1000000.0;
#endif
      noise_shuffle(&(mvars->Iproj_shuffle),Iproj,mvars->shuffle,mvars->nbframes);
#ifdef NOISE_VERBOSE
      gettimeofday(&tv,NULL);
      tick2=tv.tv_sec+tv.tv_usec/1000000.0;
      fprintf(stderr,"Time to shuffle %dx%d pixels: %f\n",Iproj->xs,Iproj->ys,tick2-tick1);
#endif
      noise_shuffle(&(mvars->Icam_shuffle),Icam,mvars->shuffle,mvars->nbframes);
#ifdef NOISE_VERBOSE
      gettimeofday(&tv,NULL);
      tick1=tv.tv_sec+tv.tv_usec/1000000.0;
      fprintf(stderr,"Time to shuffle %dx%d pixels: %f\n",Icam->xs,Icam->ys,tick1-tick2);
      fprintf(stderr,"done\n");
#endif
      index=0;
      for (i=0;i<Iproj->ys;i++)
      {
          for (j=0;j<Iproj->xs;j++)
          {
              mvars->pcodes[index].x=j;
              mvars->pcodes[index].y=i;
              mvars->pcodes[index].I=mvars->Iproj_shuffle;
              index++;
          }
      }
      index=0;
      for (i=0;i<Icam->ys;i++)
      {
          for (j=0;j<Icam->xs;j++)
          {
              mvars->ccodes[index].x=j;
              mvars->ccodes[index].y=i;
              mvars->ccodes[index].I=mvars->Icam_shuffle;
              index++;
          }
      }
#ifdef NOISE_VERBOSE
      fprintf(stderr,"Sorting codes and matching...");
#endif
      noise_sort_match(mvars->pcodes,Iproj->xs*Iproj->ys,mvars->ccodes,Icam->xs*Icam->ys);
#ifdef NOISE_VERBOSE
      fprintf(stderr,"done\n");
#endif
  }

  return 0;
}

int noise_match_get(noise_match *mvars,imgu *Iproj,imgu *Icam,int x,int y,noise_code *match)
{
  int bin_hash;
  int index;
  if (mvars==NULL) return -1;
  if (match==NULL) return -1;

  index=y*Icam->xs+x;

  if (mvars->option&NOISE_MATCH_HASH)
  {
      bin_hash=mvars->ccodes[index].hash;
      //if (bin_hash<0 || bin_hash>=mvars->hsize) return -1;
      noise_bins_min(mvars->hash_table,mvars->hsize,bin_hash,mvars->maxbinsize,Iproj,Icam,x,y,match);
  }
  else
  {
      if (index<0 || index>=Icam->xs*Icam->ys) return -1;
      match->x=mvars->ccodes[index].x;
      match->y=mvars->ccodes[index].y;
      match->cost=mvars->ccodes[index].cost;
      match->mx=mvars->ccodes[index].mx;
      match->my=mvars->ccodes[index].my;
  }
 
  return 0;
}

int noise_match_free(noise_match *mvars)
{
  if (mvars==NULL) return -1;

  if (mvars->pcodes!=NULL) free(mvars->pcodes);
  if (mvars->ccodes!=NULL) free(mvars->ccodes);

  if (mvars->option&NOISE_MATCH_HASH)
  {
    if (mvars->bin_mask!=NULL) free(mvars->bin_mask);
    if (mvars->hash_table!=NULL) free(mvars->hash_table);
  }
  else
  {
    if (mvars->shuffle!=NULL) free(mvars->shuffle);
    imguFreeMulti(&(mvars->Iproj_shuffle));
    imguFreeMulti(&(mvars->Icam_shuffle));
  }

  return 0;
}

int noise_match_backward_heuristic(imgu *Iproj,imgu *Icam,imgu *Imatch)
{
  int i,j;
  int mx,my;
  int ii,jj;
  int l,k;
  int cost;

  if (Iproj==NULL) return -1;
  if (Icam==NULL) return -1;
  if (Imatch==NULL) return -1;
  if (Icam->xs!=Imatch->xs) return -1;
  if (Icam->ys!=Imatch->ys) return -1;

  for (i=0;i<Imatch->ys;i++)
  {
    for (j=0;j<Imatch->xs;j++)
    {
      mx=(int)(COMPLEX(Imatch,j,i,0,0)+0.5);
      my=(int)(COMPLEX(Imatch,j,i,0,1)+0.5);
      for (l=-1;l<=1;l++)
      {
        for (k=-1;k<=1;k++)
        {
          //test neighborhood of camera pixel to know if they can also match the same matching projector pixel
          jj=j+k;
          ii=i+l;
          if (jj<0) jj=0;
          if (ii<0) ii=0;
          if (jj>=Icam->xs) jj=Icam->xs-1;
          if (ii>=Icam->ys) ii=Icam->ys-1;
          cost=noise_cost(Iproj,mx,my,Icam,jj,ii);
          if (cost<PIXEL(Imatch,jj,ii,0))
          {
            PIXEL(Imatch,jj,ii,0)=cost;
            COMPLEX(Imatch,jj,ii,0,0)=mx;
            COMPLEX(Imatch,jj,ii,0,1)=my;            
          }
        }
      }
    }
  }

  return 0;
}

int noise_match_forward_heuristic(imgu *Iproj,imgu *Icam,imgu *Imatch)
{
  int i,j;
  int mx,my;
  int ii,jj;
  int l,k;
  int cost;

  if (Iproj==NULL) return -1;
  if (Icam==NULL) return -1;
  if (Imatch==NULL) return -1;
  if (Icam->xs!=Imatch->xs) return -1;
  if (Icam->ys!=Imatch->ys) return -1;

  for (i=0;i<Imatch->ys;i++)
  {
    for (j=0;j<Imatch->xs;j++)
    {
      mx=(int)(COMPLEX(Imatch,j,i,0,0)+0.5);
      my=(int)(COMPLEX(Imatch,j,i,0,1)+0.5);
      for (l=-1;l<=1;l++)
      {
        for (k=-1;k<=1;k++)
        {
          //test if camera pixel can match neighborhood of matched projector pixel
          jj=mx+k;
          ii=my+l;
          if (jj<0) jj=0;
          if (ii<0) ii=0;
          if (jj>=Iproj->xs) jj=Iproj->xs-1;
          if (ii>=Iproj->ys) ii=Iproj->ys-1;
          cost=noise_cost(Iproj,jj,ii,Icam,j,i);
          if (cost<PIXEL(Imatch,j,i,0))
          {
            PIXEL(Imatch,j,i,0)=cost;
            COMPLEX(Imatch,j,i,0,0)=jj;
            COMPLEX(Imatch,j,i,0,1)=ii;            
          }
        }
      }
    }
  }

  return 0;
}

int noise_match_roundrip_check(imgu *Iproj,imgu *Icam,imgu *Imatch_c2p,imgu *Imatch_p2c,double maxdist)
{
  int i,j;
  int ii,jj;
  double dx,dy;
  double dist;

  if (Iproj==NULL || Iproj->data==NULL) return -1;
  if (Icam==NULL || Icam->data==NULL) return -1;
  if (Imatch_c2p==NULL || Imatch_c2p->data==NULL || Imatch_c2p->complex==NULL) return -1;
  if (Imatch_p2c==NULL || Imatch_p2c->data==NULL || Imatch_c2p->complex==NULL) return -1;
  if (Icam->xs!=Imatch_c2p->xs) return -1;
  if (Icam->ys!=Imatch_c2p->ys) return -1;
  if (Iproj->xs!=Imatch_p2c->xs) return -1;
  if (Iproj->ys!=Imatch_p2c->ys) return -1;

  for (i=0;i<Imatch_p2c->ys;i++)
  {
    for (j=0;j<Imatch_p2c->xs;j++)
    {
      if (COMPLEX(Imatch_p2c,j,i,0,0)<0) dist=-1.0;
      else
      {
        jj=(int)(COMPLEX(Imatch_p2c,j,i,0,0)+0.5);
        ii=(int)(COMPLEX(Imatch_p2c,j,i,0,1)+0.5);
        if (COMPLEX(Imatch_c2p,jj,ii,0,0)<0) dist=-1.0;
        else
        {
          dx=j-COMPLEX(Imatch_c2p,jj,ii,0,0);
          dy=i-COMPLEX(Imatch_c2p,jj,ii,0,1);            
          dist=sqrt(dx*dx+dy*dy);
        }
      }
      if (dist<0 || dist>maxdist)
      {
        COMPLEX(Imatch_p2c,j,i,0,0)=-0.1;
        COMPLEX(Imatch_p2c,j,i,0,1)=-0.1;
        //PIXEL(Imatch_p2c,j,i,0)=IMGU_MAXVAL;
        //PIXEL(Imatch_c2p,jj,ii,0)=IMGU_MAXVAL;
      }
    }
  }

  return 0;
}

