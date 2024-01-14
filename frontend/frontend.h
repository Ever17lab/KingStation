/* KingStation - A frontend for libks.
 * Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 * Copyright (C) 2011-2017 - Daniel De Matteis
 * Copyright (C) 2012-2015 - Michael Lelli
 *
 * KingStation is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * KingStation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with KingStation.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _KINGSN_FRONTEND_H
#define _KINGSN_FRONTEND_H

#include <stdint.h>
#include <stddef.h>

#include <ks_common_api.h>

KS_BEGIN_DECLS

/**
 * main_exit:
 *
 * Cleanly exit KingStation.
 *
 * Also saves configuration files to disk,
 * and (optionally) autosave state.
 **/
void main_exit(void *args);

/**
 * main_entry:
 *
 * Main function of KingStation.
 *
 * If HAVE_MAIN is not defined, will contain main loop and will not
 * be exited from until we exit the program. Otherwise, will
 * just do initialization.
 *
 * Returns: varies per platform.
 **/
int kingsn_main(int argc, char *argv[], void *data);

KS_END_DECLS

#endif
