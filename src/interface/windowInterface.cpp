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

#include "windowInterface.h"

unsigned char LANG=LANG_FRENCH;
static char ***lang_labels;

unsigned char langGet(){return LANG;}
int langSet(unsigned char l)
{
  if (l<NB_LANG)
  {
    LANG=l;
    return 0;
  }

  return -1;
}

char *langGetLabel(unsigned char label)
{
  if (label<NB_LANG_LABELS) return lang_labels[LANG][label];
  return NULL;
}

void langInit()
{
  int i,j;
  
  lang_labels=(char ***)(malloc(sizeof(char **)*NB_LANG));
  for (i=0;i<NB_LANG;i++)
  {
    lang_labels[i]=(char **)(malloc(sizeof(char *)*NB_LANG_LABELS));
    for (j=0;j<NB_LANG_LABELS;j++)
    {
      lang_labels[i][j]=(char *)(malloc(sizeof(char)*MSG_SIZE));
    }
  }

  sprintf(lang_labels[LANG_ENGLISH][LABEL_WELCOME],"A default project was created.\n\nDo you want to modify its settings now?\n\nYou can later on modify them in Project/Edit...");
  sprintf(lang_labels[LANG_FRENCH][LABEL_WELCOME],"Un projet par défaut a été créé.\n\nDésirez-vous modifier ses paramètres maintenant?\n\nSinon, vous pourrez les modifier dans Projet/Éditer...");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_YES],"Yes");
  sprintf(lang_labels[LANG_FRENCH][LABEL_YES],"Oui");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_NO],"No");
  sprintf(lang_labels[LANG_FRENCH][LABEL_NO],"Non");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_OK],"OK");
  sprintf(lang_labels[LANG_FRENCH][LABEL_OK],"OK");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CANCEL],"Cancel");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CANCEL],"Annuler");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_RESET],"Default");
  sprintf(lang_labels[LANG_FRENCH][LABEL_RESET],"Par défaut");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_REFRESH],"Refresh");
  sprintf(lang_labels[LANG_FRENCH][LABEL_REFRESH],"Rafraîchir");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CLOSE],"Close");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CLOSE],"Fermer");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UNDO],"Undo");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UNDO],"Annuler");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_APPLY],"Apply");
  sprintf(lang_labels[LANG_FRENCH][LABEL_APPLY],"Appliquer");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_PROJECT_OPEN],"Project/Open...");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_PROJECT_OPEN],"Projet/Ouvrir...");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_PROJECT_EDIT],"Project/Edit...");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_PROJECT_EDIT],"Projet/Éditer...");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_PROJECT_SAVE],"Project/Save");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_PROJECT_SAVE],"Projet/Sauvegarder");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_PROJECT_QUIT],"Project/Quit");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_PROJECT_QUIT],"Projet/Quitter");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_HELP_ONLINE],"?/Get Help Online...");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_HELP_ONLINE],"?/Aide en ligne...");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_HELP_PROBLEM],"?/Report A Problem");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_HELP_PROBLEM],"?/Signaler un problème");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MENU_HELP_ABOUT],"?/About Lighttwist");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MENU_HELP_ABOUT],"?/À propos de Lighttwist");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_BROWSE],"Browse...");
  sprintf(lang_labels[LANG_FRENCH][LABEL_BROWSE],"Parcourir...");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_NAME],"Project filename: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_NAME],"Nom de fichier du projet: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_CONFIGURATION],"UV Model: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_CONFIGURATION],"Modèle UV: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CONFIGURATION_HOMOGRAPHY],"Homography");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CONFIGURATION_HOMOGRAPHY],"Homographie");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_MODE],"Projection mode: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_MODE],"Mode de projection: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_VMIRROR],"Vertical flip: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_VMIRROR],"Inversion verticale: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_RATIO],"Screen ratio: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_RATIO],"Ratio de l'écran: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_NB_PROJECTORS],"Number of projectors \n(monocular projection): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_NB_PROJECTORS],"Nombre de projecteurs \n(projection monoculaire): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_RESOLUTION],"Resolution of projectors: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_RESOLUTION],"Résolution des projecteurs: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_NB_SPEAKERS],"Number of speakers: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_NB_SPEAKERS],"Nombre de haut-parleurs: ");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_MEDIA_DIRECTORY],"Media directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_MEDIA_DIRECTORY],"Répertoire des média: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_LOCAL_SCAN_DIRECTORY],"Local scan directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_LOCAL_SCAN_DIRECTORY],"Répertoire local du scan: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_LOCAL_DATA_DIRECTORY],"Local data directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_LOCAL_DATA_DIRECTORY],"Répertoire local des données: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_LOCAL_BIN_DIRECTORY],"Local bin directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_LOCAL_BIN_DIRECTORY],"Répertoire local des applications: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_LOCAL_LIB_DIRECTORY],"Local lib directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_LOCAL_LIB_DIRECTORY],"Répertoire local des libraries: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_LOCAL_SHARE_DIRECTORY],"Local share directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_LOCAL_SHARE_DIRECTORY],"Répertoire local partagé: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_REMOTE_SCAN_DIRECTORY],"Remote scan directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_REMOTE_SCAN_DIRECTORY],"Répertoire distant du scan: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_REMOTE_DATA_DIRECTORY],"Remote data directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_REMOTE_DATA_DIRECTORY],"Répertoire distant des données: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_REMOTE_BIN_DIRECTORY],"Remote bin directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_REMOTE_BIN_DIRECTORY],"Répertoire distant des applications: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_REMOTE_SHARE_DIRECTORY],"Remote share directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_REMOTE_SHARE_DIRECTORY],"Répertoire distant partagé: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_SOUND_CTRL_IP],"Sound Control IP: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_SOUND_CTRL_IP],"IP du contrôlleur de son: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_ENABLE_JACK],"Enable QJackCtl: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_ENABLE_JACK],"Activer QJackCtl: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MODE_MONO],"Mono");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MODE_MONO],"Mono");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MODE_STEREO_RC],"Stereo R-C");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MODE_STEREO_RC],"Stéréo R-C");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MODE_STEREO_P],"Stereo P");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MODE_STEREO_P],"Stéréo P");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MODE_STEREO_SYNC],"Stereo Synchronized");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MODE_STEREO_SYNC],"Stéréo synchronisée");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOOSE_DIRECTORY],"Choose a directory");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOOSE_DIRECTORY],"Choix du répertoire");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_SETUP],"Setup");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_SETUP],"Configuration");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_SCAN],"Scan");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_SCAN],"Scan");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_SOUND],"Sound");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_SOUND],"Son");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_MEDIA],"Media");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_MEDIA],"Média");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_PLAYLIST],"Playlist");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_PLAYLIST],"Montage");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TAB_SHOW],"Show");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TAB_SHOW],"Projection");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST_PATTERN],"Test pattern: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST_PATTERN],"Image test: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST_WHITE],"White");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST_WHITE],"Blanc");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST_BLACK],"Black");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST_BLACK],"Noir");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST_CHECKER],"Checker");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST_CHECKER],"Quadrillé");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_MACHINE_IP],"Projector %%d IP: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MACHINE_IP],"Projecteur %%d IP: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MACHINE_IP_SPACE],"Projector   %%d IP: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MACHINE_IP_SPACE],"Projecteur   %%d IP: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MACHINES],"Projector(s): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MACHINES],"Projecteur(s): ");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_RECONNECT_ALL],"Reconnect All");
  sprintf(lang_labels[LANG_FRENCH][LABEL_RECONNECT_ALL],"Tout reconnecter");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST],"Test");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST],"Test");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_TEST_ALL],"Test All");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TEST_ALL],"Tout tester");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_STATUS],"Status: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_STATUS],"État: ");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_NO_DATA],"No data available");
  sprintf(lang_labels[LANG_FRENCH][LABEL_NO_DATA],"Aucune donnée disponible");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_PATTERN],"Noise Patterns");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_PATTERN],"Motifs de bruit");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_CAPTURE],"Capture");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_CAPTURE],"Capture");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_SELECT],"Select");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_SELECT],"Selection");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_SUM],"Sum");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_SUM],"Somme");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_UVMAP],"UV Map");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_UVMAP],"Map UV");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_CAM2PROJ],"Cam->Proj");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_CAM2PROJ],"Cam->Proj");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_SMOOTH],"Smooth");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_SMOOTH],"Lissage");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_GAMMA],"Gamma");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_GAMMA],"Gamma");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_COLOR],"Color");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_COLOR],"Couleur");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_BLEND],"Blend");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_BLEND],"Dégradé");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_UV2PROJ],"UV->Proj");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_UV2PROJ],"UV->Proj");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_CALIBRATE],"Calibrate");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_CALIBRATE],"Calibration");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_NORMAL],"Normal");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_NORMAL],"Normal");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_ADVANCED],"Advanced");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_ADVANCED],"Avancé");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCAN_MODE],"Mode:");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCAN_MODE],"Mode:");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_BLEND_WINDOW_TITLE],"Blend");
  sprintf(lang_labels[LANG_FRENCH][LABEL_BLEND_WINDOW_TITLE],"Dégradé (zones de chevauchement)");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_WINDOW_TITLE],"Camera To Projectors");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_WINDOW_TITLE],"Caméra vers projecteurs");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UV2PROJ_WINDOW_TITLE],"UV To Projectors");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UV2PROJ_WINDOW_TITLE],"UV vers projecteurs");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_COLOR_WINDOW_TITLE],"Color Calibration");
  sprintf(lang_labels[LANG_FRENCH][LABEL_COLOR_WINDOW_TITLE],"Calibration des couleurs");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UVMAP_WINDOW_TITLE],"UV Mapping");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UVMAP_WINDOW_TITLE],"Mapping UV");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_WINDOW_TITLE],"Gamma Calibration");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_WINDOW_TITLE],"Calibration gamma");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PATTERN_WINDOW_TITLE],"Noise patterns");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PATTERN_WINDOW_TITLE],"Motifs de bruit");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_WINDOW_TITLE],"Grab");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_WINDOW_TITLE],"Capture");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MEDIA_WINDOW_TITLE],"Media");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MEDIA_WINDOW_TITLE],"Média");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAY_WINDOW_TITLE],"Play");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAY_WINDOW_TITLE],"Projection");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PRECROP_WINDOW_TITLE],"Import Media");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PRECROP_WINDOW_TITLE],"Importation de média");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_WINDOW_TITLE],"Project");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_WINDOW_TITLE],"Projet");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SHELL_WINDOW_TITLE],"Application Output");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SHELL_WINDOW_TITLE],"Messages de l'application");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SMOOTH_WINDOW_TITLE],"Smooth");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SMOOTH_WINDOW_TITLE],"Lissage");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_LIVE_WINDOW_TITLE],"LIVE Controller!");
  sprintf(lang_labels[LANG_FRENCH][LABEL_LIVE_WINDOW_TITLE],"Contrôleur temps réel!");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_MEDIA_TREE],"Media tree directory: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MEDIA_TREE],"Répertoire des média: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_AUTO_PLAYLIST],"Automatic creation of\nplaylist files");
  sprintf(lang_labels[LANG_FRENCH][LABEL_AUTO_PLAYLIST],"Création automatique de\nfichiers de montage");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SELECTED_MEDIA],"Selected Media: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SELECTED_MEDIA],"Média sélectionnés: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_IMPORT_MEDIA],"Import");
  sprintf(lang_labels[LANG_FRENCH][LABEL_IMPORT_MEDIA],"Importer");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_REMOVE_MEDIA],"Remove");
  sprintf(lang_labels[LANG_FRENCH][LABEL_REMOVE_MEDIA],"Supprimer");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CURRENT_MEDIA],"Current Media Folder: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CURRENT_MEDIA],"Répertoire courant des média: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SAVE_PLAYLIST],"Save playlist");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SAVE_PLAYLIST],"Sauvegarder");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_LOAD_PLAYLIST],"Load playlist");
  sprintf(lang_labels[LANG_FRENCH][LABEL_LOAD_PLAYLIST],"Ouvrir");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_ADD_SOUNDTRACK],"Extract soundtrack from\n video file");
  sprintf(lang_labels[LANG_FRENCH][LABEL_ADD_SOUNDTRACK], "Extraire la bande sonore");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_MASTER_VIDEO],"Video Fade\nMaster");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MASTER_VIDEO],"Alpha Vidéo\nGlobal");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_MASTER_AUDIO],"Audio Volume\nMaster");
  sprintf(lang_labels[LANG_FRENCH][LABEL_MASTER_AUDIO],"Volume Audio\nGlobal");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SCENE_SCALE],"Scene Scale: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SCENE_SCALE],"Échelle de la scène: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_STEREO_B],"Baseline of the Eyes: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_STEREO_B],"Baseline des yeux: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_STEREO_R],"Panoramic Screen  \nRadius (Inches): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_STEREO_R],"Rayon de l'écran  \npanoramique (pouces): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SOUND_SCALE],"Sound Separation Scale: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SOUND_SCALE],"Facteur de séparation du son: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_STEREO_DISTORT],"Stereo Distortion: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_STEREO_DISTORT],"Distortion stéréo: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_STEREO_SLIT],"Field of View: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_STEREO_SLIT],"Angle de vue: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_RED],"Red");
  sprintf(lang_labels[LANG_FRENCH][LABEL_RED],"Rouge");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GREEN],"Green");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GREEN],"Vert");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_BLUE],"Blue");
  sprintf(lang_labels[LANG_FRENCH][LABEL_BLUE],"Bleu");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_MIN_RANGE],"Minimum Range: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_MIN_RANGE],"Intervalle minimum: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_MIN_CONFIDENCE],"Confidence Level (range %%): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_MIN_CONFIDENCE],"Niveau de confiance d'un motif\n(%% de l'intervalle): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_CONFIDENCE_THRESHOLD],"Confidence Threshold: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_CONFIDENCE_THRESHOLD],"Seuil de confiance: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_NOISE],"Use noise patterns: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_NOISE],"Utiliser les motifs de bruit: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAM2PROJ_CROP],"Crop Build Result: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAM2PROJ_CROP],"Recadrer la reconstruction: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_COLOR_CONFIDENCE],"Confidence Threshold: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_COLOR_CONFIDENCE],"Seuil de confiance: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UVMAP_VMIDDLE],"Vertical\ncenter");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UVMAP_VMIDDLE],"Centre\nvertical");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UVMAP_INNER_SCALE],"Bottom Ellipsoid Factor: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UVMAP_INNER_SCALE],"Facteur de l'ellipse\ninférieure: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UVMAP_OUTER_SCALE],"Top Ellipsoid Factor: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UVMAP_OUTER_SCALE],"Facteur de l'ellipse\nsuppérieure: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_MODE],"Projection Mode: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_MODE],"Mode de projection: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_STL_ONLY],"STL only: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_STL_ONLY],"STL seulement: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_CAMERA],"Camera: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_CAMERA],"Caméra: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_CAMID],"ID: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_CAMID],"ID: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_DISTO],"Params. de distorsion radiale: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_DISTO],"Radial distorsion parameters: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_DISTO_K1],"k1: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_DISTO_K1],"k1: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_DISTO_K2],"k2: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_DISTO_K2],"k2: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_INTERVAL],"Exposure Time (Sec.): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_INTERVAL],"Temps d'exposition (sec.): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_TRANSFER],"Camera files transfer: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_TRANSFER],"Transfert des fichiers caméra: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAPTURE_TEST],"Test Camera");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAPTURE_TEST],"Test caméra");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PRECROP_HEADER],"For video only");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PRECROP_HEADER],"Pour vidéo seulement");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PRECROP_WIDTH],"Tile Width: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PRECROP_WIDTH],"Largeur des tuiles: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PRECROP_HEIGHT],"Tile Height: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PRECROP_HEIGHT],"Hauteur des tuiles: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SMOOTH_NB_PATCHES_X],"Number of planes (X): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SMOOTH_NB_PATCHES_X],"Nombre de plans (X): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_SMOOTH_NB_PATCHES_Y],"Number of patches (Y): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SMOOTH_NB_PATCHES_Y],"Nombre de plans (Y): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_MANUAL],"Set Gamma Manually: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_MANUAL],"Ajustement gamma manuel: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_CAMERA_R],"Gamma of Camera (R): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_CAMERA_R],"Gamma de la caméra (R): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_CAMERA_G],"Gamma of Camera (G): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_CAMERA_G],"Gamma de la caméra (V): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_CAMERA_B],"Gamma of Camera (B): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_CAMERA_B],"Gamma de la caméra (B): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_PROJECTOR_R],"Gamma of Projectors (R): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_PROJECTOR_R],"Gamma des projecteurs (R): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_PROJECTOR_G],"Gamma of Projectors (G): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_PROJECTOR_G],"Gamma des projecteurs (V): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_GAMMA_PROJECTOR_B],"Gamma of Projectors (B): ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_GAMMA_PROJECTOR_B],"Gamma des projecteurs (B): ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_UV2PROJ_ANGLE_PROJ_MAX],"Maximum projector\nview angle: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_UV2PROJ_ANGLE_PROJ_MAX],"Angle de vue maximum\nd'un projector: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PATTERN_FREQUENCY],"Frequency: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PATTERN_FREQUENCY],"Fréquence: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PATTERN_NB],"Number of patterns: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PATTERN_NB],"Nombre de motifs: ");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_SAVE_PROJECT],"Do you want to save the existing project?");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SAVE_PROJECT],"Désirez-vous sauvegarder votre projet?");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PROJECT_CHOOSE_FILE],"Choose a project file");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PROJECT_CHOOSE_FILE],"Choisissez un fichier projet");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYLIST_CHOOSE_FILENAME],"Choose a playlist file");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYLIST_CHOOSE_FILENAME],"Choisissez un fichier montage");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYLIST_FILES],"Playlist Files (*.pll)");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYLIST_FILES],"Fichiers montage (*.pll)");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYLIST_ENTER_FILENAME],"Enter a playlist filename (.pll)");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYLIST_ENTER_FILENAME],"Entrer un nom de fichier montage (.pll)");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_NEW_PLAYLIST_SAVE_PREVIOUS],"You are about to create a new playlist.\nDo you want to save the changes to the current one.");
  sprintf(lang_labels[LANG_FRENCH][LABEL_NEW_PLAYLIST_SAVE_PREVIOUS],"Vous êtes sur le point de créer un nouveau fichier de montage.\nVoulez-vous sauvegarder les changements apportés au fichier de montage courant?");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_DELETE_MEDIA],"Are you sure you want to delete the selected media from Lighttwist?\nNote that this operation will not delete the actual media on disk.");
  sprintf(lang_labels[LANG_FRENCH][LABEL_DELETE_MEDIA],"Êtes-vous sûr de vouloir supprimer de Lighttwist les média sélectionnés?\nNotez que cette opération ne supprimera pas les média sur le disque.");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_DELETE_CAMERA_FILES],"This operation will delete all existing images on your camera.\nDo you want to continue?");
  sprintf(lang_labels[LANG_FRENCH][LABEL_DELETE_CAMERA_FILES],"Cette opération supprimera toute image sauvegardée sur votre caméra.\nVoulez-vous exécuter l'opération maintenant?");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CAMERA_FILE_LOCATION],"Select the directory corresponding to the camera's data card.");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CAMERA_FILE_LOCATION],"Veuillez choisir le répertoire contenant les données de la caméra.");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_REMOVE],"Remove");
  sprintf(lang_labels[LANG_FRENCH][LABEL_REMOVE],"Supprimer");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_TIMESCALE],"Timescale");
  sprintf(lang_labels[LANG_FRENCH][LABEL_TIMESCALE],"Échelle");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_VIDEO_LAYER],"Video %%d\n");
  sprintf(lang_labels[LANG_FRENCH][LABEL_VIDEO_LAYER],"Vidéo %%d\n");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_AUDIO_LAYER],"Audio %%d\n");
  sprintf(lang_labels[LANG_FRENCH][LABEL_AUDIO_LAYER],"Audio %%d\n");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_VIDEO],"Video: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_VIDEO],"Vidéo: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_AUDIO],"Audio: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_AUDIO],"Audio: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_3D],"3D: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_3D],"3D: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_IMAGE],"Image: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_IMAGE],"Image: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_STATS],"Stats: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_STATS],"Stats: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_EXT_PLAYLIST],"Playlist: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_EXT_PLAYLIST],"Montage: ");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_THREADING],"Player Mode: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_THREADING],"Mode du lecteur: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_SELECT],"Player: ");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_SELECT],"Lecteur: ");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_SINGLE_THREAD],"Normal");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_SINGLE_THREAD],"Normal");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_MULTI_THREAD],"Optimized");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_MULTI_THREAD],"Optimisé");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_DEFAULT],"Default");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_DEFAULT],"Défaut");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_PLAYER_LUA],"Lua");
  sprintf(lang_labels[LANG_FRENCH][LABEL_PLAYER_LUA],"Lua");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_LEFT],"Left Channel");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_LEFT],"Canal gauche");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_RIGHT],"Right Channel");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_RIGHT],"Canal droit");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_ALL],"All");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_ALL],"Tous");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_PATTERNS],"Patterns");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_PATTERNS],"Motifs");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_GAMMA],"Gamma");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_GAMMA],"Gamma");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_COLOR],"Color");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_COLOR],"Couleurs");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_NOISE],"Noise");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_NOISE],"Bruit");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_AUTOMATIC],"Automatic");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_AUTOMATIC],"Automatique");
  sprintf(lang_labels[LANG_ENGLISH][LABEL_CHOICE_MANUAL],"Manual");
  sprintf(lang_labels[LANG_FRENCH][LABEL_CHOICE_MANUAL],"Manuel");

  sprintf(lang_labels[LANG_ENGLISH][LABEL_SOUND_SUBWOOFER],"Subwoofer");
  sprintf(lang_labels[LANG_FRENCH][LABEL_SOUND_SUBWOOFER],"Caisson de basse");
}

int setFloatSpinner(Fl_Spinner *spin,double vmin,double vmax,int *lineposy)
{
  if (spin==NULL || lineposy==NULL) return -1;

  //spin->value(default_val);
  spin->labelfont(FONTFACE);
  spin->labelsize(FONTSIZE);
  spin->textsize(FONTSIZE);
  spin->minimum(vmin);
  spin->maximum(vmax);
  spin->step(0.01);
  spin->format("%.02f");

  (*lineposy)+=LINESPACE;

  return 0;
}

int setIntegerSpinner(Fl_Spinner *spin,int vmin,int vmax,int *lineposy)
{
  if (spin==NULL || lineposy==NULL) return -1;

  //spin->value(default_val);
  spin->labelfont(FONTFACE);
  spin->labelsize(FONTSIZE);
  spin->textsize(FONTSIZE);
  spin->minimum((double)(vmin));
  spin->maximum((double)(vmax));
  spin->step(1.0);
  spin->format("%.f");

  (*lineposy)+=LINESPACE;

  return 0;
}

int setInput(Fl_Input *input,int *lineposy)
{
  if (input==NULL || lineposy==NULL) return -1;

  //input->value(default_val);
  input->labelfont(FONTFACE);
  input->labelsize(FONTSIZE);
  input->textsize(FONTSIZE);
  (*lineposy)+=LINESPACE;

  return 0;
}

int setButton(Fl_Button *but,int *lineposy)
{
  if (but==NULL || lineposy==NULL) return -1;

  //but->value(default_val);
  but->labelfont(FONTFACE);
  but->labelsize(FONTSIZE);
  //but->textsize(FONTSIZE);
  (*lineposy)+=LINESPACE;

  return 0;
}

int setChoice(Fl_Choice *choice,int *lineposy)
{
  if (choice==NULL || lineposy==NULL) return -1;

  //choice->value(default_val);
  choice->labelfont(FONTFACE);
  choice->labelsize(FONTSIZE);
  choice->textsize(FONTSIZE);
  (*lineposy)+=LINESPACE;

  return 0;
}

int setCheckButton(Fl_Check_Button *check,int *lineposy)
{
  if (check==NULL || lineposy==NULL) return -1;

  //check->value(default_val);
  check->labelfont(FONTFACE);
  check->labelsize(FONTSIZE);
  check->align(FL_ALIGN_LEFT);
  (*lineposy)+=LINESPACE;

  return 0;
}

int setSlider(Fl_Slider *slider,int *lineposy)
{
  if (slider==NULL || lineposy==NULL) return -1;

  //slider->value(default_val);
  slider->labelfont(FONTFACE);
  slider->labelsize(FONTSIZE);
  slider->type(FL_HORIZONTAL); 
  slider->align(FL_ALIGN_LEFT);
  (*lineposy)+=LINESPACE;

  return 0;
}

int setBox(Fl_Box *box,int *lineposy)
{
  if (box==NULL || lineposy==NULL) return -1;

  //box->value(default_val);
  box->labelfont(FONTFACE);
  box->labelsize(FONTSIZE);
  (*lineposy)+=LINESPACE;

  return 0;
}

//gray itensity from 0.0 to 1.0
Fl_Color getGrayLevel(double i)
{
  int val;

  val=(int)(FL_NUM_GRAY*i);
  if (val<0) val=0;
  if (val>FL_NUM_GRAY) val=FL_NUM_GRAY;
  return fl_gray_ramp(val);
}

void box_user_draw(int x, int y, int w, int h, Fl_Color c)
{
    fl_color(c);
    fl_rectf(x, y, w, h);
    fl_color(getGrayLevel(BORDER_INTENSITY));
    fl_rect(x, y, w, h);
}


