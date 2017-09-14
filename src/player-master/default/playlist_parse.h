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

#ifndef OSG_PLAYLIST_H
#define OSG_PLAYLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <m_pd.h>

#include <sys/file.h>
#include <string.h>
#include <math.h>
#include <bmc/bmc.h>
#include <utils/pd_send.h>
#include <utils/entry.h>
#include "remote/ltports.h"

#include <imgu/imgu.h>

#define BIG_TIME 999999

#define LOAD_DELAY 2.0

#define MAX_NB_BMC_PORTS 8

#define NB_DISP_VIDEO_LAYERS 3 //video,3d and images
#define NB_DISP_AUDIO_LAYERS 8 //nb of layers in lighttwist interface. playlists can have more than NB_AUDIO_LAYERS layers if written manually (even >NB_AUDIO_CHANNELS)
#define NB_DISP_LAYERS 11 //MAX_NB_VIDEO_LAYERS+MAX_NB_AUDIO_LAYERS
#define NB_PLAYLIST_AUDIO_LAYERS 256 //maximum nb of audio layers in a playlist file
#define NB_AUDIO_CHANNELS 16 //nb of pd audio channels

#define STEREO_B_DEFAULT 6.5
#define STEREO_R_DEFAULT 230.0
#define STEREO_DISTORTION_DEFAULT 1.0
#define STEREO_SLIT_DEFAULT 180.0
#define SPEAKER_SCALE_DEFAULT 1.0

#define PD_CONTROLLER_IP "localhost"

//// test les types de reference...

#define IS_MIDI(c)	(c=='#')
#define IS_REL(c)	(c=='<' || c=='>')
#define IS_MIDI_OR_REL(c)	(IS_MIDI(c) || IS_REL(c))

#define MAX_MIDI	128

typedef struct _playlist
{
    t_object x_obj;
    float t0;  // last time (in sec)
    float t1;  // new  time (in sec)

    t_outlet *out_msg_unsafe;
    t_outlet *out_msg_safe;
    t_outlet *out_msg_local;

    t_binbuf *bbuf;
    
	//// table des touches midi recues depuis le dernie _time
    int nb_midi;
    int midi_notes[MAX_MIDI];

    bimulticast bmc_send[MAX_NB_BMC_PORTS];
    unsigned char bmc_send_used[MAX_NB_BMC_PORTS];
    tcpcast *pd_snd_channels;
    tcpcast *pd_main_ctrl;
} t_playlist;

typedef struct {
    double start_time;
    double len_time;
    char ref[30];
    double real_start_time;
    double real_end_time;
    int real_time_valid; // utilise seulement dans resolve_playlist()
    char tag[30];
    double fps;
    int layer;
    char name[PATH_SIZE];   
    int running; // 0=pas actif, 1=actif (le film joue)
    int loaded; // for sound and movie preloading
    int done; // for fades of length 0
    int delay; //frame delay for right eye
    double goffset;
    double roffset;
    double fade_min;
    double fade_max;
    double start_fade;
    double snd_angle;
    double snd_dist;
    double snd_radius;
    vector4 position;
    int midi_change; //pour savoir si on achange la valeur de start_time
    char type; // 'Z', 'F', 'I'

    long int prev_frame;
    double prev_fade;

    double stereo_B;
    double stereo_R;
    double stereo_distortion;
    double stereo_slit;

    int lighting;
} pl_event;

int get_tag_id(char *);
void event_midi(int note, double time);
void resolve_playlist(void);
int set_media_path(char *path);
int parse_playlist(char *file);
int set_live_fade(int i,float fade);
int set_live_volume(int i,float volume);
int gettagid(char *tag);
int set_live_stereo(t_playlist *x,float B,float R,float distortion,float slit);
int set_live_spk_scale(t_playlist *x,float spk_scale);
int set_live_gamma(t_playlist *x,float gamma_r,float gamma_g,float gamma_b);
void enable_vtoggle(t_playlist *x);
void dump_playlist();
void init_playlist(void);
double largest_time_playlist(int id);

int update_position(t_playlist *x,char *kw,char *s);
int trigger_commands(t_playlist *x,double t_prev,double t_curr);

// reset all running states to not running
void reset_running(t_playlist *x);

#endif
