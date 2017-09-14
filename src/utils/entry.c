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

#include "entry.h"

/*************************************
Adds an Entry at the end of the entry list.

A pointer to the new node is returned.

A NULL pointer is returned if an error occured.
**************************************/
Entry *entryAdd(Entry **list)
{
  Entry *row;
  Entry *prev;
  Entry *curr;

  if (list==NULL) return (Entry *)(NULL);

  row=(Entry *)(malloc(sizeof(Entry)));
  if (row==NULL) return NULL;
  row->comment=NULL;

  curr=(*list);
  prev=NULL;
  while(curr!=NULL)
  {
    prev=curr;
    curr=curr->next;
  }

  if (prev==NULL)
  {
    row->next=(*list);
    (*list)=row;
  }
  else
  {
    row->next=NULL;
    prev->next=row;
  }

  return row;  
}

int entryCount(Entry *list)
{
  int count;
  Entry *curr;

  count=0;

  curr=list;
  while(curr!=NULL)
  {
    count++;
    curr=curr->next;
  }

  return count;  
}

/*************************************
Removes an entry based on the key field

returns NULL_ERROR, NOT_FOUND or SUCCESS
**************************************/
int entryRemove(Entry **list,const char *key)
{
  Entry *curr,*prev;

  if (list==NULL || key==NULL) return NULL_ERROR;

  if ((*list)==NULL)  return NULL_ERROR;

  prev=NULL;
  curr=(*list);
  while(curr!=NULL)
  {
    if (strcmp(curr->key,key)==0)
    {
      if (prev==NULL)
      {
        (*list)=curr->next;
      }
      else
      {
        prev->next=curr->next;
      }
      break;
    }
    prev=curr;
    curr=curr->next;
  }

  if (curr==NULL) return NOT_FOUND_ERROR;

  curr->next=NULL;
  entryFree(&curr);

  return SUCCESS;
}

Entry *entryGet(Entry *list, const char *key)
{
  Entry *curr;

  if (list==NULL || key==NULL) return (Entry *)(NULL);

  curr=list;

  while(curr!=NULL)
  {
    if (!strcmp(curr->key,key)) return curr;
    curr=curr->next;
  }

  return (Entry *)(NULL);
}

char *entryGetString(Entry *list, const char *key)
{
  Entry *tmp=entryGet(list,key);
  if (tmp==NULL) return NULL;
  return tmp->value;
}

int entryGetInteger(Entry *list, const char *key,int *val)
{
  char *tmp;
  tmp=entryGetString(list,key);
  (*val)=0;
  if (tmp==NULL) return NULL_ERROR;
  (*val)=atoi(tmp);
  return SUCCESS;
}

int entryGetFloat(Entry *list, const char *key,float *val)
{
  char *tmp;
  tmp=entryGetString(list,key);
  (*val)=0.0;
  if (tmp==NULL) return NULL_ERROR;
  (*val)=atof(tmp);
  return SUCCESS;
}

/***********************************
Find entry with 'key' and changes its 'value'
If the entry is not found, a new entry is created
returns NULL_ERROR, GEN_ERROR or SUCCESS
************************************/
int entryUpdate(Entry **list, const char *key, char *value)
{
  Entry *curr;

  if (list==NULL || key==NULL || value==NULL) return NULL_ERROR;

  curr=entryGet((*list), key);
  if (curr==NULL)
  {
    curr=entryAdd(list);
    if (curr==NULL) return GEN_ERROR;
    strcpy(curr->key,key);
  }
  else
  {
    strcpy(curr->value,value);  
    return DUPLICATES_ERROR;
  }

  strcpy(curr->value,value);  

  return SUCCESS;
}

/*************************************
Frees all Entries from 'list'

returns NULL_ERROR or SUCCESS
**************************************/
int entryFree(Entry **list)
{
  Entry *curr,*prev;

  if (list==NULL) return NULL_ERROR;
  if ((*list)==NULL)  return NULL_ERROR;

  prev=NULL;
  curr=(*list);
   while(curr!=NULL)
   {
     if (prev!=NULL) 
     {
       if (prev->comment!=NULL) free(prev->comment);
       free(prev);
     }
    
     prev=curr;
     curr=curr->next;
   }

   if (prev!=NULL)
   {
     if (prev->comment!=NULL) free(prev->comment);
     free(prev);
   }

   (*list)=NULL;

   return SUCCESS;
}

/*****************************************************
Extract 'key' and 'value' out of 'line' 
The format is assumed to be %s %s\n
'buf', 'bufsize' and 'offset' are passed as arguments to call entryGetLine
returns NULL_ERROR,PARSE_ERROR or SUCCESS or any error returned by getFileLine
******************************************************/
int entryExtractData(char *key,char *value,char *line)
{
  int i,j;
  int ret;
  if (key==NULL || value==NULL || line==NULL) return NULL_ERROR;    

  key[0]='\0';
  value[0]='\0';

  i=0;
  j=0;
  //Skip leading spaces
  while(i<LINE_SIZE && (line[i]==' ' || line[i]=='\t') && line[i]!='\n' && line[i]!='\0') i++;
  if (i==LINE_SIZE) return PARSE_ERROR;

  //return if comment
  if (line[i]=='#') return PARSE_ERROR;

  while(i<LINE_SIZE && j<DATA_SIZE && line[i]!='=' && line[i]!=' ' && line[i]!='\t' && line[i]!='\n' && line[i]!='\0')
  {
    key[j]=line[i];
    j++;
    i++;
  }
  if (i==LINE_SIZE) return PARSE_ERROR;

  if (j>=DATA_SIZE) j=DATA_SIZE-1;
  key[j]='\0';

  //Skip separated spaces
  while(i<LINE_SIZE && (line[i]=='=' || line[i]==' ' || line[i]=='\t') && line[i]!='\n' && line[i]!='\0') i++;

  j=0;
  //we are now at 'value' field
  //while(i<LINE_SIZE && j<PATH_SIZE && line[i]!=' ' && line[i]!='\t' && line[i]!='\n' && line[i]!='\0')
  //accept spaces
  while(i<LINE_SIZE && j<PATH_SIZE && line[i]!='\n' && line[i]!='\0')
  {
    value[j]=line[i];
    j++;
    i++;  
  }

  if (j>=PATH_SIZE) j=PATH_SIZE-1;
  value[j]='\0'; //put string ending character

  return SUCCESS;
}

int entryParseData(Entry **entries,char *line)
{
    Entry *data;
    char key[DATA_SIZE];
    char value[PATH_SIZE];

    if (line==NULL || entries==NULL) return NULL_ERROR;

    if(!entryExtractData(key,value,line))
    {
        if (key[0]!='\0')
        {
          data=entryGet((*entries), key);
          if (data!=NULL)
          {
            strcpy(data->key,key);
            strcpy(data->value,value);
          }
          else
          {
            //fprintf(stderr,"Warning: unexpected config entry\n");
            data=entryAdd(entries);        
            if (data==NULL) MEM_ERROR;
            strcpy(data->key,key);
            strcpy(data->value,value);
          }
        }
      }
      if (key[0]=='\0')
      {
        data=entryAdd(entries);        
        if (data==NULL) MEM_ERROR;
        data->comment=(char *)(malloc(sizeof(char)*LINE_SIZE));
        if (data->comment==NULL) MEM_ERROR;
        strcpy(data->comment,line);
        data->key[0]='\0';
        data->value[0]='\0';
      }

    return SUCCESS;
}

/*****************************************************
Reads 'filename' and creates entry list

returns NULL_ERROR,IO_ERROR, BUF_ERROR or SUCCESS
******************************************************/
int entryReadDataFromFile(const char *filename, Entry **entries)
{
  FILE *fp;
  int ret;
  char line[LINE_SIZE];

  if (filename==NULL || entries==NULL) return NULL_ERROR;

  ret=SUCCESS;

  if((fp = fopen(filename,"r")) != NULL)
  {
    fgets(line,LINE_SIZE,fp);
    while (!feof(fp))
    {
      if (entryParseData(entries,line)) break;
      fgets(line,LINE_SIZE,fp);
    }
    fclose(fp);
  }
  else
  {
    return IO_ERROR;
  }

  return ret;
}

/*****************************************************
reads a \n terminated 'line' in 'buf' from 'offset' position
'offset' is updated to position where line ended
see exec.h for different buffer size
returns BUF_ERROR or SUCCESS
******************************************************/
int entryGetLine(char *line,char *buf,int bsize,int *offset)
{
  int i,j;

  if (line==NULL || buf==NULL || offset==NULL) return NULL_ERROR;    

  i=(*offset);  
  j=0;
  while(i<bsize)
  {
    if (buf[i]=='\0' || j>=LINE_SIZE)
    {
      (*offset)=i;
      return BUF_ERROR;
    }
    if (buf[i]!='\n') line[j]=buf[i];
    else
    {
       line[j]='\0';     
       i++;
       break;    
    }   
    j++;
    i++;
  }
  
  (*offset)=i;
  return SUCCESS;
}

int entryReadDataFromBuffer(char *buffer, int bsize,Entry **entries)
{
  FILE *fp;
  int ret;
  int offset;
  Entry *data;
  char line[LINE_SIZE];
  char key[DATA_SIZE];
  char value[PATH_SIZE];
  if (buffer==NULL || entries==NULL) return NULL_ERROR;

  ret=SUCCESS;

  offset=0;  
  while (!entryGetLine(line,buffer,bsize,&offset))
  {
    if (entryParseData(entries,line)) break;
  }

  return 0;
}

/*****************************************************
Write all 'entries' to 'filename'

The format is %s %s\n

returns NULL_ERROR,IO_ERROR or SUCCESS
******************************************************/
int entryWriteData(const char *filename, Entry *entries)
{
  Entry *data;
  char line[LINE_SIZE];
  FILE *fp;

  if (filename==NULL) return NULL_ERROR;
  
  fp = fopen(filename,"w");
  if(fp != NULL)
  {
    data=entries;
    while(data!=NULL)
    {
      if (data->comment) sprintf(line,"%s",data->comment);
      else sprintf(line,"%s=%s\n",data->key,data->value);
      fwrite(line, strlen(line), 1, fp);
      data=data->next;
    }
    fclose(fp);
  }
  else
  {
    return IO_ERROR;
  }

  return SUCCESS;
}


