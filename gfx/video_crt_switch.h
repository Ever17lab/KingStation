/* CRT SwitchRes Core
 * Copyright (C) 2018 Alphanu / Ben Templeman.
 *
 * KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
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

#ifndef __VIDEO_CRT_SWITCH_H__
#define __VIDEO_CRT_SWITCH_H__

#include <stdint.h>

#include <boolean.h>
#include <ks_common_api.h>

KS_BEGIN_DECLS

typedef struct videocrt_switch
{
   double p_clock;

   int center_adjust;
   int porch_adjust;
   int tmp_porch_adjust;
   int tmp_center_adjust;
   unsigned ra_core_width;
   unsigned ra_core_height;
   unsigned ra_tmp_width;
   unsigned ra_tmp_height;
   unsigned ra_set_core_hz;
   unsigned index;

   float ra_core_hz;
   float ra_tmp_core_hz;
   float fly_aspect;

} videocrt_switch_t;

void crt_switch_res_core(
      videocrt_switch_t *p_switch,
      unsigned width,
      unsigned height,
      float hz,
      unsigned crt_mode,
      int crt_switch_center_adjust,
      int crt_switch_porch_adjust,
      int monitor_index,
      bool dynamic);

KS_END_DECLS

#endif
