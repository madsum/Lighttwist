
/* network using bmc */

#include <m_pd.h>

#include <sys/types.h>
#include <string.h>

#include <stdio.h>

#include <bmc/bmc.h>
#include "remote/ltports.h"

static t_class *mcnetsend_class;

typedef struct _mcnetsend
{
    t_object x_obj;
    int open; /* 0=not open for sending, 1=open for sending */
    bimulticast bmc;
} t_mcnetsend;


// called when a new box is created
static void *mcnetsend_new(void)
{
	t_mcnetsend *x = (t_mcnetsend *)pd_new(mcnetsend_class);
	outlet_new(&x->x_obj, &s_float);
	// not open!
	x->open=0;
	printf("mcnetsend new!!!\n");
	return (x);
}

/// message : disconnect
static void mcnetsend_disconnect(t_mcnetsend *x)
{
	if( x->open==0 ) {
		printf("mcnetsend : already disconnected\n");
		return;
	}
	uninit_bimulticast(&x->bmc);
	x->open=0; // not connected anymore!
    	outlet_float(x->x_obj.ob_outlet, 0);
	printf("mcnetsend: disconnect!!!\n");
}

/// message : connect
static void mcnetsend_connect(t_mcnetsend *x,
    t_float port, t_float loopback)
{
int k;
int iport=(int)(port+0.5);
int iloopback=(int)(loopback+0.5);
	if( x->open==1 ) {
		printf("Need a disconnect...\n");
		mcnetsend_disconnect(x);
	}
	if( iport<=0 ) iport=PLAYER_PORT;
	printf("mcnetsend: connect to port %d loopback=%d\n",iport,iloopback);
	k=init_bimulticast_controller(&x->bmc,iport,iloopback); // loopback : 0=no
	if( k ) {
		error("error opening connection!");
		x->open=0;
	}else{
		// We are connected! -> send 1
		x->open=1;
	}
	outlet_float(x->x_obj.ob_outlet, x->open);
}


/// message : send
/// Attention: le message n'est necessairement termine par 0...
/// Par contre, il est TOUJOURS termine par un \n ou un ';'
static void mcnetsend_send(t_mcnetsend *x, t_symbol *s, int argc, t_atom *argv)
{
	if( x->open ) {
		t_binbuf *b = binbuf_new();
		char *buf, *bp;
		int length, sent;
		t_atom at;

		binbuf_add(b, argc, argv);

		SETSEMI(&at);
		binbuf_add(b, 1, &at);

		binbuf_gettext(b, &buf, &length);

		// we replace the trailing \n by 0 and send it
		buf[length-1]=0;
		//printf("mcnetsend: Sending '%s' len=%d\n",buf,length);

		send_data_unsafe(&x->bmc,(unsigned char *) buf,length);


		t_freebytes(buf, length);
		binbuf_free(b);
	}
	else error("mcnetsend: cannot send, not connected!");
}

/// message : sendsafe
/// Attention: le message n'est necessairement termine par 0...
/// Par contre, il est TOUJOURS termine par un \n
static void mcnetsend_sendsafe(t_mcnetsend *x, t_symbol *s, int argc, t_atom *argv)
{
	if( x->open ) {
		t_binbuf *b = binbuf_new();
		char *buf, *bp;
		int length, sent;
		t_atom at;

		binbuf_add(b, argc, argv);

		SETSEMI(&at);
		binbuf_add(b, 1, &at);

		binbuf_gettext(b, &buf, &length);

		// we send the ending 0
		buf[length-1]=0;
		//printf("mcnetsend: Sending '%s' len=%d\n",buf,length);

		//printf("MCNETSEND: about to send '%s'\n",buf);
		send_data_safe(&x->bmc,(unsigned char *) buf,length);
		//printf("MCNETSEND: done sending '%s'\n",buf);


		t_freebytes(buf, length);
		binbuf_free(b);
	}
	else error("mcnetsend: cannot send, not connected!");
}


/// message : send to an accumulation buffer
/// Attention: le message n'est necessairement termine par 0...
/// Par contre, il est TOUJOURS termine par un \n
static void mcnetsend_accumulate(t_mcnetsend *x, t_symbol *s, int argc, t_atom *argv)
{
	if( x->open ) {
		t_binbuf *b = binbuf_new();
		char *buf, *bp;
		int length, sent;
		t_atom at;

		binbuf_add(b, argc, argv);

		SETSEMI(&at);
		binbuf_add(b, 1, &at);

		binbuf_gettext(b, &buf, &length);

		buffer_data(&x->bmc,(unsigned char *) buf,length);

		/***
		buf[length-1]=0;
		printf("mcnetsend: Sending '%s' len=%d\n",buf,length);
		***/

		t_freebytes(buf, length);
		binbuf_free(b);
	}
	else error("mcnetsend: cannot send, not connected!");
}

static void mcnetsend_flush(t_mcnetsend *x, t_symbol *s, int argc, t_atom *argv)
{
	buffer_flush(&x->bmc);
}

// the box was destroyed
static void mcnetsend_free(t_mcnetsend *x)
{
	if( x->open ) {
		mcnetsend_disconnect(x);
	}
	printf("mcnetsend: free!!!\n");
}

void mcnetsend_setup(void)
{
    mcnetsend_class = class_new(gensym("mcnetsend"),
	(t_newmethod)mcnetsend_new,
	(t_method)mcnetsend_free,
	sizeof(t_mcnetsend), 0, 0);

    class_addmethod(mcnetsend_class, (t_method)mcnetsend_connect,
	gensym("connect"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(mcnetsend_class, (t_method)mcnetsend_disconnect,
	gensym("disconnect"), 0);
    class_addmethod(mcnetsend_class, (t_method)mcnetsend_send,
	gensym("send"), A_GIMME, 0);
    class_addmethod(mcnetsend_class, (t_method)mcnetsend_sendsafe,
	gensym("sendsafe"), A_GIMME, 0);
    class_addmethod(mcnetsend_class, (t_method)mcnetsend_accumulate,
	gensym("accumulate"), A_GIMME, 0);
    class_addmethod(mcnetsend_class, (t_method)mcnetsend_flush,
	gensym("flush"), A_GIMME, 0);
}




