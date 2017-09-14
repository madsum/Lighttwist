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

#include "media.h"

char *Ext_List[NB_EXT_TYPES][MAX_NB_EXT];
char Ext_Media[NB_EXT_TYPES][BUFFER_SIZE];

int stripFromChar(char *str,char s)
{
    char *c;

    if (str==NULL) return -1;

    c=strrchr(str,s);
    if (c==NULL)
    {
        //fprintf(stderr,"Error parsing extension '%c' in '%s'\n",s,str);
        return -1;
    }

    c[0]='\0';

    return 0;
}

char *startFromChar(const char *str, char s)
{
    const char *c;

    if (str==NULL) return NULL;

    c=strrchr(str,s);
    if (c==NULL)
    {
        return (char *)(str);
    }
    else
    {
        return (char*)(c+1);
    }
}

char *findRelativePath(const char *root,char *absname)
{
    int k;
    char localmedia[PATH_SIZE];
    int subdirpos;
    char *subdir;

    if (root==NULL || absname==NULL) return NULL;

    strcpy(localmedia,root);
    subdirpos=strlen(localmedia);
    subdir=absname;
    for (k=0;k<subdirpos;k++) subdir++;
    if (subdir[0]=='/' || subdir[0]=='\\') subdir++;
    return subdir;
}

char *findFilename(char *name)
{
  char *ctemp;

  ctemp=strrchr(name,'/');
  if (ctemp==NULL) ctemp=strrchr(name,'\\');
  if (ctemp==NULL) return name;
  else return ctemp+1;
}

int dirAllocate(Directory **dir,Directory *root,char *name,int x,int y,int show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
    int str_w,str_h;
    int temp;
    char *found_name;

    if (dir==NULL) return -1;
    if (name==NULL) return -1;

    if ((*dir)==NULL)
    {
      (*dir)=(Directory *)(malloc(sizeof(Directory)));
      if ((*dir)==NULL) return -1;
    }

    temp=0;

    strcpy((*dir)->absname,name);
    strcpy((*dir)->name,findFilename(name));

    (*dir)->check=new Fl_Check_Button(x,y,MEDIA_CHECK_WIDTH,MEDIA_BUT_HEIGHT);
    if (!show_all) (*dir)->check->hide();
    found_name=findFilename(name);
    str_w=MEDIA_BUT_WIDTH;
    fl_measure(found_name,str_w,str_h);
    (*dir)->but=new Fl_Button(x+MEDIA_CHECK_WIDTH,y,str_w+14,MEDIA_BUT_HEIGHT,strdup(found_name));
    setButton((*dir)->but,&temp);
    (*dir)->but->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
    //(*dir)->but->color(FL_DARK_CYAN);
    (*dir)->but->color(fl_rgb_color(COLOR_DIRECTORY));
    (*dir)->but->callback(cb_directory,(*dir)->absname);
    (*dir)->check->callback(cb_check_directory,(*dir)->absname);
    (*dir)->subdirs=NULL;
    (*dir)->parent=NULL;
    (*dir)->files=NULL;
    (*dir)->show_all=show_all;
    (*dir)->next=NULL;
    if (root!=NULL) (*dir)->root=root;
    else (*dir)->root=(*dir);

    return 0;
}

Directory *dirAddToList(Directory **head,Directory *root,char *name,int x,int y,unsigned char show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
  Directory *prevdir;
  Directory *currdir;
  Directory *new_d;

  if (head==NULL) return NULL;
  if (name==NULL) return NULL;

  new_d=NULL;
  dirAllocate(&new_d,root,name,x,y,show_all,cb_directory,cb_check_directory);

  if ((*head)==NULL)
  {
    (*head)=new_d;
    return new_d;
  }

  prevdir=NULL;
  currdir=(*head);
  while(currdir!=NULL)
  {
    if (strcmp(currdir->name,new_d->name)>0) break;
    prevdir=currdir;
    currdir=currdir->next;
  }

  //insert at end of list
  //currdir->next=new_d;

  //insert in alphabetical order
  if (prevdir!=NULL)
  {
    prevdir->next=new_d;
    new_d->next=currdir;
  }
  else
  {
    new_d->next=currdir;
    (*head)=new_d;
  }

  return new_d;
}

File *dirAddFile(Directory *head,char *fullname,unsigned char type,int x,int y)
{
  if (head==NULL) return NULL;
  if (fullname==NULL) return NULL;
  if (head->root==NULL) return NULL;

  return fileAddToList(&(head->files),fullname,head->root->absname,type,x,y);
}

int dirCount(Directory *d)
{
    Directory *currd;
    int count=0;
    currd=d;

    while(currd!=NULL)
    {
        count++;
        currd=currd->subdirs;
    }

    return count;
}

Directory *dirSuccessor(Directory *dir)
{
  Directory *currdir;

  if (dir==NULL) return NULL;
  if (dir->subdirs!=NULL) return dir->subdirs;

  currdir=dir;
  while(1)
  {
    if (currdir==NULL) return NULL;
    if (currdir->next!=NULL) return currdir->next;
    currdir=currdir->parent;
  }
}

int dirToggleVisibility(Directory *dir,unsigned char visible)
{
    Directory *currdir;

    if (dir==NULL) return -1;

    if (visible)
    {
      dir->but->show();
      if (dir->show_all) dir->check->show();
    }
    else
    {
      dir->but->hide();
      dir->check->hide();
      dir->expanded=DIR_NOT_EXPANDED;
      fileToggleVisibility(dir->files,FILE_HIDE);
      currdir=dir->subdirs;
      while(currdir!=NULL)
      {
        dirToggleVisibility(currdir,DIR_HIDE);
        currdir=currdir->next;
      }
    }

    return 0;
}

void dirFree(Directory **dir)
{
  if (dir==NULL || (*dir)==NULL) return;
  free((*dir));
  (*dir)=NULL;
}

void dirFreeTree(Directory **dir)
{
  if (dir==NULL) return;
  if ((*dir)==NULL) return;
  fileFreeList(&((*dir)->files));
  dirFreeTree(&((*dir)->subdirs));
  dirFreeTree(&((*dir)->next));
  dirFree(dir);
}

int fileAllocate(File **f,char *fullname,char *rootpath,unsigned char type,int x,int y)
{
    int str_w,str_h;
    int temp;
    char *found_name;

    if (f==NULL) return -1;

    if ((*f)==NULL)
    {
      (*f)=(File *)(malloc(sizeof(File)));
      if ((*f)==NULL) return -1;
    }

    if (fullname!=NULL)
    {
      strcpy((*f)->absname,fullname);
      strcpy((*f)->relname,findRelativePath(rootpath,fullname));
      strcpy((*f)->name,findFilename(fullname));
    }
    else
    {
      (*f)->absname[0]='\0';
      (*f)->relname[0]='\0';
      (*f)->name[0]='\0';
    }

    temp=0;

    (*f)->type=type;
    (*f)->check=new Fl_Check_Button(x,y,MEDIA_CHECK_WIDTH,MEDIA_BUT_HEIGHT);
    found_name=(*f)->name;
    str_w=MEDIA_BUT_WIDTH;
    fl_measure(found_name,str_w,str_h);
    (*f)->label=new Fl_Box(x+MEDIA_CHECK_WIDTH,y,str_w+14,MEDIA_BUT_HEIGHT,strdup(found_name));
    setBox((*f)->label,&temp);
    (*f)->label->box(FL_NO_BOX);
    (*f)->label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    //if (type!=EXT_VIDEO && type!=EXT_PLAYLIST) (*f)->check->hide();
    //(*f)->but=new Fl_Button(x+MEDIA_CHECK_WIDTH,y,MEDIA_BUT_WIDTH-x,MEDIA_BUT_HEIGHT,strdup(findFilename(name)));
    //(*f)->but->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
    (*f)->next=NULL;

    return 0;
}

File *fileAddToList(File **head,char *fullname,char *rootpath,unsigned char type,int x,int y)
{
  File *prevfile;
  File *currfile;
  File *new_f;

  if (head==NULL) return NULL;
  if (fullname==NULL) return NULL;
  if (rootpath==NULL) return NULL;

  new_f=NULL;
  fileAllocate(&new_f,fullname,rootpath,type,x,y);

  if ((*head)==NULL)
  {
    (*head)=new_f;
    return new_f;
  }

  prevfile=NULL;
  currfile=(*head);
  while(currfile!=NULL)
  {
    if (strcmp(currfile->name,new_f->name)>0) break;
    prevfile=currfile;
    currfile=currfile->next;
  }

  //insert at end of list
  //currfile->next=new_f;

  //insert in alphabetical order
  if (prevfile!=NULL)
  {
    prevfile->next=new_f;
    new_f->next=currfile;
  }
  else
  {
    new_f->next=currfile;
    (*head)=new_f;
  }

  return new_f;
}

int fileCount(File *f)
{
    File *currf;
    int count=0;
    currf=f;

    while(currf!=NULL)
    {
        count++;
        currf=currf->next;
    }

    return count;
}

int fileToggleVisibility(File *f,unsigned char visible)
{
    if (f==NULL) return -1;

    if (visible)
    {
      f->label->show();
      //if (f->type==EXT_VIDEO) f->check->show();
      f->check->show();
    }
    else
    {
      f->label->hide();
      f->check->hide();
    }

    return fileToggleVisibility(f->next,visible);
}

void fileFree(File **f)
{
  if (f==NULL || (*f)==NULL) return;
  free((*f));
  (*f)=NULL;
}

void fileFreeList(File **f)
{
    if (f==NULL) return;
    if ((*f)==NULL) return;
    fileFreeList(&((*f)->next));
    fileFree(f);
}

int addSlash(char *arg)
{
  int count;

  count=0;
  while(arg[count]!='\0')
  {
    count++;
  }

  if (count==0) return -1;
  if (arg[count-1]!='/')
  {
    arg[count]='/';
    arg[count+1]='\0';
  }

  return 0;
}

int removeSlash(char *arg)
{
  int count;

  count=0;
  while(arg[count]!='\0')
  {
    count++;
  }

  if (count==0) return -1;
  if (arg[count-1]=='/')
  {
    arg[count-1]='\0';
  }

  return 0;
}

int loadExtensions(const char *ext_media[NB_EXT_TYPES])
{
  int i,j;

  //initialize to NULL
  for (i=0;i<NB_EXT_TYPES;i++)
  {
    for (j=0;j<MAX_NB_EXT;j++)
    {
      //global variable
      Ext_List[i][j]='\0';
    }
  }

  for (i=0;i<NB_EXT_TYPES;i++)
  {
    if (ext_media[i]==NULL)
    {
      Ext_List[i][0]=NULL;
      continue;
    }

    strcpy(Ext_Media[i],ext_media[i]);
    j=0;

    Ext_List[i][j]=strtok(Ext_Media[i],EXT_DELIMITERS);
    j++;
    while (j<MAX_NB_EXT)
    {
      Ext_List[i][j] = strtok(NULL, EXT_DELIMITERS);
      // check if there is no more extensions
      if (Ext_List[i][j] == NULL) break;

      j++;
    }
    if (j==MAX_NB_EXT)
    {
      fprintf(stderr,"Warning: maximum number of extensions reached.\n");
    }
  }

  return 0;
}

static int detectMediaType(const char *filename)
{
  int i,j;
  const char *c;

  if (filename==NULL) return EXT_UNKNOWN;

  c=strrchr(filename,'.');
  if (c==NULL)
  {
    return EXT_UNKNOWN;
  }
  //bypass '.' to keep only extension
  //c++;

  for(i=0;i<NB_EXT_TYPES;i++)
  {
    for(j=0;j<MAX_NB_EXT;j++)
    {
      if (Ext_List[i][j]==NULL) break;
      if (!strcasecmp(c+1, Ext_List[i][j])) return i;
    }
  }

  return EXT_UNKNOWN;
}

int loadMediaTree(Directory *subtree,Directory *root,int margin,unsigned char show_all,void (*cb_directory)(Fl_Widget *,void *),void (*cb_check_directory)(Fl_Widget *,void *))
{
    int j;
    int ftype;
    char buf[BUFFER_SIZE];
    DIR  *dip;
    struct dirent *dit;
    Directory *new_dir;
    File *new_f;
    char *c;

    if (subtree==NULL) return -1;

    subtree->expanded=DIR_EXPANDED;

    addSlash(subtree->absname);

    //j=subtree->id+1;
    if ((dip = opendir(subtree->absname)) == NULL)
    {
        perror("local media");
        return -1;
    }
    while((dit=readdir(dip)))
    {
       if (strcmp(dit->d_name,".")==0 || strcmp(dit->d_name,"..")==0) continue;
       c=strchr(dit->d_name,' ');
       //disallow spaces in filenames
       if (c!=NULL) continue;


       if (dit->d_type==DT_DIR)
       {
         strcpy(buf,subtree->absname);
         strcat(buf,dit->d_name);
         new_dir=dirAddToList(&(subtree->subdirs),root,buf,subtree->check->x()+margin,j*MEDIA_BUT_HEIGHT,show_all,cb_directory,cb_check_directory);
         new_dir->parent=subtree;
         //new_dir->but->argument(j);
         //new_dir->check->argument(j);
         //new_dir->id=j;
//fprintf(stderr,"DIR %s %d\n",buf,j);
         j=loadMediaTree(new_dir,root,margin,show_all,cb_directory,cb_check_directory);
       }
       else
       {
         ftype=detectMediaType(dit->d_name);
         if (ftype!=EXT_UNKNOWN)
         {
           strcpy(buf,subtree->absname);
           strcat(buf,dit->d_name);
           //buf has full path+filename
           new_f=dirAddFile(subtree,buf,ftype,subtree->check->x()+margin,j*MEDIA_BUT_HEIGHT);
           ////new_f->but->argument(j);
           //new_f->check->argument(j);
           //new_f->id=j;
//fprintf(stderr,"FILE %s %d\n",buf,j);
           j++;
         }
       }
    }
    if (closedir(dip) == -1)
    {
        perror("local media");
        return -1;
    }

    return j;
}

int expandMediaTree(Directory *tree)
{
  int j;
  File *currf;

  if (tree==NULL) return -1;

  if (tree->show_all) j=tree->but->y();
  else j=tree->root->but->y();
  while(tree!=NULL)
  {
    tree->but->position(tree->but->x(),j);
    tree->check->position(tree->check->x(),j);
    if (tree->but->visible()) j+=MEDIA_BUT_HEIGHT;
    currf=tree->files;
    while(currf!=NULL)
    {
      currf->label->position(currf->label->x(),j);
      currf->check->position(currf->check->x(),j);
      if (tree->expanded && tree->show_all) j+=MEDIA_BUT_HEIGHT;
      currf=currf->next;
    }
    tree=dirSuccessor(tree);
  }

  return 0;
}

int selectMediaTree(Directory *tree,int select)
{
    Directory *currdir;
    File *currf;

    if (tree==NULL) return -1;

    tree->check->value(select);

    currf=tree->files;
    while(currf!=NULL)
    {
      currf->check->value(select);
      currf=currf->next;
    }

    currdir=tree->subdirs;
    while(currdir!=NULL)
    {
      selectMediaTree(currdir,select);
      currdir=currdir->next;
    }

    return 0;
}

/*Directory *findDirectoryFromID(Directory *tree,long id)
{
    Directory *currdir,*res;
    if (tree==NULL) return NULL;

    while(1)
    {
        if (tree==NULL) break;
        if (tree->id==id) break;
        currdir=tree->subdirs;
        while(currdir!=NULL && currdir->next!=NULL)
        {
            if (currdir->next->id>id) break;
            currdir=currdir->next;
        }
        tree=currdir;
    }

    return tree;
}*/

Directory *findSubDirectoryFromName(Directory *tree,char *name)
{
    Directory *currdir;

    if (tree==NULL) return NULL;
    if (name==NULL) return NULL;

    currdir=tree->subdirs;
    while(currdir!=NULL)
    {
       if (strcmp(currdir->name,name)==0) break;
       currdir=currdir->next;
    }
    return currdir;
}

Directory *findDirectoryFromAbsname(Directory *tree,char *absname)
{
    Directory *currdir,*nextdir;
    char *c,*n;
    int pos;
    char dirname[PATH_SIZE];

    if (tree==NULL) return NULL;
    if (absname==NULL) return NULL;

    currdir=tree;

    //fprintf(stderr,"DIR %s\n",absname);
    strcpy(dirname,absname);
    pos=strlen(tree->absname);
    n=&dirname[pos];
    c=strtok(n, "/\\");
    //fprintf(stderr,"%s\n",c);
    while (1)
    {
        //fprintf(stderr,"SUBDIR %s\n",c);
        nextdir=findSubDirectoryFromName(currdir,c);
        if (nextdir==NULL)
        {
          break;
        }
        currdir=nextdir;
        //fprintf(stderr,"CURRDIR %s\n",currdir->absname);
        c=strtok(NULL, "/\\");
    }

    return currdir;
}

int identifyImportedMedia(Directory *media,Directory *imported)
{
  Directory *mcurrdir,*icurrdir;
  File *mcurrfile,*icurrfile;
  char mbufname[PATH_SIZE];
  char ibufname[PATH_SIZE];
  char *c;

  if (media==NULL || imported==NULL) return -1;

  mcurrfile=media->files;
  while(mcurrfile!=NULL)
  {
    strcpy(mbufname,mcurrfile->relname);
    c=strrchr(mbufname,'.');
    if (c!=NULL)
    {
      (*c)='\0';
      icurrfile=imported->files;
      while(icurrfile!=NULL)
      {
        strcpy(ibufname,icurrfile->relname);
        c=strrchr(ibufname,'.');
        if (c!=NULL)
        {
          (*c)='\0';
          if (strcmp(mbufname,ibufname)==0)
          {
            //fprintf(stderr,"FOUND %s\n",mcurrfile->name);
            mcurrfile->label->labelcolor(FL_BLACK);
            break;
          }
        }
        icurrfile=icurrfile->next;
      }
    }
    if (icurrfile==NULL)
    {
      //fprintf(stderr,"NOT FOUND %s\n",mcurrfile->name);
      mcurrfile->label->labelcolor(FL_RED);
    }
    mcurrfile=mcurrfile->next;
  }

  mcurrdir=media->subdirs;
  icurrdir=imported->subdirs;
  while(mcurrdir!=NULL && icurrdir!=NULL)
  {
      identifyImportedMedia(mcurrdir,icurrdir);
      mcurrdir=mcurrdir->next;
      icurrdir=icurrdir->next;
  }

  return 0;
}

File *findFile(Directory *tree,char *fullname,unsigned char absolute)
{
    Directory *currdir,*nextdir;
    File *currfile;
    char *c,*n;
    int pos;
    char name[PATH_SIZE];
    char filename[FILENAME_SIZE];

    if (tree==NULL) return NULL;
    if (name==NULL) return NULL;

    currdir=tree;

    //fprintf(stderr,"PATH %s\n",mediapath);
    //fprintf(stderr,"FILE %s\n",name);
    strcpy(name,fullname);
    strcpy(filename,findFilename(name));
    pos=strlen(tree->absname);
    if (absolute) n=&name[pos];
    else n=name;
    c=strtok(n, "/\\");
    //fprintf(stderr,"%s\n",c);
    while (1)
    {
        nextdir=findSubDirectoryFromName(currdir,c);
        if (nextdir==NULL) //next token is filename
        {
          break;
        }
        currdir=nextdir;

        c=strtok(NULL, "/\\");
        //fprintf(stderr,"%s\n",c);
        if (c == NULL)
        {
          //fprintf(stderr,"Warning: file '%s' not found!\n",name);
          return NULL;
        }
    }

    currfile=currdir->files;
    while(currfile!=NULL)
    {
      //fprintf(stderr,"%s %s\n",currfile->name,filename);
      if (strcmp(currfile->name,filename)==0) break;
      currfile=currfile->next;
    }

    return currfile;
}

int updateMediaTree(Directory *tree)
{
    Directory *currdir;

    if (tree==NULL) return -1;

    if (tree->show_all) fileToggleVisibility(tree->files,!(tree->expanded));
    else
    {
      dirToggleVisibility(tree->root,DIR_HIDE);
      fileToggleVisibility(tree->files,FILE_HIDE);
    }
    currdir=tree->subdirs;
    while(currdir!=NULL)
    {
      dirToggleVisibility(currdir,!(tree->expanded));
      currdir=currdir->next;
    }
    if (tree->expanded==DIR_NOT_EXPANDED) tree->expanded=DIR_EXPANDED;
    else tree->expanded=DIR_NOT_EXPANDED;

    if ((!tree->show_all) && tree->expanded==DIR_EXPANDED && tree->subdirs!=NULL)
    {
        tree->but->hide();
        tree->check->hide();
    }


    expandMediaTree(tree);

    return 0;
}



