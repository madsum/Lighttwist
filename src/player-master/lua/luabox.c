
/* lua box that can contains a lua interpreter */
/* there is only a parameter to the box, which is a file to execute on startup */
/* anything that comes in the box is sent to the lua interpreter */

#include <m_pd.h>

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

#include <bmc/bimulticast.h>
#include "remote/ltports.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


t_class *luabox_class;

typedef struct _luabox
{
    t_object x_obj;

    int x_nbOut;
    t_outlet **x_outlet; // [nbOut]

    lua_State *x_L; // lua interpreter
} t_luabox;

// defini dans m_sched.c de pd
#define TIMEUNITPERSEC (32.*441000.)

//
// set global variables (only one: the pd time)
//
update_time (lua_State *L)
{
	lua_pushnumber(L,(double)clock_getlogicaltime()/TIMEUNITPERSEC);
	lua_setglobal(L,"time");
}


// called when a new box is created
//
// argument: nombre d'output
//
void *luabox_new(t_symbol *s, int argc, t_atom *argv)
{
int i;
	t_luabox *x = (t_luabox *)pd_new(luabox_class);

	x->x_nbOut=0;
	if( argc>=1 && argv[0].a_type==A_FLOAT )  x->x_nbOut=(int)(argv[0].a_w.w_float+0.5);

	//printf("allocating %d outlet\n",x->x_nbOut);

	if( x->x_nbOut>0 ) {
		x->x_outlet=(t_outlet **)malloc(x->x_nbOut * sizeof(t_outlet *));
	}else{
		x->x_nbOut=0;
		x->x_outlet=NULL;
	}

	for(i=0;i<x->x_nbOut;i++) {
		x->x_outlet[i]= outlet_new(&x->x_obj,0);
	}

	// start the lua interpreter
	x->x_L=luaL_newstate();
        luaL_openlibs(x->x_L);

	update_time(x->x_L);

	printf("luabox new!!!\n");
	return (x);
}


/*
 * dans un atom, on a a_type = 
    A_NULL,
    A_FLOAT,
    A_SYMBOL,
    A_POINTER,
    A_SEMI,
    A_COMMA,
    A_DEFFLOAT,
    A_DEFSYM,
    A_DOLLAR,
    A_DOLLSYM,
    A_GIMME,
    A_CANT
*/


void luabox_anything(t_luabox *x, t_symbol *sel, int argc, t_atom *argv)
{
	int nb,i,j,k;
	int before,after;

	//printf("select symbol is '%s'\n",sel->s_name);

	update_time(x->x_L);

	before=lua_gettop(x->x_L);

	// first field is function to call
	lua_getfield(x->x_L, LUA_GLOBALSINDEX, sel->s_name); /* function to be called */

	if( lua_isnil(x->x_L,-1) ) {
		printf("lua error: function '%s' not defined\n",sel->s_name);
		lua_pop(x->x_L,1);
		return;
	}
	
	// the arguments are pushed... only symbol and float

	nb=0;
	for(i=0;i<argc;i++) {
            switch( argv[i].a_type ) {
              case A_SYMBOL:
     		lua_pushstring(x->x_L, argv[i].a_w.w_symbol->s_name);
		nb++;
		break;
              case A_FLOAT:
		lua_pushnumber(x->x_L,argv[i].a_w.w_float);
		nb++;
		break;
	      default:
		printf("lua error: arg[%d] is not float|symbol (using nil)\n",i);
		lua_pushnil(x->x_L);
	    }
        }
	//outlet_anything(x->x_rejectout, sel, argc, argv);
	//outlet_anything(e->e_outlet, argv[0].a_w.w_symbol, argc-1, argv+1);


	// call with nb args and expect any number of results
	k=lua_pcall(x->x_L, nb, LUA_MULTRET,0);
	if( k ) {
		printf("lua error %d: %s\n", k,lua_tostring(x->x_L, -1));
		lua_pop(x->x_L,1);
	}
	after=lua_gettop(x->x_L);

	// the number of returned values is the difference between before and after
	//printf("got %d results back...\n",after-before);

	// on doit envoyer les resultats a l'envers... (ordre pd classique)
	int nbres=after-before;
	for(i=nbres-1;i>=0;i--) {
		int out;
		// pour jeter les resultats supplementaires:
		//if( i>=x->x_nbOut ) continue; // too many results
		out=i;
		// pour envoyer tous les resultats supp dans le dernier...
		if( i>=x->x_nbOut ) out=x->x_nbOut-1;

		j=-1-(nbres-1-i); // that's the stack position of result i

		// results are reversed in the stack, so we do nbres-1-i instead of i
		if( lua_isnumber(x->x_L,j) ) {
			outlet_float(x->x_outlet[out], (t_float)lua_tonumber(x->x_L,j));
		}else if( lua_isstring(x->x_L,j) ) {
			outlet_symbol(x->x_outlet[out], gensym(lua_tostring(x->x_L,j)));
		}else if( lua_istable(x->x_L,j) ) {
			t_atom *arg;
			//outlet_anything(x->x_outlet[out], gensym(lua_tostring(x->x_L,j)));
			int len=lua_objlen(x->x_L,j);
			//printf("result %d at stack %d is len=%d\n",i,j,len);
			if( len<1 ) continue;
			arg=(t_atom *)malloc(len*sizeof(t_atom));

			for(k=0;k<len;k++) {
				lua_pushinteger(x->x_L,k+1); // index start at [1]
				lua_gettable(x->x_L,j-1);
				if( lua_isnumber(x->x_L,-1) ) {
					SETFLOAT(&arg[k],lua_tonumber(x->x_L,-1));
				}else if( lua_isstring(x->x_L,-1) ) {
					SETSYMBOL(&arg[k],gensym(lua_tostring(x->x_L,-1)));
				}else{
					SETSYMBOL(&arg[k],&s_bang); // pour rire...
				}
				lua_pop(x->x_L,1);
			}
			// send all elements...
			if( arg[0].a_type==A_SYMBOL ) {
				outlet_anything(x->x_outlet[out],arg[0].a_w.w_symbol, len-1, arg+1);
			}else{
				outlet_list(x->x_outlet[out],&s_list, len, arg);
			}
			free(arg);
		}else{
			//printf("result %d is not a number|string|table (skipped)\n",i);
		}
	}

	// affiche chaque resultat...ZZ

	lua_pop(x->x_L,after-before);


}




// the box was destroyed
void luabox_free(t_luabox *x)
{
	lua_close(x->x_L);
	if( x->x_outlet ) free(x->x_outlet);
	printf("luabox: free!!!\n");
}

void luabox_setup(void)
{
    luabox_class = class_new(gensym("luabox"),
	(t_newmethod)luabox_new,
	(t_method)luabox_free,
	sizeof(t_luabox), 0, A_GIMME,0);

//    class_addlist(luabox_class, luabox_list);
    class_addanything(luabox_class, luabox_anything);

#ifdef SKIP
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
#endif

//    class_addlist(route_class, route_list);



}




