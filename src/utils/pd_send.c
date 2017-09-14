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

#include "pd_send.h"

//fp will be opened only if NULL
int pdSend(tcpcast **fp,const char *ip,int port,char *msg)
{
  int k;
  char cmd[100];

  if (msg==NULL) return -1;
  if (ip==NULL) return -1;

  if ((*fp)==NULL)
  {
    /*sprintf(cmd,"pdsend %d %s",port,ip);
    //printf("%s: %s\n",cmd,msg);
    (*fp)=popen(cmd,"w");*/
    (*fp)=(tcpcast *)(malloc(sizeof(tcpcast)));
    k=tcp_client_init((*fp),ip,port);
    if (k)
    {
      free((*fp));
      (*fp)=NULL;
    }
  }

  if ((*fp)==NULL) return -1;

  strcat(msg,";\n");
  //fprintf((*fp),msg);
  //fflush((*fp));
  tcp_send_data((*fp),(unsigned char *)msg,strlen(msg));
  msg[strlen(msg)-2]='\0'; //remove ';\n'
//fprintf(stderr,"PDSEND: %s (%s:%d)\n",msg,ip,port);

  return 0;
}

int pdClose(tcpcast **fp)
{
	if ((*fp)!=NULL)
    {
      tcp_client_close((*fp));
      free((*fp));
      (*fp)=NULL;
    }

    return 0;
}



