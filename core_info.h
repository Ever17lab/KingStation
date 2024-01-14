/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
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

#ifndef CORE_INFO_H_
#define CORE_INFO_H_

#include <stddef.h>

#include <lists/string_list.h>
#include <ks_common_api.h>

KS_BEGIN_DECLS

enum core_info_list_qsort_type
{
   CORE_INFO_LIST_SORT_PATH = 0,
   CORE_INFO_LIST_SORT_DISPLAY_NAME,
   CORE_INFO_LIST_SORT_CORE_NAME,
   CORE_INFO_LIST_SORT_SYSTEM_NAME
};

typedef struct
{
   char *path;
   char *desc;
   /* Set missing once to avoid opening
    * the same file several times. */
   bool missing;
   bool optional;
} core_info_firmware_t;

/* Simple container/convenience struct for
 * holding the 'id' of a core file
 * > 'id' is the filename without extension or
 *   platform-specific suffix
 * > 'id' is used for core info searches - enables
 *   matching regardless of core file base path,
 *   and is platform-independent (e.g. an Android
 *   core file will be correctly identified on Linux)
 * > 'len' is used to cache the length of 'str', for
 *   improved performance when performing string
 *   comparisons */
typedef struct
{
   char *str;
   size_t len;
} core_file_id_t;

typedef struct
{
   char *path;
   void *config_data;
   char *display_name;
   char *display_version;
   char *core_name;
   char *system_manufacturer;
   char *systemname;
   char *system_id;
   char *supported_extensions;
   char *authors;
   char *permissions;
   char *licenses;
   char *categories;
   char *databases;
   char *notes;
   char *required_hw_api;
   char *description;
   struct string_list *categories_list;
   struct string_list *databases_list;
   struct string_list *note_list;
   struct string_list *supported_extensions_list;
   struct string_list *authors_list;
   struct string_list *permissions_list;
   struct string_list *licenses_list;
   struct string_list *required_hw_api_list;
   core_info_firmware_t *firmware;
   core_file_id_t core_file_id; /* ptr alignment */
   void *userdata;
   size_t firmware_count;
   bool supports_no_game;
   bool database_match_archive_member;
   bool is_experimental;
   bool is_locked;
} core_info_t;

/* A subset of core_info parameters required for
 * core updater tasks */
typedef struct
{
   char *display_name;
   char *description;
   char *licenses;
   bool is_experimental;
} core_updater_info_t;

typedef struct
{
   core_info_t *list;
   char *all_ext;
   size_t count;
} core_info_list_t;

typedef struct core_info_ctx_firmware
{
   const char *path;
   struct
   {
      const char *system;
   } directory;
} core_info_ctx_firmware_t;

typedef struct core_info_ctx_find
{
   core_info_t *inf;
   const char *path;
} core_info_ctx_find_t;

struct core_info_state
{
#ifdef HAVE_COMPRESSION
   const struct string_list *tmp_list;
#endif
   const char *tmp_path;
   core_info_t *current;
   core_info_list_t *curr_list;
};

typedef struct core_info_state core_info_state_t;

size_t core_info_list_num_info_files(core_info_list_t *list);

/* Non-reentrant, does not allocate. Returns pointer to internal state. */
void core_info_list_get_supported_cores(core_info_list_t *list,
      const char *path, const core_info_t **infos, size_t *num_infos);

bool core_info_list_get_display_name(core_info_list_t *list,
      const char *path, char *s, size_t len);

bool core_info_get_display_name(const char *path, char *s, size_t len);

/* Returns core_info parameters required for
 * core updater tasks, read from specified file.
 * Returned core_updater_info_t object must be
 * freed using core_info_free_core_updater_info().
 * Returns NULL if 'path' is invalid. */
core_updater_info_t *core_info_get_core_updater_info(const char *path);
void core_info_free_core_updater_info(core_updater_info_t *info);

void core_info_get_name(const char *path, char *s, size_t len,
      const char *path_info, const char *dir_cores,
      const char *exts, bool show_hidden_files,
      bool get_display_name);

core_info_t *core_info_get(core_info_list_t *list, size_t i);

void core_info_free_current_core(core_info_state_t *p_coreinfo);

bool core_info_init_current_core(void);

bool core_info_get_current_core(core_info_t **core);

void core_info_deinit_list(void);

bool core_info_init_list(const char *path_info, const char *dir_cores,
      const char *exts, bool show_hidden_files);

bool core_info_get_list(core_info_list_t **core);

/* Returns number of installed cores */
size_t core_info_count(void);

bool core_info_list_update_missing_firmware(core_info_ctx_firmware_t *info,
      bool *set_missing_bios);

bool core_info_find(core_info_ctx_find_t *info);

bool core_info_load(
      core_info_ctx_find_t *info,
      core_info_state_t *p_coreinfo);

bool core_info_database_supports_content_path(const char *database_path, const char *path);

bool core_info_database_match_archive_member(const char *database_path);

bool core_info_unsupported_content_path(const char *path);

void core_info_qsort(core_info_list_t *core_info_list, enum core_info_list_qsort_type qsort_type);

bool core_info_list_get_info(core_info_list_t *core_info_list,
      core_info_t *out_info, const char *path);

bool core_info_hw_api_supported(core_info_t *info);

/* Sets 'locked' status of specified core
 * > Returns true if successful
 * > Like all functions that access the cached
 *   core info list this is *not* thread safe */
bool core_info_set_core_lock(const char *core_path, bool lock);
/* Fetches 'locked' status of specified core
 * > If 'validate_path' is 'true', will search
 *   cached core info list for a corresponding
 *   'sanitised' core file path. This is *not*
 *   thread safe
 * > If 'validate_path' is 'false', performs a
 *   direct filesystem check. This *is* thread
 *   safe, but validity of specified core path
 *   must be checked externally */
bool core_info_get_core_lock(const char *core_path, bool validate_path);

core_info_state_t *coreinfo_get_ptr(void);

bool core_info_core_file_id_is_equal(const char* core_path_a, const char* core_path_b);

KS_END_DECLS

#endif /* CORE_INFO_H_ */
