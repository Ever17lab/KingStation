/*  KingStation - A frontend for libks.
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

#ifndef _WIN32_UI
#define _WIN32_UI

#include <stdint.h>
#include <stddef.h>

#include <boolean.h>
#include <ks_common_api.h>

#ifndef _XBOX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "../ui_companion_driver.h"

KS_BEGIN_DECLS

typedef struct ui_application_win32
{
   void *empty;
} ui_application_win32_t;

typedef struct ui_window_win32
{
   HWND hwnd;
} ui_window_win32_t;

extern VOID (WINAPI *DragAcceptFiles_func)(HWND, BOOL);

KS_END_DECLS

#endif
