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

#include "shellWindow.h"

void shellWindow::createMenuWindow()
{
    display=new Fl_Text_Display(0,0,w(),h(),NULL);
    display->labelsize(FONTSIZE);
    display->textsize(FONTSIZE);
    display->buffer(new Fl_Text_Buffer(4096));
}


int shellWindow::exec(const char *command,char *buf,int buflen, unsigned char visible,unsigned char block)
{
  int i;
  FILE *pfp;
  char line[LINE_SIZE];
  long pid;
  char *arguments[MAX_NB_ARGS];
  char cmd[CMD_SIZE];

  strcpy(cmd,command);

  line[0]='\0';
  display->buffer()->text("");
  cursorWait();
  //if (visible) show();

  fprintf(stderr,"CMD: ");
  fprintf(stderr,"%s\n",cmd);
  display->insert("COMMAND:\n");
  display->insert(cmd);
  display->insert("\n");
  display->insert("OUTPUT:\n");
  Fl::check();
  if (!block) parseCommand(cmd,arguments);

  if (buf!=NULL) buf[0]='\0';
  for (i=0;i<3;i++)
  {
    if (block)
    {
      if( (pfp=popen(cmd, "r")) != NULL ) break;
    }
    else
    {
      if( (pfp=popen_nonblocking(arguments, "r", &pid)) != NULL ) break;
    }
  }
  if (i==3) 
  {
     fprintf(stderr, "POPEN Error\n");
     return -1;
  }

  if (block)
  {
    while (fgets(line, sizeof(line), pfp)!=NULL)
    {
      if (buf!=NULL)
      {
        if (strlen(buf)+strlen(line)<(unsigned int)(buflen)) strcat(buf,line);
      }
      display->insert(line);
      //fprintf(stderr,"%s\n",line);
    }
  }
  else
  {
    while (strcmp(line,"DONE")!=0)
    {
      line[0]='\0';
      fgets(line, sizeof(line), pfp);
      if (line[0]!='\0')
      {
        if (buf!=NULL)
        {
          if (strlen(buf)+strlen(line)<(unsigned int)(buflen)) strcat(buf,line);
        }
        display->insert(line);
        //fprintf(stderr,"%s\n",line);
      }
      Fl::check();
      usleep(100000);
    }
  }
  pclose(pfp);

  //wait 0.9 seconds so we can see output
  i=0;
  while (i<9)
  {
    Fl::check();
    usleep(100000);
    i++;
  }

  hide();
  cursorDefault();

  return 0;
}


FILE *shellWindow::popen_nonblocking(char **args,const char *type,long *tid)
{
  int p[2];
  FILE *fp;

  if (*type!='r' && *type!='w') return NULL;

  if (pipe(p) < 0) return NULL;

  fcntl(p[0],F_SETFL,O_NONBLOCK);
  fcntl(p[1],F_SETFL,O_NONBLOCK);

  if ((*tid = fork()) > 0)
  { /* then we are the parent */
    if (*type == 'r') 
    {
      close(p[1]);
      fp = fdopen(p[0], type);
    }
    else 
    {
      close(p[0]);
      fp = fdopen(p[1], type);
    }
    return fp;
  }
  else if (*tid == 0)
  { /* we're the child */
    /* make our thread id the process group leader */
    setpgid(0, 0);

    if (*type == 'r')
    {
      fflush(stdout);
      fflush(stderr);
      close(1);
      if (dup(p[1]) < 0) perror("dup of write side of pipe failed");
      close(2);
      if (dup(p[1]) < 0) perror("dup of write side of pipe failed");
    }
    else
    {
      close(0);
      if (dup(p[0]) < 0) perror("dup of read side of pipe failed");
    }
    close(p[0]); /* close since we dup()'ed what we needed */
    close(p[1]);
#ifdef __APPLE__
    execve(args[0], args, NULL);
#else
    execve(args[0], args, environ);
#endif
    //execve(args[0], args, 0);
    //printf("popen_nonblocking(): execve(%s) failed!\n", args[0]);
  }
  else
  {         /* we're having major problems... */
    close(p[0]);
    close(p[1]);
    //printf("popen_nonblocking(): fork() failure!\n");
  }

  return NULL;
}

//tokenize 'cmd' into arguments array
int shellWindow::parseCommand(char *cmd,char **args)
{
    int i;
    i=0;

    /* extract first string from string sequence */
    args[i] = strtok(cmd, " ");
    i++;

    /* loop until finishied */
    while (i<MAX_NB_ARGS)
    {
        /* extract string from string sequence */
        args[i] = strtok(NULL, " ");

        /* check if there is nothing else to extract */
        if (args[i] == NULL) break;

        i++;
    }
    if (i==MAX_NB_ARGS)
    {
      fprintf(stderr,"Error in parsing command!\n");
      args[i-1]=NULL;
    }
    
    return 0;
}



