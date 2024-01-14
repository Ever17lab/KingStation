/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2020 - Daniel De Matteis
 *
 *  KingStation is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  KingStation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with KingStation.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DINPUT_JOYPAD_H
#define __DINPUT_JOYPAD_H

#include <stdint.h>
#include <boolean.h>
#include <ks_common_api.h>

#include <dinput.h>

/* For DIJOYSTATE2 struct, rgbButtons will always have 128 elements */
#define ARRAY_SIZE_RGB_BUTTONS 128

KS_BEGIN_DECLS

struct dinput_joypad_data
{
   LPDIRECTINPUTDEVICE8 joypad;
   DIJOYSTATE2 joy_state;
   char* joy_name;
   char* joy_friendly_name;
   int32_t vid;
   int32_t pid;
   LPDIRECTINPUTEFFECT rumble_iface[2];
   DIEFFECT rumble_props;
};

KS_END_DECLS

#endif
