/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2019 - Daniel De Matteis
 *  Copyright (C) 2016-2019 - Brad Parker
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

#ifndef __PATHS_H
#define __PATHS_H

#include <boolean.h>
#include <ks_common_api.h>

#include <lists/string_list.h>

KS_BEGIN_DECLS

enum kingsn_dir_type
{
   KINGSN_DIR_NONE = 0,
   KINGSN_DIR_SAVEFILE,
   KINGSN_DIR_SAVESTATE,
   KINGSN_DIR_CURRENT_SAVEFILE,
   KINGSN_DIR_CURRENT_SAVESTATE,
   KINGSN_DIR_SYSTEM
};

enum kingsn_content_type
{
   KINGSN_CONTENT_NONE = 0,
   KINGSN_CONTENT_MOVIE,
   KINGSN_CONTENT_MUSIC,
   KINGSN_CONTENT_IMAGE,
   KINGSN_CONTENT_GONG
};

enum kingsn_path_type
{
   KINGSN_PATH_NONE = 0,
   KINGSN_PATH_CORE,
   KINGSN_PATH_NAMES,
   KINGSN_PATH_CONFIG,
   KINGSN_PATH_CONTENT,
   KINGSN_PATH_CONFIG_APPEND,
   KINGSN_PATH_CORE_OPTIONS,
   KINGSN_PATH_DEFAULT_SHADER_PRESET,
   KINGSN_PATH_BASENAME,
   KINGSN_PATH_SUBSYSTEM
};

void dir_clear(enum kingsn_dir_type type);

size_t dir_get_size(enum kingsn_dir_type type);

char *dir_get_ptr(enum kingsn_dir_type type);

void dir_set(enum kingsn_dir_type type, const char *path);

void dir_check_defaults(const char *custom_ini_path);

void path_deinit_savefile(void);

bool path_set(enum kingsn_path_type type, const char *path);

void path_set_special(char **argv, unsigned num_content);

size_t path_get_realsize(enum kingsn_path_type type);

struct string_list *path_get_subsystem_list(void);

char *path_get_ptr(enum kingsn_path_type type);

const char *path_get(enum kingsn_path_type type);

void path_clear(enum kingsn_path_type type);

bool path_is_empty(enum kingsn_path_type type);

enum kingsn_content_type path_is_media_type(const char *path);

KS_END_DECLS

#endif
