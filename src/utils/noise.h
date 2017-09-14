#ifndef NOISE_H
#define NOISE_H

#include <stdlib.h>
#include <string.h>
#include <imgu/imgu.h>

#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define CHANNEL_NB_BITS sizeof(pix_t)*8
#define CHANNEL_NB_BITS 16

#define NOISE_X 0
#define NOISE_Y 1

#define NOISE_COST_INF IMGU_MAXVAL

//#define NOISE_VERBOSE

#define NOISE_MATCH_HASH 0x1
#define NOISE_MATCH_SORT 0x2

typedef struct noise_next{
  int x,y;
  int cost;
  int hash;
  char *code;
  int mx,my; //match position, needed for sort algorithm
  imgu *I; //pointer to image code, needed for sort algorithm
  int listlen; //length of list, internal variable for hash table
  unsigned char duplicate;
  struct noise_next *next;
}noise_code;

typedef struct{
    int psize,nbbits;
    int nbframes;
    int maxbinsize;
    unsigned char option;

    noise_code *pcodes;
    noise_code *ccodes;

    //hash table
    char* bin_mask;
    int hsize;
    noise_code **hash_table;

    //sort variables
    int *shuffle;
    imgu *Iproj_shuffle;
    imgu *Icam_shuffle;
}noise_match;

int noise_texture_2d(imgu **I,int sx,int sy,double alpha,double min_freq,double max_freq,unsigned char binarize,unsigned char blur);
int noise_texture_1d(imgu **I,int sx,int sy,double alpha,double min_freq,double max_freq,unsigned char binarize,unsigned char blur,int dim);

//compress noise in list of imgu images of 1 channel where each channel is 16 patterns (1 per bit)
int noise_compress(imgu **Icmp,imgu *I);
int noise_compress_pattern(imgu **Icmp,imgu *I,int index);
int noise_decompress_pattern_at(imgu *Icmp,int pat,int x,int y);
int noise_decompress_pattern(imgu **I,imgu *Icmp,int index);
int noise_decompress_patterns(imgu **I,imgu *Icmp);

int noise_count_total(imgu *Icmp);
int noise_print(imgu *Icmp,int x,int y);
int noise_bit_count_init();
int noise_cost(imgu *Iproj,int x1,int y1,imgu *Icam,int x2,int y2);
void noise_bins_stats(noise_code **bins,int bsize);

int noise_compare( const void *a, const void *b );

int noise_match_init(noise_match *mvars,unsigned char moption,int maxbinsize,imgu *Iproj,imgu *Icam);
int noise_match_execute(noise_match *mvars,imgu *Iproj,imgu *Icam);
int noise_match_get(noise_match *mvars,imgu *Iproj,imgu *Icam,int x,int y,noise_code *match);
int noise_match_free(noise_match *mvars);

int noise_match_backward_heuristic(imgu *Iproj,imgu *Icam,imgu *Imatch);
int noise_match_forward_heuristic(imgu *Iproj,imgu *Icam,imgu *Imatch);

int noise_match_roundrip_check(imgu *Iproj,imgu *Icam,imgu *Imatch_c2p,imgu *Imatch_p2c,double maxdist);

#ifdef __cplusplus
}
#endif

#endif
