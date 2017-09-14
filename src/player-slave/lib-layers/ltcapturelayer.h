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

#ifndef LTCAPTURELAYER_H
#define LTCAPTURELAYER_H

#include <osg/Image>
#include <imgu/imgu.h>

#include "ltlayer.h"

#define NB_IMGU 20

class OSG_EXPORT LTCaptureLayer : public LTLayer {

  public:

    //LTCaptureLayer( const char* shaderFile, int camid, int exposure, int fps );
	LTCaptureLayer( const char* shaderFile, int camid, int exposure, int fps, 
					float xTexMin, float xTexMax, float yTexMin, float yTexMax);
    virtual ~LTCaptureLayer();
    void nodeCallback();
    void setLatestImg( imgu* i );
    void updateImage();

    // scanner
    void initScannerEffect(double Lmin,double Lmax,double StepIn,double StepOut);
    void scannerEffect(int status); // 0=normal viewing, 1=take reference image and scan
    void processScanCB(imgu *I); // pour le callback

    void initFreezeEffect(void);
    void freezeEffect(int status); // 0=normal viewing, 1=freeze
    void processFreezeCB(imgu *I); // pour le callback

  protected:

  private:
    //capture _cap;
    char _capDevice[30];
    imgu* _capImgu[NB_IMGU];
    osg::Image* _capImg;
    bool _stopCallback;
    imgu* _latestImg;


  /// pour l'effet de scan...
  double LimiteMin,LimiteMax;
  double StepIn,StepOut;
  imgu *lastScan;
  int scanMode;
  double scanFade;

  // pour l'effet de freeze
  int freezeON; // 0=non, 1=oui. 2=fade to freeze
  imgu *lastFreeze; // pour le fade
  double freezeFade;

    int scanComputeDiff(imgu *I,imgu *Ir,double fade); // pour effet scanner



};


#endif // LTCAPTURELAYER_H
