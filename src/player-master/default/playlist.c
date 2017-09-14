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

#include <sys/types.h>
#include <string.h>

#include <stdio.h>

#include <stdlib.h>

#include "playlist_parse.h"

//#define VERBOSE

#define ENABLE_JOYSTICK

static t_class *playlist_class;

static void playlist_reset(t_playlist *x)
{
	printf("reset\n");

	// considere tous le layers 'not running' et envoie un unload si necessaire
	reset_running(x);
}

// set le temps
static void playlist_time(t_playlist *x,float t1)
{
int pos,i;
int k;
	x->t1=t1;

	// vide les notes midi en attente
	for(i=0;i<x->nb_midi;i++) event_midi(x->midi_notes[i],t1);
	x->nb_midi=0;

	// execute actions for time interval t0..t1
	// si le temps est a l'envers, on ne fait rien...
	if( x->t0 > x->t1 ) {
		x->t0=x->t1;
		return;
	}

#ifdef VERBOSE
	printf("exec time=%12.6f - %12.6f\n",x->t0,x->t1);
#endif

	trigger_commands(x,(double)x->t0,(double)x->t1);

	// reset time for next timestep
	x->t0=x->t1;

	//message_doit(x,"bonjour ceci est un test; 2 3 4; allo 45");
	//message_doit(x,"un dernier; un autre");
}

// called when a new box is created
// we can give a file name and a grain
static void *playlist_new(void)
{
    int i;
    char cmd[100];
	t_playlist *x = (t_playlist *)pd_new(playlist_class);

	x->t0=0; // current time at start. need flag

	// les inlet pour les midi et autres bidules
    //inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("t"));
    //inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("v"));

	x->out_msg_unsafe=outlet_new(&x->x_obj, &s_float);
	x->out_msg_safe=outlet_new(&x->x_obj, &s_float);
    x->out_msg_local=outlet_new(&x->x_obj, &s_float);

    x->bbuf= binbuf_new();

	// pas de midi en attente
	x->nb_midi=0;

#ifdef VERBOSE
	printf("playlist new\n");
#endif

    for (i=0;i<MAX_NB_BMC_PORTS;i++)    
    {
      //bmc_send[i]=NULL;
      x->bmc_send_used[i]=0;
    }
    x->pd_snd_channels=NULL;
    x->pd_main_ctrl=NULL;

	/// initialisation avec des statics.. (?)
	init_playlist();

	return (x);
}


// une note midi in
static void playlist_notein_2(t_playlist *x,t_float note,t_float touch)
{
	printf("notein! note=%f touch=%f\n",note,touch);
}

static void playlist_notein_1(t_playlist *x,t_float note)
{
	printf("notein! %d note=%f\n",x->nb_midi,note);
	// ajoute le midi seulement si il y a de la place...
	if( x->nb_midi<MAX_MIDI-1 ) {
		x->midi_notes[x->nb_midi++]=(int)(note+0.5);
	}
}

static void playlist_load_playlist(t_playlist *x,t_symbol *s)
{
	printf("load playlist: %s\n",s->s_name);

	if (parse_playlist(s->s_name)!=0) {
		printf("error loading playlist...\n");
	}
}

static void playlist_media_path(t_playlist *x,t_symbol *s)
{
	printf("media path: %s\n",s->s_name);

    set_media_path(s->s_name);
}

static void playlist_camera(t_playlist *x,t_symbol *kw,t_symbol *s)
{
	//printf("camera: %s %s\n",kw->s_name,s->s_name);
    update_position(x,kw->s_name,s->s_name);
}

static void playlist_live_fade(t_playlist *x,t_float layer,t_float fade)
{
    int i;
//printf("live fade %f %f\n",layer,fade);

    i=(int)(layer+0.5);
    set_live_fade(i,fade);
}

static void playlist_live_volume(t_playlist *x,t_float layer,t_float volume)
{
    int i;
//printf("live volume %f %f\n",layer,volume);

    i=(int)(layer+0.5);
    set_live_volume(i,volume);
}

static void playlist_live_stereo(t_playlist *x,t_float B,t_float R,t_float distortion,t_float slit)
{
//printf("PD STEREO: %f %f %f %f %f\n",B,R,scene_scale,distortion,slit);
    set_live_stereo(x,B,R,distortion,slit);
}

static void playlist_live_spk_scale(t_playlist *x,t_float speaker_scale)
{
//printf("PD STEREO: %f\n",speaker_scale);
    set_live_spk_scale(x,speaker_scale);
}

static void playlist_live_gamma(t_playlist *x,t_float gamma_r,t_float gamma_g,t_float gamma_b)
{
    set_live_gamma(x,gamma_r,gamma_g,gamma_b);
}

static void playlist_port(t_playlist *x,t_float port)
{
    int i,iport;
//printf("live volume %f %f\n",layer,volume);

    iport=(int)(port+0.5);
    for (i=0;i<MAX_NB_BMC_PORTS;i++)
    {
      if (x->bmc_send_used[i]==0)
      {
printf("PD PORT: %d\n",iport);
        x->bmc_send_used[i]=1;
        //bmc_send[i]=(bimulticast *)(malloc(sizeof(bimulticast)));
        init_bimulticast_controller(&x->bmc_send[i], iport, 1);
        break;
      }
      else if (x->bmc_send[i].destPort==iport) 
      {
        break;
      }
    }
}

static void playlist_vtoggle(t_playlist *x)
{
    enable_vtoggle(x);
}

// the box was destroyed
static void playlist_free(t_playlist *x)
{
    int i;

    for (i=0;i<MAX_NB_BMC_PORTS;i++)
    {
      if (x->bmc_send_used[i]!=0)
      {
        uninit_bimulticast(&x->bmc_send[i]);
        //free(bmc_send[i]);
        //bmc_send[i]=NULL;
      }
    }

	pdClose(&(x->pd_snd_channels));
	pdClose(&(x->pd_main_ctrl));

	printf("playlist free!!!\n");
}

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////


void playlist_setup(void)
{
	playlist_class = class_new(gensym("playlist"),
		(t_newmethod)playlist_new, (t_method)playlist_free,
		sizeof(t_playlist), CLASS_DEFAULT, 0);

	//class_addbang(playlist_class, (t_method)playlist_bang);
	class_addfloat(playlist_class, (t_method)playlist_time);
	//class_addlist(playlist_class, (t_method)playlist_list);
	//class_addmethod(playlist_class, (t_method)playlist_float_d,gensym("d"),A_FLOAT,0);
	//class_addmethod(playlist_class, (t_method)playlist_notein_2,gensym("notein"),A_FLOAT,A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_notein_1,gensym("notein"),A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_load_playlist,gensym("load"),A_SYMBOL,0);
	class_addmethod(playlist_class, (t_method)playlist_live_fade,gensym("fade"),A_FLOAT,A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_live_volume,gensym("volume"),A_FLOAT,A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_media_path,gensym("path"),A_SYMBOL,0);
	class_addmethod(playlist_class, (t_method)playlist_live_stereo,gensym("stereo"),A_FLOAT,A_FLOAT,A_FLOAT,A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_live_spk_scale,gensym("spk_scale"),A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_live_gamma,gensym("gamma"),A_FLOAT,A_FLOAT,A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_camera,gensym("camera"),A_SYMBOL,A_SYMBOL,0);
	class_addmethod(playlist_class, (t_method)playlist_port,gensym("port"),A_FLOAT,0);
	class_addmethod(playlist_class, (t_method)playlist_vtoggle,gensym("vtoggle"),0);
	class_addmethod(playlist_class, (t_method)playlist_reset,gensym("reset"),0);
    //class_addmethod(playlist_class, (t_method)playlist_stop, gensym("stop"),0);
	printf("playlist setup!!!\n");
}


