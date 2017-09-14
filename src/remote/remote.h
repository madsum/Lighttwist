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

#ifndef LT_REMOTE_H
#define LT_REMOTE_H

#include <stdio.h>
#include <stdlib.h>

#include "utils/entry.h"
#include "remote/ltports.h"

#include <bmc/bmc.h>

//#define REMOTERUN

#define MAX_NB_MACHINES 64 //number of machines in left and right channels
#define MAX_NB_MACHINES_PER_CHANNEL MAX_NB_MACHINES/2
#define MAX_NB_PIDS 128

typedef struct {
  tcpcast remote;
  unsigned char connected;
  int pids[MAX_NB_PIDS];
  char ip[DATA_SIZE];
  char label[DATA_SIZE];
  int selected; //non-zero value if selected for an operation, 0 otherwise
} Machine;


#ifdef __cplusplus
extern "C" {
#endif

int desactivate_remote_run ( int i );
int activate_remote_run (int i);
int remote_exec_ssh_ip (const char *ip_address,char *cmd);
int remote_exec_ssh (int i,char *cmd);
int remote_exec (int i,char *cmd);
int remote_wait (int i);
int remote_kill(int i,const char *process);

#ifdef __cplusplus
}
#endif

#endif
