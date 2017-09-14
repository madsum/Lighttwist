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

#include "lighttwist.h"

//global images
imgu *nodata;

//global variables
blendWindow *blendwin;
cam2projWindow *cam2projwin;
uv2projWindow *uv2projwin;
colorWindow *colorwin;
uvmapWindow *uvwin;
shellWindow *shell;
gammaWindow *gammawin;
captureWindow *capwin;
liveWindow *livewin;
playWindow *playwin;
precropWindow *precropwin;
projectWindow *prj;
smoothWindow *smoothwin;
patternWindow *patwin;

char *login; //user linux login name
extern Machine machines[MAX_NB_MACHINES];
extern char remote_bin_path[PATH_SIZE];

char config_file[PATH_SIZE];
Entry *config;
Entry *project;
imgu *img;
lighttwistUI *ui;

Directory *mediatree;
Directory *playlisttree;
Directory *importedtree;

tcpcast *pd_main_ctrl;
tcpcast *pd_playlist;
tcpcast *pd_snd_channels;

imgu *uv_ctrl_img,*uv_ctrl_img_x;

unsigned char OPTION_DISPLAY_PD;
unsigned char OPTION_FORCE_SELECT;
unsigned char OPTION_FORCE_SELECT_NOISE;
char OPTION_START_PLL[PATH_SIZE];
unsigned char OPTION_START_JOYSTICK;

#define TIME_DELAY 150000

//the string keys in the project file are only used in the following 2 functions (loadProject() and saveProject())
//hence, they can safely be modified
int loadProject(Entry **project,char *name)
{
    int i;
    int ret;
    char key[DATA_SIZE];
    char *str;
    int ival;
    int ret2,ival2;
    float fval;

    if (name==NULL) return -1;

    prj->project->value(name);

    entryReadDataFromFile(name,project);

    //ret=entryGetInteger((*project),"CONFIGURATION",&ival);
    //if (ret==0)
    //{
    //  prj->configuration->value(ival);
    //  updateUVMapWindow();
    //}
    str=entryGetString((*project),"CONFIGURATION");
    if (str!=NULL)
    {
        prj->configuration->value(findListItemFromLabel(prj->configuration_items,str));
        updateUVMapWindow();
    }
    ret=entryGetInteger((*project),"VMIRROR",&ival);
    if (ret==0) prj->vmirror->value(ival);
    ret=entryGetInteger((*project),"MODE",&ival);
    if (ret==0) prj->mode->value(ival);
    ret=entryGetFloat((*project),"SCREEN_RATIO",&fval);
    if (ret==0) prj->screen_ratio->value(fval);
    ret=entryGetInteger((*project),"NB_PROJECTORS",&ival);
    if (ret==0) prj->nb_projectors->value(ival);
    //ret=entryGetInteger((*project),"PROJECTOR_RESOLUTION",&ival);
    //if (ret==0) prj->resolution->value(ival);
    str=entryGetString((*project),"PROJECTOR_RESOLUTION");
    if (str!=NULL) prj->resolution->value(findListItemFromLabel(prj->resolution_items,str));
    ret=entryGetInteger((*project),"NB_SPEAKERS",&ival);
    if (ret==0) prj->nb_speakers->value(ival);
    str=entryGetString((*project),"SOUND_CTRL_IP");
    if (str!=NULL) prj->sound_ctrl_ip->value(str);
    ret=entryGetInteger((*project),"ENABLE_JACK",&ival);
    if (ret==0) prj->enable_jack->value(ival);

    str=entryGetString((*project),"LOCAL_MEDIA");
    if (str!=NULL) prj->local_media->value(str);
    str=entryGetString((*project),"LOCAL_SCAN");
    if (str!=NULL) prj->local_scan->value(str);
    str=entryGetString((*project),"LOCAL_DATA");
    if (str!=NULL) prj->local_data->value(str);
    str=entryGetString((*project),"LOCAL_BIN");
    if (str!=NULL) prj->local_bin->value(str);
    str=entryGetString((*project),"LOCAL_LIB");
    if (str!=NULL) prj->local_lib->value(str);
    str=entryGetString((*project),"LOCAL_SHARE");
    if (str!=NULL) prj->local_share->value(str);

    str=entryGetString((*project),"REMOTE_SCAN");
    if (str!=NULL) prj->remote_scan->value(str);
    str=entryGetString((*project),"REMOTE_DATA");
    if (str!=NULL) prj->remote_data->value(str);
    str=entryGetString((*project),"REMOTE_BIN");
    if (str!=NULL)
    {
      prj->remote_bin->value(str);
      strcpy(remote_bin_path,str);
    }
    str=entryGetString((*project),"REMOTE_SHARE");
    if (str!=NULL) prj->remote_share->value(str);
    for (i=0;i<MAX_NB_MACHINES;i++)
    {
        sprintf(key,"MACHINE_%d_IP",i+1);
        str=entryGetString((*project),key);
        if (str!=NULL)
        {
          ui->m_ip[i]->value(str);
        }
    }
    for (i=0;i<MAX_NB_SPEAKERS;i++)
    {
        sprintf(key,"SPEAKER_%d_X",i+1);
        ret=entryGetInteger((*project),key,&ival);
        sprintf(key,"SPEAKER_%d_Y",i+1);
        ret2=entryGetInteger((*project),key,&ival2);
        if (ret==0 && ret2==0) ui->speakers[i]->setPosition(ival,ival2);
    }
    ret=entryGetInteger((*project),"SUBWOOFER_INDEX",&ival);
    if (ret==0) ui->subwoofer_index->value(ival);

    //several values in the subwindows (scan process for instance)
    str=entryGetString((*project),"PATTERN_FREQUENCY");
    if (str!=NULL) patwin->frequency->value(findListItemFromLabel(patwin->frequency_items,str));
    str=entryGetString((*project),"PATTERN_NB");
    if (str!=NULL) patwin->nb_patterns->value(findListItemFromLabel(patwin->nb_patterns_items,str));

    ret=entryGetInteger((*project),"CAPTURE_CAMERA",&ival);
    if (ret==0) capwin->cam->value(ival);
    ret=entryGetFloat((*project),"CAPTURE_EXPOSURE_TIME",&fval);
    if (ret==0) capwin->exposure_time->value(fval);    
    str=entryGetString((*project),"CAPTURE_CAMID");
    if (str!=NULL) capwin->camId->value(str);    
    ret=entryGetFloat((*project),"CAPTURE_DISTO_K1",&fval);
    if (ret==0) capwin->camDistoK1->value(fval);
    ret=entryGetFloat((*project),"CAPTURE_DISTO_K2",&fval);
    if (ret==0) capwin->camDistoK2->value(fval);
    ret=entryGetInteger((*project),"CAPTURE_TRANSFER",&ival);
    if (ret==0) capwin->transfer->value(ival);

    ret=entryGetFloat((*project),"CAM2PROJ_MIN_CONFIDENCE",&fval);
    if (ret==0) cam2projwin->setMinRangePercentageValue(fval);
    ret=entryGetFloat((*project),"CAM2PROJ_MIN_RANGE",&fval);
    if (ret==0) cam2projwin->setMinRangeValue(fval);
    ret=entryGetFloat((*project),"CAM2PROJ_CONFIDENCE",&fval);
    if (ret==0) cam2projwin->setConfidenceValue(fval);
    ret=entryGetInteger((*project),"CAM2PROJ_NOISE",&ival);
    if (ret==0) cam2projwin->noise_mode->value(ival);
    ret=entryGetInteger((*project),"CAM2PROJ_CROP",&ival);
    if (ret==0) cam2projwin->crop_triangles->value(ival);

    ret=entryGetInteger((*project),"SMOOTH_NB_PATCHES_X",&ival);
    if (ret==0) smoothwin->nb_patches_x->value(ival);
    ret=entryGetInteger((*project),"SMOOTH_NB_PATCHES_Y",&ival);
    if (ret==0) smoothwin->nb_patches_y->value(ival);

    ret=entryGetInteger((*project),"GAMMA_MANUAL",&ival);
    if (ret==0) gammawin->gamma_manual->value(ival);
    ret=entryGetFloat((*project),"GAMMA_CAMERA_R",&fval);
    if (ret==0) gammawin->gamma_camera[0]->value(fval);
    ret=entryGetFloat((*project),"GAMMA_CAMERA_G",&fval);
    if (ret==0) gammawin->gamma_camera[1]->value(fval);
    ret=entryGetFloat((*project),"GAMMA_CAMERA_B",&fval);
    if (ret==0) gammawin->gamma_camera[2]->value(fval);
    ret=entryGetFloat((*project),"GAMMA_PROJECTOR_R",&fval);
    if (ret==0) gammawin->gamma_projector[0]->value(fval);
    ret=entryGetFloat((*project),"GAMMA_PROJECTOR_G",&fval);
    if (ret==0) gammawin->gamma_projector[1]->value(fval);
    ret=entryGetFloat((*project),"GAMMA_PROJECTOR_B",&fval);
    if (ret==0) gammawin->gamma_projector[2]->value(fval);
    gammawin->cb_manual(gammawin->gamma_manual,NULL);

    ret=entryGetFloat((*project),"UV2PROJ_ANGLE_PROJ_MAX",&fval);
    if (ret==0) uv2projwin->angle_proj_max->value(fval);

    ret=entryGetInteger((*project),"SCAN_MODE",&ival);
    if (ret==0)
    {
      ui->scan_mode->value(ival);
      ui->updateScanDisplay();
    }

    updateMachineConfiguration();
    updateSpeakerConfiguration();

    return 0;
}

int saveProject(Entry **project,char *name)
{
    int i;
    char key[DATA_SIZE];
    char str[PATH_SIZE];
    char dir[PATH_SIZE];
    char *temp;

    if (name==NULL) return -1;

    //sprintf(str,"%d",prj->configuration->value());
    sprintf(str,"%s",prj->configuration->mvalue()->label());
    entryUpdate(project,"CONFIGURATION",str);
    sprintf(str,"%d",prj->vmirror->value());
    entryUpdate(project,"VMIRROR",str);
    sprintf(str,"%d",prj->mode->value());
    entryUpdate(project,"MODE",str);
    sprintf(str,"%f",prj->screen_ratio->value());
    entryUpdate(project,"SCREEN_RATIO",str);
    sprintf(str,"%d",(int)(prj->nb_projectors->value()));
    entryUpdate(project,"NB_PROJECTORS",str);
    //sprintf(str,"%d",prj->resolution->value());
    sprintf(str,"%s",prj->resolution->mvalue()->label());
    entryUpdate(project,"PROJECTOR_RESOLUTION",str);
    sprintf(str,"%d",(int)(prj->nb_speakers->value()));
    entryUpdate(project,"NB_SPEAKERS",str);
    sprintf(str,"%s",prj->sound_ctrl_ip->value());
    entryUpdate(project,"SOUND_CTRL_IP",str);
    sprintf(str,"%d",prj->enable_jack->value());
    entryUpdate(project,"ENABLE_JACK",str);

    sprintf(str,"%s",prj->local_media->value());
    entryUpdate(project,"LOCAL_MEDIA",str);
    sprintf(str,"%s",prj->local_scan->value());
    entryUpdate(project,"LOCAL_SCAN",str);
    sprintf(str,"%s",prj->local_data->value());
    entryUpdate(project,"LOCAL_DATA",str);
    sprintf(str,"%s",prj->local_bin->value());
    entryUpdate(project,"LOCAL_BIN",str);
    sprintf(str,"%s",prj->local_lib->value());
    entryUpdate(project,"LOCAL_LIB",str);
    sprintf(str,"%s",prj->local_share->value());
    entryUpdate(project,"LOCAL_SHARE",str);

    sprintf(str,"%s",prj->remote_scan->value());
    entryUpdate(project,"REMOTE_SCAN",str);
    sprintf(str,"%s",prj->remote_data->value());
    entryUpdate(project,"REMOTE_DATA",str);
    sprintf(str,"%s",prj->remote_bin->value());
    entryUpdate(project,"REMOTE_BIN",str);
    sprintf(str,"%s",prj->remote_share->value());
    entryUpdate(project,"REMOTE_SHARE",str);

    for (i=0;i<MAX_NB_MACHINES;i++)
    {
        sprintf(key,"MACHINE_%d_IP",i+1);
        sprintf(str,"%s",ui->m_ip[i]->value());
        entryUpdate(project,key,str);
    }
    for (i=0;i<MAX_NB_SPEAKERS;i++)
    {
        sprintf(key,"SPEAKER_%d_X",i+1);
        sprintf(str,"%d",ui->speakers[i]->x());
        entryUpdate(project,key,str);
        sprintf(key,"SPEAKER_%d_Y",i+1);
        sprintf(str,"%d",ui->speakers[i]->y());
        entryUpdate(project,key,str);
    }
    sprintf(str,"%d",(int)(ui->subwoofer_index->value()));
    entryUpdate(project,"SUBWOOFER_INDEX",str);

    //several values in the subwindows (scan process for instance)
    sprintf(str,"%s",patwin->frequency->mvalue()->label());
    entryUpdate(project,"PATTERN_FREQUENCY",str);
    sprintf(str,"%s",patwin->nb_patterns->mvalue()->label());
    entryUpdate(project,"PATTERN_NB",str);

    sprintf(str,"%d",capwin->cam->value());
    entryUpdate(project,"CAPTURE_CAMERA",str);
    sprintf(str,"%f",capwin->exposure_time->value());
    entryUpdate(project,"CAPTURE_EXPOSURE_TIME",str);
    if (capwin->camId->value() != NULL) {
        sprintf(str,"%s",capwin->camId->value());
        entryUpdate(project,"CAPTURE_CAMID",str);
    }
    sprintf(str,"%f",capwin->camDistoK1->value());
    entryUpdate(project,"CAPTURE_DISTO_K1",str);
    sprintf(str,"%f",capwin->camDistoK2->value());
    entryUpdate(project,"CAPTURE_DISTO_K2",str);
    sprintf(str,"%d",capwin->transfer->value());
    entryUpdate(project,"CAPTURE_TRANSFER",str);

    sprintf(str,"%f",cam2projwin->getMinRangePercentageValue());
    entryUpdate(project,"CAM2PROJ_MIN_CONFIDENCE",str);
    sprintf(str,"%f",cam2projwin->getMinRangeValue());
    entryUpdate(project,"CAM2PROJ_MIN_RANGE",str);
    sprintf(str,"%f",cam2projwin->getConfidenceValue());
    entryUpdate(project,"CAM2PROJ_CONFIDENCE",str);
    sprintf(str,"%d",cam2projwin->noise_mode->value());
    entryUpdate(project,"CAM2PROJ_NOISE",str);
    sprintf(str,"%d",cam2projwin->crop_triangles->value());
    entryUpdate(project,"CAM2PROJ_CROP",str);

    sprintf(str,"%d",(int)(smoothwin->nb_patches_x->value()));
    entryUpdate(project,"SMOOTH_NB_PATCHES_X",str);
    sprintf(str,"%d",(int)(smoothwin->nb_patches_y->value()));
    entryUpdate(project,"SMOOTH_NB_PATCHES_Y",str);

    sprintf(str,"%d",(int)(gammawin->gamma_manual->value()));
    entryUpdate(project,"GAMMA_MANUAL",str);
    sprintf(str,"%f",gammawin->gamma_camera[0]->value());
    entryUpdate(project,"GAMMA_CAMERA_R",str);
    sprintf(str,"%f",gammawin->gamma_camera[1]->value());
    entryUpdate(project,"GAMMA_CAMERA_G",str);
    sprintf(str,"%f",gammawin->gamma_camera[2]->value());
    entryUpdate(project,"GAMMA_CAMERA_B",str);
    sprintf(str,"%f",gammawin->gamma_projector[0]->value());
    entryUpdate(project,"GAMMA_PROJECTOR_R",str);
    sprintf(str,"%f",gammawin->gamma_projector[1]->value());
    entryUpdate(project,"GAMMA_PROJECTOR_G",str);
    sprintf(str,"%f",gammawin->gamma_projector[2]->value());
    entryUpdate(project,"GAMMA_PROJECTOR_B",str);

    sprintf(str,"%f",uv2projwin->angle_proj_max->value());
    entryUpdate(project,"UV2PROJ_ANGLE_PROJ_MAX",str);

    sprintf(str,"%d",ui->scan_mode->value());
    entryUpdate(project,"SCAN_MODE",str);

    strcpy(dir, name);
    temp = strrchr(dir, '/');
    dir[strlen(name)-strlen(temp)] = '\0';

    fioCreateDirectories(dir,0,-1);
    entryWriteData(name,(*project));

    return 0;
}

int loadCrop()
{
    int i;
    char uvimgfile[PATH_SIZE];
    char uvfile[PATH_SIZE];
    int uv_type;
    double uv_inner_scale,uv_outer_scale;
    double uv_vcenter;
    uv_coord_struct uv_coord[NB_UV_COORD_MAX];

    sprintf(uvfile,"%s/out/uvmap.txt",prj->local_scan->value());

    if (uvReadFile(uvfile)) return -1;

    uvGetType(&uv_type);
    uvGetScales(&uv_inner_scale,&uv_outer_scale);
    uvGetCoords(uv_coord);
    uvGetVCenter(&uv_vcenter);

    uvwin->scale_inner->value(uv_inner_scale);
    uvwin->scale_outer->value(uv_outer_scale);
    uvwin->vcenter->value(uv_vcenter);

    for (i=0;i<MAX_NB_UV_CTRLS;i++)
    {
        uvwin->controls_inner[i]->setPosition(uv_coord[i].x-uvwin->controls_inner[i]->w()/2-uvwin->background->x(),uv_coord[i].y-uvwin->controls_inner[i]->h()/2-uvwin->background->y());
    }

    for (i=0;i<MAX_NB_UV_CTRLS;i++)
    {
        uvwin->controls_outer[i]->setPosition(uv_coord[MAX_NB_UV_CTRLS+i].x-uvwin->controls_outer[i]->w()/2-uvwin->background->x(),uv_coord[MAX_NB_UV_CTRLS+i].y-uvwin->controls_outer[i]->h()/2-uvwin->background->y());
    }

    sprintf(uvimgfile,"%s/out/uvmap.png",prj->local_scan->value());
    //uvComputeImg(ubimgfile,uvwin->background->getImgWidth(),uvwin->background->getImgHeight());

    return 0;
}

int saveCrop()
{
    int i;
    char uvfile[PATH_SIZE];
    char uvimgfile[PATH_SIZE];
    uv_coord_struct uv_coord[NB_UV_COORD_MAX];

    sprintf(uvfile,"%s/out/uvmap.txt",prj->local_scan->value());

    for (i=0;i<MAX_NB_UV_CTRLS;i++)
    {
        uv_coord[i].x=uvwin->controls_inner[i]->x()+uvwin->controls_inner[i]->w()/2-uvwin->background->x();
        uv_coord[i].y=uvwin->controls_inner[i]->y()+uvwin->controls_inner[i]->h()/2-uvwin->background->y();
    }


    for (i=0;i<MAX_NB_UV_CTRLS;i++)
    {
        uv_coord[MAX_NB_UV_CTRLS+i].x=uvwin->controls_outer[i]->x()+uvwin->controls_outer[i]->w()/2-uvwin->background->x();
        uv_coord[MAX_NB_UV_CTRLS+i].y=uvwin->controls_outer[i]->y()+uvwin->controls_outer[i]->h()/2-uvwin->background->y();
    }

    uvSetCoords(uv_coord);

    uvSetScales(uvwin->scale_inner->value(),uvwin->scale_outer->value());
    uvSetVCenter(uvwin->vcenter->value());

    if (prj->configuration->value()==CYCLORAMA_CONFIGURATION) uvSetType(UV_DUAL_CIRCLES);
    else if (prj->configuration->value()==HOMOGRAPHY_CONFIGURATION) uvSetType(UV_HOMOGRAPHY);
    else if (prj->configuration->value()==BEZIER_CONFIGURATION) uvSetType(UV_BEZIER);
    else uvSetType(UV_RECTANGLE);

    sprintf(uvimgfile,"%s/out/uvmap.png",prj->local_scan->value());

    if (uvSaveFile(uvfile)) return -1;
    if (uvReadFile(uvfile)) return -1; //read the crop file to set internal variables in crop.c
    uvComputeImg(uvimgfile,uvwin->background->getImgWidth(),uvwin->background->getImgHeight());
    updateDataImage(uvimgfile);

    return 0;
}

//various functions related to updates of the user interface
//update the image of the scan tab
int updateDataImage(const char *filename)
{
    int ret;
    double sx,sy;
    imgu *I;

    I=NULL;
    if (imguLoad(&I,filename,LOAD_16_BITS))
    {
        ui->scan_data->align(FL_ALIGN_INSIDE|FL_ALIGN_CENTER);
        ui->scan_data->label(langGetLabel(LABEL_NO_DATA));
        ui->scan_data->setImage(NULL);
        if (nodata!=NULL) ui->scan_data->setSize(nodata->xs,nodata->ys);
        ret=-1;
    }
    else
    {
        sx=((double)(nodata->xs))/I->xs;
        sy=((double)(nodata->ys))/I->ys;
        if (sx<sy) sy=sx;
        else sx=sy;
        imguScale(&I,I,sx,sy);
        ui->scan_data->label(NULL);
        ui->scan_data->setImage(I);
        ret=0;
    }

    imguFree(&I);

    ui->redraw();
    Fl::check();
    return ret;
}

//update the image of the sound tab
int updateSoundImage(const char *filename)
{
    int ret;
    imgu *I=NULL;

    if (imguLoad(&I,filename,LOAD_16_BITS))
    {
        ui->sound_space->setImage(nodata);
        ret=-1;
    }
    else
    {
        if (nodata!=NULL) imguScale(&I,I,((double)(nodata->xs))/I->xs,((double)(nodata->ys))/I->ys);
        ui->sound_space->setImage(I);
        ret=0;
    }

    imguFree(&I);

    ui->redraw();
    Fl::check();
    return ret;
}

//update the status message
int updateStatus(const char *msg)
{
    if (msg==NULL) return -1;
    else
    {
        ui->status->label(msg);
        return 0;
    }
}

//update media tree in media tab
void updateLocalMedia()
{
    const char *ext_media[NB_EXT_TYPES];
    ext_media[EXT_VIDEO]=ui->ext_video->value();
    ext_media[EXT_AUDIO]=ui->ext_audio->value();
    ext_media[EXT_3D]=ui->ext_3d->value();
    ext_media[EXT_IMAGE]=ui->ext_image->value();
    ext_media[EXT_STATS]=NULL;
    ext_media[EXT_PLAYLIST]=NULL;
    ui->loadLocalMedia(&mediatree,prj->local_media->value(),MEDIA_X,MEDIA_Y,ext_media,cb_media_directory,cb_media_check_directory);
    updateMediaTree(mediatree); //minimize tree
    updateMediaTree(mediatree); //expand first level
    ui->redraw();
}

//update playlist tree in show tab
void updateLocalPlaylists()
{
    const char *ext_media[NB_EXT_TYPES];
    ext_media[EXT_VIDEO]=NULL;
    ext_media[EXT_AUDIO]=NULL;
    ext_media[EXT_3D]=NULL;
    ext_media[EXT_IMAGE]=NULL;
    ext_media[EXT_STATS]=NULL;
    ext_media[EXT_PLAYLIST]=ui->ext_playlist->value();
    ui->loadLocalPlaylists(&playlisttree,prj->local_media->value(),MEDIA_X,MEDIA_Y,ext_media,cb_playlist_directory,cb_playlist_check_directory);
    updateMediaTree(playlisttree); //minimize tree
    updateMediaTree(playlisttree); //expand first level
    ui->redraw();
}

//update playlist tree in show tab
void updateImportedMedia()
{
    const char *ext_media[NB_EXT_TYPES];
    ext_media[EXT_VIDEO]=NULL;
    ext_media[EXT_AUDIO]=NULL;
    ext_media[EXT_3D]=NULL;
    ext_media[EXT_IMAGE]=NULL;
    ext_media[EXT_STATS]=ui->ext_stats->value();
    ext_media[EXT_PLAYLIST]=NULL;
    ui->loadImportedMedia(&importedtree,prj->local_media->value(),0,0,ext_media,cb_imported_directory,cb_imported_check_directory);
    if (ui->pl_path_label!=NULL)
    {
        if (importedtree!=NULL) ui->pl_path_label->label(importedtree->absname);
        else ui->pl_path_label->label("");
    }
    if (ui->media_path_label!=NULL)
    {
        if (importedtree!=NULL) ui->media_path_label->label(importedtree->absname);
        else ui->media_path_label->label("");
    }
    identifyImportedMedia(mediatree,importedtree);
    ui->redraw();
}

void updateUVMapWindow()
{
    uvwin->setUVConfiguration(prj->configuration->value());
    if (prj->configuration->value()==BEZIER_CONFIGURATION) 
        uvwin->controls_outer[2]->setImage(uv_ctrl_img);
    else if (prj->configuration->value()==HOMOGRAPHY_CONFIGURATION)
        // Le coin haut-gauche de l'image resultante correspond a un X rouge
        uvwin->controls_inner[3]->setImage(uv_ctrl_img_x);
    else 
        uvwin->controls_outer[2]->setImage(uv_ctrl_img_x);
}

//change menus based on the number of machines and mono/stereo mode
void updateMachineConfiguration()
{
    int i;
    char str[DATA_SIZE];

    //change the Setup tab
    for (i=0;i<prj->nb_projectors->value();i++)
    {
        ui->m_ip[i]->show();
        ui->m_test[i]->show();
        if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))) //stereo
        {
            ui->m_ip[i]->resize(MARGIN_LEFT_STEREO,ui->m_ip[i]->y(),ui->m_ip[i]->w(),ui->m_ip[i]->h());
            ui->m_test[i]->position(MARGIN_LEFT_STEREO+ui->m_ip[i]->w(),ui->m_test[i]->y());
            ui->m_ip[i+MAX_NB_MACHINES_PER_CHANNEL]->show();
            ui->m_test[i+MAX_NB_MACHINES_PER_CHANNEL]->show();
        }
        else
        {
            ui->m_ip[i]->resize(MARGIN_LEFT_MONO,ui->m_ip[i]->y(),ui->m_ip[i]->w(),ui->m_ip[i]->h());
            ui->m_test[i]->position(MARGIN_LEFT_MONO+ui->m_ip[i]->w(),ui->m_test[i]->y());
            ui->m_ip[i+MAX_NB_MACHINES_PER_CHANNEL]->hide();
            ui->m_test[i+MAX_NB_MACHINES_PER_CHANNEL]->hide();
        }
    }
    for (;i<MAX_NB_MACHINES_PER_CHANNEL;i++)
    {
        ui->m_ip[i]->hide();
        ui->m_test[i]->hide();
        ui->m_ip[i+MAX_NB_MACHINES_PER_CHANNEL]->hide();
        ui->m_test[i+MAX_NB_MACHINES_PER_CHANNEL]->hide();
    }

    ui->show();
    ui->redraw();

    //change 'Machine' dropdown list in menus
    capwin->machine->clear();
    cam2projwin->machine->clear();
    smoothwin->machine->clear();
    gammawin->machine->clear();
    colorwin->machine->clear();
    blendwin->machine->clear();
    uv2projwin->machine->clear();
    precropwin->machine->clear();
    if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P)))
    {
        capwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        cam2projwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        smoothwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        gammawin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        colorwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        blendwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        uv2projwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        precropwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));

        capwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        capwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        cam2projwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        cam2projwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        smoothwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        smoothwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        gammawin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        gammawin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        colorwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        colorwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        blendwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        blendwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        uv2projwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        uv2projwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));
        precropwin->machine->add(langGetLabel(LABEL_CHOICE_LEFT));
        precropwin->machine->add(langGetLabel(LABEL_CHOICE_RIGHT));

        for (i=0;i<prj->nb_projectors->value();i++)
        {
            sprintf(str,"%d",i);
            capwin->machine->add(str);
            cam2projwin->machine->add(str);
            smoothwin->machine->add(str);
            gammawin->machine->add(str);
            colorwin->machine->add(str);
            //blendwin->machine->add(str);
            uv2projwin->machine->add(str);
            //precropwin->machine->add(str);
        }
        for (i=0;i<prj->nb_projectors->value();i++)
        {
            sprintf(str,"%d",i+MAX_NB_MACHINES_PER_CHANNEL);
            capwin->machine->add(str);
            cam2projwin->machine->add(str);
            smoothwin->machine->add(str);
            gammawin->machine->add(str);
            colorwin->machine->add(str);
            //blendwin->machine->add(str);
            uv2projwin->machine->add(str);
            //precropwin->machine->add(str);
        }
    }
    else
    {
        capwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        cam2projwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        smoothwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        gammawin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        colorwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        blendwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        uv2projwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        precropwin->machine->add(langGetLabel(LABEL_CHOICE_ALL));
        for (i=0;i<prj->nb_projectors->value();i++)
        {
            sprintf(str,"%d",i);
            capwin->machine->add(str);
            cam2projwin->machine->add(str);
            smoothwin->machine->add(str);
            gammawin->machine->add(str);
            colorwin->machine->add(str);
            //blendwin->machine->add(str);
            uv2projwin->machine->add(str);
            //precropwin->machine->add(str);
        }
    }
    capwin->machine->value(0);
    cam2projwin->machine->value(0);
    smoothwin->machine->value(0);
    gammawin->machine->value(0);
    colorwin->machine->value(0);
    blendwin->machine->value(0);
    uv2projwin->machine->value(0);
    precropwin->machine->value(0);
}

void updateSpeakerConfiguration()
{
    int i;

    for (i=0;i<MAX_NB_SPEAKERS;i++)
    {
        ui->speakers[i]->hide();
    }

    for (i=0;i<prj->nb_speakers->value();i++)
    {
        ui->speakers[i]->show();
    }

    if (ui->subwoofer_index->value()>prj->nb_speakers->value()) ui->subwoofer_index->value(1);
    ui->subwoofer_index->maximum(prj->nb_speakers->value());
}

//select machines for operation
//'str' is a selected menu item (either a machine number or 'All','Left channel','Right channel')
int selectMachinesAndExecute(const char *str,int (*operfunc)(int i))
{
    int i,k,ret;

    for (i=0;i<MAX_NB_MACHINES;i++) machines[i].selected=0;

    if (str==NULL) return -1;

    if (!strcmp(str,langGetLabel(LABEL_CHOICE_LEFT)))
    {
        for (i=0;i<MAX_NB_MACHINES_PER_CHANNEL;i++)
        {
            if (ui->m_ip[i]->visible()) machines[i].selected=1;
        }
    }
    else if (!strcmp(str,langGetLabel(LABEL_CHOICE_RIGHT)))
    {
        for (i=0;i<MAX_NB_MACHINES_PER_CHANNEL;i++)
        {
            if (ui->m_ip[i+MAX_NB_MACHINES_PER_CHANNEL]->visible()) machines[i+MAX_NB_MACHINES_PER_CHANNEL].selected=1;
        }
    }
    else if (!strcmp(str,langGetLabel(LABEL_CHOICE_ALL)))
    {
        for (i=0;i<MAX_NB_MACHINES;i++)
        {
            if (ui->m_ip[i]->visible()) machines[i].selected=1;
        }
    }
    else
    {
        i=atoi(str);
        machines[i].selected=1;
    }

    ret=0;
    for (i=0;i<MAX_NB_MACHINES;i++)
    {
        if (machines[i].selected)
        {
            k=operfunc(i);
            ret|=k;
            if (k)
            {
                //fprintf(stderr,"Error!");
            }
            Fl::check();
            usleep(TIME_DELAY);
        }
    }

    return ret;
}

//select media in media tree and execute
int selectMediaAndExecute(Directory *tree,int (*operfunc)(const char *,int))
{
    File *currf;

    if (tree==NULL) return -1;

    while(tree!=NULL)
    {
        currf=tree->files;
        while(currf!=NULL)
        {
            if (currf->check->value()!=0)
            {
                operfunc(currf->absname,currf->type);
                usleep(TIME_DELAY);
            }
            currf=currf->next;
        }
        tree=dirSuccessor(tree);
    }

    return 0;
}

////////////////////////////////////////////////////////////
//all callbacks required for the interface
//menus will use these by declaring these functions as external
void cb_save(Fl_Widget *w, void*)
{
    char str[PATH_SIZE];

    sprintf(str,"%s",prj->project->value());
    saveProject(&project,str);
}

void cb_open(Fl_Widget *w, void *arg)
{
    int choice;
    char *file;

    choice=fl_choice(langGetLabel(LABEL_SAVE_PROJECT),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),langGetLabel(LABEL_UNDO));
    if (choice==1)
    {
        cb_save(w,arg);
    }
    if (choice!=2)
    {
        file=fl_file_chooser(langGetLabel(LABEL_PROJECT_CHOOSE_FILE),NULL,NULL,Fl_File_Chooser::SINGLE);
        if (file!=NULL)
        {
            selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),desactivate_remote_run);
            loadProject(&project,file);
            ui->show_setup_flag=BUTTON_DISABLED;
            cb_show_setup(NULL,NULL);
            cb_connect(NULL,0);
        }
    }
}

void cb_exit(Fl_Widget* w, void* data)
{
    char str[PATH_SIZE];
    int choice;

    //save project name first
    sprintf(str,"%s",prj->project->value());
    entryUpdate(&config,PROJECT_KEY,str);
    sprintf(str,"%s",ui->ext_video->value());
    entryUpdate(&config,EXT_VIDEO_KEY,str);
    sprintf(str,"%s",ui->ext_audio->value());
    entryUpdate(&config,EXT_AUDIO_KEY,str);
    sprintf(str,"%s",ui->ext_3d->value());
    entryUpdate(&config,EXT_3D_KEY,str);
    sprintf(str,"%s",ui->ext_image->value());
    entryUpdate(&config,EXT_IMAGE_KEY,str);
    sprintf(str,"%s",ui->ext_playlist->value());
    entryUpdate(&config,EXT_PLAYLIST_KEY,str);
    sprintf(str,"%d",ui->threading->value());
    entryUpdate(&config,PLAYER_THREADING,str);
    sprintf(str,"%d",ui->player_select->value());
    entryUpdate(&config,PLAYER_SELECT,str);
    entryWriteData(config_file,config);
    if (ui->usertype!=USER_ARTIST)
    {
        choice=fl_choice(langGetLabel(LABEL_SAVE_PROJECT),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),langGetLabel(LABEL_CANCEL));
        if (choice==1) //save
        {
            cb_save(w,data);
        }
        else if (choice==2) //cancel
        {
            return;
        }
    }

    stop_jack();
    stop_player_master();
    selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),stop_player_slave);
    selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),stop_background);
    selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),desactivate_remote_run);

    fflush(stderr);
    fflush(stdout);

    exit(0);
}

void cb_project(Fl_Widget *w, void*)
{
    prj->show();
}

void cb_update_project()
{
    updateMachineConfiguration();
    updateSpeakerConfiguration();
    updateLocalMedia();
    updateLocalPlaylists();
    updateImportedMedia();
    updateUVMapWindow();
}

int setTestPattern(int p)
{
    int i;
    if (p<TEST_PATTERN_WHITE || p>TEST_PATTERN_MAX) return -1;
    //unselect all patterns in menu
    for (i=0;i<TEST_PATTERN_MAX;i++) ui->test_pat[i]->value(0);
    //select pattern p
    ui->test_pat[p]->value(1);
    return 0;
}

void cb_connect(Fl_Widget *w, long index)
{
    int i,k;
    for (i=0;i<MAX_NB_MACHINES;i++)
    {
       strcpy(machines[i].ip,ui->m_ip[i]->value());
    }
    strcpy(remote_bin_path,prj->remote_bin->value());

    k=selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),activate_remote_run);

    if (k) //an error occured
    {
        ui->show_setup_flag=BUTTON_DISABLED;
    }
    else
    {
        ui->show_setup_flag=BUTTON_ON;
    }

    for (i=0;i<MAX_NB_MACHINES;i++)
    {
      if (machines[i].connected) ui->m_ip[i]->color(FL_GREEN);
      else ui->m_ip[i]->color(FL_RED);
    }

    cb_show_setup(NULL,NULL);
    ui->redraw();
}

void cb_test_camera(Fl_Widget *w, void*)
{
    char outdir[PATH_SIZE];
    char cam[DATA_SIZE];
    int choice;

    capwin->hide();
    Fl::check();
    sprintf(outdir,"%s/out/",prj->local_scan->value());

    strcpy(cam,capwin->cam->mvalue()->label());

    if (strcmp(cam,CAMERA_GPHOTO2)==0 && strcmp(capwin->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_AUTOMATIC))==0)
    {
      choice=fl_choice(langGetLabel(LABEL_DELETE_CAMERA_FILES),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),NULL);
    }
    else choice=1;

    if (choice==1)
    {
      do_grab_camera(0,"test%02d",outdir,1,1,0,0); 
    }
}

void cb_scan_pattern(Fl_Widget *w, void*)
{
    patwin->show();
}

void cb_scan_grab(Fl_Widget *w, void*)
{
    capwin->show();
}

void cb_scan_uvmap(Fl_Widget *w, void*)
{
    do_uvmap();
}

void cb_scan_cam2proj(Fl_Widget *w, void*)
{
    cam2projwin->show();
}

void cb_scan_smooth(Fl_Widget *w, void*)
{
    smoothwin->show();
}

void cb_scan_gamma(Fl_Widget *w, void*)
{
    gammawin->show();
}

void cb_scan_color(Fl_Widget *w, void*)
{
    colorwin->show();
}

void cb_scan_blend(Fl_Widget *w, void*)
{
    blendwin->show();
}

void cb_scan_uv2proj(Fl_Widget *w, void*)
{
    uv2projwin->show();
}

void cb_scan_calibrate(Fl_Widget *w, void*)
{
  const char *str;

  str=cam2projwin->machine->mvalue()->label();
  selectMachinesAndExecute(str,do_cam2proj);
  str=smoothwin->machine->mvalue()->label();
  selectMachinesAndExecute(str,do_smooth);
  str=gammawin->machine->mvalue()->label();
  selectMachinesAndExecute(str,do_gamma);
  //str=colorwin->machine->mvalue()->label();
  //selectMachinesAndExecute(str,do_color);
  str=blendwin->machine->mvalue()->label();


  if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0) //stereo
  {
    do_blend(CHANNEL_LEFT);
    do_blend(CHANNEL_RIGHT);
  }
  else
  {
    do_blend(CHANNEL_MONO);
  }
  str=uv2projwin->machine->mvalue()->label();
  selectMachinesAndExecute(str,do_uv2proj);
}

void cb_media_precrop(Fl_Widget *w, void*)
{
    precropwin->mediatree=mediatree;
    precropwin->show();
}

void cb_media_delete(Fl_Widget *w, void*)
{
    int choice;
    choice=fl_choice(langGetLabel(LABEL_DELETE_MEDIA),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),langGetLabel(LABEL_UNDO));
    if (choice==1)
    {
        selectMediaAndExecute(mediatree,do_stats_delete);
    }
    updateImportedMedia();
    identifyImportedMedia(mediatree,importedtree);
}

void cb_show_setup(Fl_Widget *w, void*)
{
    if (ui->show_setup_flag==BUTTON_DISABLED)
    {
        ui->show_play_flag=BUTTON_DISABLED;
        ui->show_sound_flag=BUTTON_DISABLED;
        ui->show_loop_flag=BUTTON_DISABLED;
        ui->show_joystick_flag=BUTTON_DISABLED;
        ui->show_live_but->deactivate();
    }
    else if (ui->show_setup_flag==BUTTON_OFF) //setup was off, now on: activate other buttons
    {
        start_jack();
        selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),start_player_slave);
        start_player_master();
        ui->show_setup_flag=BUTTON_ON;
        ui->show_play_flag=BUTTON_OFF;
        ui->show_loop_flag=BUTTON_DISABLED;
        ui->show_sound_flag=BUTTON_ON;
        ui->show_joystick_flag=BUTTON_OFF;
        ui->show_live_but->activate();
    }
    else //setup was on, now off: deactivate other buttons
    {
        stop_joystick_ctrl();
        stop_player_master();
        selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),stop_background);
        selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),stop_player_slave);
        stop_jack();
        ui->show_setup_flag=BUTTON_OFF;
        ui->show_play_flag=BUTTON_DISABLED;
        ui->show_sound_flag=BUTTON_DISABLED;
        ui->show_loop_flag=BUTTON_DISABLED;
        ui->show_joystick_flag=BUTTON_DISABLED;
        livewin->hide();
        ui->show_live_but->deactivate();
    }
    ui->updateShowImages();
}

void cb_show_play(Fl_Widget *w, void*)
{
    if (ui->show_setup_flag!=BUTTON_DISABLED)
    {
        if (ui->show_play_flag==BUTTON_OFF) //play
        {
            //playwin->playlisttree=playlisttree;
            //playwin->show();
            selectMediaAndExecute(playlisttree,enqueue_playlist);
            play_playlist();
        }
        else //stop
        {
            stop_playlist();
        }

              #if 0
        if ( ui->show_play_flag==BUTTON_OFF )
        {
            printf ( "%s -- Start  Playing\n",__FUNCTION__ );
            selectMediaAndExecute ( playlisttree,enqueue_playlist );
            play_playlist();
            //ui->show_play_flag = BUTTON_ON;
        }
        else if ( ui->show_play_flag==BUTTON_ON )
        {
            printf ( "%s -- Stop Playing\n",__FUNCTION__ );
            //ui->show_play_flag = BUTTON_OFF;
            stop_playlist();
        }
            #endif
    }
}
// this is Fl_Idle_Handler funcgtion call back
// we are ignoring parameter.
// added in main function as Fl::add_idle(cb_hook_play)
// its delaration in lightwistUI.h
void cb_hook_play ( void* )
{
    // sate keeper for lighttwist player
    if ( ui->m_IsTimeToPlay &&  ui->m_ignore_cb )
    {
        {
            printf ( "%s -- Start  Playing\n",__FUNCTION__ );
            selectMediaAndExecute ( playlisttree,enqueue_playlist );
            play_playlist();
            ui->m_ignore_cb = false;
        }
    }
    else
    {
        if( !ui->m_IsTimeToPlay && ui->m_ignore_cb )
        {
            printf ( "%s -- Stop Playing\n",__FUNCTION__ );
            ui->m_ignore_cb = false;
            stop_playlist();
        }
    }
}

void cb_show_joystick(Fl_Widget *w, void*)
{
    if (ui->show_joystick_flag!=BUTTON_DISABLED)
    {
        if (ui->show_joystick_flag==BUTTON_OFF) //activate joystick
        {
          start_joystick_ctrl();
        }
        else //stop
        {
          stop_joystick_ctrl();   
        }
    }
}

void cb_show_live(Fl_Widget *w, void*)
{
    livewin->show();
    livewin->sendAllValues();
}

void cb_show_live_fade(Fl_Widget *w, long i)
{
    char message_pd[CMD_SIZE];

    sprintf(message_pd,"fade %f %f",(double)(i),(double)(1.0-livewin->live_fade[i]->value()));
    //printf("fade %f %f;\n",(double)(i),(double)(1.0-livewin->live_fade[i]->value()));

    pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);

    usleep(33333); //30 times per second
}

void cb_show_live_volume(Fl_Widget *w, long i)
{
    char message_pd[CMD_SIZE];

    if (ui->show_sound_flag==BUTTON_ON)
    {
        //update master volume
        sprintf(message_pd,"volume %f %f",(double)(i),(double)(1.0-livewin->live_volume[i]->value()));

        pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);
    }

    usleep(33333); //30 times per second
}

void cb_show_live_stereo(Fl_Widget *w, long i)
{
    char message_pd[CMD_SIZE];

    sprintf(message_pd,"stereo %f %f %f %f",(double)(livewin->live_stereo_B->value()),(double)(livewin->live_stereo_R->value()),(double)(livewin->live_stereo_distortion->value()),(double)(livewin->live_stereo_slit->value()));
    printf("%s\n",message_pd);
    pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);

    usleep(200000); //5 times per second
}

void cb_show_live_spk_scale(Fl_Widget *w, long i)
{
    char message_pd[CMD_SIZE];

    sprintf(message_pd,"spk_scale %f",(double)(livewin->live_speaker_scale->value()));
    printf("%s\n",message_pd);
    pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);

    usleep(200000); //5 times per second
}

void cb_show_live_gamma(Fl_Widget *w, long i)
{
    char message_pd[CMD_SIZE];
    vector3 gamma;
    int j;

    for (j=0;j<3;j++)
    {
      gamma[j]=livewin->live_gamma[j]->value();
      gamma[j]=2.0*(1.0-gamma[j])+1.0;
    }

    sprintf(message_pd,"gamma %f %f %f",gamma[0],gamma[1],gamma[2]);
    printf("%s\n",message_pd);
    pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);

    usleep(200000); //5 times per second
}

void cb_show_mute(Fl_Widget *w, void*)
{
    char message_pd[CMD_SIZE];

    if (ui->show_sound_flag==BUTTON_ON)
    {
        ui->show_sound_flag=BUTTON_OFF;
        sprintf(message_pd,"volume 0 0");
        pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd);
    }
    else if (ui->show_sound_flag==BUTTON_OFF)
    {
        ui->show_sound_flag=BUTTON_ON;
        cb_show_live_volume(NULL,0);
    }

    ui->updateShowImages();
}

Directory *updateDirectory(Directory *tree,void *absname)
{
    Directory *dir;
    char *str=(char *)(absname);

    //dir=findDirectoryFromID(tree,id);
    //if (dir==NULL) fprintf(stderr,"Warning: couldn't find id %ld\n",id);
    dir=findDirectoryFromAbsname(tree,str);
    if (dir==NULL) fprintf(stderr,"Warning: couldn't find %s\n",str);
    else updateMediaTree(dir);


    ui->redraw();
    return dir;
}

void updateCheckDirectory(Directory *tree,void *absname)
{
    Directory *dir;
    char *str=(char *)(absname);

    //dir=findDirectoryFromID(tree,id);
    //if (dir==NULL) fprintf(stderr,"Warning: couldn't find id %ld\n",id);
    dir=findDirectoryFromAbsname(tree,str);
    if (dir==NULL) fprintf(stderr,"Warning: couldn't find %s\n",str);
    else selectMediaTree(dir,dir->check->value());
}

void cb_media_directory(Fl_Widget* w,void *absname)
{
    updateDirectory(mediatree,absname);
}

void cb_media_check_directory(Fl_Widget* w,void *absname)
{
    updateCheckDirectory(mediatree,absname);
    w->parent()->redraw();
}

void cb_playlist_directory(Fl_Widget* w,void *absname)
{
    updateDirectory(playlisttree,absname);
}

void cb_playlist_check_directory(Fl_Widget* w,void *absname)
{
    updateCheckDirectory(playlisttree,absname);
    w->parent()->redraw();
}

void cb_imported_directory(Fl_Widget* w,void *absname)
{
    Directory *dir;
    dir=updateDirectory(importedtree,absname);
    if (dir->absname!=NULL)
    {
        ui->pl_current_dir=dir;
        ui->pl_path_label->label(ui->pl_current_dir->absname);
    }
    ui->redraw();
}

void cb_imported_check_directory(Fl_Widget* w,void *absname)
{
    updateCheckDirectory(importedtree,absname);
    w->parent()->redraw();
}

void cb_layer_add(Fl_Widget *w, void *d)
{
    playlistLayer *layer;
    Entry *config;
    File *file;
    char *str,*name;
    int ret;
    float length,fps;

    fps=0;
    config=NULL;
    layer=(playlistLayer *)(w);
    str=(char *)(d);
    if (layer->label()!=NULL)
    {
        entryReadDataFromFile(str,&config);
        name=entryGetString(config,STATS_FILE);
        //fprintf(stderr,"%s\n",name);
        ret=entryGetFloat(config,STATS_LENGTH,&length);
        if (ret==0)
        {
            ret=entryGetFloat(config,STATS_FPS,&fps);
            file=findFile(mediatree,name,0);
            if (file!=NULL)
            {
                //if (file->type==EXT_AUDIO) layer->addMedia(file->type,file->absname,(int)(ui->timeline->pointer_line->start_x),(double)(length),(double)(fps));
                //else
                layer->addMedia(file->type,file->relname,(int)(ui->timeline->pointer_line->start_x),(double)(length),(double)(fps));
            }
        }
    }

    entryFree(&config);

    ui->redraw();
}

void cb_layer_right_click(Fl_Widget *w, void*)
{
    File *currf,*file;
    Entry *config;
    char *str;
    playlistLayer *layer=(playlistLayer *)(w);

    //fprintf(stderr,"TIME %f\n",getTime());

    config=NULL;
    layer->resetMenuFiles();
    if (ui->pl_current_dir!=NULL)
    {
        currf=ui->pl_current_dir->files;
        while(currf!=NULL)
        {
            entryReadDataFromFile(currf->absname,&config);
            str=entryGetString(config,STATS_FILE);
            if (str!=NULL)
            {
                file=findFile(mediatree,str,0);
                if (file!=NULL)
                {
                    if (layer->layer_type==LAYER_VIDEO_TYPE)
                    {
                        if (file->type==EXT_VIDEO || file->type==EXT_3D || file->type==EXT_IMAGE) layer->addMenuFile(findFilename(str),cb_layer_add,currf->absname);
                    }
                    else //audio type
                    {
                        if (file->type==EXT_AUDIO) layer->addMenuFile(findFilename(str),cb_layer_add,currf->absname);
                    }
                }
            }
            currf=currf->next;
        }
    }
    entryFree(&config);

    layer->showMenuFile(w);

    ui->redraw();
}

void cb_media_remove(Fl_Widget *w, void *d)
{
    int j;
    playlistMedia *media=(playlistMedia *)(w);
    if (media->label()!=NULL)
    {
        media->active=0;
        //fprintf(stderr,"%s\n",media->label());
        for (j=0;j<4;j++)
        {
            media->fade_ctrls[j]->hide();
        }
        media->hide();
        ui->redraw();
    }
}

void cb_media_right_click(Fl_Widget *w, void*)
{
    playlistMedia *media=(playlistMedia *)(w);

    media->resetMenuFiles();
    media->addMenuFile(langGetLabel(LABEL_REMOVE),cb_media_remove,NULL);
    media->showMenuFile(w);
    ui->redraw();
}

//end of callback functions

void cursorWait()
{
    ui->cursor(FL_CURSOR_WAIT);
    Fl::check();
    usleep(1000);
}

void cursorDefault()
{
    ui->cursor(FL_CURSOR_DEFAULT);
    Fl::check();
    usleep(1000);
}

int main(int argc, char **argv)
{
    int i,j;
    int ival,ret;
    char cmd[CMD_SIZE];
    char file[PATH_SIZE];
    char label[DATA_SIZE];
    char *str;
    int choice;
    unsigned char project_loaded;
    imgu *I;

    //this is to define our own box drawing function
    Fl::set_boxtype(BOX_USER_DRAW, box_user_draw, 1, 1, 2, 2);

    fl_font(FL_HELVETICA, FONTSIZE);

    Fl::scheme("plastic");
    
    login=getlogin();
    sprintf(config_file,"%s/%s/%s/%s",BASE_PATH,login,config_path,config_filename);

    OPTION_DISPLAY_PD=0;
    OPTION_FORCE_SELECT=0;
    OPTION_FORCE_SELECT_NOISE=0;
    OPTION_START_PLL[0]='\0';
    OPTION_START_JOYSTICK=0;
    langInit();
    langSet(LANG_FRENCH);
    for(i=1;i<argc;i++) {
        if( strcmp("-lang",argv[i])==0 && i+1<argc ) {
            if (strcmp(LANG_FRENCH_STR,argv[i+1])==0) langSet(LANG_FRENCH);
            else if (strcmp(LANG_ENGLISH_STR,argv[i+1])==0) langSet(LANG_ENGLISH);
            i++;continue;
        }
        if( strcmp("-pd",argv[i])==0) {
            OPTION_DISPLAY_PD=1;
            continue;
        }
        if( strcmp("-select",argv[i])==0) { //force select operation before crop
            OPTION_FORCE_SELECT=1;
            continue;
        }
        if( strcmp("-select_noise",argv[i])==0) { //force select operation before crop
            OPTION_FORCE_SELECT_NOISE=1;
            continue;
        }
        if( strcmp("-pll",argv[i])==0 && i+1<argc ) {
            strcpy(OPTION_START_PLL,argv[i+1]);
            i++;continue;
        }
        if( strcmp("-joystick",argv[i])==0) {
            OPTION_START_JOYSTICK=1;
            continue;
        }
    }

    for (i=0;i<MAX_NB_MACHINES;i++)
    {
        machines[i].connected=0;
        for (j=0;j<MAX_NB_PIDS;j++) machines[i].pids[j]=-1;
    }

    pd_playlist=NULL;
    pd_main_ctrl=NULL;
    pd_snd_channels=NULL;

    //create windows
    blendwin=new blendWindow(50,50,320,180,langGetLabel(LABEL_BLEND_WINDOW_TITLE));
    cam2projwin=new cam2projWindow(50,50,360,360,langGetLabel(LABEL_CAM2PROJ_WINDOW_TITLE));
    uv2projwin=new uv2projWindow(50,50,320,180,langGetLabel(LABEL_UV2PROJ_WINDOW_TITLE));
    colorwin=new colorWindow(50,50,410,200,langGetLabel(LABEL_COLOR_WINDOW_TITLE));
    uvwin=new uvmapWindow(50,50,600,600,langGetLabel(LABEL_UVMAP_WINDOW_TITLE));
    uvwin->resizable(uvwin);
    shell=new shellWindow(50,50,400,300,langGetLabel(LABEL_SHELL_WINDOW_TITLE));
    shell->resizable(shell);
    gammawin=new gammaWindow(50,50,410,360,langGetLabel(LABEL_GAMMA_WINDOW_TITLE));
    capwin=new captureWindow(50,50,380,400,langGetLabel(LABEL_CAPTURE_WINDOW_TITLE));
    livewin=new liveWindow(50,50,680,720,langGetLabel(LABEL_LIVE_WINDOW_TITLE));
    playwin=new playWindow(50,50,320,240,langGetLabel(LABEL_PLAY_WINDOW_TITLE));
    precropwin=new precropWindow(50,50,320,320,langGetLabel(LABEL_PRECROP_WINDOW_TITLE));
    prj=new projectWindow(50,50,680,720,langGetLabel(LABEL_PROJECT_WINDOW_TITLE),login);
    smoothwin=new smoothWindow(50,50,410,200,langGetLabel(LABEL_SMOOTH_WINDOW_TITLE));
    patwin=new patternWindow(50,50,320,200,langGetLabel(LABEL_PATTERN_WINDOW_TITLE));

    ui = new lighttwistUI(argc,argv,OFFSET,8*LINESPACE,800,460, "Lighttwist");
    ui->createUIWindow(login,machines);
    ui->loadDefaultValues();

    mediatree=NULL;
    playlisttree=NULL;
    importedtree=NULL;
    config=NULL;
    project=NULL;
    project_loaded=0; //project loaded at startup, local variable in main only

    //config file for now only contains the path and filename of project
    entryReadDataFromFile(config_file,&config);
    if (config==NULL)
    {
        //there was an error reading config, create config directory
        sprintf(cmd,"mkdir -p %s/%s/%s",BASE_PATH,login,config_path);
        system(cmd);
    }
    str=entryGetString(config,PROJECT_KEY);
    if (str==NULL)
    {
        //the first time Lighttwist is run, a project needs to be defined
        choice=fl_choice(langGetLabel(LABEL_WELCOME),langGetLabel(LABEL_NO),langGetLabel(LABEL_YES),NULL);
        if (choice==1) prj->show();
        else
        {
            cb_update_project();
        }
    }
    else //load project and configuration
    {
        loadProject(&project,str);
        str=entryGetString(config,EXT_VIDEO_KEY);
        if (str!=NULL) ui->ext_video->value(str);
        str=entryGetString(config,EXT_AUDIO_KEY);
        if (str!=NULL) ui->ext_audio->value(str);
        str=entryGetString(config,EXT_3D_KEY);
        if (str!=NULL) ui->ext_3d->value(str);
        str=entryGetString(config,EXT_IMAGE_KEY);
        if (str!=NULL) ui->ext_image->value(str);
        str=entryGetString(config,EXT_PLAYLIST_KEY);
        if (str!=NULL) ui->ext_playlist->value(str);

        ret=entryGetInteger(config,PLAYER_THREADING,&ival);
        if (ret==0) ui->threading->value(ival);
        ret=entryGetInteger(config,PLAYER_SELECT,&ival);
        if (ret==0) ui->player_select->value(ival);

        updateLocalMedia();
        updateLocalPlaylists();
        updateImportedMedia();
        ui->show();
        project_loaded=1;
    }

    nodata=NULL;
    sprintf(file,"%s/nodata.png",prj->local_share->value());
    //sprintf(file,"%s/logo-lt.png",prj->local_share->value());
    imguLoad(&nodata,file,LOAD_16_BITS);
    imguScale(&nodata,nodata,0.42,0.42);
    updateDataImage(NULL);

    sprintf(file,"%s/camera-top.png",prj->local_share->value());
    updateSoundImage(file);

    ui->loadShowImages(ui->show_setup,prj->local_share->value(),"setup_on.png","setup_off.png","setup_disabled.png");
    ui->loadShowImages(ui->show_play,prj->local_share->value(),"play_on.png","play_off.png","play_disabled.png");
    ui->loadShowImages(ui->show_sound,prj->local_share->value(),"sound_on.png","sound_off.png","sound_disabled.png");
    ui->loadShowImages(ui->show_loop,prj->local_share->value(),"loop_on.png","loop_off.png","loop_disabled.png");
    ui->loadShowImages(ui->show_joystick,prj->local_share->value(),"joystick_on.png","joystick_off.png","joystick_disabled.png");
    ui->show_setup_flag=BUTTON_DISABLED;
    cb_show_setup(NULL,NULL);

    I=NULL;

    sprintf(file,"%s/loop_on.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.11,0.11);
    ui->media_refresh_but->setImage(I);

    sprintf(file,"%s/new.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.17,0.17);
    ui->pl_new_but->setImage(I);

    sprintf(file,"%s/save.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.17,0.17);
    ui->pl_save_but->setImage(I);

    sprintf(file,"%s/open.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.15,0.15);
    ui->pl_load_but->setImage(I);

    sprintf(file,"%s/tl_pointer.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.25,0.25);
    ui->timeline->setPointerImg(I);

    sprintf(file,"%s/speaker.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.125,0.125);
    for (i=0;i<MAX_NB_SPEAKERS;i++)
    {
        sprintf(label,"%d",i+1);
        ui->speakers[i]->label(strdup(label));
        ui->speakers[i]->setImage(I);
        ui->speakers[i]->xmin=(double)(ui->sound_space->x());
        ui->speakers[i]->ymin=(double)(ui->sound_space->y())+10.0; //+10 is because label shifts image a bit, 10 here is a guess
        ui->speakers[i]->xmax=(double)(ui->sound_space->x()+ui->sound_space->w());
        ui->speakers[i]->ymax=(double)(ui->sound_space->y()+ui->sound_space->h());
    }

    //read icons for crop control
    sprintf(file,"%s/crop_ctrl_inner.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.125,0.125);
    for (i=0;i<MAX_NB_UV_CTRLS;i++) uvwin->controls_inner[i]->setImage(I);
    sprintf(file,"%s/crop_ctrl_outer.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.125,0.125);
    for (i=0;i<MAX_NB_UV_CTRLS;i++) uvwin->controls_outer[i]->setImage(I);
    uv_ctrl_img=NULL;
    imguCopy(&uv_ctrl_img,I);
    sprintf(file,"%s/error.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.125,0.125);
    uv_ctrl_img_x=NULL;
    imguCopy(&uv_ctrl_img_x,I);

    updateUVMapWindow();

    //read icons for fade control
    sprintf(file,"%s/fade_ctrl_low.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.08,0.08);
    sprintf(file,"%s/fade_ctrl_high.png",prj->local_share->value());
    imguLoad(&I,file,LOAD_16_BITS);
    imguScale(&I,I,0.08,0.08);
    for (i=0;i<NB_DISP_LAYERS;i++)
    {
        for (j=0;j<MAX_NB_MEDIA;j++)
        {
            ui->timeline->layers[i]->media[j]->fade_ctrls[0]->setImage(I);
            ui->timeline->layers[i]->media[j]->fade_ctrls[1]->setImage(I);
            ui->timeline->layers[i]->media[j]->fade_ctrls[2]->setImage(I);
            ui->timeline->layers[i]->media[j]->fade_ctrls[3]->setImage(I);
        }
    }
    imguFree(&I);

    if (project_loaded) cb_connect(NULL,0);

    ui->end();
    ui->callback(cb_exit);

    //Initial global objects.
    //Fl::args(argc, argv);

    Fl::visual(FL_DOUBLE|FL_INDEX);
    cursorDefault();

    if (OPTION_START_PLL[0]!='\0')
    {
      cb_show_setup(NULL,NULL);
      enqueue_playlist(OPTION_START_PLL,0);
      play_playlist();
      if (OPTION_START_JOYSTICK) start_joystick_ctrl();
    }
    
    Fl::add_idle(cb_hook_play); //adding my hook to be invoked by fl::wait();

    Fl::check();
    Fl::run();

    return 0;
}

int confirm_reset_menu(char *oper)
{
    char msg[MSG_SIZE];
    sprintf(msg,"Do you really want to reset operation '%s'?",oper);

    return (fl_choice(msg,"No","Yes",NULL));
}

int get_videostats(const char *file, int *height,int *width,float *fps,float *len)
{

    char cmd[CMD_SIZE];
    char buffer[BUFFER_SIZE];
    Entry *cout;
    cout=NULL;

    //fprintf(stderr,"SOURCE: %s\n",file);

    sprintf(cmd,"mplayer -frames 0 -vo null -ao null -identify -slave %s 2>/dev/null | grep ID", file);
    //sprintf(cmd,"%s/%s -i %s -identify | grep ID",prj->remote_bin->value(),CMD_OSGPLAYER,file);

    if (shell->exec(cmd,buffer,BUFFER_SIZE,SHELL_HIDDEN,SHELL_BLOCK)) return -1;

    //printf("CMD: %s\n",cmd);

    entryReadDataFromBuffer(buffer,BUFFER_SIZE,&cout);
    entryGetInteger(cout,"ID_VIDEO_WIDTH",width);
    entryGetInteger(cout,"ID_VIDEO_HEIGHT",height);
    entryGetFloat(cout,"ID_VIDEO_FPS",fps);
    entryGetFloat(cout,"ID_LENGTH",len);

    printf("Stats of %s: width=%d; height=%d; fps=%f; len=%f\n",file,(*width),(*height),(*fps),(*len));

    entryFree(&cout);

    return 0;
}

//machine i
//creates directory if it doesn't exist already
//with path provided
//remote - on local or remote machine
int fioCreateDirectories(const char *path,unsigned char remote,int i)
{
   // char pathcpy[PATH_SIZE];
    char cmd[CMD_SIZE];
   // char *cstart,*cend;
   struct stat s;

    if (remote && (i<0 || i>=MAX_NB_MACHINES)) return -1;

    /*
    strcpy(pathcpy,path);
    cstart=pathcpy;
    cend=strchr(pathcpy,'/');
    while(cend!=NULL)
    {
        (*cend)='\0';
        if (strcmp(cstart,"..")!=0 && strcmp(cstart,".")!=0)
        {
            //is there a standard way to check if a directory already exists?
            sprintf(cmd,"mkdir %s",pathcpy);
            //printf("%s\n",cmd);
            if (remote) remote_exec(i,cmd);
            else system(cmd);
            usleep(100000);
        }
        (*cend)='/';
        cstart=cend+1;
        cend=strchr(cstart,'/');
    }
    */
    if (stat(path, &s) != 0 && strlen(path)) //check if dir exists and path is not empty
    {
        //mkdir with -p option creates all intermediate directories
        //in the path if they don't exist and doesn't return
        //errors if dirs exist
        sprintf(cmd, "mkdir -p %s", path);
        printf("%s\n", cmd);

        if (remote) remote_exec(i, cmd);
        else system(cmd);
    }
    usleep(100000);
    system("sync"); //force disk writes

    return 0;
}

static int compare_strings(const void *a, const void *b){
    const char *a_str = (char *)a;
    const char *b_str = (char *)b;
    return strcmp(a_str,b_str);
}

int fioReadFolderFiles(char namelist[MAX_NB_FILES][FILENAME_SIZE],char *inpath)
{
    int i;
    char cmd[CMD_SIZE];
    char line[FILENAME_SIZE];
    char *buffer;
    int bsize;
    int offset;

    if (inpath==NULL) return -1;

    bsize=MAX_NB_FILES*(FILENAME_SIZE+1);
    buffer=(char *)(malloc(sizeof(char)*bsize));

    //sprintf(cmd,"find %s -type f -name \"*.JPG\"",inpath);
    sprintf(cmd,"find %s -type f",inpath);

    if (shell->exec(cmd,buffer,bsize,SHELL_HIDDEN,SHELL_BLOCK)) return -1;

    i=0;
    offset=0;
    while(entryGetLine(line,buffer,bsize,&offset)==SUCCESS)
    {
      strcpy(namelist[i],line);
      i++;
    }

    qsort(namelist, i, sizeof(char)*FILENAME_SIZE, compare_strings);

    free(buffer);

    return i;
}

//demux sound from video file
//(.mov, .avi or .mgpeg)
//soundtrack gets name video_filename_soundtrack.wav
//and is added to folder where video file is
int demux_audio(const char *name, int type)
{
    if (type == EXT_VIDEO)
    {
         char cmd[CMD_SIZE];
         char audioname[PATH_SIZE];
         struct stat s;

         strcpy(audioname, name);
         stripFromChar(audioname, '.');
         strcat(audioname, "_soundtrack.wav");

         if (stat(audioname, &s) != 0)
         {
            //-ab - audio bitrate in kbit/s
            //-ar - sample rate in Hz (default 44100 Hz)
            sprintf(cmd, "ffmpeg -i %s -ab 128k -ar 44100 -f wav %s &", name, audioname);
            printf("CMD: %s\n", cmd);
            system(cmd);
         }
    }
    updateLocalMedia();

    return 0;
}

int do_precrop (const char *name,int type)
{
    FILE *fp;
    int i,j,k,l,p;
    int height,width;
    int nb_tiles_x,nb_tiles_y;
    int xmin,xmax,ymin,ymax;
    float fps,len;
    char statsname[PATH_SIZE];
    char pllname[PATH_SIZE];
    char basename[PATH_SIZE];
    char indirname[PATH_SIZE];
    char outdirname[PATH_SIZE];
    char filename[FILENAME_SIZE];
    char inmovie[PATH_SIZE];
    char fullname[PATH_SIZE];
    char relname[PATH_SIZE];
    char outmovie[PATH_SIZE];
    char localmedia[PATH_SIZE];
    char moviecopy[PATH_SIZE];
    char ip[DATA_SIZE];
    char *subdir;
    char *ip_address;
    int subdirpos;
    char cmd[CMD_SIZE];
    char tag[DATA_SIZE];
    int tile_width,tile_height;
    int nbpass;

    //fl_choice("L'opération 'Precrop' est très longue. Continuer? (Ancien precrop seront effacés)","Non","Oui",NULL)

    if (type!=EXT_3D && type!=EXT_IMAGE) get_videostats(name,&height,&width,&fps,&len);

    tile_width=atoi(precropwin->tile_width->mvalue()->label());
    tile_height=atoi(precropwin->tile_height->mvalue()->label());

    strcpy(indirname,name);
    strcpy(filename,findFilename(indirname));
    stripFromChar(indirname,'/');
    strcpy(basename,name);
    stripFromChar(basename,'.');
    strcpy(statsname,basename);
    strcat(statsname,".stats");
    strcpy(pllname,basename);
    strcat(pllname,".pll");

    fp=fopen(statsname,"w");
    if (fp==NULL) return -1;
    //fprintf(fp,"%s=%s\n",STATS_FILE,filename);
    strcpy(fullname,name);
    strcpy(localmedia,prj->local_media->value());
    //remove '/' at end of local media path
    if (strlen(localmedia)>0 && (localmedia[strlen(localmedia)-1]=='/' || localmedia[strlen(localmedia)-1]=='\\')) localmedia[strlen(localmedia)-1]='\0';
    subdirpos=strlen(localmedia);
    strcpy(relname,findRelativePath(localmedia,fullname));

    fprintf(fp,"%s=%s\n",STATS_FILE,relname);
    fprintf(fp,"%s=%d\n",STATS_TYPE,type);
    if (type==EXT_VIDEO)
    {
        fprintf(fp,"%s=%d\n",STATS_WIDTH,width);
        fprintf(fp,"%s=%d\n",STATS_HEIGHT,height);
        fprintf(fp,"%s=%f\n",STATS_FPS,fps);
        fprintf(fp,"%s=%f\n",STATS_LENGTH,len);
    }
    else if (type==EXT_AUDIO)
    {
        fprintf(fp,"%s=%f\n",STATS_LENGTH,len);
    }
    else
    {
        len=30.0;
        fprintf(fp,"%s=%f\n",STATS_LENGTH,len);
    }
    fclose(fp);

    subdir=indirname;
    for (i=0;i<subdirpos;i++) subdir++;
    //create directory on local machine
    if(strlen(subdir) > 0) sprintf(outdirname,"%s/%s/",prj->local_data->value(),subdir); //if subdirectory is provided
    else sprintf(outdirname,"%s/",prj->local_data->value());

    fioCreateDirectories(outdirname,0,-1);

    if (type==EXT_3D || type==EXT_IMAGE)
    {
        sprintf(cmd,"cp %s/%s %s/%s",prj->local_media->value(),relname,prj->local_data->value(),relname);
        fprintf(stderr,"%s\n",cmd);
        system(cmd);

        for (i=0;i<prj->nb_projectors->value();i++)
        {
            if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0) nbpass=2;
            else nbpass=1;

            for (p=0;p<nbpass;p++)
            {
                if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_RIGHT))==0) k=i+MAX_NB_MACHINES_PER_CHANNEL;
                else k=i+p*MAX_NB_MACHINES_PER_CHANNEL;

                //create directory on other machines
                sprintf(outdirname,"%s/%s/",prj->remote_data->value(),subdir);
                fioCreateDirectories(outdirname,1,k);

                strcpy(ip,ui->m_ip[k]->value());
                stripFromChar(ip,':');
                sprintf(cmd,"scp %s/%s %s:%s/%s",localmedia,relname,ip,prj->remote_data->value(),relname);
                fprintf(stderr,"%s\n",cmd);
                system(cmd);
            }
        }
    }
    else if (type==EXT_VIDEO)
    {
        if (width != 0 && height != 0)
        {
            nb_tiles_x=width/tile_width;
            if (width%tile_width!=0) nb_tiles_x++;
            nb_tiles_y=height/tile_height;
            if (height%tile_height!=0) nb_tiles_y++;

            sprintf(inmovie,"%s",name);
            sprintf(outmovie,"%s",findFilename(basename));

            //erase old precrop (locally)
            if (outdirname[strlen(outdirname)-1] == '/') sprintf(cmd,"rm -f %s%s_Tile*",outdirname,outmovie);
            else sprintf(cmd,"rm -f %s/%s_Tile*",outdirname,outmovie);
            system(cmd);

            //erase old precrop (remotely)
            for (i=0;i<prj->nb_projectors->value();i++)
            {
                if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0) nbpass=2;
                else nbpass=1;

                for (p=0;p<nbpass;p++)
                {
                    if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_RIGHT))==0) k=i+MAX_NB_MACHINES_PER_CHANNEL;
                    else k=i+p*MAX_NB_MACHINES_PER_CHANNEL;

                    if (machines[k].connected)
                    {
                        if (strlen(subdir)) sprintf(outdirname,"%s/%s/",prj->remote_data->value(),subdir);
                        else sprintf(outdirname,"%s/",prj->remote_data->value());

                        strcpy(ip,ui->m_ip[k]->value());

                        stripFromChar(ip,':');
                        ip_address = startFromChar(ip, '@');

                        if (strcmp(ip_address,"localhost")!=0 && strcmp(ip_address,"127.0.0.1")!=0) //if player is on localhost, do not remove old tiles, BUT local and remote data paths should be the same
                        {
                            if (outdirname[strlen(outdirname)-1] == '/') sprintf(cmd,"rm -f %s%s_Tile*",outdirname,outmovie);
                            else sprintf(cmd,"rm -f %s/%s_Tile*",outdirname,outmovie);

                            remote_exec(k,cmd);
                        }
                    }
                }
            }

            //call ltprecrop on local machine
            sprintf(cmd,"%s/%s -opath %s -i %s -o %s -w %d -h %d -f %f -tw %d -th %d",prj->local_bin->value(),CMD_PRECROP,outdirname,inmovie,outmovie,width,height,fps,tile_width,tile_height);
            //shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);
            system(cmd);

            //copy stats file in local data
            sprintf(cmd,"cp %s %s",statsname,outdirname);
            fprintf(stderr,"%s\n",cmd);
            system(cmd);

            for (i=0;i<prj->nb_projectors->value();i++)
            {
                if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_ALL))==0) nbpass=2;
                else nbpass=1;

                for (p=0;p<nbpass;p++)
                {
                    if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P))==0 && strcmp(precropwin->machine->mvalue()->label(),langGetLabel(LABEL_CHOICE_RIGHT))==0) k=i+MAX_NB_MACHINES_PER_CHANNEL;
                    else k=i+p*MAX_NB_MACHINES_PER_CHANNEL;

                    if (machines[k].connected)
                    {
                        if (strlen(subdir)) sprintf(outdirname,"%s/%s/",prj->remote_data->value(),subdir);
                        else sprintf(outdirname,"%s/",prj->remote_data->value());

                        strcpy(ip,ui->m_ip[k]->value());

                        stripFromChar(ip,':');
                        ip_address = startFromChar(ip, '@');

                        //create directory on other machines
                        fioCreateDirectories(outdirname,1,k);

                        //copy stats file remotely and tiles
                        sprintf(cmd,"scp %s/%s/%s* %s:%s",prj->local_data->value(),subdir,findFilename(basename),ip,outdirname);
                        fprintf(stderr,"%s\n",cmd);
                        system(cmd);
                    }
                }
            }
        }
    }
    strcpy(tag,"default_tag");

    if (precropwin->auto_playlist_flag->value()==1)
    {
        fp=fopen(pllname,"w");
        if (fp==NULL) return -1;
        if (type==EXT_VIDEO)
        {
            fprintf(fp,"M %s %lg %lg %d %s %f %lg %s\n",
                    ZERO_ABSOLUTE,
                    0.0,
                    len,
                    0,
                    relname,
                    fps,
                    0.0,
                    tag);
        }
        else if (type==EXT_3D)
        {
            fps=30.0;
            fprintf(fp,"T %s %lg %lg %d %s %lg %s\n",
                    ZERO_ABSOLUTE,
                    0.0,
                    len,
                    0,
                    relname,
                    0.0,
                    tag);
        }
        else if (type==EXT_IMAGE)
        {
            fps=30.0;
            fprintf(fp,"I %s %lg %lg %d %s %lg %s\n",
                    ZERO_ABSOLUTE,
                    0.0,
                    len,
                    0,
                    relname,
                    0.0,
                    tag);
        }
        else if (type==EXT_AUDIO)
        {
            fprintf(fp,"A %s %lg %lg %d %s %lg %lg %lg %lg %s\n",
                    ZERO_ABSOLUTE,
                    0.0,
                    len,
                    0,
                    relname,
                    0.0,
                    1.0,
                    1.0,
                    0.0,
                    tag);
            printf("%s, %s\n\n\n", fullname, relname);
        }
        fps=15.0;
        //add fade enveloppe
        if (type==EXT_AUDIO)
        {
            fprintf(fp,"B <%s 0.0 %lg 0 1.0 1.0 %lg %s\n",
                    tag,
                    len,
                    fps,
                    NO_TAG);
            fprintf(fp,"B <%s 0.0 0.0 0 0.0 1.0 %lg %s\n",
                    tag,
                    fps,
                    NO_TAG);
            fprintf(fp,"B >%s 0.0 0.0 0 1.0 0.0 %lg %s\n",
                    tag,
                    fps,
                    NO_TAG);
        }
        else //video, 3d or images
        {
            fprintf(fp,"F <%s 0.0 %lg 0 1.0 1.0 %f %s\n",
                    tag,
                    len,
                    fps,
                    NO_TAG);
            fprintf(fp,"F <%s 0.0 0.0 0 0.0 1.0 %f %s\n",
                    tag,
                    fps,
                    NO_TAG);
            fprintf(fp,"F >%s 0.0 0.0 0 1.0 0.0 %f %s\n",
                    tag,
                    fps,
                    NO_TAG);
        }
        fclose(fp);
    }


    //demux soundtrack from video file
    if (precropwin->auto_soundtrack_flag->value()==1) demux_audio(fullname, type);
    return 0;
}

int do_stats_delete (const char *name,int type)
{
    char cmd[CMD_SIZE];
    char statsname[PATH_SIZE];
    char basename[PATH_SIZE];

    strcpy(basename,name);
    stripFromChar(basename,'.');
    strcpy(statsname,basename);
    strcat(statsname,".stats");

    sprintf(cmd,"rm -f %s",statsname);
    system(cmd);

    return 0;
}

int do_stl(int i)
{
    char cmd[CMD_SIZE];
    char args[CMD_SIZE];
    int resx,resy;
    int f;

    //remote_kill(i,CMD_STL);

    prj->getResolution(&resx,&resy);

    f=atoi(patwin->frequency->mvalue()->label());

    sprintf(cmd,"%s/%s -XSize %d -YSize %d ",prj->remote_bin->value(),CMD_STL,resx,resy);
    if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0) sprintf(args,"--mode patterns --nb_patterns %d",2*(getnbbits(resx)+getnbbits(resy)));
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_GAMMA))==0) sprintf(args,"--mode gamma --nb_patterns %d",STL_GAMMA_NB);
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_COLOR))==0) sprintf(args,"--mode color --nb_patterns %d",STL_COLOR_NB);
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_NOISE))==0) sprintf(args,"--mode noise --nb_patterns %d -freq %d -seed %d",atoi(patwin->nb_patterns->mvalue()->label()),f,i*1000);

    strcat(cmd,args);

    //printf("%s\n",cmd);
    remote_exec(i,cmd);

    return 0;
}

void wait_for_done(tcpcast *tcp)
{
    char buf[DATA_SIZE];
    for(;;) {
        tcp_receive_data(tcp,(unsigned char *)(buf),DATA_SIZE);
        if( strcmp(buf,"done")==0 ) break;
        usleep(1000);
    } 
}

//all is non zero number of all projectors of a channel are to be captured at the same time
int do_grab_camera(int m_index,const char *name,const char *outdir,int nbimgs,int color,int sync,int all)
{
    int i,j,l;
    tcpcast *tcp;
    char cam[DATA_SIZE];
    char camId[PATH_SIZE];
    char cmd[CMD_SIZE];
    char buf[PATH_SIZE];
    char buf2[PATH_SIZE];
    char filetodelete[PATH_SIZE];
    char cmdtodelete[PATH_SIZE];
    char tempname[PATH_SIZE];
    char network_buffer[DATA_SIZE];
    unsigned char imgu_capture;
    imgu *I;
    rqueue *Qrecycle;
    rqueue *Qdisto;
    rqueue *Qvideoout;
    paramlist *pl_camera;
    int tid_camera;
    int tid_disto;
    int status;
    int max_nb_grab_attempts;
    char *buf_status;
    char address[DATA_SIZE];
    char ip_buf[DATA_SIZE];
    char *ip;
    char *cport;
    int xpos,ypos,port;
    char tmp[100];

    if (all && m_index!=0 && m_index!=MAX_NB_MACHINES_PER_CHANNEL) return 0;
    tcp=(tcpcast *)(malloc(sizeof(tcpcast)*prj->nb_projectors->value()));

    I=NULL;
    imgu_capture=0;

    max_nb_grab_attempts=30;

    strcpy(cam,capwin->cam->mvalue()->label());
    strcpy( camId, capwin->camId->value() );
    if ( strcmp(camId, "") == 0 ) strcpy( camId, "0" );

    strcpy(network_buffer,"go");

    fioCreateDirectories(outdir,0,-1);

    for (j=0;j<prj->nb_projectors->value();j++)     
    {
      strcpy(address,ui->m_ip[m_index+j]->value());
      cport=startFromChar(address,':');
      port=STL_PORT+j*10;
      xpos=0;
      ypos=0;
      if (cport!=NULL)
      {
        sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
      }
      strcpy(ip_buf,ui->m_ip[m_index+j]->value());
      ip=startFromChar(address,'@');
      stripFromChar(ip,':');

      //request first pattern
      if (sync)
      {
        while(tcp_client_init(&tcp[j],ip,port))
        {
          printf("Connecting to stl...");
          usleep(100000);
        }  
        tcp_send_string(&tcp[j],(unsigned char *)(network_buffer));
        wait_for_done(&tcp[j]);
      }
      if (!all) break;
    }

    if (!strcmp(cam,CAMERA_PROSILICA) || !strcmp(cam,CAMERA_V4L))
    {
        imgu_capture=1;
    }

    if (imgu_capture) {
        Qrecycle=imguRegisterQueue((char *)("recycle"));
        Qdisto=imguRegisterQueue((char*)("disto"));
        Qvideoout=imguRegisterQueue((char *)("videoout"));

        // capwin->camDistoK1->value()
        double k1 = capwin->camDistoK1->value();
        double k2 = capwin->camDistoK2->value();
        

        // no need for radial-distorsion correction
        if ( k1==1.0 && k2==1.0) {

            sprintf(tmp, "-in recycle -out videoout -camid %s -fps 0", camId );
            if (!strcmp(cam,CAMERA_PROSILICA)) {
                
                tid_camera=imguStartPlugin((char *)("gige"),
                                           (char *)("camera"),
                                           (char *) tmp);
                
            } else {
                tid_camera=imguStartPlugin((char *)("v4l"),
                                           (char *)("camera"),
                                           (char *) tmp);
                ///("-in recycle -out videoout -camid 0 -fps 0"));
            }
        } 
        // correcting camera's radial distorsion
        else {
            sprintf(tmp, "-in recycle -out disto -camid %s -fps 0", camId );
            if (!strcmp(cam,CAMERA_PROSILICA)) {
                
                tid_camera=imguStartPlugin((char *)("gige"),
                                           (char *)("camera"),
                                           (char *) tmp);
                
                sprintf(tmp, "-in disto -out videoout -width 659 -height 493 -k1 %f -k2 %f", k1, k2 );

                tid_disto=imguStartPlugin((char *)("radialdist"),
                                          (char *)("gpufilter"),
                                          (char *) tmp);

            } else {
                tid_camera=imguStartPlugin((char *)("v4l"),
                                           (char *)("camera"),
                                           (char *) tmp);

                sprintf(tmp, "-in disto -out videoout -k1 %f -k2 %f", k1, k2 );

                tid_disto=imguStartPlugin((char *)("radialdist"),
                                          (char *)("gpufilter"),
                                          (char *) tmp);
                ///("-in recycle -out videoout -camid 0 -fps 0"));
            }
        }

        if( tid_camera<0 ) return(-1);

        // access camera parameter
        pl_camera=imguGetPluginParameters(tid_camera);

        // to see the available parameters
        paramDump(pl_camera);

        // fill recycle queue...
        for(i=0;i<8;i++) RQueueAddLast(Qrecycle,(unsigned char *)&I);

        // Start capture! (since fps=0, it will wait for SNAPSHOT)
        paramInvokeCommand(pl_camera,(char *)("START"));

        if (!strcmp(cam,CAMERA_PROSILICA))
        {
          paramSetString(pl_camera,(char *)("ExposureMode"),(char *)("Manual"));
          paramSetInt(pl_camera,(char *)("ExposureValue"),capwin->exposure_time->value()*1000000);
        }
    }

    if (!strcmp(cam,CAMERA_GPHOTO2))
    {
      system("gphoto2 --set-config capture=on");
      system("gphoto2 --set-config capturetarget=1");
    }

    for(i=0;i<nbimgs;i++)
    {
        fprintf(stderr,"GRABBING %d\n",i);
        if (!strcmp(cam,CAMERA_GPHOTO2))
        {
            while(1)
            {
              //take snapshot
              if (!strcmp(capwin->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_AUTOMATIC)))
                system("gphoto2 --delete-all-files --recurse");

              system("gphoto2 --capture-image");

              if (!strcmp(capwin->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_AUTOMATIC)))
              {
                //transfer, convert and mv file
                strcpy(tempname,outdir);
                strcat(tempname,name);

                sprintf(buf,tempname,1);
                strcat(buf,"_temp"); //temporary name on disk, without extension
                sprintf(buf2,tempname,i);  //final name on disk, without extension
                //replace %d format by %n format
                tempname[strlen(tempname)-1]='n'; //name for gphoto2 capture, as 'buf' but with %n instead of %d
                strcat(tempname,"_temp");

                sprintf(cmd,"gphoto2 --get-file 1 --filename %s.jpg",tempname);
                sprintf(filetodelete,"%s.jpg",buf);
                sprintf(cmdtodelete,"rm -f %s",filetodelete);
                strcpy(tempname,buf);
                strcat(tempname,".jpg");

                //waiting for image...
                l=0;
                while(l<max_nb_grab_attempts)
                {
                  printf("Waiting...(%d)\n",l);
                  system(cmdtodelete);
                  system(cmd);
                  if (imguLoad(&I,tempname,LOAD_AS_IS)==0)
                  {
                    //file read successfully
                    break;
                  }
                  usleep(100000);
                  l++;
                }
                status=-1;
                if (I!=NULL && l<max_nb_grab_attempts)
                {
                  status=0; //success
                }
                if (status)
                {
                  printf("Error: %d\n",status);
                }
                else break;
              }
              else break;
            }//end of while(1)
            sprintf(buf,name,i);
        }else if( imgu_capture ) {
            sprintf(buf,name,i);

            while(1)
            {
              status=paramInvokeCommand(pl_camera,(char *)("SNAPSHOT"));
              //waiting for image...
              l=0;
              while(RQueueRemoveFirst(Qvideoout,(unsigned char *)&I)!=0 && l<max_nb_grab_attempts)
              {
                printf("Waiting...(%d)\n",l);
                usleep(100000);
                l++;
              }
              status=-1;
              if (I!=NULL && l<max_nb_grab_attempts)
              {
                buf_status=imguGetText(I,"STATUS");
                if (buf_status!=NULL)
                {
                  sscanf(buf_status,"%d",&status);
                }
              }
              if (status)
              {
                printf("Error: %d\n",status);
                imguRecycle(I); //something was wrong with the capture
              }
              else break;
            }//end of while(1)
        }

        //buf should have filename
        if (!strcmp(capwin->transfer->mvalue()->label(),langGetLabel(LABEL_CHOICE_AUTOMATIC)))
        {
          strcpy(tempname,outdir);
          strcat(tempname,buf);
          strcat(tempname,".png");
          if (color) imguConvertToRGB(&I,I);
          else imguConvertToGray(&I,I);
          // 16-bits images have to be converted to 8-bits (if not using gphoto2)
          if (strcmp(cam,CAMERA_GPHOTO2)!=0) imguConvert16bitTo8bit(&I,I);
          imguSave(I,tempname,1,SAVE_8_BITS_LOW);
          if (imgu_capture)
          {
            imguRecycle(I); //we are finished with the image
            I=NULL;
          }

          updateDataImage(tempname);
        }

        //change pattern
        if (sync && i!=nbimgs-1)
        {
          for (j=0;j<prj->nb_projectors->value();j++)     
          {
            tcp_send_string(&tcp[j],(unsigned char *)(network_buffer));
            wait_for_done(&tcp[j]);
            if (!all) break;
          }

        }

        imguFree(&I);
    }

    //all images are captured!

    if (sync)
    {
      for (j=0;j<prj->nb_projectors->value();j++)     
      {
        tcp_client_close(&tcp[j]);
        if (!all) break;
      }
    }
    if (imgu_capture)
    {
        // pas necessaire dans ce cas-ci...
        paramInvokeCommand(pl_camera,(char *)("STOP"));
        
        // stats as a nice graph...
        //imguPluginDumpDot("out.dot");

        // kill the plugin. we are done.
        imguStopPlugin(tid_camera);
        
        //imguUnregisterQueue((char *)("recycle"),NULL);
        //imguUnregisterQueue((char *)("videoout"),NULL);
        
        //if ( k1!=1.0 || k2!=1.0) {
            //imguStopPlugin(tid_disto);
            //printf("disto plugin stopped  ======================== \n");
            //imguUnregisterQueue((char *)("disto"),NULL);
        //}

    }

    imguFree(&I);
    free(tcp);

    return 0;
}

int do_grab_helper (int i,int all)
{
    char outdir[PATH_SIZE];
    char fname[FILENAME_SIZE];
    int nb_patterns;
    int color;
    int resx,resy;

    prj->getResolution(&resx,&resy);

    sprintf(outdir,"%s/in%02d/",prj->local_scan->value(),i);

    if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0)
    {
        sprintf(fname,"grab_%02d_%%05d",i);
        nb_patterns=2*(getnbbits(resx)+getnbbits(resy));
        color=0;
    }
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_GAMMA))==0)
    {
        sprintf(fname,"grab_gamma_%02d_%%05d",i);
        nb_patterns=STL_GAMMA_NB;
        color=0;
    }
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_COLOR))==0)
    {
        sprintf(fname,"grab_color_%02d_%%05d",i);
        nb_patterns=STL_COLOR_NB;
        color=1;
    }
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_NOISE))==0)
    {
        sprintf(fname,"grab_noise_%02d_%%05d",i);
        nb_patterns=atoi(patwin->nb_patterns->mvalue()->label());
        color=0;
    }

    do_grab_camera(i,fname,outdir,nb_patterns,color,1,all);

    return 0;
}

int do_grab (int i)
{
    char cmd[CMD_SIZE];
    char args[CMD_SIZE];
    int resx,resy;
    char address[DATA_SIZE];
    char *cport;
    int xpos,ypos,port;
    int f;

    //stop_background(i,CMD_STL);

    prj->getResolution(&resx,&resy);

    strcpy(address,ui->m_ip[i]->value());
    cport=startFromChar(address,':');
    port=STL_PORT;
    xpos=0;
    ypos=0;
    if (cport!=NULL)
    {
      sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
    }

    f=atoi(patwin->frequency->mvalue()->label());

    sprintf(cmd,"%s/%s -XPos %d -YPos %d -XSize %d -YSize %d -port %d -interval 0 -s 0 -e 0 ",prj->remote_bin->value(),CMD_STL,xpos,ypos,resx,resy,port);
    if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_PATTERNS))==0) sprintf(args,"--mode patterns --nb_patterns %d --sync &",2*(getnbbits(resx)+getnbbits(resy)));
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_GAMMA))==0) sprintf(args,"--mode gamma --nb_patterns %d --sync &",STL_GAMMA_NB);
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_COLOR))==0) sprintf(args,"--mode color --nb_patterns %d --sync &",STL_COLOR_NB);
    else if (strcmp(capwin->mode->mvalue()->label(),langGetLabel(LABEL_CHOICE_NOISE))==0) sprintf(args,"--mode noise --nb_patterns %d -freq %d -seed %d --sync &",atoi(patwin->nb_patterns->mvalue()->label()),f,i*1000);

    strcat(cmd,args);

    remote_exec(i,cmd);
    sleep(1);

    do_grab_helper(i,0);

    //put back 'fond noir'
    start_background(i);

    return 0;
}

int do_grab_noise (int i)
{
    int j;
    char cmd[CMD_SIZE];
    char args[CMD_SIZE];
    int resx,resy;    
    char address[DATA_SIZE];
    char *cport;
    int xpos,ypos,port;
    int f;

    if (i!=0 && i!=MAX_NB_MACHINES_PER_CHANNEL) return 0;

    //stop_background(i,CMD_STL);

    prj->getResolution(&resx,&resy);

    f=atoi(patwin->frequency->mvalue()->label());

    for (j=0;j<prj->nb_projectors->value();j++)
    {
      strcpy(address,ui->m_ip[i]->value());
      cport=startFromChar(address,':');
      port=STL_PORT+j*10;
      xpos=0;
      ypos=0;
      if (cport!=NULL)
      {
        sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
      }

      sprintf(cmd,"%s/%s -XPos %d -YPos %d -XSize %d -YSize %d -port %d -interval 0 -s 0 -e 0 ",prj->remote_bin->value(),CMD_STL,xpos,ypos,resx,resy,port);
      sprintf(args,"--mode noise --nb_patterns %d -freq %d -seed %d --sync &",atoi(patwin->nb_patterns->mvalue()->label()),f,j*1000);

      strcat(cmd,args);

      remote_exec(i+j,cmd);
    }
    sleep(1);

    do_grab_helper(i,1);

    //put back 'fond noir'
    for (j=0;j<prj->nb_projectors->value();j++)
    {
      start_background(i+j);
    }

    return 0;
}

int do_rename_patterns(int i)
{
  int j;
  int resx,resy;
  char filename[FILENAME_SIZE];
  imgu *I;

  prj->getResolution(&resx,&resy);

  I=NULL;
  for (j=0;j<2*(getnbbits(resx)+getnbbits(resy));j++)
  {
    imguLoad(&I,capwin->filenames[capwin->findex],LOAD_16_BITS);
    imguConvertToGray(&I,I);
    sprintf(filename,"%s/in%02d/grab_%02d_%05d.png",prj->local_scan->value(),i,i,j);
    printf("%s\n",filename);
    imguSave(I,filename,1,SAVE_8_BITS_HIGH);
    capwin->findex++;
  }

  imguFree(&I);

  return 0;
}

int do_rename_gamma(int i)
{
  int j;
  char filename[FILENAME_SIZE];
  imgu *I;

  I=NULL;
  for (j=0;j<STL_GAMMA_NB;j++)
  {
    imguLoad(&I,capwin->filenames[capwin->findex],LOAD_16_BITS);
    imguConvertToGray(&I,I);
    sprintf(filename,"%s/in%02d/grab_gamma_%02d_%05d.png",prj->local_scan->value(),i,i,j);
    printf("%s\n",filename);
    imguSave(I,filename,1,SAVE_8_BITS_HIGH);
    capwin->findex++;
  }

  imguFree(&I);

  return 0;
}

int do_rename_color(int i)
{
  int j;
  char filename[FILENAME_SIZE];
  imgu *I;

  I=NULL;
  for (j=0;j<STL_COLOR_NB;j++)
  {
    imguLoad(&I,capwin->filenames[capwin->findex],LOAD_16_BITS);
    imguConvertToRGB(&I,I);
    sprintf(filename,"%s/in%02d/grab_color_%02d_%05d.png",prj->local_scan->value(),i,i,j);
    printf("%s\n",filename);
    imguSave(I,filename,1,SAVE_8_BITS_HIGH);
    capwin->findex++;
  }

  imguFree(&I);

  return 0;
}

int do_rename_noise(int i)
{
  int j;
  char filename[FILENAME_SIZE];
  imgu *I;

#ifdef CAPTURE_SIMULTANEOUS_NOISE
  if (i!=0 && i!=MAX_NB_MACHINES_PER_CHANNEL) return 0;
#endif

  I=NULL;
  for (j=0;j<atoi(patwin->nb_patterns->mvalue()->label());j++)
  {
    imguLoad(&I,capwin->filenames[capwin->findex],LOAD_16_BITS);
    imguConvertToGray(&I,I);
    sprintf(filename,"%s/in%02d/grab_noise_%02d_%05d.png",prj->local_scan->value(),i,i,j);
    printf("%s\n",filename);
    imguSave(I,filename,1,SAVE_8_BITS_HIGH);
    capwin->findex++;
  }

  imguFree(&I);

  return 0;
}

int do_select (int i)
{
    char indir[PATH_SIZE];
    char outdir[PATH_SIZE];
    char cmd[CMD_SIZE];
    int resx,resy;

    prj->getResolution(&resx,&resy);

    sprintf(indir,"%s/in%02d",prj->local_scan->value(),i);
    sprintf(outdir,"%s/out",prj->local_scan->value());
    //check if it exists first?
    sprintf(cmd,"mkdir -p %s",outdir);
    system(cmd);

    
    sprintf(cmd,"%s/%s -c %d -i %s/grab_%%02d_%%05d.png -o %s/ -s 1 -d 1.0 -nb_patterns %d",prj->local_bin->value(),CMD_SELECT,i,indir,outdir,2*(getnbbits(resx)+getnbbits(resy)));
    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    return 0;
}

int do_select_noise (int i)
{
    char indir[PATH_SIZE];
    char outdir[PATH_SIZE];
    char cmd[CMD_SIZE];
    int resx,resy;

    prj->getResolution(&resx,&resy);

    sprintf(indir,"%s/in%02d",prj->local_scan->value(),i);
    sprintf(outdir,"%s/out",prj->local_scan->value());
    //check if it exists first?
    sprintf(cmd,"mkdir -p %s",outdir);
    system(cmd);

    
    sprintf(cmd,"%s/%s -c %d -i %s/grab_noise_%%02d_%%05d.png -o %s/ -s 1 -d 1.0 -nb_patterns %d",prj->local_bin->value(),CMD_SELECT,i,indir,outdir,atoi(patwin->nb_patterns->mvalue()->label()));
    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    return 0;
}

int do_sum(int channel)
{
    int istart;
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];
    char sum_img_filename[FILENAME_SIZE];
    int nbprojs;

    if (OPTION_FORCE_SELECT) selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),do_select);
    if (OPTION_FORCE_SELECT_NOISE) selectMachinesAndExecute(langGetLabel(LABEL_CHOICE_ALL),do_select_noise);

    sprintf(dir,"%s/out/",prj->local_scan->value());

    //number of projectors per channel
    nbprojs=(int)(prj->nb_projectors->value()+0.5);

    istart=0;
    if (channel==CHANNEL_RIGHT) istart+=MAX_NB_MACHINES_PER_CHANNEL;

    sprintf(cmd,"%s/%s -i %s -n %d -start %d -o %s",prj->local_bin->value(),CMD_SUM,dir,nbprojs,istart,dir);
    if (channel==CHANNEL_RIGHT) strcpy(sum_img_filename,"sum_Trange2.png");
    else strcpy(sum_img_filename,"sum_Trange.png");
    strcat(cmd,sum_img_filename);
    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    strcat(dir,sum_img_filename);

    updateDataImage(dir);

    return 0;
}

int do_uvmap()
{
    int i;
    char file[PATH_SIZE];
    imgu *Ileft,*Iright;
    Ileft=NULL;
    Iright=NULL;
    struct stat s;

    if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P)))
    {
        do_sum(CHANNEL_RIGHT);
        do_sum(CHANNEL_LEFT);
        sprintf(file,"%s/out/sum_Trange.png",prj->local_scan->value());
        imguLoad(&Ileft,file,LOAD_16_BITS);
        sprintf(file,"%s/out/sum_Trange2.png",prj->local_scan->value());
        imguLoad(&Iright,file,LOAD_16_BITS);
        //for stereo, take min of both Trange images
        if (Ileft!=NULL && Iright!=NULL)
        {
          for (i=0;i<Ileft->xs*Ileft->ys;i++)
          {
            if (Ileft->data[i]<Iright->data[i]) Ileft->data[i]=Iright->data[i];
          }  
        }
    }
    else
    {
        do_sum(CHANNEL_MONO);
        sprintf(file,"%s/out/sum_Trange.png",prj->local_scan->value());
        imguLoad(&Ileft,file,LOAD_16_BITS);
    }

    uvwin->scroll->position(0,0);
    uvwin->background->setImage(Ileft);
    uvwin->background->position(0,0);

    uvwin->loadDefaultValues();
    sprintf(file, "%s/out/uvmap.txt", prj->local_scan->value()); //default name and location for crop file
    if (stat(file, &s) != 0) saveCrop(); //if uvmap.txt is not found, save the defaults
    else loadCrop();

    uvwin->setUVConfiguration(prj->configuration->value());

    uvwin->redraw();
    uvwin->show();

    imguFree(&Ileft);
    imguFree(&Iright);

    return 0;
}

int do_copylut(int i)
{
    char file[FILENAME_SIZE];
    char cmd[CMD_SIZE];
    char outdirname[PATH_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    sprintf(file,"imapXY_%02d.lut.png",i);

    sprintf(outdirname,"%s/out/",prj->remote_scan->value());
    fioCreateDirectories(outdirname,1,i);

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    if (machines[i].connected)
    {
        sprintf(cmd,"scp %s/out/%s %s:%s/out/%s",prj->local_scan->value(),file,ip,prj->remote_scan->value(),file);
        system(cmd);
    }

    return 0;
}

int do_cam2proj (int i)
{
    int ii;
    char indir[PATH_SIZE];
    char outdir[PATH_SIZE];
    char cmd[CMD_SIZE];
    char crop[PATH_SIZE];
    char outfile[PATH_SIZE];
    char patfile[FILENAME_SIZE];
    int resx,resy;
    int conf,r,threshold;

    prj->getResolution(&resx,&resy);
    conf=atoi(cam2projwin->min_range_percentage_box->label());
    r=atoi(cam2projwin->min_range_box->label());
    threshold=atoi(cam2projwin->confidence_box->label());

    sprintf(outdir,"%s/out",prj->local_scan->value());

    if (cam2projwin->noise_mode->value()==0)
    {
      sprintf(indir,"%s/in%02d",prj->local_scan->value(),i);
      sprintf(cmd,"%s/%s -c %d -i %s/ -o %s/ -t 0 -B 0.5 -imap %d %d -w 2 -conf %d -r %d -proj %d %d -nb_patterns %d -threshold %d",prj->local_bin->value(),CMD_CAM2PROJ,i,indir,outdir,getpowerof2(getnbbits(resx)),getpowerof2(getnbbits(resy)),conf,r,resx,resy,2*(getnbbits(resx)+getnbbits(resy)),threshold);
    }
    else
    {    
#ifdef CAPTURE_SIMULTANEOUS_NOISE
      if (i<MAX_NB_MACHINES_PER_CHANNEL) ii=0;
      else ii=MAX_NB_MACHINES_PER_CHANNEL;
#else
      ii=i;
#endif
      sprintf(indir,"%s/in%02d",prj->local_scan->value(),ii);
      sprintf(patfile,"patterns_%02d_%04d.png",i,atoi(patwin->frequency->mvalue()->label()));
      sprintf(cmd,"%s/%s -c %d -i %s/ -o %s/ -imap %d %d -proj %d %d -nb_patterns %d -conf %d -pat %s -n %d -g %d",prj->local_bin->value(),CMD_CAM2PROJ_NOISE,i,indir,outdir,getpowerof2(getnbbits(resx)),getpowerof2(getnbbits(resy)),resx,resy,atoi(patwin->nb_patterns->mvalue()->label()),r,patfile,3,ii);
    }

    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    do_copylut (i);

    sprintf(outfile,"%s/out/imapXY_%02d.lut.png",prj->local_scan->value(),i);
    updateDataImage(outfile);

    return 0;
}

int do_smooth (int i)
{
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];
    char outfile[PATH_SIZE];
    int nb_patches_x,nb_patches_y;

    nb_patches_x=atoi(smoothwin->nb_patches_x->mvalue()->label());
    nb_patches_y=atoi(smoothwin->nb_patches_y->mvalue()->label());

    sprintf(dir,"%s/out/",prj->local_scan->value());

    sprintf(cmd,"%s/%s -i %s/imapXY_%%02d-copy.lut.png -o %s/imapXY_%%02d.lut.png -n %d -uv %s/uvmap.txt -uvimg %s/uvmap.png -camfile %s/sum_Trange.png -nb_patches_x %d -nb_patches_y %d",prj->local_bin->value(),CMD_SMOOTH,dir,dir,i,dir,dir,dir,nb_patches_x,nb_patches_y);

    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    do_copylut(i);

    sprintf(outfile,"%s/out/imapXY_%02d.lut.png",prj->local_scan->value(),i);
    updateDataImage(outfile);

    return 0;
}

int do_smooth_reset(int i)
{
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];

    sprintf(dir,"%s/out",prj->local_scan->value());

    sprintf(cmd,"cp -f %s/imapXY_%02d-copy.lut.png %s/imapXY_%02d.lut.png",dir,i,dir,i);
    system(cmd);

    return 0;
}

int do_copygamma (int i)
{
    char file[FILENAME_SIZE];
    char cmd[CMD_SIZE];
    char outdirname[PATH_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    sprintf(file,"gamma_%02d.txt",i);

    sprintf(outdirname,"%s/out/",prj->remote_scan->value());
    fioCreateDirectories(outdirname,1,i);

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    if (machines[i].connected)
    {
        sprintf(cmd,"scp %s/out/%s %s:%s/out/%s",prj->local_scan->value(),file,ip,prj->remote_scan->value(),file);
        system(cmd);
    }

    return 0;
}

int do_gamma (int i)
{
    char dir[PATH_SIZE];
    char outfile[PATH_SIZE];
    char cmd[CMD_SIZE];
    int intensity_thres;
    char gmanual[PATH_SIZE];
    vector3 gamma,max;

    sprintf(dir,"%s",prj->local_scan->value());

    if (gammawin->gamma_manual->value())
    {
      sprintf(gmanual,"%s/out/gamma_%02d.txt",dir,i);
      max[0]=1.0;
      max[1]=1.0;
      max[2]=1.0;
      gamma[0]=gammawin->gamma_projector[0]->value();
      gamma[1]=gammawin->gamma_projector[1]->value();
      gamma[2]=gammawin->gamma_projector[2]->value();
      write_gamma_and_max(gamma,max,gmanual);
    }
    else
    {
      intensity_thres=55;

      sprintf(cmd,"%s/%s -ipath %s/in%%02d/ -opath %s/out/ -nbimgs %d -pindex %d -ithres %d -gamma_cam %f %f %f",prj->local_bin->value(),CMD_GAMMA,dir,dir,STL_GAMMA_NB,i,intensity_thres,gammawin->gamma_camera[0]->value(),gammawin->gamma_camera[1]->value(),gammawin->gamma_camera[2]->value());

      shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

      sprintf(outfile,"%s/out/mask_gamma_%02d.png",prj->local_scan->value(),i);
      updateDataImage(outfile);
    }

    do_copygamma(i);

    return 0;
}

int do_gamma_reset(int i)
{
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    sprintf(dir,"%s/out",prj->local_scan->value());
    sprintf(cmd,"ssh %s rm -f %s/gamma_%02d.txt",ip,dir,i);

    system(cmd);

    return 0;
}

int do_copycolor (int i)
{
    char file[FILENAME_SIZE];
    char cmd[CMD_SIZE];
    char outdirname[PATH_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    sprintf(file,"color_%02d.txt",i);

    sprintf(outdirname,"%s/out/",prj->remote_scan->value());
    fioCreateDirectories(outdirname,1,i);

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    sprintf(cmd,"scp %s/out/%s %s:%s/out/%s",prj->local_scan->value(),file,ip,prj->remote_scan->value(),file);

    if (machines[i].connected)
    {
        system(cmd);
    }

    return 0;
}

int do_color(int i)
{
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    sprintf(dir,"%s",prj->local_scan->value());

    sprintf(cmd,"%s/%s -ipath %s/in%%02d/ -opath %s/out/ -ifname grab_color_%%02d_%%05d.png -nbimgs 24 -pindex %02d -uv %s/out/uvmap.txt -fmax %s/out/gamma_%%02d.txt",prj->local_bin->value(),CMD_COLOR,dir,dir,i,dir,dir);

    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    sprintf(cmd,"%s/%s -ipath %s/out/ -ifname1 color_%02d.txt -ifname2 color_%02d.txt -ofname transmat_%02d.txt",prj->local_bin->value(),CMD_COLOR_FIT,dir,i,i,i);

    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    do_copycolor(i);

    return 0;
}

int do_color_reset(int i)
{
    char dir[PATH_SIZE];
    char cmd[CMD_SIZE];
    char ip[DATA_SIZE];

    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    sprintf(dir,"%s/out",prj->local_scan->value());
    sprintf(cmd,"ssh %s rm -f %s/color_%02d.txt",ip,dir,i);

    system(cmd);

    return 0;
}

int do_blend (int channel)
{
    int istart;
    char cmd[CMD_SIZE];
    char outfile[PATH_SIZE];
    char dir[PATH_SIZE];
    int nbprojs;

    sprintf(dir,"%s",prj->local_scan->value());

    //number of projectors per channel
    nbprojs=(int)(prj->nb_projectors->value()+0.5);

    //if (mcount==1) return 0;

    istart=0;
    if (channel==CHANNEL_RIGHT) istart+=MAX_NB_MACHINES_PER_CHANNEL;

    sprintf(cmd,"%s/%s -path %s/out/ -uv %s/out/uvmap.txt -uvimg %s/out/uvmap.png -ifname xyvalid_%%02d.png -lut imapXY_%%02d.lut.png -ofname %%02d_support.png -nbprojs %d -start %d -fmax gamma_%%02d.txt",prj->local_bin->value(),CMD_BLEND,dir,dir,dir,nbprojs,istart);

    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    sprintf(outfile,"%s/out/blend_smoothmaxsum.png",prj->local_scan->value());
    updateDataImage(outfile);

    return 0;
}

int do_uv2proj (int i)
{
    char cmd[CMD_SIZE];
    char dir[PATH_SIZE];
    char outdirname[PATH_SIZE];
    char where[PATH_SIZE];
    char name[FILENAME_SIZE];
    char nameinv[FILENAME_SIZE];
    char lut[FILENAME_SIZE];
    char olut[FILENAME_SIZE];
    char xyvalid[FILENAME_SIZE];
    char outfile[PATH_SIZE];
    int angle_proj_max;
    int resx,resy;
    char ip[DATA_SIZE];
    int nbprojs;

    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    prj->getResolution(&resx,&resy);

    sprintf(dir,"%s/out",prj->local_scan->value());

    //angle_proj_max=cam2projwin->angle_proj_max->value();
    //number of projectors per channel
    nbprojs=(int)(prj->nb_projectors->value()+0.5);
    angle_proj_max=360.0/nbprojs*2;
    //angle_proj_max=120;

    strcpy(ip,ui->m_ip[i]->value());
    stripFromChar(ip,':');
    strcpy(where,ip);
    sprintf(name,"%02d_support.png",i);
    sprintf(nameinv,"%02d_support_proj.png",i);
    sprintf(lut,"imapXY_%02d.lut.png",i);
    sprintf(olut,"imapXY_%02d_crop.lut.png",i);
    sprintf(xyvalid,"xyvalid_%02d.png",i);

    sprintf(cmd,"%s/%s -uv %s/uvmap.txt -uvimg %s/uvmap.png -lut %s/%s -i %s/%s -o %s/%s -olut %s/%s -angle_proj_max %d -proj %d %d",prj->local_bin->value(),CMD_UV2PROJ,dir,dir,dir,lut,dir,name,dir,nameinv,dir,olut,angle_proj_max,resx,resy);
    if (prj->vmirror->value())
    {
        strcat(cmd," -vmirror");
    }
    //if (buildwin->crop_triangles->value()==0)
    //{
        strcat(cmd," -extend_blend");
    //}
    shell->exec(cmd,NULL,0,SHELL_VISIBLE,SHELL_NON_BLOCKING);

    sprintf(outdirname,"%s/out/",prj->remote_scan->value());
    fioCreateDirectories(outdirname,1,i);

    if (machines[i].connected)
    {
        sprintf(cmd,"scp %s/%s %s:%s/out/%s",dir,olut,ip,prj->remote_scan->value(),olut);
        system(cmd);
        sprintf(cmd,"scp %s/%s %s:%s/out/%s",dir,nameinv,ip,prj->remote_scan->value(),nameinv);
        system(cmd);
    }

    sprintf(outfile,"%s/out/%02d_support_proj.png",prj->local_scan->value(),i);
    updateDataImage(outfile);

    return 0;
}

int start_background(int i)
{
    char fond[DATA_SIZE];
    char cmd[CMD_SIZE];
    int resx,resy;
    char address[DATA_SIZE];
    char *cport;
    int port,xpos,ypos;

    //stop_background(i);

    prj->getResolution(&resx,&resy);

    //fprintf(stderr,"%d %d\n",resx,resy);

    if (ui->test_pat[0]->value())
    {
        strcpy(fond,"blanc");
    }
    else if (ui->test_pat[1]->value())
    {
        strcpy(fond,"noir");
    }
    else
    {
        strcpy(fond,"damier");
    }

    strcpy(address,ui->m_ip[i]->value());
    cport=startFromChar(address,':');
    port=STL_PORT;
    xpos=0;
    ypos=0;
    if (cport!=NULL)
    {
      sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
    }

    sprintf(cmd,"%s/%s -XPos %d -YPos %d -XSize %d -YSize %d --mode image -i %s/fond_%s.png &",prj->remote_bin->value(),CMD_STL,xpos,ypos,resx,resy,prj->remote_share->value(),fond);
    if (remote_exec (i,cmd)==0) ui->m_test[i]->color(FL_GREEN);
    else ui->m_test[i]->color(FL_RED);

    return 0;
}

int stop_background (int i)
{
    ui->m_test[i]->color(FL_GRAY);
    return remote_kill(i,CMD_STL);
}

int stop_joystick_ctrl()
{
  char cmd[CMD_SIZE];

  sprintf(cmd,"killall %s",CMD_JOYSTICK_CONTROL);
  system(cmd);

  ui->show_joystick_flag=BUTTON_OFF;
  ui->updateShowImages();

  return 0;
}

int start_joystick_ctrl()
{
  char cmd[CMD_SIZE];

  stop_joystick_ctrl();

  sprintf(cmd,"%s/%s -calib %s/joystick_calib &",prj->local_bin->value(),CMD_JOYSTICK_CONTROL,prj->local_share->value());
  system(cmd);
  fprintf(stderr,"%s\n",cmd);

  ui->show_joystick_flag=BUTTON_ON;
  ui->updateShowImages();

  return 0;
}

int stop_jack()
{
  char cmd[CMD_SIZE];

  sprintf(cmd,"killall qjackctl.bin");
  if (prj->enable_jack->value())
  {
    system(cmd);
  }
  sprintf(cmd,"killall jackd");
  if (prj->enable_jack->value())
  {
    system(cmd);
  }

  return 0;
}

int start_jack()
{
  char cmd[CMD_SIZE];

  stop_jack();

  sprintf(cmd,"qjackctl &");
  if (prj->enable_jack->value())
  {
    system(cmd);
    usleep(500000); //wait for jack to initialise
  }

  return 0;
}

int stop_player_slave(int i)
{
    if (i<0 || i>=MAX_NB_MACHINES) return -1;

    if (ui->player_select->value()==PLAYER_LUA) remote_kill(i,CMD_PLAYER_LUA);
    else remote_kill(i,CMD_PLAYER_DEFAULT);

    return 0;
}

int start_player_slave(int i)
{
    char cmd[CMD_SIZE];
    char out[PATH_SIZE];
    char lut[FILENAME_SIZE];
    char nameinv[FILENAME_SIZE];
    char vangle[DATA_SIZE];
    int zeroloc;
    int resx,resy;
    int port,xpos,ypos;
    char address[DATA_SIZE];
    char *cport;
    int nbprojs;

    if (i<0 || i>=MAX_NB_MACHINES) return -1;
    stop_background(i);
    //stop_player(i);

    prj->getResolution(&resx,&resy);

    //number of projectors per channel
    nbprojs=(int)(prj->nb_projectors->value()+0.5);

    strcpy(address,ui->m_ip[i]->value());
    cport=startFromChar(address,':');
    port=PLAYER_PORT;
    xpos=0;
    ypos=0;
    if (cport!=NULL)
    {
      sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
    }

    zeroloc=0;
    sprintf(out,"%s/out",prj->remote_scan->value());
    sprintf(lut,"imapXY_%02d_crop.lut.png",i);
    sprintf(nameinv,"%02d_support_proj.png",i);

    if (ui->player_select->value()==PLAYER_LUA)
    {
      sprintf(cmd,"LUA_PATH=\"%s/?.lua\" %s/%s -path %s -lut %s/%s -blend %s/%s \
        -share %s -video_shader deform_video.glsl -3d_shader deform_3d.glsl \
        -vstereo_shader stereo_rotate_vert.glsl -fstereo_shader stereo_rotate_frag.glsl \
        -m %d -n %d -zero %d -sync -dlutsx %d -dlutsy %d -vpxmin %d -vpymin %d \
        -vpwidth %d -vpheight %d -port %d",
        prj->remote_share->value(),prj->remote_bin->value(),CMD_PLAYER_LUA,prj->remote_data->value(),out,lut,out,nameinv,
        prj->remote_share->value(),
        i,nbprojs,zeroloc,getpowerof2(getnbbits(resx)),getpowerof2(getnbbits(resy)),xpos,ypos,
        resx,resy,port);
    }
    else
    {
      sprintf(cmd,"%s/%s -path %s -lut %s/%s -blend %s/%s -share %s \
        -video_shader deform_video.glsl -3d_shader deform_3d.glsl \
        -vstereo_shader stereo_rotate_vert.glsl -fstereo_shader \
        stereo_rotate_frag.glsl -m %d -n %d -zero %d -sync \
        -dlutsx %d -dlutsy %d -vpxmin %d -vpymin %d -vpwidth %d -vpheight %d \
        -port %d",
        prj->remote_bin->value(),CMD_PLAYER_DEFAULT,prj->remote_data->value(),out,lut,out,nameinv,prj->remote_share->value(),
        i,nbprojs,zeroloc,
        getpowerof2(getnbbits(resx)),getpowerof2(getnbbits(resy)),xpos,ypos,resx,resy,
        port);
    }
    if (prj->configuration->value()==CYCLORAMA_CONFIGURATION)
    {
      sprintf(vangle," -view_angle %f",360.0/prj->screen_ratio->value());
      strcat(cmd,vangle);
    }

    if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_P)))
    {
        if (i>=MAX_NB_MACHINES_PER_CHANNEL) strcat(cmd," -stereo_p_right");
        else strcat(cmd," -stereo_p_left");
    }
    else if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_RC)))
    {
        strcat(cmd," -stereo_rc");
    }
    else if (!strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_SYNC)))
    {
        strcat(cmd," -stereo_p_left");
    }
    if (!strcmp(ui->threading->mvalue()->label(),langGetLabel(LABEL_PLAYER_MULTI_THREAD)))
    {
        strcat(cmd," -threading 1");
    }

    //remote_exec(i,cmd);
    //remote_run starts the player but content is not displayed??
    //this is the only case where we actually force the ssh connection instead of remote_run
    //remote_exec_ssh(i,cmd,NULL);
    remote_exec_ssh(i,cmd);

    return 0;
}

int stop_player_master()
{
    char cmd[CMD_SIZE];

    /*fflush(pd_playlist);
    //pclose hangs...
    //if (pd_playlist!=NULL) pclose(pd_playlist);
    pd_playlist=NULL;

    fflush(pd_main_ctrl);
    //if (pd_main_ctrl!=NULL) pclose(pd_main_ctrl);
    pd_main_ctrl=NULL;*/

    pdClose(&pd_main_ctrl);
    pdClose(&pd_playlist);
    pdClose(&pd_snd_channels);

    sprintf(cmd,"killall pd");
    if (strcmp(prj->sound_ctrl_ip->value(),"localhost")==0 || strcmp(prj->sound_ctrl_ip->value(),"127.0.0.1")==0)
    {
      system(cmd);
      return 0;
    }
    else return remote_exec_ssh_ip(startFromChar(prj->sound_ctrl_ip->value(),'@'),cmd);
}

int start_player_master()
{
    int i;
    char cmd[CMD_SIZE];
    char message_pd[CMD_SIZE];
    char ctrl_ip[DATA_SIZE];
    int ret;
    char address[DATA_SIZE];
    int port,xpos,ypos;
    char *cport;

    stop_player_master();

    //if (DEBUG) sprintf(cmd,"pd -path %s -lib lighttwist %s/mixer_8_channels.pd &",prj->local_lib->value(),prj->local_share->value());
    //else sprintf(cmd,"pd -nogui -path %s -lib lighttwist %s/mixer_8_channels.pd &",prj->local_lib->value(),prj->local_share->value());

    //fprintf(stderr,"%s\n",cmd);
    //system(cmd);

    if (ui->player_select->value()==PLAYER_LUA)
    {
      sprintf(cmd,"LUA_PATH=\"%s/?.lua\" pd -path %s -lib player_master_lua -mididev 3 %s/midipatch_lune.pd &",prj->local_share->value(),prj->local_lib->value(),prj->local_share->value());
      //sprintf(cmd,"LUA_PATH=\"%s/?.lua\" pd -path %s -lib player_master_lua -mididev 2 -alsamidi %s/midipatch_lune.pd &",prj->local_share->value(),prj->local_lib->value(),prj->local_share->value());
    }
    else
    {
#ifdef __APPLE__ //don't specify '-jack' option with pd-extended
        if (OPTION_DISPLAY_PD) sprintf(cmd,"pd -outchannels %d -path %s -lib player_master_default %s/playlist_player.pd &",(int)(prj->nb_speakers->value()),prj->local_lib->value(),prj->local_share->value());
        else sprintf(cmd,"pd -nogui -outchannels %d -path %s -lib player_master_default %s/playlist_player.pd &",(int)(prj->nb_speakers->value()),prj->local_lib->value(),prj->local_share->value());
#else
        if (OPTION_DISPLAY_PD) sprintf(cmd,"pd -jack -outchannels %d -path %s -lib player_master_default %s/playlist_player.pd &",(int)(prj->nb_speakers->value()),prj->local_lib->value(),prj->local_share->value());
        else sprintf(cmd,"pd -nogui -jack -outchannels %d -path %s -lib player_master_default %s/playlist_player.pd &",(int)(prj->nb_speakers->value()),prj->local_lib->value(),prj->local_share->value());
#endif
    }

    fprintf(stderr,"%s\n",cmd);
    if (strcmp(prj->sound_ctrl_ip->value(),"localhost")==0 || strcmp(prj->sound_ctrl_ip->value(),"127.0.0.1")==0)
    {
      system(cmd);
      ret=0;
    }
    else
    {
      ret=remote_exec_ssh_ip(startFromChar(prj->sound_ctrl_ip->value(),'@'),cmd);
    }

    //sleep(2);

    /*if (pd_playlist==NULL)
    {
      sprintf(cmd,"pdsend %d",PD_PLAYLIST_PORT);
      pd_playlist=popen(cmd,"w");
      }
      if (pd_main_ctrl==NULL)
      {
      sprintf(cmd,"pdsend %d",PD_MAIN_CTRL_PORT);
      pd_main_ctrl=popen(cmd,"w");
    }*/

    strcpy(ctrl_ip,startFromChar(prj->sound_ctrl_ip->value(),'@'));

    sprintf(message_pd,"path %s/",prj->local_media->value());
    while (pdSend(&pd_playlist,ctrl_ip,PD_PLAYLIST_PORT,message_pd))
    {
        usleep(500000); //wait until we can connect to pd
    }
    if (strcmp(prj->mode->mvalue()->label(),langGetLabel(LABEL_MODE_STEREO_SYNC))==0)
    sprintf(message_pd,"vtoggle");
    pdSend(&pd_playlist,ctrl_ip,PD_PLAYLIST_PORT,message_pd);

    for (i=0;i<MAX_NB_MACHINES;i++)
    {
      if (ui->m_ip[i]->visible())
      {
        strcpy(address,ui->m_ip[i]->value());
        cport=startFromChar(address,':');
        port=PLAYER_PORT;
        xpos=0;
        ypos=0;
        if (cport!=NULL)
        {
          sscanf(cport,"%d+%d+%d",&port,&xpos,&ypos);
        }
        sprintf(message_pd,"port %d",port);
        pdSend(&pd_playlist,ctrl_ip,PD_PLAYLIST_PORT,message_pd);
      }
    }

    //cb_show_live_stereo(NULL,0);

    return ret;
}

int enqueue_playlist(const char *playlist,int type)
{
    char message_pd[CMD_SIZE];

    sprintf(message_pd,"load %s",playlist);

    if (pdSend(&pd_playlist,startFromChar(prj->sound_ctrl_ip->value(),'@'),PD_PLAYLIST_PORT,message_pd)) fprintf(stderr,"Warning: unable to open pdsend\n");

    return 0;
}

int play_playlist()
{
    int i;
    //double angle;
    double x,y,len;
    char message_pd[CMD_SIZE];
    char temp[24];
    char ctrl_ip[DATA_SIZE];

    strcpy(ctrl_ip,startFromChar(prj->sound_ctrl_ip->value(),'@'));

    sprintf(message_pd,"volume 0 100");
    pdSend(&pd_main_ctrl,ctrl_ip,PD_MAIN_CTRL_PORT,message_pd);
    for (i=0;i<(int)(prj->nb_speakers->value());i++)
    {
      sprintf(message_pd,"volume %d 1",i+1);
      pdSend(&pd_main_ctrl,ctrl_ip,PD_MAIN_CTRL_PORT,message_pd);
    }
    for (i=0;i<=NB_DISP_VIDEO_LAYERS;i++)
    {
      cb_show_live_fade(NULL,i);
    }
    for (i=0;i<=NB_DISP_AUDIO_LAYERS;i++)
    {
      cb_show_live_volume(NULL,i);
    }
    sprintf(message_pd,"spk_scale all all 1.0");
    pdSend(&pd_snd_channels,PD_CONTROLLER_IP,PD_SND_CHANNELS_PORT,message_pd);

    strcpy(message_pd,"spk_pos all all");
    for(i=0;i<prj->nb_speakers->value();i++)
    {
        x=ui->speakers[i]->x()+ui->speakers[i]->w()/2-ui->sound_space->x()-ui->sound_space->w()/2;
        y=ui->speakers[i]->y()+ui->speakers[i]->h()/2-ui->sound_space->y()-ui->sound_space->h()/2;
        len=sqrt(x*x+y*y);
        /*if (len<0.0001) angle=0.0;
        else
        {
            angle=atan2(x,-y)*180.0/M_PI;
        }*/
        if (len<1e-8)
        {
            x=0.0; //default values
            y=1.0;
        }
        else
        {
            x/=len;
            y/=len;
        }
        sprintf(temp," %f %f",x,y);
        strcat(message_pd,temp);
    }
    for(;i<MAX_NB_SPEAKERS;i++)
    {
        sprintf(temp," 0 1");
        strcat(message_pd,temp);
    }
    printf("SPEAKERS: %s\n",message_pd);
    pdSend(&pd_snd_channels,ctrl_ip,PD_SND_CHANNELS_PORT,message_pd);

    strcpy(message_pd,"reset");
    pdSend(&pd_main_ctrl,ctrl_ip,PD_MAIN_CTRL_PORT,message_pd);
    strcpy(message_pd,"start");
    pdSend(&pd_main_ctrl,ctrl_ip,PD_MAIN_CTRL_PORT,message_pd);

    ui->show_play_flag=BUTTON_ON;
    ui->updateShowImages();

    return 0;
}

int stop_playlist()
{
    char message_pd[CMD_SIZE];
    char ctrl_ip[DATA_SIZE];

    strcpy(ctrl_ip,startFromChar(prj->sound_ctrl_ip->value(),'@'));

    strcpy(message_pd,"stop");
    pdSend(&pd_main_ctrl,ctrl_ip,PD_MAIN_CTRL_PORT,message_pd);
    strcpy(message_pd,"reset");
    pdSend(&pd_playlist,ctrl_ip,PD_PLAYLIST_PORT,message_pd);

    ui->show_play_flag=BUTTON_OFF;
    ui->updateShowImages();

    return 0;
}


