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

#include "playlist_parse.h"

char media_path[200];
double midi[128]; //last_time of each midi notes

#define MAX_NB_PLAYLISTS 16
#define MAX_NB_EVENTS 1000
int PL_ID=0;
int QUEUE_ID=0;

pl_event pl_events[MAX_NB_PLAYLISTS][MAX_NB_EVENTS];
char pl_name[MAX_NB_PLAYLISTS][PATH_SIZE];
int nb_events[MAX_NB_PLAYLISTS];
double end_times[MAX_NB_PLAYLISTS];

double live_fades[NB_DISP_VIDEO_LAYERS+1];
double live_volumes[NB_DISP_AUDIO_LAYERS+1];

double speaker_scale=1.0;
double stereo_distortion=STEREO_DISTORTION_DEFAULT;

int l_channel[NB_PLAYLIST_AUDIO_LAYERS];
int l_channel_grp[NB_PLAYLIST_AUDIO_LAYERS];
int l_channel_i[NB_PLAYLIST_AUDIO_LAYERS];
unsigned char channel_status[NB_AUDIO_CHANNELS];

#define OBS_ASIZE 6
//t_atom observer_position[OBS_ASIZE];
camera *cam=NULL;
vector4 snd_pos_world;

unsigned char vtoggle=0;
unsigned char ctoggle=0;
double time_offset=-LOAD_DELAY;
int refresh_frame=-1;
int prev_refresh_frame=-1;

//#define DEBUG

int set_media_path(char *path)
{
	strcpy(media_path,path);
}

int gettagid(char *tag)
{
	int i;
	if (tag==NULL) return -1;

	for (i=0;i<nb_events[PL_ID];i++)
	{
		if (strcmp(pl_events[PL_ID][i].tag,&(tag[1]))==0) return i;
	}

	return -1;
}

int send_string_safe_multi(t_playlist *x, char *msg)
{
	int i;

	for (i=0;i<MAX_NB_BMC_PORTS;i++)
	{
		if (x->bmc_send_used[i]!=0)
		{
			//printf("PD CMD SAFE (%d:%d) %s\n",i,x->bmc_send[i].destPort,msg);
			send_string_safe(&x->bmc_send[i],(unsigned char*)msg);
		}
	}

	return 0;
}

int send_string_unsafe_multi(t_playlist *x, char *msg)
{
	int i;

	for (i=0;i<MAX_NB_BMC_PORTS;i++)
	{
		if (x->bmc_send_used[i]!=0)
		{
			//printf("PD CMD UNSAFE (%d:%d) %s\n",i,x->bmc_send[i].destPort,msg);
			send_string_unsafe(&x->bmc_send[i],(unsigned char*)msg);
		}
	}

	return 0;
}

int parse_playlist(char *file)
{
	int k=1;
	char line[LINE_SIZE];
	char name[PATH_SIZE];
	char temp[PATH_SIZE];
	FILE *f;
	int linenum;
	unsigned char playlist_ref;
	int QUEUE_ID_LOCAL;

	playlist_ref=0;
	nb_events[QUEUE_ID]=0;
	time_offset=-LOAD_DELAY;

	if (QUEUE_ID<0 || QUEUE_ID>=MAX_NB_PLAYLISTS) return;

	f = fopen(file,"r");    
	if (f==NULL) {
		printf("unable to open file '%s'\n",file);
		return (-1);
	}

	strcpy(pl_name[QUEUE_ID],file);
	QUEUE_ID_LOCAL=QUEUE_ID;

	linenum=0;
	while (nb_events[QUEUE_ID_LOCAL]<MAX_NB_EVENTS)
	{
		linenum++;
		k = fscanf(f," %[^\n]",line);
		if (k!=1) break;
		if (line[0] == '#') continue; //commentaires
		if (line[0] == 'P')
		{
			if (sscanf(line,"P %s",
						temp)!=-1)
			{
				strcpy(name,media_path);
				strcat(name,temp);
				if (!playlist_ref) //first time a playlist ref was encountered
				{
					QUEUE_ID++;
					nb_events[QUEUE_ID_LOCAL]++;
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].real_end_time=0;
				}
				playlist_ref=1;
				parse_playlist(name);
			}
		}
		else
		{
			if (line[0] == 'I')
			{
				if (sscanf(line,"I %s %lg %lg %d %s %lg %s",
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				{
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='I';
					nb_events[QUEUE_ID_LOCAL]++;
				}
			} else if (line[0] == 'M')
			{
				if (sscanf(line,"M %s %lg %lg %d %s %lg %lg %s",
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				{
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='M';
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].prev_frame=-1;
					nb_events[QUEUE_ID_LOCAL]++;
				}             
			} else if (line[0] == 'F') 
			{
				if (sscanf(line,"F %s %lg %lg %d %lg %lg %lg %s",
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fade_min,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fade_max,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)

				{
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='F';
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].prev_fade=-1.0;
					nb_events[QUEUE_ID_LOCAL]++;
				}
			}
			else if (line[0] == 'A') 
			{
				if (sscanf(line,"A %s %lg %lg %d %s %lg %lg %lg %lg %s",
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_angle,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_dist,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_radius,
							&pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
							pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)                
				{
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='A';
					strcpy(name,pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name);
					strcpy(pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,media_path);
					strcat(pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,name);
					pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps=15;
					nb_events[QUEUE_ID_LOCAL]++;
				}
			}
			/*else if (line[0] == 'A') //old sound format
			  {
			  if (sscanf(line,"A %s %lg %d %s %lg %lg %lg %s",
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
			  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
			  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
			  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_angle,
			  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_dist,
			  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_radius,
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)                
			  {
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='A';
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps=3;
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time=0.0;
			  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time=1000000.0;
			  nb_events[QUEUE_ID_LOCAL]++;
			  }
			  }*/
			  else if (line[0] == 'B') 
			  {
				  if (sscanf(line,"B %s %lg %lg %d %lg %lg %lg %s",
							  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fade_min,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fade_max,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps,
							  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)                
				  {
					  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='B';
					  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].prev_fade=-1.0;
					  nb_events[QUEUE_ID_LOCAL]++;
				  }
			  }
			  else if (line[0] == 'C') //same as A but do not stop the sound when out of radius
			  {
				  if (sscanf(line,"C %s %lg %lg %d %s %lg %lg %lg %lg %s",
							  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_angle,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_dist,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].snd_radius,
							  &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
							  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)                
				  {
					  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='A';
					  strcpy(name,pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name);
					  strcpy(pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,media_path);
					  strcat(pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,name);
					  pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].fps=15;
					  nb_events[QUEUE_ID_LOCAL]++;
				  }
			  }
			  /*else if (line[0] == 'C') 
			    {
			    if (sscanf(line,"C %s %lg %lg %d %lg %s",
			    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
			    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
			    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
			    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
			    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
			    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)                
			    {
			    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='C';
			    nb_events[QUEUE_ID_LOCAL]++;
			    }
			    }*/ 
			    else if (line[0] == 'T')
			    {
				    if (sscanf(line,"T %s %lg %lg %d %s %lg %s",
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].len_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].layer,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].name,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_fade,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				    {
					    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='T';
					    nb_events[QUEUE_ID_LOCAL]++;
				    }
			    }
			    else if (line[0] == 'S')
			    {
				    if (sscanf(line,"S %s %lg %lg %lg %lg %lg %s",
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].stereo_B,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].stereo_R,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].stereo_distortion,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].stereo_slit,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				    {
					    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='S';
					    nb_events[QUEUE_ID_LOCAL]++;
				    }
			    } 
			    else if (line[0] == 'L')
			    {
				    if (sscanf(line,"L %s %lg %d %s",
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].lighting,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				    {
					    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='L';
					    nb_events[QUEUE_ID_LOCAL]++;
				    }
			    } 
			    else if (line[0] == 'D')
			    {
				    if (sscanf(line,"D %s %lg %d %s",
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].delay,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				    {
					    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='D';
					    nb_events[QUEUE_ID_LOCAL]++;
				    }
			    } 
			    else if (line[0] == 'O')
			    {
				    if (sscanf(line,"O %s %lg %lg %lg %s",
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].ref,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].start_time,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].goffset,
							    &pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].roffset,
							    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].tag)!=-1)
				    {
					    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]].type='O';
					    nb_events[QUEUE_ID_LOCAL]++;
				    }
			    } 
			    else
			    {
				    printf("commande non reconnue ligne %d, >%s<\n",linenum,line);
				    //exit(-1);
			    }
			    char *ref=pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].ref;
			    int t; //absolute start time
			    if ( !IS_MIDI_OR_REL(ref[0]) )
			    {
				    t = atoi(ref);
			    }
			    else
			    {
				    int id=gettagid(ref);
				    if (id==-1)
				    {
					    fprintf(stderr,"Media tag '%s' not found!\n",ref);
				    }
				    if (ref[0]=='>') t=pl_events[QUEUE_ID_LOCAL][id].real_end_time;
				    else t=pl_events[QUEUE_ID_LOCAL][id].real_start_time;
			    }

			    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].real_start_time=pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].start_time+t;
			    pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].real_end_time=pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].real_start_time+pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].len_time;
		}
		// initialise les champs dynamiques/volatiles
		pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].loaded=0;
		pl_events[QUEUE_ID_LOCAL][nb_events[QUEUE_ID_LOCAL]-1].running=0;
	}

	end_times[QUEUE_ID_LOCAL]=largest_time_playlist(QUEUE_ID_LOCAL);
	dump_playlist();
	printf("Playlist length (%d): %f sec.\n",QUEUE_ID_LOCAL,end_times[QUEUE_ID_LOCAL]);
	if (!playlist_ref) QUEUE_ID++;

	fclose(f);

	return (0);
}

void dump_playlist()
{
	int j;

	if (QUEUE_ID<0 || QUEUE_ID>=MAX_NB_PLAYLISTS) return;

	for (j=0;j<nb_events[QUEUE_ID];j++)
	{
		printf("id=%d type=%c tag= %s r = %s %lf %lf %d %s %f st = %lf et = %lf\n",
				QUEUE_ID,
				pl_events[QUEUE_ID][j].type,
				pl_events[QUEUE_ID][j].tag,
				pl_events[QUEUE_ID][j].ref,
				pl_events[QUEUE_ID][j].start_time,
				pl_events[QUEUE_ID][j].len_time,
				pl_events[QUEUE_ID][j].layer,
				pl_events[QUEUE_ID][j].name,
				pl_events[QUEUE_ID][j].fps,
				pl_events[QUEUE_ID][j].real_start_time,
				pl_events[QUEUE_ID][j].real_end_time);

	}
}

int exist_new_frame(double abs_d, double rel_d, int movien)
{
	return exist_new_frame_fps(abs_d,rel_d,pl_events[PL_ID][movien].real_start_time,pl_events[PL_ID][movien].fps);
}

int exist_new_frame_fps(double abs_d, double rel_d,double real_start_time,double fps)
{
	double start = real_start_time;
	double delay_inter_frame = 1/fps;

	double di = (abs_d-start)/delay_inter_frame;

	double dj = (abs_d-rel_d-start)/delay_inter_frame;

	if (di >= 0)
		if (floor(di)>floor(dj)) return di;
	return -1;
}

void init_playlist(void)
{
	int i;
	char message_pd[MSG_SIZE];

	for (i=0;i<=NB_DISP_VIDEO_LAYERS;i++)
	{
		live_fades[i]=1.0;
	}

	for (i=0;i<=NB_DISP_AUDIO_LAYERS;i++)
	{
		live_volumes[i]=1.0;
	}

	for (i=0;i<NB_PLAYLIST_AUDIO_LAYERS;i++)
	{
		l_channel[i]=-1;
		l_channel_grp[i]=-1;
		l_channel_i[i]=-1;
	}
	for (i=0;i<NB_AUDIO_CHANNELS;i++)
	{
		channel_status[i]=0;
	}

	for(i=0;i<MAX_NB_EVENTS;i++) {
		pl_events[PL_ID][i].loaded=0;
		pl_events[PL_ID][i].running=0;
		pl_events[PL_ID][i].done=0;
		pl_events[PL_ID][i].prev_frame=-1;
		pl_events[PL_ID][i].prev_fade=-1.0;
	}
	for(i=0;i<128;i++) midi[i] = BIG_TIME;

	PL_ID=0;
	QUEUE_ID=0;
	nb_events[PL_ID]=0;

	refresh_frame=-1;
	prev_refresh_frame=-1;
}

int update_position(t_playlist *x,char *kw,char *s)
{
	int i,k;
	double rx,ry,rz;
	int toggle,trigger;
	double yaw;
	matrix4 view_matrixT;
	char vm[MSG_SIZE];
	vector3 ptemp;
	double temp;

	sscanf(s," %d_%d_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg",&trigger,&toggle,&yaw,
			&(view_matrixT[0]),&(view_matrixT[1]),&(view_matrixT[2]),&(view_matrixT[3]),
			&(view_matrixT[4]),&(view_matrixT[5]),&(view_matrixT[6]),&(view_matrixT[7]),
			&(view_matrixT[8]),&(view_matrixT[9]),&(view_matrixT[10]),&(view_matrixT[11]),
			&(view_matrixT[12]),&(view_matrixT[13]),&(view_matrixT[14]),&(view_matrixT[15]));
	if (cam==NULL) camAllocate(&cam,NULL);
	if (cam!=NULL)
	{
		mat4Transpose(view_matrixT,cam->M);
		camSetExternalParamsFromMatrix(cam,cam->M);

		//vect3Print(cam->position);
		//if (cam->position[1]<0.5) cam->position[1]=0.5;
		//if (cam->position[1]>26.5) cam->position[1]=26.5;
		//backup original position
		vect3Copy(cam->orientation,ptemp);

		camSetExternalParams(cam,cam->position,cam->orientation);

		mat4Transpose(cam->M,view_matrixT);
		sprintf(vm,"%s %lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg_%lg",kw,
				view_matrixT[0],view_matrixT[1],view_matrixT[2],view_matrixT[3],
				view_matrixT[4],view_matrixT[5],view_matrixT[6],view_matrixT[7],
				view_matrixT[8],view_matrixT[9],view_matrixT[10],view_matrixT[11],
				view_matrixT[12],view_matrixT[13],view_matrixT[14],view_matrixT[15]);

#ifdef DEBUG
		fprintf(stderr,"%s\n",vm);
#endif
		send_string_unsafe_multi(x,vm);                

		//update image position (on top of model)
		//temp=ptemp[1];
		temp=yaw;
		//if (!toggle) temp=0;
		temp=fmod(temp,2*M_PI);
		if (temp<0) temp+=2*M_PI;
		temp/=2*M_PI;
		sprintf(vm,"scaletrans 0 1.0 1.0 %f 0.0",temp);
#ifdef DEBUG
		fprintf(stderr,"%s\n",vm);
		vect3Print(cam->position);
#endif
		send_string_unsafe_multi(x,vm);

		//load back original camera position
		vect3Copy(ptemp,cam->orientation);

		//adjust position and orientation for sound spatialization
		//Y is pointing upwards
		cam->position[1]=0.0;
		cam->orientation[0]=0.0;
		cam->orientation[2]=0.0;

		camSetExternalParams(cam,cam->position,cam->orientation);
		//mat4Print(cam->M);

		if (toggle)
		{
			if (stereo_distortion<0.5) stereo_distortion=1.0;
			else stereo_distortion=0.0;
			set_live_stereo(x,-1.0,-1.0,stereo_distortion,-1.0);
		}
	}
	//mat4ExtractRotations(cam.R,&rx,&ry,&rz);
	//mat4Print(cam.M);
	//vect3Print(cam.position);
	//fprintf(stderr,"R %f %f %f\n",rx,ry,rz);
	//fprintf(stderr,"OBS: %f %f %f\n",-cam.position[0],-cam.position[1],-ry);
	//SETSYMBOL(&(observer_position[0]), &s_obs_pos);
	//SETSYMBOL(&(observer_position[1]), &s_all);
	//SETSYMBOL(&(observer_position[2]), &s_all);
	//SETFLOAT(&(observer_position[3]), -cam.position[0]);
	//SETFLOAT(&(observer_position[4]), -cam.position[1]);
	//SETFLOAT(&(observer_position[5]), -ry);
	//outlet_list(x->out_msg_unsafe,&s_list,OBS_ASIZE,observer_position);
	/*outlet_float(x->out_msg_ry,-ry);
	  outlet_float(x->out_msg_y,-cam.position[1]);
	  outlet_float(x->out_msg_x,-cam.position[0]);*/

	return 0;
}

int set_live_fade(int i,float fade)
{
	if (i<0 || i>=NB_DISP_VIDEO_LAYERS) return -1;

#ifdef DEBUG
	//fprintf(stderr,"fade %d %f\n",i,fade);
#endif
	live_fades[i]=fade;

	return 0;
}

int set_live_volume(int i,float volume)
{
	if (i<0 || i>=NB_DISP_AUDIO_LAYERS) return -1;

#ifdef DEBUG
	//fprintf(stderr,"volume %d %f\n",i,volume);
#endif
	live_volumes[i]=volume;

	return 0;
}

int set_live_stereo(t_playlist *x,float B,float R,float distortion,float slit)
{
	char cmd[MSG_SIZE];

	stereo_distortion=distortion;

	if (distortion<0.5) printf("TRUE STEREO VIEW (%f)\n",distortion);
	else printf("OMNISTEREO VIEW (%f)\n",distortion);

	sprintf(cmd,"stereo %f %f %f %f",B,R,distortion,slit);
#ifdef DEBUG
	printf("%s\n",cmd);  
	fflush(stdout);
#endif
	send_string_safe_multi(x,cmd);  

	return 0;
}

int set_live_spk_scale(t_playlist *x,float spk_scale)
{
	char message_pd[MSG_SIZE];

	speaker_scale=spk_scale;
	sprintf(message_pd,"spk_scale all all %f",speaker_scale);
	pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);

	return 0;
}

int set_live_gamma(t_playlist *x,float gamma_r,float gamma_g,float gamma_b)
{
	char cmd[MSG_SIZE];

	sprintf(cmd,"gamma %f %f %f",gamma_r,gamma_g,gamma_b);
	send_string_safe_multi(x,cmd);   

	return 0;
}

void enable_vtoggle(t_playlist *x)
{
	vtoggle=1;
}

double live_compute_fade(int i,double fade)
{
	double res;
	if (i<0 || i>=NB_DISP_VIDEO_LAYERS) return fade*live_fades[0];

	res=fade*live_fades[0]*live_fades[i+1];

	return res;
}

double live_compute_volume(int i,float volume)
{
	double res;
	if (i<0 || i>=NB_DISP_AUDIO_LAYERS) return volume*live_volumes[0];

	res=volume*live_volumes[0]*live_volumes[i+1];

	return res;
}

void event_midi(int note, double time)
{
	midi[note]=time;
}

void resolve_playlist()
{
	int i;
	int change=1;
	double time;
	// reset les valides
	for(i=0;i<nb_events[PL_ID];i++) {
		pl_events[PL_ID][i].real_time_valid=(IS_MIDI_OR_REL(pl_events[PL_ID][i].ref[0])?0:1);
		pl_events[PL_ID][i].midi_change=0;
	}    
	while (change)
	{
		change=0;
		for(i=0;i<nb_events[PL_ID];i++)
		{
			char *ref = pl_events[PL_ID][i].ref;
			if( pl_events[PL_ID][i].real_time_valid ) continue;
			if ( IS_MIDI(ref[0]) )
			{
				//midi
				int note;
				sscanf(ref,"#%d",&note);
				if (midi[note]+pl_events[PL_ID][i].start_time>pl_events[PL_ID][i].real_start_time) pl_events[PL_ID][i].midi_change = 1;	
				pl_events[PL_ID][i].real_start_time=midi[note]+ pl_events[PL_ID][i].start_time;
				pl_events[PL_ID][i].real_end_time  =midi[note]+ pl_events[PL_ID][i].len_time; 
				pl_events[PL_ID][i].real_time_valid=1;
				change=1;                
			} else if ( IS_REL(ref[0]) ) {
				//ref
				int id = gettagid(ref+1);
				if( id<0 ) { printf("!!! Unknown tag [commande %d] '%s'\n",i,ref); continue; }
				if (!pl_events[PL_ID][id].real_time_valid) continue;
				if( pl_events[PL_ID][i].ref[0]=='<' )  time=pl_events[PL_ID][id].real_start_time;
				else				time=pl_events[PL_ID][id].real_end_time;
				//printf("resolve '%s' to time %f\n",pl_events[PL_ID][i].ref,time);
				pl_events[PL_ID][i].real_start_time= time + pl_events[PL_ID][i].start_time;
				pl_events[PL_ID][i].real_end_time= time + pl_events[PL_ID][i].len_time;
				pl_events[PL_ID][i].real_time_valid=1;
				change=1;
			}
		}
	}
}

double largest_time_playlist(int id)
{
	int i;
	double largest_time=0;

	if (id<0 || id>=MAX_NB_PLAYLISTS) return;

	for (i=0;i<nb_events[id];i++)
	{
		if (pl_events[id][i].real_end_time>largest_time) largest_time = pl_events[id][i].real_end_time;
	}
	return(largest_time);
}

// ajoute la string s au buffer de taille bs, a la position pos
// retourne la nouvelle position dans le buffer.
// ajoute un ';' si le buffer n'est pas a 0
int append(char *s,char *buf,int pos,int bs)
{
	int sz;
	sz=strlen(s);
	if( pos+sz+2>bs ) return(bs+1); // plus de place! (+2 a cause du ;)
	strcpy(buf+pos,s);
	// ajoute une string vide juste apres...
	buf[pos+sz+1]=0;
	return(pos+sz+1); // position juste apres le 0, en fait
}

void reset_running(t_playlist *x)
{
	int i;
	char cmd[MSG_SIZE];
	char message_pd[MSG_SIZE];

	for (i=0;i<nb_events[PL_ID];i++)
	{
		if ((pl_events[PL_ID][i].type=='M' || pl_events[PL_ID][i].type=='I' || pl_events[PL_ID][i].type=='T')
				&& pl_events[PL_ID][i].running)
		{
			sprintf(cmd,"unload %d",pl_events[PL_ID][i].layer);
			send_string_safe_multi(x,cmd);   
		}

		pl_events[PL_ID][i].running=0;
		pl_events[PL_ID][i].loaded=0;
		pl_events[PL_ID][i].done=0;
	}
	for(i=0;i<NB_PLAYLIST_AUDIO_LAYERS;i++)
	{
		if (l_channel[i]>=0)
		{
			sprintf(message_pd,"stop %d %d",l_channel_grp[i],l_channel_i[i]);
			pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);
			channel_status[l_channel[i]]=0;
			l_channel_grp[i]=-1;
			l_channel_i[i]=-1;
			l_channel[i]=-1;
		}
	}

	// reset le midi
	for(i=0;i<128;i++) midi[i] = BIG_TIME;

	PL_ID=0;
	QUEUE_ID=0;
	nb_events[PL_ID]=0;

	time_offset=-LOAD_DELAY;

	refresh_frame=-1;
	prev_refresh_frame=-1;
}

int findEmptyChannel(int layer)
{
	int i,ch;

	for (i=0;i<NB_AUDIO_CHANNELS;i++)
	{
		if (channel_status[i]==0)
		{
			channel_status[i]=1;
			ch=i;
			break;
		}
	}
	if (i==NB_AUDIO_CHANNELS) return -1;

	l_channel[layer]=ch;
	l_channel_grp[layer]=ch/16; //each channel group in pd has 16 channels
	l_channel_i[layer]=ch-16*l_channel_grp[layer];
	l_channel_grp[layer]++; //starts counting at 1 instead of 0
	l_channel_i[layer]++;

	return 0;
}

// intervale de temps t0..t1 -> trigger les commandes de cet intervalle
int trigger_commands(t_playlist *x,double t_prev,double t_curr)
{
	int i,j;
	int frame;
	int nb_frame;
	int layer;
	char message_pd[MSG_SIZE];
	char cmd[MSG_SIZE];
	double t0,t1;
	double radangle;
	double temp;

	t0=t_prev+time_offset;
	t1=t_curr+time_offset;

	message_pd[0]='\0';
	cmd[0]='\0';

	//resolve_playlist();

	for (i=0;i<nb_events[PL_ID];i++) {
		layer=pl_events[PL_ID][i].layer;
		if (pl_events[PL_ID][i].type=='S' && pl_events[PL_ID][i].real_start_time<=t1 && !pl_events[PL_ID][i].running) {
			set_live_stereo(x,pl_events[PL_ID][i].stereo_B,pl_events[PL_ID][i].stereo_R,pl_events[PL_ID][i].stereo_distortion,pl_events[PL_ID][i].stereo_slit);
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='L' && pl_events[PL_ID][i].real_start_time<=t1 && !pl_events[PL_ID][i].running) {
			sprintf(cmd,"lighting %d",pl_events[PL_ID][i].lighting);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='D' && pl_events[PL_ID][i].real_start_time<=t1 && !pl_events[PL_ID][i].running) {
			sprintf(cmd,"delay %d",pl_events[PL_ID][i].delay);
#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
#endif
			send_string_safe_multi(x,cmd);
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='O' && pl_events[PL_ID][i].real_start_time<=t1 && !pl_events[PL_ID][i].running) {
			sprintf(cmd,"offset %f %f",pl_events[PL_ID][i].goffset,pl_events[PL_ID][i].roffset);
#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
#endif
			send_string_safe_multi(x,cmd);
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='M' && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].loaded) {
			sprintf(cmd,"tile %d %s",layer,pl_events[PL_ID][i].name);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);

			pl_events[PL_ID][i].loaded=1;
		}
		if (pl_events[PL_ID][i].type=='M' && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].running) {
			sprintf(cmd,"fade %d %lf",layer,live_compute_fade(layer,pl_events[PL_ID][i].start_fade));
#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
#endif
			send_string_unsafe_multi(x,cmd);

			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='M' && pl_events[PL_ID][i].real_end_time<=t1 && pl_events[PL_ID][i].running) {
			sprintf(cmd,"unload %d",layer);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);                
			pl_events[PL_ID][i].running=0;
		}
		if (pl_events[PL_ID][i].type=='M' && pl_events[PL_ID][i].running && 
				(frame=exist_new_frame(t1,t1-t0,i))!=-1) {
			if (pl_events[PL_ID][i].prev_frame!=frame) 
			{
				pl_events[PL_ID][i].prev_frame=frame;
				sprintf(cmd,"frame %d %d",layer,frame);
#ifdef DEBUG
				printf("MFRAME: %s %f\n",cmd,t1);  
				fflush(stdout);
#endif
				send_string_unsafe_multi(x,cmd);  
			}
		}            
		if (pl_events[PL_ID][i].type=='F' && pl_events[PL_ID][i].real_start_time<=t1 && pl_events[PL_ID][i].done==0 && !pl_events[PL_ID][i].running)                
		{
#ifdef DEBUG
			printf("==> fade start (%d,%d)\n",pl_events[PL_ID][i].layer,i);  
			fflush(stdout);
#endif
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='F' && pl_events[PL_ID][i].running && 
				(frame=exist_new_frame(t1,t1-t0,i))!=-1) 
		{
			double f,fade;
			nb_frame=(pl_events[PL_ID][i].real_end_time-pl_events[PL_ID][i].real_start_time)*pl_events[PL_ID][i].fps;
			if (nb_frame!=0)
			{
				f = (double)frame/nb_frame;
				fade = pl_events[PL_ID][i].fade_min*(1-f)+pl_events[PL_ID][i].fade_max*f;
			}
			else
			{
				fade=pl_events[PL_ID][i].fade_max;
			}

			if (pl_events[PL_ID][i].prev_fade!=fade) 
			{
				pl_events[PL_ID][i].prev_fade=fade;

				sprintf(cmd,"fade %d %lf",layer,live_compute_fade(layer,fade));
//#ifdef DEBUG
				printf("FADE: %s\n",cmd);  
				fflush(stdout);
//#endif
				send_string_unsafe_multi(x,cmd);  
			}
		}
		if (pl_events[PL_ID][i].type=='F' && pl_events[PL_ID][i].real_end_time<=t1 && pl_events[PL_ID][i].running) {
#ifdef DEBUG
			printf("==> fade end (%d,%d)\n",pl_events[PL_ID][i].layer,i);  
			fflush(stdout);
#endif
			pl_events[PL_ID][i].running=0;
			pl_events[PL_ID][i].done=1;
		} 
		if ((pl_events[PL_ID][i].type=='A' || pl_events[PL_ID][i].type=='C') && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1) //update position
		{
			radangle=pl_events[PL_ID][i].snd_angle*M_PI/180.0;
                        //radangle+=M_PI;
                        //radangle=fmod(radangle,2.0*M_PI);
			snd_pos_world[0]=pl_events[PL_ID][i].snd_dist*sin(radangle);
			snd_pos_world[1]=0.0;
			snd_pos_world[2]=-pl_events[PL_ID][i].snd_dist*cos(radangle);
			snd_pos_world[3]=1.0;

			if (cam!=NULL)
			{
				//pl_events[PL_ID][i].position[0]=snd_pos_world[0]-cam->position[0];
				//pl_events[PL_ID][i].position[1]=snd_pos_world[1]-cam->position[1];
				//pl_events[PL_ID][i].position[2]=snd_pos_world[2]-cam->position[2];
				mat4MultiplyVector(cam->M,snd_pos_world,pl_events[PL_ID][i].position);
			}
			else
			{
				vectxCopy(snd_pos_world,pl_events[PL_ID][i].position,4);
			}
			//sound is 2D only fo r now
			pl_events[PL_ID][i].position[1]=pl_events[PL_ID][i].position[2];
			pl_events[PL_ID][i].position[2]=0;
#ifdef DEBUG
			printf("Sound update:\n");
			if (cam!=NULL) vect3Print(cam->position);
			vect3Print(snd_pos_world);
			vect3Print(pl_events[PL_ID][i].position);
#endif
		}
		if ((pl_events[PL_ID][i].type=='A' || pl_events[PL_ID][i].type=='C') && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].loaded && vectxNorm(pl_events[PL_ID][i].position,2)-pl_events[PL_ID][i].snd_radius<1.2*speaker_scale)
		{
			int nb_hp = 0;

			if (layer>=0 && layer<NB_PLAYLIST_AUDIO_LAYERS)
			{
				//find channel
				if (!findEmptyChannel(layer))
				{
					pl_events[PL_ID][i].loaded=1;

					sprintf(message_pd,"load %d %d open %s",l_channel_grp[layer],l_channel_i[layer],pl_events[PL_ID][i].name);
#ifdef DEBUG
					printf("%s\n",message_pd);  
					fflush(stdout);
#endif
					pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);

					//radangle=pl_events[PL_ID][i].snd_angle*M_PI/180.0;
					sprintf(message_pd,"snd_pos %d %d %f %f %f",l_channel_grp[layer],l_channel_i[layer],pl_events[PL_ID][i].snd_radius,pl_events[PL_ID][i].position[0],pl_events[PL_ID][i].position[1]);
//#ifdef DEBUG
					printf("%s\n",message_pd);  
					fflush(stdout);
//#endif
					pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);
				}
			}
		}
		if ((pl_events[PL_ID][i].type=='A' || pl_events[PL_ID][i].type=='C') && pl_events[PL_ID][i].real_start_time<=t1 && pl_events[PL_ID][i].real_end_time>t1 && pl_events[PL_ID][i].loaded && !pl_events[PL_ID][i].running)
		{
			if (layer>=0 && layer<NB_PLAYLIST_AUDIO_LAYERS)
			{
				if (l_channel[layer]!=-1)
				{
					pl_events[PL_ID][i].running=1;

					sprintf(message_pd,"start %d %d",l_channel_grp[layer],l_channel_i[layer]);
					//#ifdef DEBUG
					printf("sound start %s\n",pl_events[PL_ID][i].name);  
					fflush(stdout);
					//#endif
					pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);
				}
			}
		}
		if ((pl_events[PL_ID][i].type=='A' || pl_events[PL_ID][i].type=='C') && pl_events[PL_ID][i].running && (pl_events[PL_ID][i].real_end_time<=t1 ||  (pl_events[PL_ID][i].type=='A' && vectxNorm(pl_events[PL_ID][i].position,2)-pl_events[PL_ID][i].snd_radius>1.5*speaker_scale)))
		{
			if (layer>=0 && layer<NB_PLAYLIST_AUDIO_LAYERS)
			{ 
				if (l_channel[layer]!=-1)
				{
					pl_events[PL_ID][i].running=0;
					pl_events[PL_ID][i].loaded=0;
					sprintf(message_pd,"stop %d %d",l_channel_grp[layer],l_channel_i[layer]);
					//#ifdef DEBUG
					printf("sound stop %s\n",pl_events[PL_ID][i].name);  
					fflush(stdout);
					//#endif
					pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);

					channel_status[l_channel[layer]]=0;
					l_channel[layer]=-1;
					l_channel_grp[layer]=-1;
					l_channel_i[layer]=-1;
				}
			}
		}
		if ((pl_events[PL_ID][i].type=='A' || pl_events[PL_ID][i].type=='C') && pl_events[PL_ID][i].loaded && (frame=exist_new_frame(t1,t1-t0,i))!=-1) //send updated position, only if loaded
		{
			if (layer>=0 && layer<NB_PLAYLIST_AUDIO_LAYERS)
			{
				if (l_channel[layer]!=-1)
				{
					sprintf(message_pd,"snd_pos %d %d %f %f %f",l_channel_grp[layer],l_channel_i[layer],pl_events[PL_ID][i].snd_radius,pl_events[PL_ID][i].position[0],pl_events[PL_ID][i].position[1]);
#ifdef DEBUG
					printf("%s\n",message_pd);  
					fflush(stdout);
#endif
					pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);
				}
			}
		}
		if (pl_events[PL_ID][i].type=='B' && pl_events[PL_ID][i].real_start_time<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].running)                
		{
#ifdef DEBUG
			printf("==> sound fade start\n");  
			fflush(stdout);
#endif
			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='B' && pl_events[PL_ID][i].real_end_time<=t1 && pl_events[PL_ID][i].running) {
#ifdef DEBUG
			printf("==> sound fade end\n");  
			fflush(stdout);
#endif
			pl_events[PL_ID][i].running=0;
		} 
		if (pl_events[PL_ID][i].type=='B' && pl_events[PL_ID][i].running && 
				(frame=exist_new_frame(t1,t1-t0,i))!=-1)
		{
			if (layer>=0 && layer<NB_PLAYLIST_AUDIO_LAYERS)
			{ 
				if (l_channel[layer]!=-1)
				{	
					double f,fade;
					nb_frame=(pl_events[PL_ID][i].real_end_time-pl_events[PL_ID][i].real_start_time)*pl_events[PL_ID][i].fps;
					if (nb_frame!=0)
					{
						f = (double)frame/nb_frame;
						fade = pl_events[PL_ID][i].fade_min*(1-f)+pl_events[PL_ID][i].fade_max*f;
					}
					else
					{
						fade=pl_events[PL_ID][i].fade_max;
					}

					if (pl_events[PL_ID][i].prev_fade!=fade) 
					{
						pl_events[PL_ID][i].prev_fade=fade;

						sprintf(message_pd,"volume %d %d %lf",l_channel_grp[layer],l_channel_i[layer],live_compute_volume(pl_events[PL_ID][i].layer,fade));
#ifdef DEBUG
						printf("%s\n",message_pd);
						fflush(stdout);
#endif
						pdSend(&(x->pd_snd_channels),PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);
					}
				}
			}
		}
		//////// images /////////
		if (pl_events[PL_ID][i].type=='I' && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].loaded) {
			sprintf(cmd,"image %d %s",layer,pl_events[PL_ID][i].name);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);

			pl_events[PL_ID][i].loaded=1;
		}
		if (pl_events[PL_ID][i].type=='I' && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].running) {
			sprintf(cmd,"fade %d %lf",layer,live_compute_fade(layer,pl_events[PL_ID][i].start_fade));
#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
#endif
			send_string_safe_multi(x,cmd);

			pl_events[PL_ID][i].running=1;
		}
		if (pl_events[PL_ID][i].type=='I' && pl_events[PL_ID][i].real_end_time<=t1 && pl_events[PL_ID][i].running) {
			sprintf(cmd,"unload %d",layer);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);                
			pl_events[PL_ID][i].running=0;
		}
		if (pl_events[PL_ID][i].type=='T' && pl_events[PL_ID][i].real_start_time-LOAD_DELAY<=t1 && pl_events[PL_ID][i].real_end_time>t1 && !pl_events[PL_ID][i].loaded) {
			sprintf(cmd,"model %d %s",layer,pl_events[PL_ID][i].name);
			//#ifdef DEBUG
			printf("%s\n",cmd);  
			fflush(stdout);
			//#endif
			send_string_safe_multi(x,cmd);

			pl_events[PL_ID][i].loaded=1;
		}
		if (pl_events[PL_ID][i].type=='M' && pl_events[PL_ID][i].running && 
				(frame=exist_new_frame(t1,t1-t0,i))!=-1) {
			if (pl_events[PL_ID][i].prev_frame!=frame) 
			{
				pl_events[PL_ID][i].prev_frame=frame;
				sprintf(cmd,"frame %d %d",layer,frame);
#ifdef DEBUG
				printf("MFRAME: %s %f\n",cmd,t1);  
				fflush(stdout);
#endif
				send_string_unsafe_multi(x,cmd);  
			}
		}            
		if (pl_events[PL_ID][i].type=='T' && pl_events[PL_ID][i].running &&
				(frame=exist_new_frame(t1,t1-t0,i))!=-1)
		{
			if (vtoggle)
			{
				if (pl_events[PL_ID][i].prev_frame!=frame) 
				{
					pl_events[PL_ID][i].prev_frame=frame;
					sprintf(cmd,"vtoggle %d",ctoggle);
					send_string_unsafe_multi(x,cmd);
					if (ctoggle) ctoggle=0;
					else ctoggle=1;
				}
			}
		}            
		if (pl_events[PL_ID][i].type=='T' && pl_events[PL_ID][i].running ) {
			if ((int)t1>(int)t0)
			{
				sprintf(cmd,"time %lf",t1);
#ifdef DEBUG
				printf("T: %s\n",cmd);  
				fflush(stdout);
#endif
				send_string_unsafe_multi(x,cmd);
			}
			if (pl_events[PL_ID][i].midi_change) 
			{
				sprintf(cmd,"unload %d",layer);
#ifdef DEBUG
				printf("%s\n",cmd);  
				fflush(stdout);
#endif
				sprintf(cmd,"model %d %s",layer,pl_events[PL_ID][i].name);
#ifdef DEBUG
				printf("%s\n",cmd);  
				fflush(stdout);
#endif
			}
		}
	}

	//control refresh rate of remote player
	/*refresh_frame=exist_new_frame_fps(t1,t1-t0,0.0,2.0);
	if (refresh_frame!=prev_refresh_frame)
	{
		refresh_frame=prev_refresh_frame;
		sprintf(cmd,"refresh %f",t1);
#ifdef DEBUG
		printf("%s\n",cmd);  
		fflush(stdout);
#endif
		send_string_unsafe_multi(x,cmd);
	}*/

	if (nb_events[PL_ID]!=0 && t1>end_times[PL_ID])
	{
		printf("FINISHED PLAYING(%d): %s\n",PL_ID,pl_name[PL_ID]);
		time_offset=time_offset-end_times[PL_ID]-LOAD_DELAY-2.0; //2 seconds between playlists
		//printf("TIME OFFSET: %f\n",time_offset);
		for (i=0;i<nb_events[PL_ID];i++)
		{
			pl_events[PL_ID][i].loaded=0;
			pl_events[PL_ID][i].running=0;
			pl_events[PL_ID][i].done=0;
                        pl_events[PL_ID][i].prev_fade=-1.0;
		}  
		PL_ID=(PL_ID+1)%QUEUE_ID;
		printf("STARTING PLAYING(%d): %s\n",PL_ID,pl_name[PL_ID]);
	}

	return 0;
}






