/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2019-2020 - James Leaver
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

#ifndef _DINGUX_UTILS_H
#define _DINGUX_UTILS_H

#include <ks_common_api.h>
#include <libks.h>

#include <boolean.h>

KS_BEGIN_DECLS

/* Specifies all possible image filtering
 * methods when using the IPU hardware scaler
 * > Note: We do not allow 'fine tuning' of the
 *   bicubic sharpness factor, since anything
 *   other than the default value looks terrible... */
enum dingux_ipu_filter_type
{
   DINGUX_IPU_FILTER_BICUBIC = 0,
   DINGUX_IPU_FILTER_BILINEAR,
   DINGUX_IPU_FILTER_NEAREST,
   DINGUX_IPU_FILTER_LAST
};

/* Enables/disables downscaling when using
 * the IPU hardware scaler */
bool dingux_ipu_set_downscaling_enable(bool enable);

/* Enables/disables aspect ratio correction
 * (1:1 PAR) when using the IPU hardware
 * scaler (disabling this will stretch the
 * image to the full screen dimensions) */
bool dingux_ipu_set_aspect_ratio_enable(bool enable);

/* Enables/disables integer scaling when
 * using the IPU hardware scaler */
bool dingux_ipu_set_integer_scaling_enable(bool enable);

/* Sets the image filtering method when
 * using the IPU hardware scaler */
bool dingux_ipu_set_filter_type(enum dingux_ipu_filter_type filter_type);

/* Fetches internal battery level */
int dingux_get_battery_level(void);

KS_END_DECLS

#endif
