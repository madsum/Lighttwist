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

/**

  /home/vision/bin/ltprecrop -i /home/vision/fabrique/exemples/screens1234.avi -o /home/vision/fabrique/exemples/screens1234_precrop_00.avi -l /home/vision/grabs/grab_24mai2006/out/imapXY_00_crop.lut

 **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ltports.h"
#include <bmc/bmc.h>

//#define VERBOSE


#define MAX_PID	1000
#define EMPTY	-1

// un pid de EMPTY est disponible
int pidTable[MAX_PID];

int lastpid; // PID de la derniere commande executee (ou -1)

void initPidTable(void)
{
	int i;
	for(i=0;i<MAX_PID;i++) pidTable[i]=EMPTY;
}

int addPid(int pid)
{
	int i;
	// trouve un pid vide
	for(i=0;i<MAX_PID;i++) {
		if( pidTable[i]==EMPTY ) {
			pidTable[i]=pid;
			return(0);
		}
	}
	return(-1);
}


int checkPid(int pid)
{
	int i;
	for(i=0;i<MAX_PID;i++) if( pidTable[i]==pid ) return(1);
	return(0);
}

void removePid(int pid)
{
	int i;
	for(i=0;i<MAX_PID;i++) if( pidTable[i]==pid ) pidTable[i]=EMPTY;
}

void killAllPid(int sig)
{
	int i;
	for(i=0;i<MAX_PID;i++) {
		if( pidTable[i]==EMPTY ) continue;
#ifdef VERBOSE
		printf("killing pid %d with %d\n",pidTable[i],sig);
		fflush(stdout);
#endif
		kill(pidTable[i],sig);
		waitpid(pidTable[i],NULL,WNOHANG); // zombie!
		pidTable[i]=EMPTY;
	}
}


// elimine les zombie
void flushPid(void)
{
	int i;
	int k;
	for(i=0;i<MAX_PID;i++) {
		if( pidTable[i]==EMPTY ) continue;
		k=waitpid(pidTable[i],NULL,WNOHANG);
		// k<0 : no such process
		// k==0 : still running
		// k==pid : process has exited
		if( k!=0 ) pidTable[i]=EMPTY;
		//printf("GOT %d is wait %d\n",pidTable[i],k);
	}
}


int chop(char *S,char *arg[],int nb)
{
	int i;
	char *p;
	int inQuote,inWord;
	i=0;
	inQuote=0;
	inWord=0;
	for(p=S;*p;p++) {
		if( *p=='\"' ) {
			*p=' ';
			inQuote=1-inQuote;
			if( inQuote ) continue;
		}
		if( isspace(*p) && inQuote ) continue;
		if( inWord==0 ) {
			// PAS dans un mot (possiblement avec "")
			if( !isspace(*p) ) {
				// nouveau mot!
				arg[i++]=p;
				inWord=1;
				if( i>=nb ) return(-1);
			}
		}else{
			// DANS un mot
			if( isspace(*p) ) {
				// fin du mot!
				*p=0;
				inWord=0;
			}
		}
	}
	arg[i]=NULL;
	/*printf("got %d words\n",i);*/
	return(i);
}

int main(int argc,char *argv[])
{
	FILE *log;
	tcpcast tcp;
	char logfile[255];
	char buf[2000];
	int i,k;
	char *arg[100];
	int pid;
	int sig;
	int hello;
#ifdef VERBOSE
	printf("-- remote run --\n");
	fflush(stdout);
#endif

	hello=0;
	logfile[0]='\0';
	log=NULL;
	for(i=1;i<argc;i++) {
		if( strcmp("-log",argv[i])==0 && i+1<argc ) {
			strcpy(logfile,argv[i+1]);
			i++;continue;
		}
		if( strcmp("-hello",argv[i])==0) {
			hello=1;
			continue;
		}
	}

	tcp_server_init(&tcp,REMOTE_PORT);

	log=fopen(logfile,"w");

	if (hello)
	{
		printf("bonjour\n");
		fflush(stdout);
		return 0;
	}

	initPidTable();

	lastpid=-1;

	for(;;) {
		tcp_server_wait(&tcp);
#ifdef VERBOSE
		printf("Waiting cmd...\n");
		fflush(stdout);
#endif
		k=tcp_receive_data(&tcp,buf,2000);

		// EOF?
		if( k<0 ) 
		{
			if (log!=NULL) fclose(log);
			exit(0);
		}

		// fini?
		if( strcmp(buf,"!quit")==0 ) {
			if (log!=NULL) fclose(log);
			killAllPid(sig);
			exit(0);
		}

		if( strcmp(buf,"!wait")==0 ) {
			if( checkPid(lastpid) ) {
				k=waitpid(lastpid,NULL,0); // zombie!
				removePid(lastpid);
			}
			printf("done %d\n",k);
			fflush(stdout);
			lastpid=-1;
			continue;
		}


		// kill all running pids?
		if( strncmp(buf,"!killall",8)==0 ) {
			k=sscanf(buf+8," %d",&sig);
#ifdef VERBOSE
			printf("killall\n");
			fflush(stdout);
#endif
			if( k==0 ) sig=9;
			killAllPid(sig);
		}

		// kill a pid?
		if( strncmp(buf,"!kill",5)==0 ) {
			k=sscanf(buf+5," %d %d",&pid,&sig);
			if( k==1 ) sig=9; // SIG_KILL
			if( checkPid(pid) ) {
				kill(pid,sig);
				waitpid(pid,NULL,WNOHANG); // zombie!
				removePid(pid);
#ifdef VERBOSE
				printf("killing pid %d with %d\n",pid,sig);
				fflush(stdout);
#endif
			}
			continue;
		}

		// flush les zombies
		flushPid();

		// commande!!!
#ifdef VERBOSE
		printf("%d got >%s<\n",k,buf);
		fflush(stdout);
#endif
		if (log!=NULL)
		{
			fprintf(log,"%d got >%s<\n",k,buf);
			fflush(log);
		}

		/// chop la commande en morceaux
		k=chop(buf,arg,100);
		if( k<=0 ) {
			printf("-1\n");fflush(stdout);
			continue;
		}
#ifdef VERBOSE
		printf("CMD:\n");
		fflush(stdout);
		for(i=0;arg[i];i++) {
			printf("argv[%2d]=>%s<\n",i,arg[i]);
			fflush(stdout);
		}
		fflush(stdout);
#endif


		// exec!
		if( (pid=fork())!=0 ) {
			// parent!
			addPid(pid);
#ifdef VERBOSE
			printf("%d\n",pid);
#endif
			fflush(stdout);
			lastpid=pid;
		}else{
			// child!
#ifdef VERBOSE
			printf("EXEC %s\n",arg[0]);
#endif


			close(0);
			close(1);
			close(2);

			open("/dev/null",O_RDONLY);
			open("/dev/null",O_WRONLY);
			open("/dev/null",O_WRONLY);

			setenv("DISPLAY",":0.0",1);
			if (log!=NULL)
			{
				fprintf(log,"EXEC!\n");
				fflush(log);
			}
			execvp(arg[0],arg);
			//exit(0);
		}
		tcp_server_close_connection(&tcp);
	}

	if (log!=NULL) fclose(log);

	tcp_server_close(&tcp);
}


