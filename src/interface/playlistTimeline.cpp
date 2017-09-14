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

#include "playlistTimeline.h"

void playlistTimeline::initLayers()
{
    int i,index;
    int templine;
    char str[24];

    templine=0;

    //create layer space
    for (i=0;i<NB_DISP_VIDEO_LAYERS;i++)
    {
      layers[i]=new playlistLayer(win,x()+TL_LAYER_START,y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,w()-TL_LAYER_START,TL_LAYER_HEIGHT,LAYER_VIDEO_TYPE);
    }
    for (index=0;index<NB_DISP_AUDIO_LAYERS;index++)
    {
      i=index+NB_DISP_VIDEO_LAYERS;
      layers[i]=new playlistLayer(win,x()+TL_LAYER_START,y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,w()-TL_LAYER_START,TL_LAYER_HEIGHT,LAYER_AUDIO_TYPE);
    }
    //create layer headers
    for (i=0;i<NB_DISP_VIDEO_LAYERS;i++)
    {
      sprintf(str,langGetLabel(LABEL_VIDEO_LAYER),i+1);
      layers_box[i]=new Fl_Button(x(),y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,TL_LAYER_START,TL_LAYER_HEIGHT,NULL);
      layers_box[i]->color(fl_rgb_color(COLOR_DIRECTORY));
      layers_box[i]->labelsize(FONTSIZE);
      layers_box[i]->deactivate();
      layers[i]->label(strdup(str));
      layers[i]->labelcolor(FL_GRAY);
      layers_lbl[i]=new Fl_Button(x(),2+y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,TL_LAYER_START,20,strdup(str));
      setButton(layers_lbl[i],&templine);
      layers_lbl[i]->box(FL_NO_BOX);
    }
    for (index=0;index<NB_DISP_AUDIO_LAYERS;index++)
    {
      i=index+NB_DISP_VIDEO_LAYERS;
      sprintf(str,langGetLabel(LABEL_AUDIO_LAYER),index+1);
      layers_box[i]=new Fl_Button(x(),y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,TL_LAYER_START,TL_LAYER_HEIGHT,NULL);
      layers_box[i]->color(fl_rgb_color(COLOR_DIRECTORY));
      layers_box[i]->labelsize(FONTSIZE);
      layers_box[i]->deactivate();
      layers[i]->label(strdup(str));
      layers[i]->labelcolor(FL_GRAY);
      layers_angle[index]=new Fl_Dial(x()+10,-4+TL_LAYER_HEIGHT/2+y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,24,24,NULL);
      layers_angle[index]->angles(180,540);
      layers_angle[index]->range(0.0,360.0);
      //layers_angle[index]->type(FL_LINE_DIAL);
      layers_angle[index]->value(index*360.0/NB_DISP_AUDIO_LAYERS);
      layers_lbl[i]=new Fl_Button(x(),2+y()+TL_LAYER_HEIGHT+i*TL_LAYER_HEIGHT,TL_LAYER_START,20,strdup(str));
      setButton(layers_lbl[i],&templine);
      layers_lbl[i]->box(FL_NO_BOX);
    }
    size(w(),layers_box[i]->y()+TL_LAYER_HEIGHT-y());
}

void playlistTimeline::draw()
{
    int i;
	char numbers[1024],*numbers_ptr;
	int temp,temp2,temp3,NumDivisions,SizeOfText,TxtH;
	int DivStart, DivStep;
	int Hour,Min,Sec,Frame,TL_Temp;
    int X,Y,W,H;

    //Fl_Box::draw();
    Fl_Scroll::draw();
    for (i=0;i<NB_DISP_LAYERS;i++)
    {
      layers[i]->redraw();
    }

	X=x()+TL_LAYER_START;
    Y=y();
    W=w()-TL_LAYER_START;
    H=TL_LAYER_HEIGHT;

	//Calculate the Division Spacing and Values
	fl_measure(ZOOM_NB_FORMAT, SizeOfText, TxtH, 1);
	//SizeOfText=(int)fl_width(ZOOM_NB_FORMAT);
	//TxtH=(int)fl_height();

	NumDivisions=(W-(TL_LEAD*2)+SCALE_WIDTH)/SCALE_WIDTH;  //Cal num of Divisions
	DivStart=(int)(disp_start+0.5);         //Calculate Start and step
    //fprintf(stderr,"DIVSTART %d\n",DivStart);
	DivStep=scales[scale_pos];

    numbers_ptr=numbers;

	//Draw the divisions
	for(temp=0;temp<(NumDivisions-1)*SCALE_WIDTH;temp+=SCALE_WIDTH)
	{
		fl_color(FL_BLACK);
		fl_line(X+TL_LEAD+temp,Y+H-5,X+TL_LEAD+temp,Y+H-20);

		//Calculate time to display
		Hour=DivStart/60/60/fps;
		TL_Temp=DivStart-(Hour*60*60*fps);
		Min=TL_Temp/60/fps;
		TL_Temp-=Min*60*fps;
		Sec=TL_Temp/fps;
		TL_Temp-=Sec*fps;
		Frame=TL_Temp;
		if(Frame > 99) Frame=99;
		if(Frame <1) Frame=0;
		DivStart+=DivStep;

		sprintf(numbers_ptr,"%02d:%02d:%02d.%02d",Hour,Min,Sec,Frame);

		//Print Time
		SizeOfText=(int)fl_width(numbers_ptr);

		fl_draw(numbers_ptr,X+45+temp -((SizeOfText)/2),Y+((H-20)/2)+(TxtH/2));

		numbers_ptr+=sizeof(char) * 14;

		if(scales[scale_pos] < fps)
		{
			temp3=SCALE_WIDTH/scales[scale_pos];
		}
		else
		{
			if(scales[scale_pos] < fps*60)
			{
				temp3=SCALE_WIDTH/(scales[scale_pos]/fps);
			}
			else
			{
				if(scales[scale_pos] < fps*60*60)
				{
					temp3=SCALE_WIDTH/(scales[scale_pos]/fps/60);
				}
				else
				{
					if(scales[scale_pos] < fps*60*60*60)
					{
						temp3=SCALE_WIDTH/(scales[scale_pos]/fps/60/60);
					}
				}

			}
		}
		if(temp3<1) temp3=1;

		for(temp2=temp+temp3;temp2<temp+SCALE_WIDTH;temp2+=temp3)
		{
			if(X+TL_LEAD+temp2 < (X+W-TL_LEAD))
			{
				fl_color(FL_BLACK);
				fl_line(X+TL_LEAD+temp2,Y+H-5,X+TL_LEAD+temp2,Y+H-10);
			}
		}
	}
}

void playlistTimeline::setScaleValue(double val)
{
    int i,temp;
    int prev_scale_pos=scale_pos;

    scale_pos = (int)(scale_slider->value() * (NB_SCALES-1));

    if(scale_pos < 0) scale_pos=0;
    if(scale_pos >= NB_SCALES) scale_pos=NB_SCALES-1;

    scaleCompute(prev_scale_pos);
    for (i=0;i<NB_DISP_LAYERS;i++)
    {
      if (layers[i]!=NULL)
      {
        layers[i]->setScale(fps,disp_start,(double)(SCALE_WIDTH)/scales[scale_pos],x()+TL_LAYER_START+TL_LEAD,timeline_header->w()-(TL_LEAD*2));
      }
    }

	if(scales[scale_pos] >= fps*60*60)
	{
        temp=scales[scale_pos]/60/60/fps;
        if (temp==1) sprintf(scale_buf,"%d hour",temp);
        else sprintf(scale_buf,"%d hours",temp);
	}
	else if(scales[scale_pos] >= fps*60)
	{
        temp=scales[scale_pos]/60/fps;
        if (temp==1) sprintf(scale_buf,"%d minute",temp);
        else sprintf(scale_buf,"%d minutes",temp);
	}
	else if(scales[scale_pos] >= fps)
	{
        temp=scales[scale_pos]/fps;
        if (temp==1) sprintf(scale_buf,"%d second",temp);
        else sprintf(scale_buf,"%d seconds",temp);
	}
	else
	{
        temp=scales[scale_pos];
        if (temp==1) sprintf(scale_buf,"%d frame",temp);
        else sprintf(scale_buf,"%d frames",temp);
	}
    strcpy(scale_buf,langGetLabel(LABEL_TIMESCALE));
    scale_slider->label(scale_buf);
   
    win->redraw();
}

/*******************************************************
 *
 * calculates displayed frames
 *
 ******************************************************/
void playlistTimeline::scaleCompute(int prev_scale_pos)
{
	double W;
    double ratio;
    double actual_frame_pos;
    double prev_disp_nb_frames;

	W=timeline_header->w()-(TL_LEAD*2.0);
    ratio=(double)(pointer_line->start_x-x()-TL_LAYER_START-TL_LEAD)/W;
	
    if (prev_scale_pos!=scale_pos)
    {
      if (prev_scale_pos!=-1)
      {
        prev_disp_nb_frames=(W / SCALE_WIDTH) * scales[prev_scale_pos];
        actual_frame_pos=disp_start+prev_disp_nb_frames*ratio;
      }
      else actual_frame_pos=0;

      //fprintf(stderr,"%f %f %f %f\n",actual_frame_pos,disp_start,prev_disp_nb_frames,ratio);

	  //Calculate the number of frames that can be displayed
	  disp_nb_frames=(W / SCALE_WIDTH) * scales[scale_pos];
     
	  //Calculate In point
	  disp_start=actual_frame_pos-ratio*disp_nb_frames;
	  if(disp_start < 0) disp_start=0;
      if (scale_pos==NB_SCALES-1) disp_start=0;

      //fprintf(stderr,"DISP_START %f\n",disp_start);
      //fprintf(stderr,"######################\n");
      //fprintf(stderr,"MARGIN %d\n",pointer_line->x()-x()-TL_LAYER_START-TL_LEAD);
      //fprintf(stderr,"W %f\n",W);
      //fprintf(stderr,"RATIO %f\n",ratio);
      //fprintf(stderr,"PREV_NB_DISP_FRAMES %f\n",prev_disp_nb_frames);
      //fprintf(stderr,"ACTUAL_FRAME_POS %f\n",actual_frame_pos);
      //fprintf(stderr,"DISP_START %f\n",disp_start);
      //fprintf(stderr,"START_X %f\n",((double)(actual_frame_pos-disp_start))*SCALE_WIDTH/scales[scale_pos]);
      //fprintf(stderr,"POSITION_A %f\n",pointer_line->start_x-x()-TL_LAYER_START-TL_LEAD);
      pointer_line->start_x=x()+TL_LAYER_START+TL_LEAD+(actual_frame_pos-disp_start)/scales[scale_pos]*SCALE_WIDTH;
      //fprintf(stderr,"POSITION_B %f\n",pointer_line->start_x-x()-TL_LAYER_START-TL_LEAD);
      //fprintf(stderr,"######################\n");
    }
}

/************************************************************
 *
 * Sets up the default scale range using the Current fps
 ************************************************************/
void playlistTimeline::setDefaultScales()
{
	int index=0;

	//frames resolution
	scales[index++]=1;
	scales[index++]=2;
	scales[index++]=5;
	scales[index++]=10;

	//seconds resolution
	scales[index++]=1*fps;
	scales[index++]=2*fps;
	scales[index++]=5*fps;
	scales[index++]=10*fps;
	scales[index++]=20*fps;
	scales[index++]=30*fps;

	//minute resolution
	scales[index++]=1*fps*60;
	scales[index++]=2*fps*60;
	scales[index++]=5*fps*60;
	scales[index++]=10*fps*60;
	scales[index++]=20*fps*60;
	scales[index++]=30*fps*60;

	//hour resolution
	scales[index++]=1*fps*60*60;
	//scales[index++]=2*fps*60*60;
}


//F <field 0.0 5.0 1 0 1 50 z
//F {ref1 off1 off2 layer fade_min fade_max fps tag}

//audio
//A <ciel0 0.0 0 /home/vision/Desktop/audio/johnny_cut.aif 0.0 180.0 0.1 0.2 0.3 0.4 0.5 0.6 audio1
//A {ref1 off1 off2 layer name s0 s1 s2 s3 s4 s5 s6 s7 tag}

//B <audio1 0 1 0 20 s
//B {ref1 off1 off2 layer fade_max tag}

//C <ciel0 3.4 205.0 0 0.0 audio1
//C {ref1 off1 off2 layer start_fade tag}

//3d scene
//T ref 0.0 450.0 3 /home/vision/svn/lt/trunk/totem/pano_cosmo_complet.osg 1 alea
//T {ref1 off1 off2 layer name start_fade tag}

int playlistTimeline::resolveTime(char *tag,char *start_ref,double *abs_start_time,double *start_time,double *end_time)
{
  playlistMedia *media;

  if (tag==NULL) return -1;
  if (start_ref==NULL) return -1;
  if (abs_start_time==NULL) return -1;
  if (start_time==NULL) return -1;
  if (end_time==NULL) return -1;

  if (!IS_MIDI_OR_REL(start_ref[0]))
  {
    (*abs_start_time)=atof(start_ref);
  }
  else
  {
    media=findTagMedia(tag);
    if (media==NULL)
    {
      fprintf(stderr,"Media tag '%s' not found!\n",tag);
      return -1;
    }
    if (tag[0]=='>') (*abs_start_time)=media->start_x+media->len_x;
    else (*abs_start_time)=media->start_x;
  }

  (*start_time)+=(*abs_start_time);

  return 0;
}

playlistMedia *playlistTimeline::findTagMedia(char *tag)
{
  int i,j;
  if (tag==NULL) return NULL;

  for (i=0;i<NB_DISP_LAYERS;i++)
  {
    for (j=0;j<MAX_NB_MEDIA;j++)
    {
      if (layers[i]->media[j]->active)
      {
        if (strcmp(layers[i]->media[j]->m_tag,&(tag[1]))==0) return layers[i]->media[j];         
      }
    }
  }

  return NULL;
}

void playlistTimeline::resetMedia()
{
  int i,j,k;

  for (i=0;i<NB_DISP_LAYERS;i++)
  {
    for (j=0;j<MAX_NB_MEDIA;j++)
    {
      layers[i]->media[j]->active=0;         
      for (k=0;k<4;k++)
      {
        layers[i]->media[j]->fade_ctrls[k]->hide();
      }
      layers[i]->media[j]->hide();
    }
  }

  for (i=0;i<NB_DISP_AUDIO_LAYERS;i++)
  {
      layers_angle[i]->value(i*360.0/NB_DISP_AUDIO_LAYERS);
  }
}

/************************************************************
Functions to load/save a playlist, following 
osgplaylist.c

If a change is made in the commands,
DON'T FORGET to change the quick playlist save in do_precrop() in lighttwist.cpp
**************************************************************/
int playlistTimeline::loadPlaylist(Directory *mediatree)
{
    char line[LINE_SIZE];
    int lineno;
    unsigned char error_flag;
    int k;
    FILE *fp;
    File *file;
    playlistMedia *med;
    char statsname[PATH_SIZE];
    //unsigned char m_cmd_type;
    char m_start_ref[DATA_SIZE];
    double m_abs_start_time;
    double m_start_time;
    double m_end_time;
    int m_layer;
    char m_fname[PATH_SIZE];
    double m_start_fade;
    double m_fade_min;
    double m_fade_max;
    float m_fps;
    char m_tag[DATA_SIZE];
    double m_angle;
    double m_dist;
    double m_radius;
    Entry *config;
    int ret;
    float length;

    if (mediatree==NULL) return -1;

    config=NULL;

    resetMedia();

    fp = fopen(playlist_file,"r");    
    if (fp==NULL) {
	    fprintf(stderr,"Error: unable to open file '%s'\n",playlist_file);
	    return -1;
    }

    lineno=1;
    while (1)
    {
        med=NULL;
        error_flag=0;
        k = fscanf(fp," %[^\n]",line);
        if (k!=1) break;
        if (line[0] == '#') continue; //commentaires
        if (line[0] == 'M')
        {
            if (sscanf(line,"M %s %lg %lg %d %s %f %lg %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       m_fname,
                       &m_fps,
                       &m_start_fade,
                       m_tag)!=8)
            {
                error_flag=1;
            }             
            else
            {
              resolveTime(m_start_ref,m_start_ref,&m_abs_start_time,&m_start_time,&m_end_time);
              if (m_layer>=0 && m_layer<NB_DISP_LAYERS) med=layers[m_layer]->addMedia(EXT_VIDEO,m_fname,layers[m_layer]->media[0]->convertTimeToPixelAbsolute(m_start_time),m_end_time,m_fps);
              if (med!=NULL)
              {
                med->m_fps=m_fps;
                strcpy(med->m_tag,m_tag);
                file=findFile(mediatree,m_fname,0);
                if (file==NULL) med->color(FL_RED);
                else
                {
                  strcpy(statsname,file->absname);
                  stripFromChar(statsname,'.');
                  strcat(statsname,".stats");
                  entryReadDataFromFile(statsname,&config); 
                  ret=entryGetFloat(config,STATS_LENGTH,&length);
                  if (ret==0)
                  {
                    med->len_x_orig=(double)(length);
                  }
                } 
              }
            }
        }             
        else if (line[0] == 'T')
        {
            if (sscanf(line,"T %s %lg %lg %d %s %lg %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       m_fname,
                       &m_start_fade,
                       m_tag)!=7)
            {
                error_flag=1;
            }
            else
            {
              resolveTime(m_start_ref,m_start_ref,&m_abs_start_time,&m_start_time,&m_end_time);
              if (m_layer>=0 && m_layer<NB_DISP_LAYERS) med=layers[m_layer]->addMedia(EXT_3D,m_fname,layers[m_layer]->media[0]->convertTimeToPixelAbsolute(m_start_time),m_end_time,m_fps);
              if (med!=NULL)
              {
                strcpy(med->m_tag,m_tag);
                file=findFile(mediatree,m_fname,0);
                if (file==NULL) med->color(FL_RED);
              }
            }
        } 
        else if (line[0] == 'I')
        {
            if (sscanf(line,"I %s %lg %lg %d %s %lg %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       m_fname,
                       &m_start_fade,
                       m_tag)!=7)
            {
                error_flag=1;
            }
            else
            {
              resolveTime(m_start_ref,m_start_ref,&m_abs_start_time,&m_start_time,&m_end_time);
              if (m_layer>=0 && m_layer<NB_DISP_LAYERS) med=layers[m_layer]->addMedia(EXT_IMAGE,m_fname,layers[m_layer]->media[0]->convertTimeToPixelAbsolute(m_start_time),m_end_time,m_fps);
              if (med!=NULL)
              {
                strcpy(med->m_tag,m_tag);
                file=findFile(mediatree,m_fname,0);
                if (file==NULL) med->color(FL_RED);
              }
            }
        }
        else if (line[0] == 'F') 
        {
            if (sscanf(line,"F %s %lg %lg %d %lg %lg %f %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       &m_fade_min,
                       &m_fade_max,
                       &m_fps,
                       m_tag)!=8)
            {
                error_flag=1;            
            }
            else
            {
              med=findTagMedia(m_start_ref);
              if (med!=NULL)
              {
                if (fabs(m_start_ref[0])=='<')
                {
                  med->setControl(0,m_start_time,m_fade_min);
                  med->setControl(1,m_end_time,m_fade_max);
                }
                else
                {
                  med->setControl(2,m_start_time,m_fade_min);
                  med->setControl(3,m_end_time,m_fade_max);
                }
              }
            }
        }
        else if (line[0] == 'A') 
        {
            if (sscanf(line,"A %s %lg %lg %d %s %lg %lg %lg %lg %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       m_fname,
                       &m_angle,
                       &m_dist,
                       &m_radius,
                       &m_start_fade,
                       m_tag)!=10)                
            {
                error_flag=1;
            }
            else
            {
              m_layer+=NB_DISP_VIDEO_LAYERS;
              resolveTime(m_start_ref,m_start_ref,&m_abs_start_time,&m_start_time,&m_end_time);
              if (m_layer>=0 && m_layer<NB_DISP_LAYERS) med=layers[m_layer]->addMedia(EXT_AUDIO,m_fname,layers[m_layer]->media[0]->convertTimeToPixelAbsolute(m_start_time),m_end_time,m_fps);
              if (med!=NULL)
              {
                strcpy(med->m_tag,m_tag);
                file=findFile(mediatree,m_fname,0);
                if (file==NULL) med->color(FL_RED);
                else
                {
                  strcpy(statsname,file->absname);
                  stripFromChar(statsname,'.');
                  strcat(statsname,".stats");
                  entryReadDataFromFile(statsname,&config); 
                  ret=entryGetFloat(config,STATS_LENGTH,&length);
                  if (ret==0)
                  {
                    med->len_x_orig=(double)(length);
                  }
                } 
              }
              if (m_layer>=NB_DISP_VIDEO_LAYERS && m_layer<NB_DISP_AUDIO_LAYERS+NB_DISP_VIDEO_LAYERS)
              {
                layers_angle[m_layer-NB_DISP_VIDEO_LAYERS]->value(m_angle);
              }
            }
        }
        else if (line[0] == 'B') 
        {
            if (sscanf(line,"B %s %lg %lg %d %lg %lg %f %s",
                       m_start_ref,
                       &m_start_time,
                       &m_end_time,
                       &m_layer,
                       &m_fade_min,
                       &m_fade_max,
                       &m_fps,
                       m_tag)!=8)                
            {
                error_flag=1;          
            }
            else
            {
              med=findTagMedia(m_start_ref);
              if (med!=NULL)
              {
                if (fabs(m_start_ref[0])=='<')
                {
                  med->setControl(0,m_start_time,m_fade_min);
                  med->setControl(1,m_end_time,m_fade_max);
                }
                else
                {
                  med->setControl(2,m_start_time,m_fade_min);
                  med->setControl(3,m_end_time,m_fade_max);
                }
              }
            }
        }
        else
        {
            error_flag=1;
        }
        if (error_flag) fprintf(stderr,"Warning: could not parse playlist file '%s' at line %d.\n",playlist_file,lineno);
        lineno++;
        entryFree(&config);
    }

    fclose(fp);

    return (0);
}

int playlistTimeline::savePlaylist(Directory *mediatree)
{
    int i,j,index;
    int type;
    playlistMedia *media;
    FILE *fp;
    int tag_vid_id;
    int tag_img_id;
    int tag_3d_id;
    int tag_aud_id;
    double fade_fps;
    unsigned char add_fade;
    double fade_start,fade_end;

    if (mediatree==NULL) return -1;

    //sprintf(playlist_file,"/home/chapdelv/fabrique/test.pll");
    
    fp = fopen(playlist_file,"w");    
    if (fp==NULL) {
	    fprintf(stderr,"Error: unable to open file '%s'\n",playlist_file);
	    return -1;
    }

    tag_vid_id=0;
    tag_img_id=0;
    tag_3d_id=0;
    tag_aud_id=0;
    fade_fps=15.0;

    for (i=0;i<NB_DISP_VIDEO_LAYERS;i++)
    {
      for (j=0;j<MAX_NB_MEDIA;j++)
      {
        if (layers[i]->media[j]->active)
        {
          add_fade=0;
          media=layers[i]->media[j];
          type=media->m_type;
          //strcpy(relname,findRelativePath(mediatree->absname,media->file->absname));
          if (type==EXT_VIDEO)
          {
            sprintf(media->m_tag,"video_%02d",tag_vid_id);
            fprintf(fp,"M %s %lg %lg %d %s %f %lg %s\n",
                       ZERO_ABSOLUTE,
                       media->start_x,
                       media->len_x,
                       i,
                       media->m_filename,
                       media->m_fps,
                       0.0,
                       media->m_tag);
            add_fade=1;
            tag_vid_id++;
          }
          else if (type==EXT_3D)
          {
            sprintf(media->m_tag,"3d_%02d",tag_3d_id);
            fprintf(fp,"T %s %lg %lg %d %s %lg %s\n",
                       ZERO_ABSOLUTE,
                       media->start_x,
                       media->len_x,
                       i,
                       media->m_filename,
                       0.0,
                       media->m_tag);
            add_fade=1;
            tag_3d_id++;
          }
          else if (type==EXT_IMAGE)
          {
            sprintf(media->m_tag,"img_%02d",tag_img_id);
            fprintf(fp,"I %s %lg %lg %d %s %lg %s\n",
                       ZERO_ABSOLUTE,
                       media->start_x,
                       media->len_x,
                       i,
                       media->m_filename,
                       0.0,
                       media->m_tag);
            add_fade=1;
            tag_img_id++;
          }
          else
          { 
            fprintf(stderr,"Warning: unrecognized media type.\n");
          }
          if (add_fade)
          {
            if (media->fade_ctrls[1]->start_x-media->start_x<media->fade_ctrls[2]->start_x-media->start_x)
            {
              fade_start=media->fade_ctrls[1]->start_x-media->start_x;
              fade_end=media->fade_ctrls[2]->start_x-media->start_x;
              if (fabs(fade_start)<0.0001) fade_start=0;
              if (fabs(fade_end)<0.0001) fade_end=0;
              fprintf(fp,"F <%s %lg %lg %d %lg %lg %f %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       i,
                       1.0-((double)(media->fade_ctrls[1]->y()+media->fade_ctrls[1]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[2]->y()+media->fade_ctrls[2]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);            
            }
            fade_start=media->fade_ctrls[0]->start_x-media->start_x;
            fade_end=media->fade_ctrls[1]->start_x-media->start_x;
            if (fabs(fade_start)<0.0001) fade_start=0;
            if (fabs(fade_end)<0.0001) fade_end=0;
            fprintf(fp,"F <%s %lg %lg %d %lg %lg %f %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       i,
                       1.0-((double)(media->fade_ctrls[0]->y()+media->fade_ctrls[0]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[1]->y()+media->fade_ctrls[1]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);            
            fade_start=media->fade_ctrls[2]->start_x-media->start_x-media->len_x;
            fade_end=media->fade_ctrls[3]->start_x-media->start_x-media->len_x;
            if (fabs(fade_start)<0.0001) fade_start=0;
            if (fabs(fade_end)<0.0001) fade_end=0;
            fprintf(fp,"F >%s %lg %lg %d %lg %lg %f %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       i,
                       1.0-((double)(media->fade_ctrls[2]->y()+media->fade_ctrls[2]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[3]->y()+media->fade_ctrls[3]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);            
          }
        }
      }
    }
    for (index=0;index<NB_DISP_AUDIO_LAYERS;index++)
    {
      i=index+NB_DISP_VIDEO_LAYERS;
      for (j=0;j<MAX_NB_MEDIA;j++)
      {
        if (layers[i]->media[j]->active) //audio file
        {
          media=layers[i]->media[j];
          //strcpy(relname,findRelativePath(mediatree->absname,media->file->absname));
          sprintf(media->m_tag,"aud_%02d",tag_aud_id);
          //load and play
          fprintf(fp,"A %s %lg %lg %d %s %lg %lg %lg %lg %s\n",
                       ZERO_ABSOLUTE,
                       media->start_x,
                       media->len_x,
                       index,
                       media->m_filename,
                       layers_angle[index]->value(),
                       1.0,
                       1.0,
                       0.0,
                       media->m_tag);
            tag_aud_id++;
          //fade
          if (media->fade_ctrls[1]->start_x-media->start_x<media->fade_ctrls[2]->start_x-media->start_x)
          {
            fade_start=media->fade_ctrls[1]->start_x-media->start_x;
            fade_end=media->fade_ctrls[2]->start_x-media->start_x;
            if (fabs(fade_start)<0.0001) fade_start=0;
            if (fabs(fade_end)<0.0001) fade_end=0;
            fprintf(fp,"B <%s %lg %lg %d %lg %lg %lg %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       index,
                       1.0-((double)(media->fade_ctrls[1]->y()+media->fade_ctrls[1]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[2]->y()+media->fade_ctrls[2]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);
          }
          fade_start=media->fade_ctrls[0]->start_x-media->start_x;
          fade_end=media->fade_ctrls[1]->start_x-media->start_x;
          if (fabs(fade_start)<0.0001) fade_start=0;
          if (fabs(fade_end)<0.0001) fade_end=0;
          fprintf(fp,"B <%s %lg %lg %d %lg %lg %lg %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       index,
                       1.0-((double)(media->fade_ctrls[0]->y()+media->fade_ctrls[0]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[1]->y()+media->fade_ctrls[1]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);
          fade_start=media->fade_ctrls[2]->start_x-media->start_x-media->len_x;
          fade_end=media->fade_ctrls[3]->start_x-media->start_x-media->len_x;
          if (fabs(fade_start)<0.0001) fade_start=0;
          if (fabs(fade_end)<0.0001) fade_end=0;
          fprintf(fp,"B >%s %lg %lg %d %lg %lg %lg %s\n",
                       media->m_tag,
                       fade_start,
                       fade_end,
                       index,
                       1.0-((double)(media->fade_ctrls[2]->y()+media->fade_ctrls[2]->h()/2)-layers[i]->y())/layers[i]->h(),
                       1.0-((double)(media->fade_ctrls[3]->y()+media->fade_ctrls[3]->h()/2)-layers[i]->y())/layers[i]->h(),
                       fade_fps,
                       NO_TAG);
        }
      }
    }

    fclose(fp);


    return 0;
}

