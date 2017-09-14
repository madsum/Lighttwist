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

#include "ReceiveThread.h"

ReceiveThread::ReceiveThread(int p)
{
  int it=0;
  ti=new ThreadInterface();
  cmds=NULL;
  port=p;
  Start((void *)(&it));
}

void ReceiveThread::Execute(void * arg)
{
  int k;
  int ret;
  char buffer[BS+1];
  char cmd[50];
  char params[BS];
  Entry *curr_entry;
  Entry *tmp_entry;

  init_bimulticast_player(&bmc, port, 1);

  for(;;) {
    cmd[0]='\0';
    params[0]='\0';
    curr_entry=NULL;
    tmp_entry=NULL;
    k=bi_receive_data(&bmc,(unsigned char *)buffer,BS);
    if( k>0 )
    {
      buffer[k]='\0';
      //fprintf(stderr,"ReceiveThread: buffer '%s'\n",buffer);
      strcat(buffer,"\n"); 
      entryReadDataFromBuffer(buffer, BS+1,&curr_entry);
   
      if (curr_entry!=NULL) 
      {
        strcpy(cmd,curr_entry->key);
        sprintf(params,"%s %s",curr_entry->key,curr_entry->value);

        LockData();
        //if there is a command of the same type not yet executed, update the params
        //otherwise, new command is inserted at the end of the 'cmds' list
        if (1 || strcmp(cmd,"load")==0 || strcmp(cmd,"unload")==0)
        {
          tmp_entry=entryAdd(&cmds);
          if (tmp_entry!=NULL)
          {
            strcpy(tmp_entry->key,cmd);
            strcpy(tmp_entry->value,params);  
          }
        }
        else //overwrite duplicates
        {
          entryUpdate(&cmds, cmd, params); 
        }
        UnlockData();
        entryFree(&curr_entry);
      } 
    }
  }
  ti->ThreadCallback(0,ret);

  uninit_bimulticast(&bmc);
  entryFree(&cmds);
}

int ReceiveThread::GetNextCommand(Entry *cpy_entry)
{
  int ret;

  LockData();
  //return first command in list
  if (cmds!=NULL)
  {
    strcpy(cpy_entry->key,cmds->key);
    strcpy(cpy_entry->value,cmds->value);
    ret=entryRemove(&cmds,cpy_entry->key);
    ret=0;
  }
  else
  {
    ret=-1;
  }

  UnlockData();

  return ret;
}

int ReceiveThread::ContainsRefresh()
{
  int ret;
  Entry *tmp;

  LockData();
  tmp=entryGet(cmds,"refresh");
  if (tmp==NULL) ret=0;
  else ret=1;
  UnlockData();

  return ret;
}

void ReceiveThread::Setup()
{
    // Do any setup here
}

