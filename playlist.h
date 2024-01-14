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

#ifndef _PLAYLIST_H__
#define _PLAYLIST_H__

#include <stddef.h>

#include <ks_common_api.h>
#include <boolean.h>
#include <lists/string_list.h>

#include "core_info.h"

KS_BEGIN_DECLS

/* Default maximum playlist size */
#define COLLECTION_SIZE 0x7FFFFFFF

typedef struct content_playlist playlist_t;

enum playlist_runtime_status
{
   PLAYLIST_RUNTIME_UNKNOWN = 0,
   PLAYLIST_RUNTIME_MISSING,
   PLAYLIST_RUNTIME_VALID
};

enum playlist_file_mode
{
   PLAYLIST_LOAD = 0,
   PLAYLIST_SAVE
};

enum playlist_label_display_mode
{
   LABEL_DISPLAY_MODE_DEFAULT = 0,
   LABEL_DISPLAY_MODE_REMOVE_PARENTHESES,
   LABEL_DISPLAY_MODE_REMOVE_BRACKETS,
   LABEL_DISPLAY_MODE_REMOVE_PARENTHESES_AND_BRACKETS,
   LABEL_DISPLAY_MODE_KEEP_REGION,
   LABEL_DISPLAY_MODE_KEEP_DISC_INDEX,
   LABEL_DISPLAY_MODE_KEEP_REGION_AND_DISC_INDEX
};

enum playlist_thumbnail_mode
{
   PLAYLIST_THUMBNAIL_MODE_DEFAULT = 0,
   PLAYLIST_THUMBNAIL_MODE_OFF,
   PLAYLIST_THUMBNAIL_MODE_SCREENSHOTS,
   PLAYLIST_THUMBNAIL_MODE_TITLE_SCREENS,
   PLAYLIST_THUMBNAIL_MODE_BOXARTS
};

enum playlist_sort_mode
{
   PLAYLIST_SORT_MODE_DEFAULT = 0,
   PLAYLIST_SORT_MODE_ALPHABETICAL,
   PLAYLIST_SORT_MODE_OFF
};

/* TODO/FIXME - since gfx_thumbnail_path.h has now
 * been divorced from the menu code, perhaps jdgleaver
 * can refactor this? */

/* Note: We already have a left/right enum defined
 * in gfx_thumbnail_path.h - but we can't include
 * menu code here, so have to make a 'duplicate'... */
enum playlist_thumbnail_id
{
   PLAYLIST_THUMBNAIL_RIGHT = 0,
   PLAYLIST_THUMBNAIL_LEFT
};

struct playlist_entry
{
   char *path;
   char *label;
   char *core_path;
   char *core_name;
   char *db_name;
   char *crc32;
   char *subsystem_ident;
   char *subsystem_name;
   char *runtime_str;
   char *last_played_str;
   struct string_list *subsystem_roms;
   unsigned runtime_hours;
   unsigned runtime_minutes;
   unsigned runtime_seconds;
   /* Note: due to platform dependence, have to record
    * timestamp as either a string or independent integer
    * values. The latter is more verbose, but more efficient. */
   unsigned last_played_year;
   unsigned last_played_month;
   unsigned last_played_day;
   unsigned last_played_hour;
   unsigned last_played_minute;
   unsigned last_played_second;
   enum playlist_runtime_status runtime_status;
};

/* Holds all configuration parameters required
 * when initialising/saving playlists */
typedef struct
{
   size_t capacity;
   bool old_format;
   bool compress;
   bool fuzzy_archive_match;
   bool autofix_paths;   
   char path[PATH_MAX_LENGTH];
   char base_content_directory[PATH_MAX_LENGTH];
} playlist_config_t;

/* Convenience function: copies specified playlist
 * path to specified playlist configuration object */
void playlist_config_set_path(playlist_config_t *config, const char *path);

/* Convenience function: copies base content directory
 * path to specified playlist configuration object */
void playlist_config_set_base_content_directory(playlist_config_t* config, const char* path);

/* Creates a copy of the specified playlist configuration.
 * Returns false in the event of an error */
bool playlist_config_copy(const playlist_config_t *src, playlist_config_t *dst);

/* Returns internal playlist configuration object
 * of specified playlist.
 * Returns NULL it the event of an error. */
playlist_config_t *playlist_get_config(playlist_t *playlist);

/**
 * playlist_init:
 * @config            	: Playlist configuration object.
 *
 * Creates and initializes a playlist.
 *
 * Returns: handle to new playlist if successful, otherwise NULL
 **/
playlist_t *playlist_init(const playlist_config_t *config);

/**
 * playlist_free:
 * @playlist        	   : Playlist handle.
 *
 * Frees playlist handle.
 */
void playlist_free(playlist_t *playlist);

/**
 * playlist_clear:
 * @playlist        	   : Playlist handle.
 *
 * Clears all playlist entries in playlist.
 **/
void playlist_clear(playlist_t *playlist);

/**
 * playlist_size:
 * @playlist        	   : Playlist handle.
 *
 * Gets size of playlist.
 * Returns: size of playlist.
 **/
size_t playlist_size(playlist_t *playlist);

/**
 * playlist_capacity:
 * @playlist        	   : Playlist handle.
 *
 * Gets maximum capacity of playlist.
 * Returns: maximum capacity of playlist.
 **/
size_t playlist_capacity(playlist_t *playlist);

/**
 * playlist_get_index:
 * @playlist               : Playlist handle.
 * @idx                 : Index of playlist entry.
 *
 * Gets values of playlist index:
 **/
void playlist_get_index(playlist_t *playlist,
      size_t idx,
      const struct playlist_entry **entry);

/**
 * playlist_delete_index:
 * @playlist               : Playlist handle.
 * @idx                 : Index of playlist entry.
 *
 * Deletes the entry at index:
 **/
void playlist_delete_index(playlist_t *playlist,
      size_t idx);

/**
 * playlist_delete_by_path:
 * @playlist            : Playlist handle.
 * @search_path         : Content path.
 *
 * Deletes all entries with content path
 * matching 'search_path'
 **/
void playlist_delete_by_path(playlist_t *playlist,
      const char *search_path);

/**
 * playlist_resolve_path:
 * @mode      : PLAYLIST_LOAD or PLAYLIST_SAVE
 * @is_core   : Set true if path to be resolved is a core file
 * @path      : The path to be modified
 *
 * Resolves the path of an item, such as the content path or path to the core, to a format
 * appropriate for saving or loading depending on the @mode parameter
 *
 * Can be platform specific. File paths for saving can be abbreviated to avoid saving absolute
 * paths, as the base directory (home or application dir) may change after each subsequent
 * install (iOS)
 **/
void playlist_resolve_path(enum playlist_file_mode mode,
      bool is_core, char *path, size_t len);

/**
 * playlist_push:
 * @playlist        	   : Playlist handle.
 *
 * Push entry to top of playlist.
 **/
bool playlist_push(playlist_t *playlist,
      const struct playlist_entry *entry);

bool playlist_push_runtime(playlist_t *playlist,
      const struct playlist_entry *entry);

void playlist_update(playlist_t *playlist, size_t idx,
      const struct playlist_entry *update_entry);

/* Note: register_update determines whether the internal
 * 'playlist->modified' flag is set when updating runtime
 * values. Since these are normally set temporarily (for
 * display purposes), we do not always want this function
 * to trigger a re-write of the playlist file. */
void playlist_update_runtime(playlist_t *playlist, size_t idx,
      const struct playlist_entry *update_entry,
      bool register_update);

void playlist_get_index_by_path(playlist_t *playlist,
      const char *search_path,
      const struct playlist_entry **entry);

bool playlist_entry_exists(playlist_t *playlist,
      const char *path);

char *playlist_get_conf_path(playlist_t *playlist);

uint32_t playlist_get_size(playlist_t *playlist);

void playlist_write_file(playlist_t *playlist);

void playlist_write_runtime_file(playlist_t *playlist);

void playlist_qsort(playlist_t *playlist);

void playlist_free_cached(void);

playlist_t *playlist_get_cached(void);

/* If current on-disk playlist file referenced
 * by 'config->path' does not match requested
 * 'old format' or 'compression' state, file will
 * be updated automatically
 * > Since this function is called whenever a
 *   playlist is browsed via the menu, this is
 *   a simple method for ensuring that files
 *   are always kept synced with user settings */
bool playlist_init_cached(const playlist_config_t *config);

void command_playlist_push_write(
      playlist_t *playlist,
      const struct playlist_entry *entry);

void command_playlist_update_write(
      playlist_t *playlist,
      size_t idx,
      const struct playlist_entry *entry);

/* Returns true if specified playlist index matches
 * specified content/core paths */
bool playlist_index_is_valid(playlist_t *playlist, size_t idx,
      const char *path, const char *core_path);

/* Returns true if specified playlist entries have
 * identical content and core paths */
bool playlist_entries_are_equal(
      const struct playlist_entry *entry_a,
      const struct playlist_entry *entry_b,
      const playlist_config_t *config);

void playlist_get_crc32(playlist_t *playlist, size_t idx,
      const char **crc32);

/* If db_name is empty, 'returns' playlist file basename */
void playlist_get_db_name(playlist_t *playlist, size_t idx,
      const char **db_name);

char *playlist_get_default_core_path(playlist_t *playlist);
char *playlist_get_default_core_name(playlist_t *playlist);
enum playlist_label_display_mode playlist_get_label_display_mode(playlist_t *playlist);
enum playlist_thumbnail_mode playlist_get_thumbnail_mode(
      playlist_t *playlist, enum playlist_thumbnail_id thumbnail_id);
enum playlist_sort_mode playlist_get_sort_mode(playlist_t *playlist);

void playlist_set_default_core_path(playlist_t *playlist, const char *core_path);
void playlist_set_default_core_name(playlist_t *playlist, const char *core_name);
void playlist_set_label_display_mode(playlist_t *playlist, enum playlist_label_display_mode label_display_mode);
void playlist_set_thumbnail_mode(
      playlist_t *playlist, enum playlist_thumbnail_id thumbnail_id, enum playlist_thumbnail_mode thumbnail_mode);
void playlist_set_sort_mode(playlist_t *playlist, enum playlist_sort_mode sort_mode);

/* Returns true if specified entry has a valid
 * core association (i.e. a non-empty string
 * other than DETECT) */
bool playlist_entry_has_core(const struct playlist_entry *entry);

/* Fetches core info object corresponding to the
 * currently associated core of the specified
 * playlist entry.
 * Returns NULL if entry does not have a valid
 * core association */
core_info_t *playlist_entry_get_core_info(const struct playlist_entry* entry);

/* Fetches core info object corresponding to the
 * currently associated default core of the
 * specified playlist.
 * Returns NULL if playlist does not have a valid
 * default core association */
core_info_t *playlist_get_default_core_info(playlist_t* playlist);

void playlist_set_cached_external(playlist_t* pl);

KS_END_DECLS

#endif
