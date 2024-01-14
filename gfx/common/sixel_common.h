/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  copyright (c) 2011-2017 - Daniel De Matteis
 *  copyright (c) 2016-2019 - Brad Parker
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

#ifndef __SIXEL_COMMON_H
#define __SIXEL_COMMON_H

#include <sixel.h>

#define SIXEL_COLORS 256

typedef struct sixel
{
   SIXELSTATUS sixel_status;
   unsigned video_width;
   unsigned video_height;
   unsigned screen_width;
   unsigned screen_height;
} sixel_t;

#endif