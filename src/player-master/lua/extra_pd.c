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

#include <m_pd.h>

#include <sys/types.h>
#include <string.h>

#include <stdio.h>

#include <bmc/bimulticast.h>

extern void mcnetsend_setup(void);
extern void luabox_setup(void);

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////


void player_master_lua_setup(void)
{
    // le receive fonctionne seulement en mode UNSAFE,
    // a cause du stupide poll de socket qui va mal (en fait, pd surveille le socket,
    // mais lit directement le contenu et le parse. On ne peut donc pas implanter le
    // protocole 'safe'.
    mcnetsend_setup();
    luabox_setup();

    printf("*** Lighttwist PD Lib V0.1 ***\n");
}


