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

#include "remote.h"

Machine machines[MAX_NB_MACHINES];
char remote_bin_path[PATH_SIZE];

static int addPid(int i,int pid)
{
    int j;

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    for (j=0;j<MAX_NB_PIDS;j++)
    {
        if (machines[i].pids[j]==-1) break;
    }
    if (j==MAX_NB_PIDS) return -1;
    machines[i].pids[j]=pid;

    return 0;
}

static int removePid(int i,int pid)
{
    int j;

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    for (j=0;j<MAX_NB_PIDS;j++)
    {
        if (machines[i].pids[j]==pid) break;
    }
    if (j==MAX_NB_PIDS) return -1;
    machines[i].pids[j]=-1;

    return 0;
}

static int remote_killpids (int i)
{
    char cmd[CMD_SIZE];
    int j;

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    if (machines[i].connected!=0)
    {
        for (j=0;j<MAX_NB_PIDS;j++)
        {
            if (machines[i].pids[j]!=-1)
            {
                sprintf(cmd,"!kill %d",machines[i].pids[j]);
                tcp_send_string(&(machines[i].remote),(unsigned char *)(cmd));
            }
            removePid(i,machines[i].pids[j]);
        }
    }

    return 0;
}

int desactivate_remote_run ( int i )
{
    //FILE *fp;
    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    machines[i].connected=0;

    /*if (machines[i].remote==NULL) return -1;

      fp=machines[i].remote;
      fprintf(fp,"!killall");
      fflush(fp);
      fprintf(fp,"!quit");*/

    //pclose hangs...
    //pclose(fp);
    tcp_client_close(&(machines[i].remote));

    return 0;
}

int activate_remote_run (int i)
{
    char cmd[CMD_SIZE];
    FILE *remote_read;
    char cmd_read[CMD_SIZE];
    char line[LINE_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    desactivate_remote_run(i);
    if (machines[i].connected!=0) return -1;

    remote_read=NULL;
    line[0]='\0';

    strcpy(ip,machines[i].ip);
    stripFromChar(ip,':');

    sprintf(cmd,"ssh %s %s/ltremote",ip,remote_bin_path);
    //sprintf(cmd,"ssh %s '%s/ltremote -log /home/%s/ltremote.log'",ip,remote_bin_path,login);
    sprintf(cmd_read,"ssh -o ConnectTimeout=2 %s %s/ltremote -hello",ip,remote_bin_path);

    remote_read=popen(cmd_read,"r");

    if (remote_read==NULL)
    {
        fprintf(stderr,"Warning: there was an error connecting to %s\n",ip);
        //machines[i].ip->color(FL_RED);
        return -1;
    }

    fgets(line,LINE_SIZE,remote_read);
    pclose(remote_read);

    if (strcmp(line,"bonjour\n") != 0)
    {
        fprintf(stderr,"Warning: there was an error connecting to %s\n",ip);
        return -1;
    }
#ifndef REMOTERUN
    else
    {
        fprintf(stderr,"Successfully connected to %s\n",ip);
        machines[i].connected=1;
        return 0;
    }
#endif

    if (tcp_client_init(&(machines[i].remote),machines[i].ip,REMOTE_PORT))
    {
        fprintf(stderr,"Warning: there was an error connecting to %s\n",ip);
        return -1;
    }
    else
    {
        fprintf(stderr,"Successfully connected to %s\n",ip);
        machines[i].connected=1;
        return 0;
    }
}

int remote_exec_ssh_ip (const char *ip_address,char *cmd)
{
    char fcmd[CMD_SIZE];
    char ip[DATA_SIZE];

    strcpy(ip,ip_address);
    stripFromChar(ip,':');

    //do we have to add \r\n at the end of the command?
    sprintf(fcmd,"ssh %s \"%s < /dev/null > /dev/null\" &",ip,cmd);
    fprintf(stderr,"%s\n",fcmd);
    system(fcmd);

    return 0;
}

int remote_exec_ssh (int i,char *cmd)
{
    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    if (machines[i].connected==0) return -1;

    return remote_exec_ssh_ip (machines[i].ip,cmd);
}

int remote_exec (int i,char *cmd)
{
#ifdef REMOTERUN
    FILE *fp;
    int pid;
    char line[LINE_SIZE];
    char fcmd[CMD_SIZE];

    line[0]='\0';
    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    if (machines[i].remote==NULL) return -1;

    pid=0;
    remote_killpids(i);
    fp=machines[i].remote;
    sprintf(fcmd,"%s\r\n",cmd);
    fprintf(stderr,"%s\n",cmd);
    fprintf(fp,fcmd);
    fflush(fp);
    //fgets(line,LINE_SIZE,fp);
    //fprintf(stderr,"%s\n",line);

    //pid=atoi(line);
    //fprintf(stderr,"PID on machine %d is %d\n",(i+1),pid);
    addPid(i,pid);

    return 0;
#else
    return (remote_exec_ssh(i,cmd));
#endif
}

int remote_wait (int i)
{
    char cmd[CMD_SIZE];
    char line[LINE_SIZE];
    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    if (machines[i].connected==0) return -1;

    sprintf(cmd,"!wait");
    tcp_send_string(&(machines[i].remote),(unsigned char *)(cmd));
    while (tcp_receive_data(&(machines[i].remote),(unsigned char *)(line),LINE_SIZE)<=0) usleep(1000);
    printf("wait result is: %s\n",line);
    return 0;
}

int remote_kill(int i,const char *process)
{
    char cmd[CMD_SIZE];

    sprintf(cmd,"killall %s",process);
    remote_exec (i,cmd);
    //usleep(150000);

    return 0;
}

