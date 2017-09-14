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

#include <utils/ThreadInterface.h>
#include <utils/Thread.h>
#include <stdlib.h>
#include <string.h>
#include <stdincast.h>

#include "utils/entry.h"

#include <bmc/bmc.h>

#define BS 500

#define INPUT_BMC 0
#define INPUT_STDIN 1

class ReceiveThread: public Thread
{
private:
  ThreadInterface *ti;
  bimulticast bmc;
  Entry *cmds;
  int port;

  void Execute(void*);
  void Setup();
public:
  ReceiveThread(int p);
  int GetNextCommand(Entry *cpy_entry);
  int ContainsRefresh();
};

