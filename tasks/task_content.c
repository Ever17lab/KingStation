/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2016-2019 - Brad Parker
 *  Copyright (C) 2016-2019 - Andrés Suárez
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

/* TODO/FIXME - turn this into actual task */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <file/file_path.h>
#include <queues/task_queue.h>
#include <string/stdstring.h>

#ifdef _WIN32
#ifdef _XBOX
#include <xtl.h>
#define setmode _setmode
#define INVALID_FILE_ATTRIBUTES -1
#else
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#endif
#endif

#ifdef __WINRT__
#include <uwp/uwp_func.h>
#endif

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <boolean.h>

#include <encodings/crc32.h>
#include <compat/strl.h>
#include <compat/posix_string.h>
#include <file/file_path.h>
#include <file/archive_file.h>
#include <string/stdstring.h>

#include <vfs/vfs_implementation.h>
#ifdef HAVE_CDROM
#include <vfs/vfs_implementation_cdrom.h>
#endif

#include <ks_miscellaneous.h>
#include <streams/file_stream.h>
#include <ks_assert.h>

#include <lists/string_list.h>
#include <string/stdstring.h>

#ifdef HAVE_MENU
#include "../menu/menu_driver.h"
#endif

#ifdef HAVE_GFX_WIDGETS
#include "../gfx/gfx_widgets.h"
#endif

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
#include "../menu/menu_shader.h"
#endif

#ifdef HAVE_CHEEVOS
#include "../cheevos/cheevos.h"
#endif

#include "task_content.h"
#include "tasks_internal.h"

#include "../command.h"
#include "../core_info.h"
#include "../content.h"
#include "../configuration.h"
#include "../defaults.h"
#include "../frontend/frontend.h"
#include "../playlist.h"
#include "../paths.h"
#include "../KingStation.h"
#include "../verbosity.h"

#include "../msg_hash.h"
#include "../content.h"
#include "../dynamic.h"
#include "../KingStation.h"
#include "../file_path_special.h"
#include "../core.h"
#include "../paths.h"
#include "../verbosity.h"

#ifdef HAVE_DISCORD
#include "../network/discord.h"

/* TODO/FIXME - get rid of this public global */
extern bool discord_is_inited;
#endif

#define MAX_ARGS 32

typedef struct content_stream content_stream_t;
typedef struct content_information_ctx content_information_ctx_t;

struct content_stream
{
   const uint8_t *b;
   size_t c;
   uint32_t a;
   uint32_t crc;
};

struct content_information_ctx
{
   char *name_ips;
   char *name_bps;
   char *name_ups;

   char *valid_extensions;
   char *directory_cache;
   char *directory_system;

   struct string_list *temporary_content;

   struct
   {
      struct ks_subsystem_info *data;
      unsigned size;
   } subsystem;

   bool block_extract;
   bool need_fullpath;
   bool set_supports_no_game_enable;
#ifdef HAVE_PATCH
   bool is_ips_pref;
   bool is_bps_pref;
   bool is_ups_pref;
   bool patch_is_blocked;
#endif
   bool bios_is_missing;
   bool check_firmware_before_loading;
};

#ifdef HAVE_CDROM
enum cdrom_dump_state
{
   DUMP_STATE_TOC_PENDING = 0,
   DUMP_STATE_WRITE_CUE,
   DUMP_STATE_NEXT_TRACK,
   DUMP_STATE_READ_TRACK
};

typedef struct
{
   int64_t cur_track_bytes;
   int64_t track_written_bytes;
   int64_t disc_total_bytes;
   int64_t disc_read_bytes;

   RFILE *file;
   RFILE *output_file;
   libks_vfs_implementation_file *stream;
   const cdrom_toc_t *toc;

   enum cdrom_dump_state state;
   unsigned char cur_track;
   char drive_letter[2];
   char cdrom_path[64];
   char title[512];
   bool next;
} task_cdrom_dump_state_t;

static void task_cdrom_dump_handler(ks_task_t *task)
{
   task_cdrom_dump_state_t *state    = (task_cdrom_dump_state_t*)task->state;
   settings_t              *settings = config_get_ptr();
   const char *directory_core_assets = settings 
      ? settings->paths.directory_core_assets : NULL;

   if (task_get_progress(task) == 100)
   {
      if (state->file)
         filestream_close(state->file);
      if (state->output_file)
         filestream_close(state->output_file);
      state->file        = NULL;
      state->output_file = NULL;

      task_set_finished(task, true);

      KINGSN_LOG("[CDROM]: Dump finished.\n");

      return;
   }

   switch (state->state)
   {
      case DUMP_STATE_TOC_PENDING:
         {
            /* open cuesheet file from drive */
            char cue_path[PATH_MAX_LENGTH] = {0};

            cdrom_device_fillpath(cue_path, sizeof(cue_path), state->drive_letter[0], 0, true);

            state->file = filestream_open(cue_path, KS_VFS_FILE_ACCESS_READ, 0);

            if (!state->file)
            {
               KINGSN_ERR("[CDROM]: Error opening file for reading: %s\n", cue_path);
               task_set_progress(task, 100);
               task_free_title(task);
               task_set_title(task, strdup(msg_hash_to_str(MENU_ENUM_LABEL_VALUE_QT_FILE_READ_OPEN_FAILED)));
               return;
            }

            state->state = DUMP_STATE_WRITE_CUE;

            break;
         }
      case DUMP_STATE_WRITE_CUE:
         {
            char output_file[PATH_MAX_LENGTH];
            char cue_filename[PATH_MAX_LENGTH];
            /* write cuesheet to a file */
            int64_t cue_size     = filestream_get_size(state->file);
            char *cue_data       = (char*)calloc(1, cue_size);

            output_file[0]       = cue_filename[0] = '\0';

            filestream_read(state->file, cue_data, cue_size);

            state->stream        = filestream_get_vfs_handle(state->file);
            state->toc           = ks_vfs_file_get_cdrom_toc();

            if (cdrom_has_atip(state->stream))
               KINGSN_LOG("[CDROM]: This disc is not genuine.\n");

            filestream_close(state->file);

            snprintf(cue_filename, sizeof(cue_filename), "%s.cue", state->title);

            fill_pathname_join(output_file,
                  directory_core_assets, cue_filename, sizeof(output_file));

            {
               RFILE         *file = filestream_open(output_file, KS_VFS_FILE_ACCESS_WRITE, 0);
               unsigned char point = 0;

               if (!file)
               {
                  KINGSN_ERR("[CDROM]: Error opening file for writing: %s\n", output_file);
                  task_set_progress(task, 100);
                  task_free_title(task);
                  task_set_title(task, strdup(msg_hash_to_str(MENU_ENUM_LABEL_VALUE_QT_FILE_WRITE_OPEN_FAILED)));
                  return;
               }

               for (point = 1; point <= state->toc->num_tracks; point++)
               {
                  const char *track_type = "MODE1/2352";
                  char track_filename[PATH_MAX_LENGTH];

                  state->disc_total_bytes += state->toc->track[point - 1].track_bytes;

                  track_filename[0] = '\0';

                  if (state->toc->track[point - 1].audio)
                     track_type = "AUDIO";
                  else if (state->toc->track[point - 1].mode == 1)
                     track_type = "MODE1/2352";
                  else if (state->toc->track[point - 1].mode == 2)
                     track_type = "MODE2/2352";

                  snprintf(track_filename, sizeof(track_filename), "%s (Track %02d).bin", state->title, point);

                  filestream_printf(file, "FILE \"%s\" BINARY\n", track_filename);
                  filestream_printf(file, "  TRACK %02d %s\n", point, track_type);

                  {
                     unsigned pregap_lba_len = state->toc->track[point - 1].lba - state->toc->track[point - 1].lba_start;

                     if (state->toc->track[point - 1].audio && pregap_lba_len > 0)
                     {
                        unsigned char min = 0;
                        unsigned char sec = 0;
                        unsigned char frame = 0;

                        cdrom_lba_to_msf(pregap_lba_len, &min, &sec, &frame);

                        filestream_printf(file, "    INDEX 00 00:00:00\n");
                        filestream_printf(file, "    INDEX 01 %02u:%02u:%02u\n", (unsigned)min, (unsigned)sec, (unsigned)frame);
                     }
                     else
                        filestream_printf(file, "    INDEX 01 00:00:00\n");
                  }
               }

               filestream_close(file);
            }

            state->file  = NULL;
            state->state = DUMP_STATE_NEXT_TRACK;

            free(cue_data);
         }
         break;
      case DUMP_STATE_NEXT_TRACK:
         {
            /* no file is open as we either just started or just finished a track, need to start dumping the next track */
            state->cur_track++;

            /* no more tracks to dump, we're done */
            if (state->toc && state->cur_track > state->toc->num_tracks)
            {
               task_set_progress(task, 100);
               return;
            }

            KINGSN_LOG("[CDROM]: Dumping track %d...\n", state->cur_track);

            memset(state->cdrom_path, 0, sizeof(state->cdrom_path));

            cdrom_device_fillpath(state->cdrom_path, sizeof(state->cdrom_path), state->drive_letter[0], state->cur_track, false);

            state->track_written_bytes = 0;
            state->file = filestream_open(state->cdrom_path, KS_VFS_FILE_ACCESS_READ, 0);

            /* open a new file for writing for this next track */
            if (state->file)
            {
               char output_path[PATH_MAX_LENGTH];
               char track_filename[PATH_MAX_LENGTH];

               output_path[0] = track_filename[0] = '\0';

               snprintf(track_filename, sizeof(track_filename), "%s (Track %02d).bin", state->title, state->cur_track);

               state->cur_track_bytes = filestream_get_size(state->file);

               fill_pathname_join(output_path,
                     directory_core_assets, track_filename, sizeof(output_path));

               state->output_file = filestream_open(output_path, KS_VFS_FILE_ACCESS_WRITE, 0);

               if (!state->output_file)
               {
                  KINGSN_ERR("[CDROM]: Error opening file for writing: %s\n", output_path);
                  task_set_progress(task, 100);
                  task_free_title(task);
                  task_set_title(task, strdup(msg_hash_to_str(MENU_ENUM_LABEL_VALUE_QT_FILE_WRITE_OPEN_FAILED)));
                  return;
               }
            }
            else
            {
               KINGSN_ERR("[CDROM]: Error opening file for writing: %s\n", state->cdrom_path);
               task_set_progress(task, 100);
               task_free_title(task);
               task_set_title(task, strdup(msg_hash_to_str(MENU_ENUM_LABEL_VALUE_QT_FILE_WRITE_OPEN_FAILED)));
               return;
            }

            state->state = DUMP_STATE_READ_TRACK;
         }
         break;
      case DUMP_STATE_READ_TRACK:
         /* read data from track and write it to a file in chunks */
         if (state->cur_track_bytes > state->track_written_bytes)
         {
            char data[2352 * 2] = {0};
            int64_t read_bytes  = filestream_read(state->file, data, sizeof(data));
            int progress        = 0;

            if (read_bytes <= 0)
            {
               task_set_progress(task, 100);
               task_free_title(task);
               task_set_title(task, strdup(msg_hash_to_str(MSG_DISC_DUMP_FAILED_TO_READ_FROM_DRIVE)));
               return;
            }

            state->track_written_bytes += read_bytes;
            state->disc_read_bytes += read_bytes;
            progress = (state->disc_read_bytes / (double)state->disc_total_bytes) * 100.0;

#ifdef CDROM_DEBUG
            KINGSN_LOG("[CDROM]: Read %" PRId64 " bytes, totalling %" PRId64 " of %" PRId64 " bytes. Progress: %d%%\n", read_bytes, state->track_written_bytes, state->cur_track_bytes, progress);
#endif

            if (filestream_write(state->output_file, data, read_bytes) <= 0)
            {
               task_set_progress(task, 100);
               task_free_title(task);
               task_set_title(task, strdup(msg_hash_to_str(MSG_DISC_DUMP_FAILED_TO_WRITE_TO_DISK)));
               return;
            }

            task_set_progress(task, progress);
         }
         else if (state->cur_track_bytes == state->track_written_bytes)
         {
            /* TODO: FIXME: this stops after only the first track */
            if (state->file)
            {
               filestream_close(state->file);
               state->file = NULL;
            }
            if (state->output_file)
            {
               filestream_close(state->output_file);
               state->file = NULL;
            }

            state->state = DUMP_STATE_NEXT_TRACK;
         }
         break;
   }
}

static void task_cdrom_dump_callback(ks_task_t *task,
      void *task_data,
      void *user_data, const char *error)
{
   task_cdrom_dump_state_t *state = (task_cdrom_dump_state_t*)task->state;

   if (state)
      free(state);
}

void task_push_cdrom_dump(const char *drive)
{
   ks_task_t *task             = task_init();
   task_cdrom_dump_state_t *state = (task_cdrom_dump_state_t*)calloc(1, sizeof(*state));

   state->drive_letter[0]         = drive[0];
   state->next                    = true;
   state->cur_track               = 0;
   state->state                   = DUMP_STATE_TOC_PENDING;

   fill_str_dated_filename(state->title, "cdrom", NULL, sizeof(state->title));

   task->state                    = state;
   task->handler                  = task_cdrom_dump_handler;
   task->callback                 = task_cdrom_dump_callback;
   task->title                    = strdup(msg_hash_to_str(MSG_DUMPING_DISC));

   KINGSN_LOG("[CDROM]: Starting disc dump...\n");

   task_queue_push(task);
}
#endif

static int64_t content_file_read(const char *path, void **buf, int64_t *length)
{
#ifdef HAVE_COMPRESSION
   if (     path_contains_compressed_file(path)
         && file_archive_compressed_read(path, buf, NULL, length))
      return 1;
#endif
   return filestream_read_file(path, buf, length);
}

/**
 * content_load_init_wrap:
 * @args                 : Input arguments.
 * @argc                 : Count of arguments.
 * @argv                 : Arguments.
 *
 * Generates an @argc and @argv pair based on @args
 * of type kingsn_main_wrap.
 **/
static void content_load_init_wrap(
      const struct kingsn_main_wrap *args,
      int *argc, char **argv,
      bool print_args)
{
   *argc = 0;
   argv[(*argc)++] = strdup("KingStation");

   if (args->content_path)
   {
      KINGSN_LOG("[CORE]: Using content: %s.\n", args->content_path);
      argv[(*argc)++] = strdup(args->content_path);
   }
#ifdef HAVE_MENU
   else
   {
      KINGSN_LOG("[CORE]: %s\n",
            msg_hash_to_str(MSG_NO_CONTENT_STARTING_DUMMY_CORE));
      argv[(*argc)++] = strdup("--menu");
   }
#endif

   if (args->sram_path)
   {
      argv[(*argc)++] = strdup("-s");
      argv[(*argc)++] = strdup(args->sram_path);
   }

   if (args->state_path)
   {
      argv[(*argc)++] = strdup("-S");
      argv[(*argc)++] = strdup(args->state_path);
   }

   if (args->config_path)
   {
      argv[(*argc)++] = strdup("-c");
      argv[(*argc)++] = strdup(args->config_path);
   }

#ifdef HAVE_DYNAMIC
   if (args->libks_path)
   {
      argv[(*argc)++] = strdup("-L");
      argv[(*argc)++] = strdup(args->libks_path);
   }
#endif

   if (args->verbose)
      argv[(*argc)++] = strdup("-v");

   if (print_args)
   {
      int i;
      for (i = 0; i < *argc; i++)
         KINGSN_LOG("[CORE]: Arg #%d: %s\n", i, argv[i]);
   }
}

/**
 * content_load:
 *
 * Loads content file and starts up KingStation.
 * If no content file can be loaded, will start up KingStation
 * as-is.
 *
 * Returns: false (0) if KingStation_main_init failed,
 * otherwise true (1).
 **/
static bool content_load(content_ctx_info_t *info,
      content_state_t *p_content)
{
   unsigned i                        = 0;
   bool success                      = false;
   int kingsn_argc                    = 0;
   char *kingsn_argv[MAX_ARGS]        = {NULL};
   char *argv_copy [MAX_ARGS]        = {NULL};
   char **kingsn_argv_ptr             = (char**)info->argv;
   int *kingsn_argc_ptr               = (int*)&info->argc;
   struct kingsn_main_wrap *wrap_args = NULL;

   if (!(wrap_args = (struct kingsn_main_wrap*)
      malloc(sizeof(*wrap_args))))
      return false;

   ks_assert(wrap_args);

   wrap_args->argv           = NULL;
   wrap_args->content_path   = NULL;
   wrap_args->sram_path      = NULL;
   wrap_args->state_path     = NULL;
   wrap_args->config_path    = NULL;
   wrap_args->libks_path  = NULL;
   wrap_args->verbose        = false;
   wrap_args->no_content     = false;
   wrap_args->touched        = false;
   wrap_args->argc           = 0;

   if (info->environ_get)
      info->environ_get(kingsn_argc_ptr,
            kingsn_argv_ptr, info->args, wrap_args);

   if (wrap_args->touched)
   {
      content_load_init_wrap(wrap_args, &kingsn_argc, kingsn_argv,
            false);
      memcpy(argv_copy, kingsn_argv, sizeof(kingsn_argv));
      kingsn_argv_ptr = (char**)kingsn_argv;
      kingsn_argc_ptr = (int*)&kingsn_argc;
   }

   kingsn_ctl(KINGSN_CTL_MAIN_DEINIT, NULL);

   wrap_args->argc = *kingsn_argc_ptr;
   wrap_args->argv = kingsn_argv_ptr;

   success         = KingStation_main_init(wrap_args->argc, wrap_args->argv);

   for (i = 0; i < ARRAY_SIZE(argv_copy); i++)
      free(argv_copy[i]);
   free(wrap_args);

   if (!success)
      return false;

   if (p_content->pending_subsystem_init)
   {
      command_event(CMD_EVENT_CORE_INIT, NULL);
      content_clear_subsystem();
   }

#ifdef HAVE_GFX_WIDGETS
   /* If KingStation_main_init() returned true, we
    * can safely trigger a load content animation */
   if (gfx_widgets_ready())
   {
      /* Note: Have to read settings value here
       * (It will be invalid if we try to read
       *  it earlier...) */
#ifdef HAVE_CONFIGFILE
      settings_t *settings              = config_get_ptr();
      bool show_load_content_animation  = settings && settings->bools.menu_show_load_content_animation;
#else
      bool show_load_content_animation  = false;
#endif

      if (show_load_content_animation)
         gfx_widget_start_load_content_animation();
   }
#endif

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
   menu_shader_manager_init();
#endif

   command_event(CMD_EVENT_HISTORY_INIT, NULL);
   kingsn_favorites_init();
   command_event(CMD_EVENT_RESUME, NULL);
   command_event(CMD_EVENT_VIDEO_SET_ASPECT_RATIO, NULL);

   frontend_driver_process_args(kingsn_argc_ptr, kingsn_argv_ptr);
   frontend_driver_content_loaded();

   return true;
}

/**
 * load_content_into_memory:
 * @path         : buffer of the content file.
 * @buf          : size   of the content file.
 * @length       : size of the content file that has been read from.
 *
 * Read the content file. If read into memory, also performs soft patching
 * (see patch_content function) in case soft patching has not been
 * blocked by the enduser.
 *
 * Returns: true if successful, false on error.
 **/
static bool load_content_into_memory(
      content_information_ctx_t *content_ctx,
      content_state_t *p_content,
      unsigned i, const char *path, void **buf,
      int64_t *length)
{
   uint8_t *ret_buf           = NULL;

   KINGSN_LOG("[CONTENT LOAD]: %s: %s.\n",
         msg_hash_to_str(MSG_LOADING_CONTENT_FILE), path);

   if (!content_file_read(path, (void**) &ret_buf, length))
      return false;

   if (*length < 0)
      return false;

   if (i == 0)
   {
      enum kingsn_content_type type = path_is_media_type(path);

      /* If we have a media type, ignore CRC32 calculation. */
      if (type == KINGSN_CONTENT_NONE)
      {
#ifdef HAVE_PATCH
         bool has_patch = false;

         /* First content file is significant, attempt to do patching,
          * CRC checking, etc. */

         /* Attempt to apply a patch. */
         if (!content_ctx->patch_is_blocked)
            has_patch = patch_content(
                  content_ctx->is_ips_pref,
                  content_ctx->is_bps_pref,
                  content_ctx->is_ups_pref,
                  content_ctx->name_ips,
                  content_ctx->name_bps,
                  content_ctx->name_ups,
                  (uint8_t**)&ret_buf,
                  (void*)length);

         if (has_patch)
         {
            p_content->rom_crc = encoding_crc32(0, ret_buf, (size_t)*length);
            KINGSN_LOG("[CONTENT LOAD]: CRC32: 0x%x .\n", (unsigned)p_content->rom_crc);
         }
         else
#endif
         {
            strlcpy(p_content->pending_rom_crc_path,
                  path, sizeof(p_content->pending_rom_crc_path));
            p_content->pending_rom_crc      = true;
         }
      }
      else
         p_content->rom_crc = 0;
   }

   *buf = ret_buf;

   return true;
}

#ifdef HAVE_COMPRESSION
static bool load_content_from_compressed_archive(
      content_information_ctx_t *content_ctx,
      struct ks_game_info *info,
      unsigned i,
      struct string_list* additional_path_allocs,
      bool need_fullpath,
      const char *path,
      enum msg_hash_enums *error_enum,
      char **error_string)
{
   union string_list_elem_attr attr;
   int64_t new_path_len              = 0;
   char new_basedir[PATH_MAX_LENGTH];
   char new_path[PATH_MAX_LENGTH];

   new_path[0]                       = '\0';
   new_basedir[0]                    = '\0';
   attr.i                            = 0;

   KINGSN_LOG("[CONTENT LOAD]: Compressed file in case of need_fullpath."
         " Now extracting to temporary directory.\n");

   if (!string_is_empty(content_ctx->directory_cache))
      strlcpy(new_basedir, content_ctx->directory_cache,
            sizeof(new_basedir));

   if (!path_is_directory(new_basedir))
   {
      KINGSN_WARN("[CONTENT LOAD]: Tried extracting to cache directory, but "
            "cache directory was not set or found. "
            "Setting cache directory to directory "
            "derived by basename...\n");
      fill_pathname_basedir(new_basedir, path, sizeof(new_basedir));
   }

   fill_pathname_join(new_path, new_basedir,
         path_basename(path), sizeof(new_path));

   if (!file_archive_compressed_read(path,
         NULL, new_path, &new_path_len) || new_path_len < 0)
   {
      char str[1024];
      str[0] = '\0';
      snprintf(str, sizeof(str),
            "%s \"%s\".\n",
            msg_hash_to_str(MSG_COULD_NOT_READ_CONTENT_FILE),
            path);
      *error_string = strdup(str);
      return false;
   }

   string_list_append(additional_path_allocs, new_path, attr);
   info[i].path =
      additional_path_allocs->elems[additional_path_allocs->size - 1].data;

   return string_list_append(content_ctx->temporary_content,
            new_path, attr);
}

/* Try to extract all content we're going to load if appropriate. */

static bool content_file_init_extract(
      struct string_list *content,
      content_information_ctx_t *content_ctx,
      const struct ks_subsystem_info *special,
      enum msg_hash_enums *error_enum,
      char **error_string,
      union string_list_elem_attr *attr
      )
{
   unsigned i;

   for (i = 0; i < content->size; i++)
   {
      bool block_extract                 = content->elems[i].attr.i & 1;
      const char *path                   = content->elems[i].data;
      bool contains_compressed           = path_contains_compressed_file(path);

      /* Block extract check. */
      if (block_extract)
         continue;

      /* just use the first file in the archive */
      if (!contains_compressed && !path_is_compressed_file(path))
         continue;

      {
         char temp_content[PATH_MAX_LENGTH];
         char new_path    [PATH_MAX_LENGTH];
         const char *valid_ext    = special ?
            special->roms[i].valid_extensions :
            content_ctx->valid_extensions;

         temp_content[0]          = new_path[0] = '\0';

         if (!string_is_empty(path))
            strlcpy(temp_content, path, sizeof(temp_content));

         if (!valid_ext || !file_archive_extract_file(
                  temp_content,
                  sizeof(temp_content),
                  valid_ext,
                  !string_is_empty(content_ctx->directory_cache) ?
                  content_ctx->directory_cache : NULL,
                  new_path,
                  sizeof(new_path) 
                  ))
         {
            char str[1024];
            str[0] = '\0';

            snprintf(str, sizeof(str),
                  "%s: %s.\n",
                  msg_hash_to_str(
                     MSG_FAILED_TO_EXTRACT_CONTENT_FROM_COMPRESSED_FILE),
                  temp_content);
            *error_string = strdup(str);
            return false;
         }

         string_list_set(content, i, new_path);

         if (!string_list_append(
                  content_ctx->temporary_content,
                  new_path, *attr))
            return false;
      }
   }

   return true;
}
#endif

/**
 * content_file_load:
 * @special          : subsystem of content to be loaded. Can be NULL.
 * content           :
 *
 * Load content file (for libks core).
 *
 * Returns : true if successful, otherwise false.
 **/
static bool content_file_load(
      struct ks_game_info *info,
      content_state_t *p_content,
      const struct string_list *content,
      content_information_ctx_t *content_ctx,
      enum msg_hash_enums *error_enum,
      char **error_string,
      const struct ks_subsystem_info *special,
      struct string_list *additional_path_allocs
      )
{
   unsigned i;
   ks_ctx_load_content_info_t load_info;
   bool used_vfs_fallback_copy = false;
#ifdef __WINRT__
   kingsn_system_info_t *system = runloop_get_system_info();
#endif

   for (i = 0; i < content->size; i++)
   {
      int         attr     = content->elems[i].attr.i;
      const char *path     = content->elems[i].data;
      bool need_fullpath   = attr & 2;
      bool require_content = attr & 4;
      bool path_empty      = string_is_empty(path);

      if (require_content && path_empty)
      {
         *error_enum = MSG_ERROR_LIBKS_CORE_REQUIRES_CONTENT;
         return false;
      }

      info[i].path = NULL;

      if (!path_empty)
         info[i].path = path;

      if (!need_fullpath && !path_empty)
      {
         /* Load the content into memory. */

         int64_t len = 0;

         if (!load_content_into_memory(
                  content_ctx, p_content,
                  i, path, (void**)&info[i].data, &len))
         {
            char msg[1024];
            msg[0]          = '\0';

            snprintf(msg, sizeof(msg),
                  "%s \"%s\".\n",
                  msg_hash_to_str(MSG_COULD_NOT_READ_CONTENT_FILE),
                  path);
            *error_string = strdup(msg);
            return false;
         }

         info[i].size = len;
      }
      else
      {
#ifdef HAVE_COMPRESSION
         if (     !content_ctx->block_extract
               && need_fullpath
               && path_contains_compressed_file(path)
               && !load_content_from_compressed_archive(
                  content_ctx,
                  &info[i], i,
                  additional_path_allocs, need_fullpath, path,
                  error_enum,
                  error_string))
            return false;
#endif

#ifdef __WINRT__
         /* TODO: When support for the 'actual' VFS is added, there will need to be some more logic here */
         if (!system->supports_vfs && !is_path_accessible_using_standard_io(path))
         {
            /* Fallback to a file copy into an accessible directory */
            char* buf;
            int64_t len;
            union string_list_elem_attr attr;
            char new_basedir[PATH_MAX_LENGTH];
            char new_path[PATH_MAX_LENGTH];

            new_path[0]             = '\0';
            new_basedir[0]          = '\0';
            attr.i                  = 0;

            KINGSN_LOG("[CONTENT LOAD]: Core does not support VFS - copying to cache directory\n");

            if (!string_is_empty(content_ctx->directory_cache))
               strlcpy(new_basedir, content_ctx->directory_cache, sizeof(new_basedir));
            if (   string_is_empty(new_basedir)   || 
                  !path_is_directory(new_basedir) || 
                  !is_path_accessible_using_standard_io(new_basedir))
            {
               KINGSN_WARN("[CONTENT LOAD]: Tried copying to cache directory"
                     ", but "
                     "cache directory was not set or found. "
                     "Setting cache directory to root of "
                     "writable app directory...\n");
               strlcpy(new_basedir, uwp_dir_data, sizeof(new_basedir));
            }

            fill_pathname_join(new_path, new_basedir,
                  path_basename(path), sizeof(new_path));

            /* TODO: This may fail on very large files...
             * but copying large files is not a good idea anyway */
            if (!filestream_read_file(path, &buf, &len))
            {
               char msg[1024];
               msg[0]          = '\0';

               snprintf(msg,
                     sizeof(msg),
                     "%s \"%s\". (during copy read)\n",
                     msg_hash_to_str(MSG_COULD_NOT_READ_CONTENT_FILE),
                     path);
               *error_string = strdup(msg);
               return false;
            }

            if (!filestream_write_file(new_path, buf, len))
            {
               char msg[1024];
               msg[0]          = '\0';

               free(buf);
               snprintf(msg,
                     sizeof(msg),
                     "%s \"%s\". (during copy write)\n",
                     msg_hash_to_str(MSG_COULD_NOT_READ_CONTENT_FILE),
                     path);
               *error_string = strdup(msg);
               return false;
            }

            free(buf);

            string_list_append(additional_path_allocs, new_path, attr);
            info[i].path =
               additional_path_allocs->elems[additional_path_allocs->size - 1].data;

            string_list_append(content_ctx->temporary_content,
                  new_path, attr);

            used_vfs_fallback_copy = true;
         }
#endif

         KINGSN_LOG("[CONTENT LOAD]: %s\n", msg_hash_to_str(
                  MSG_CONTENT_LOADING_SKIPPED_IMPLEMENTATION_WILL_DO_IT));
         strlcpy(p_content->pending_rom_crc_path,
               path, sizeof(p_content->pending_rom_crc_path));
         p_content->pending_rom_crc      = true;
      }
   }

   load_info.content = content;
   load_info.special = special;
   load_info.info    = info;

   if (!core_load_game(&load_info))
   {
      /* This is probably going to fail on multifile ROMs etc.
       * so give a visible explanation of what is likely wrong */
      if (used_vfs_fallback_copy)
         *error_enum   = MSG_ERROR_LIBKS_CORE_REQUIRES_VFS;
      else
         *error_enum   = MSG_FAILED_TO_LOAD_CONTENT;

      return false;
   }

#ifdef HAVE_CHEEVOS
   if (!special)
   {
      const char *content_path     = content->elems[0].data;
      enum kingsn_content_type type = path_is_media_type(content_path);

      if (type == KINGSN_CONTENT_NONE && !string_is_empty(content_path))
         rcheevos_load(info);
      else
         rcheevos_pause_hardcore();
   }
   else
      rcheevos_pause_hardcore();
#endif

   return true;
}

static const struct
ks_subsystem_info *content_file_init_subsystem(
      const struct ks_subsystem_info *subsystem_data,
      size_t subsystem_current_count,
      enum msg_hash_enums *error_enum,
      char **error_string,
      bool *ret)
{
   struct string_list *subsystem              = path_get_subsystem_list();
   const struct ks_subsystem_info *special = libks_find_subsystem_info(
            subsystem_data, (unsigned)subsystem_current_count,
            path_get(KINGSN_PATH_SUBSYSTEM));

   if (!special)
   {
      char msg[1024];
      msg[0] = '\0';
      snprintf(msg, sizeof(msg),
            "Failed to find subsystem \"%s\" in libks implementation.\n",
            path_get(KINGSN_PATH_SUBSYSTEM));
      *error_string = strdup(msg);
      goto error;
   }

   if (special->num_roms)
   {
      if (!subsystem)
      {
         *error_enum   = MSG_ERROR_LIBKS_CORE_REQUIRES_SPECIAL_CONTENT;
         goto error;
      }

      if (special->num_roms != subsystem->size)
      {
         char msg[1024];
         msg[0] = '\0';
         snprintf(msg,
               sizeof(msg),
               "Libks core requires %u content files for "
               "subsystem \"%s\", but %u content files were provided.\n",
               special->num_roms, special->desc,
               (unsigned)subsystem->size);
         *error_string = strdup(msg);
         goto error;
      }
   }
   else if (subsystem && subsystem->size)
   {
      char msg[1024];
      msg[0] = '\0';
      snprintf(msg,
            sizeof(msg),
            "Libks core takes no content for subsystem \"%s\", "
            "but %u content files were provided.\n",
            special->desc,
            (unsigned)subsystem->size);
      *error_string = strdup(msg);
      goto error;
   }

   *ret = true;
   return special;

error:
   *ret = false;
   return NULL;
}

static void content_file_init_set_attribs(
      struct string_list *content,
      const struct ks_subsystem_info *special,
      content_information_ctx_t *content_ctx,
      char **error_string,
      union string_list_elem_attr *attr)
{
   struct string_list *subsystem    = path_get_subsystem_list();

   attr->i                          = 0;

   if (!path_is_empty(KINGSN_PATH_SUBSYSTEM) && special)
   {
      unsigned i;

      for (i = 0; i < subsystem->size; i++)
      {
         attr->i            = special->roms[i].block_extract;
         attr->i           |= special->roms[i].need_fullpath << 1;
         attr->i           |= special->roms[i].required      << 2;

         string_list_append(content, subsystem->elems[i].data, *attr);
      }
   }
   else
   {
      bool contentless           = false;
      bool is_inited             = false;
      bool content_path_is_empty = path_is_empty(KINGSN_PATH_CONTENT);

      content_get_status(&contentless, &is_inited);

      attr->i               = content_ctx->block_extract;
      attr->i              |= content_ctx->need_fullpath << 1;
      attr->i              |= (!contentless)  << 2;

      if (content_path_is_empty
            && contentless
            && content_ctx->set_supports_no_game_enable)
         string_list_append(content, "", *attr);
      else if (!content_path_is_empty)
         string_list_append(content, path_get(KINGSN_PATH_CONTENT), *attr);
   }
}

/**
 * content_init_file:
 *
 * Initializes and loads a content file for the currently
 * selected libks core.
 *
 * Returns : true if successful, otherwise false.
 **/
static bool content_file_init(
      content_information_ctx_t *content_ctx,
      content_state_t *p_content,
      struct string_list *content,
      enum msg_hash_enums *error_enum,
      char **error_string)
{
   union string_list_elem_attr attr;
   struct ks_game_info               *info = NULL;
   bool subsystem_path_is_empty               = path_is_empty(KINGSN_PATH_SUBSYSTEM);
   bool ret                                   = subsystem_path_is_empty;
   const struct ks_subsystem_info *special =
     subsystem_path_is_empty 
     ? NULL : content_file_init_subsystem(content_ctx->subsystem.data,
           content_ctx->subsystem.size, error_enum, error_string, &ret);

   if (!ret)
      return false;

   content_file_init_set_attribs(content, special, content_ctx, error_string, &attr);
#ifdef HAVE_COMPRESSION
   content_file_init_extract(content, content_ctx, special, error_enum, error_string, &attr);
#endif

   if (content->size > 0)
      info                   = (struct ks_game_info*)
         calloc(content->size, sizeof(*info));

   if (info)
   {
      unsigned i;
      struct string_list additional_path_allocs;
      
      if (string_list_initialize(&additional_path_allocs))
      {
         ret = content_file_load(info, p_content,
               content, content_ctx, error_enum,
               error_string,
               special, &additional_path_allocs);
         string_list_deinitialize(&additional_path_allocs);
      }

      for (i = 0; i < content->size; i++)
         free((void*)info[i].data);

      free(info);
   }
   else if (!special)
   {
      *error_enum   = MSG_ERROR_LIBKS_CORE_REQUIRES_CONTENT;
      return false;
   }

   return ret;
}

void menu_content_environment_get(int *argc, char *argv[],
      void *args, void *params_data);

/**
 * task_push_to_history_list:
 *
 * Will push the content entry to the history playlist.
 **/
static void task_push_to_history_list(
      content_state_t *p_content,
      bool launched_from_menu,
      bool launched_from_cli,
      bool launched_from_companion_ui)
{
   bool            contentless = false;
   bool            is_inited   = false;

   content_get_status(&contentless, &is_inited);

   /* Push entry to top of history playlist */
   if (is_inited || contentless)
   {
      char tmp[PATH_MAX_LENGTH];
      const char *path_content       = path_get(KINGSN_PATH_CONTENT);
      struct ks_system_info *info = runloop_get_libks_system_info();

      tmp[0] = '\0';

      if (!string_is_empty(path_content))
         strlcpy(tmp, path_content, sizeof(tmp));

      /* Path can be relative here.
       * Ensure we're pushing absolute path. */
      if (!launched_from_menu && !string_is_empty(tmp))
         path_resolve_realpath(tmp, sizeof(tmp), true);

#ifdef HAVE_MENU
      /* Push quick menu onto menu stack */
      if (launched_from_cli)
         menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

      if (info && !string_is_empty(tmp))
      {
         const char *core_path      = NULL;
         const char *core_name      = NULL;
         const char *label          = NULL;
         const char *crc32          = NULL;
         const char *db_name        = NULL;
         playlist_t *playlist_hist  = g_defaults.content_history;
         settings_t *settings       = config_get_ptr();
         global_t *global           = global_get_ptr();

         switch (path_is_media_type(tmp))
         {
            case KINGSN_CONTENT_MOVIE:
#ifdef HAVE_FFMPEG
               playlist_hist        = g_defaults.video_history;
               core_name            = "movieplayer";
               core_path            = "builtin";
#endif
               break;
            case KINGSN_CONTENT_MUSIC:
               playlist_hist        = g_defaults.music_history;
               core_name            = "musicplayer";
               core_path            = "builtin";
               break;
            case KINGSN_CONTENT_IMAGE:
#ifdef HAVE_IMAGEVIEWER
               playlist_hist        = g_defaults.image_history;
               core_name            = "imageviewer";
               core_path            = "builtin";
#endif
               break;
            default:
            {
               core_info_t *core_info = NULL;
               /* Set core display name
                * (As far as I can tell, core_info_get_current_core()
                * should always provide a valid pointer here...) */
               core_info_get_current_core(&core_info);

               /* Set core path */
               core_path            = path_get(KINGSN_PATH_CORE);

               if (core_info)
                  core_name         = core_info->display_name;

               if (string_is_empty(core_name))
                  core_name         = info->library_name;

               if (launched_from_companion_ui)
               {
                  /* Database name + checksum are supplied
                   * by the companion UI itself */
                  if (!string_is_empty(p_content->companion_ui_crc32))
                     crc32 = p_content->companion_ui_crc32;

                  if (!string_is_empty(p_content->companion_ui_db_name))
                     db_name = p_content->companion_ui_db_name;
               }
#ifdef HAVE_MENU
               else
               {
                  menu_handle_t *menu = menu_driver_get_ptr();
                  /* Set database name + checksum */
                  if (menu)
                  {
                     playlist_t *playlist_curr = playlist_get_cached();

                     if (playlist_index_is_valid(playlist_curr, menu->rpl_entry_selection_ptr, tmp, core_path))
                     {
                        playlist_get_crc32(playlist_curr, menu->rpl_entry_selection_ptr,   &crc32);
                        playlist_get_db_name(playlist_curr, menu->rpl_entry_selection_ptr, &db_name);
                     }
                  }
               }
#endif
               break;
            }
         }

         if (global && !string_is_empty(global->name.label))
            label = global->name.label;

         if (
              settings && settings->bools.history_list_enable 
               && playlist_hist)
         {
            char subsystem_name[PATH_MAX_LENGTH];
            struct playlist_entry entry = {0};

            subsystem_name[0] = '\0';

            content_get_subsystem_friendly_name(path_get(KINGSN_PATH_SUBSYSTEM), subsystem_name, sizeof(subsystem_name));

            /* The push function reads our entry as const, 
             * so these casts are safe */
            entry.path            = (char*)tmp;
            entry.label           = (char*)label;
            entry.core_path       = (char*)core_path;
            entry.core_name       = (char*)core_name;
            entry.crc32           = (char*)crc32;
            entry.db_name         = (char*)db_name;
            entry.subsystem_ident = (char*)path_get(KINGSN_PATH_SUBSYSTEM);
            entry.subsystem_name  = (char*)subsystem_name;
            entry.subsystem_roms  = (struct string_list*)path_get_subsystem_list();

            command_playlist_push_write(playlist_hist, &entry);
         }
      }
   }
}

#ifdef HAVE_MENU
static bool command_event_cmd_exec(
      content_state_t *p_content,
      const char *data,
      content_information_ctx_t *content_ctx,
      bool launched_from_cli,
      char **error_string)
{
   if (path_get(KINGSN_PATH_CONTENT) != data)
   {
      path_clear(KINGSN_PATH_CONTENT);
      if (!string_is_empty(data))
         path_set(KINGSN_PATH_CONTENT, data);
   }

#if defined(HAVE_DYNAMIC)
   {
      content_ctx_info_t content_info;

      content_info.argc        = 0;
      content_info.argv        = NULL;
      content_info.args        = NULL;
      content_info.environ_get = menu_content_environment_get;

      /* Loads content into currently selected core. */
      if (!content_load(&content_info, p_content))
         return false;
      task_push_to_history_list(p_content, true, launched_from_cli, false);
   }
#else
   frontend_driver_set_fork(FRONTEND_FORK_CORE_WITH_ARGS);
#endif

   return true;
}
#endif

static bool firmware_update_status(
      content_information_ctx_t *content_ctx)
{
   char s[PATH_MAX_LENGTH];
   core_info_ctx_firmware_t firmware_info;
   bool set_missing_firmware  = false;
   core_info_t *core_info     = NULL;
   
   core_info_get_current_core(&core_info);

   if (!core_info)
      return false;

   s[0]                       = '\0';
   firmware_info.path         = core_info->path;

   if (!string_is_empty(content_ctx->directory_system))
      firmware_info.directory.system = content_ctx->directory_system;
   else
   {
      strlcpy(s, path_get(KINGSN_PATH_CONTENT), sizeof(s));
      path_basedir_wrapper(s);
      firmware_info.directory.system = s;
   }

   KINGSN_LOG("[CONTENT LOAD]: Updating firmware status for: %s on %s\n",
         core_info->path,
         firmware_info.directory.system);

   kingsn_ctl(KINGSN_CTL_UNSET_MISSING_BIOS, NULL);

   core_info_list_update_missing_firmware(&firmware_info,
         &set_missing_firmware);

   if (set_missing_firmware)
      kingsn_ctl(KINGSN_CTL_SET_MISSING_BIOS, NULL);

   if (
         content_ctx->bios_is_missing &&
         content_ctx->check_firmware_before_loading)
   {
      runloop_msg_queue_push(
            msg_hash_to_str(MSG_FIRMWARE),
            100, 500, true, NULL,
            MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
      KINGSN_LOG("[CONTENT LOAD]: Load content blocked. Reason: %s\n",
            msg_hash_to_str(MSG_FIRMWARE));

      return true;
   }

   return false;
}

bool task_push_start_dummy_core(content_ctx_info_t *content_info)
{
   content_information_ctx_t content_ctx;
   content_state_t                 *p_content = content_state_get_ptr();
   bool ret                                   = true;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   settings_t *settings                       = config_get_ptr();
   kingsn_system_info_t *sys_info              = runloop_get_system_info();
   const char *path_dir_system                = settings->paths.directory_system;
   bool check_firmware_before_loading         = settings->bools.check_firmware_before_loading;

   if (!content_info)
      return false;

   content_ctx.check_firmware_before_loading  = check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.bios_is_missing                = kingsn_ctl(KINGSN_CTL_IS_MISSING_BIOS, NULL);
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);
   }

   if (!string_is_empty(path_dir_system))
      content_ctx.directory_system            = strdup(path_dir_system);

   if (!content_info->environ_get)
      content_info->environ_get = menu_content_environment_get;

   /* Clear content path */
   path_clear(KINGSN_PATH_CONTENT);

   /* Preliminary stuff that has to be done before we
    * load the actual content. Can differ per mode. */
   sys_info->load_no_content = false;
   kingsn_ctl(KINGSN_CTL_STATE_FREE, NULL);
   task_queue_deinit();
   KingStation_init_task_queue();

   /* Loads content into currently selected core. */
   if ((ret = content_load(content_info, p_content)))
      task_push_to_history_list(p_content, false, false, false);

   /* Handle load content failure */
   if (!ret)
   {
      if (error_string)
      {
         runloop_msg_queue_push(error_string, 2, 90, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
         free(error_string);
      }
   }

   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);

   return ret;
}

#ifdef HAVE_MENU

bool task_push_load_content_from_playlist_from_menu(
      const char *core_path,
      const char *fullpath,
      const char *label,
      content_ctx_info_t *content_info,
      ks_task_callback_t cb,
      void *user_data)
{
   content_information_ctx_t content_ctx;

   content_state_t                 *p_content = content_state_get_ptr();
   bool ret                                   = true;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   settings_t *settings                       = config_get_ptr();
   kingsn_system_info_t *sys_info              = runloop_get_system_info();
   const char *path_dir_system                = settings->paths.directory_system;
#ifndef HAVE_DYNAMIC
   bool force_core_reload                     = settings->bools.always_reload_core_on_run_content;
#endif

   content_ctx.check_firmware_before_loading  = settings->bools.check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.bios_is_missing                = kingsn_ctl(KINGSN_CTL_IS_MISSING_BIOS, NULL);
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);
      if (label)
         strlcpy(global->name.label, label, sizeof(global->name.label));
      else
         global->name.label[0] = '\0';
   }

   if (!string_is_empty(path_dir_system))
      content_ctx.directory_system            = strdup(path_dir_system);

   /* Is content required by this core? */
   if (fullpath)
      sys_info->load_no_content = false;
   else
      sys_info->load_no_content = true;

#ifndef HAVE_DYNAMIC
   /* Check whether specified core is already loaded
    * > If so, content can be launched directly with
    *   the currently loaded core */
   if (!force_core_reload &&
       kingsn_ctl(KINGSN_CTL_IS_CORE_LOADED, (void*)core_path))
   {
      if (!content_info->environ_get)
         content_info->environ_get = menu_content_environment_get;

      /* Register content path */
      path_clear(KINGSN_PATH_CONTENT);
      if (!string_is_empty(fullpath))
         path_set(KINGSN_PATH_CONTENT, fullpath);

      /* Load content */
      ret = content_load(content_info, p_content);

      if (!ret)
         goto end;

      /* Update content history */
      task_push_to_history_list(p_content, true, false, false);

      goto end;
   }
#endif

   /* Specified core is not loaded
    * > Load it */
   path_set(KINGSN_PATH_CORE, core_path);
#ifdef HAVE_DYNAMIC
   command_event(CMD_EVENT_LOAD_CORE, NULL);
#endif

   /* Load content
    * > On targets that do not support dynamic core loading,
    *   command_event_cmd_exec() will fork a new instance */
   if (!(ret = command_event_cmd_exec(p_content,
         fullpath, &content_ctx, false, &error_string)))
      goto end;

#ifdef HAVE_COCOATOUCH
   /* This seems to be needed for iOS for some reason
    * to show the quick menu after the menu is shown */
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

#ifndef HAVE_DYNAMIC
   /* No dynamic core loading support: if we reach
    * this point then a new instance has been
    * forked - have to shut down this one */
   kingsn_ctl(KINGSN_CTL_SET_SHUTDOWN, NULL);
   KingStation_menu_running_finished(true);
#endif

end:
   /* Handle load content failure */
   if (!ret)
   {
      if (error_string)
      {
         runloop_msg_queue_push(error_string, 2, 90, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
         free(error_string);
      }

      KingStation_menu_running();
   }

   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);

   return ret;
}
#endif

bool task_push_start_current_core(content_ctx_info_t *content_info)
{
   content_information_ctx_t content_ctx;

   content_state_t                 *p_content = content_state_get_ptr();
   bool ret                                   = true;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   settings_t *settings                       = config_get_ptr();
   const char *path_dir_system                = settings->paths.directory_system;
   bool check_firmware_before_loading         = settings->bools.check_firmware_before_loading;

   if (!content_info)
      return false;

   content_ctx.check_firmware_before_loading  = check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.bios_is_missing                = kingsn_ctl(KINGSN_CTL_IS_MISSING_BIOS, NULL);
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);
   }

   if (!string_is_empty(path_dir_system))
      content_ctx.directory_system            = strdup(path_dir_system);

   if (!content_info->environ_get)
      content_info->environ_get = menu_content_environment_get;

   /* Clear content path */
   path_clear(KINGSN_PATH_CONTENT);

   /* Preliminary stuff that has to be done before we
    * load the actual content. Can differ per mode. */
   KingStation_set_current_core_type(CORE_TYPE_PLAIN, true);

   /* Load content */
   if (firmware_update_status(&content_ctx))
      goto end;

   /* Loads content into currently selected core. */
   if (!(ret = content_load(content_info, p_content)))
   {
      if (error_string)
      {
         runloop_msg_queue_push(error_string, 2, 90, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
         free(error_string);
      }

      KingStation_menu_running();
      goto end;
   }

   task_push_to_history_list(p_content, true, false, false);

#ifdef HAVE_MENU
   /* Push quick menu onto menu stack */
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

end:
   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);

   return ret;
}

bool task_push_load_new_core(
      const char *core_path,
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   path_set(KINGSN_PATH_CORE, core_path);

   /* Load core */
   command_event(CMD_EVENT_LOAD_CORE, NULL);

#ifndef HAVE_DYNAMIC
   /* Fork core? */
   if (!frontend_driver_set_fork(FRONTEND_FORK_CORE))
      return false;
#endif

   /* Preliminary stuff that has to be done before we
    * load the actual content. Can differ per mode. */
   KingStation_set_current_core_type(type, true);

   return true;
}

#ifdef HAVE_MENU
bool task_push_load_content_with_new_core_from_menu(
      const char *core_path,
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   content_information_ctx_t content_ctx;

   content_state_t                 *p_content = content_state_get_ptr();
   bool ret                                   = true;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   settings_t *settings                       = config_get_ptr();
   bool check_firmware_before_loading         = settings->bools.check_firmware_before_loading;
   const char *path_dir_system                = settings->paths.directory_system;
#ifndef HAVE_DYNAMIC
   bool force_core_reload                     = settings->bools.always_reload_core_on_run_content;

   /* Check whether specified core is already loaded
    * > If so, we can skip loading the core and
    *   just load the content directly */
   if (!force_core_reload &&
       (type == CORE_TYPE_PLAIN) &&
       kingsn_ctl(KINGSN_CTL_IS_CORE_LOADED, (void*)core_path))
      return task_push_load_content_with_core_from_menu(
            fullpath, content_info,
            type, cb, user_data);
#endif

   content_ctx.check_firmware_before_loading  = check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.bios_is_missing                = kingsn_ctl(KINGSN_CTL_IS_MISSING_BIOS, NULL);
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);

      global->name.label[0]                   = '\0';
   }

   if (!string_is_empty(path_dir_system))
      content_ctx.directory_system            = strdup(path_dir_system);

   path_set(KINGSN_PATH_CONTENT, fullpath);
   path_set(KINGSN_PATH_CORE, core_path);

#ifdef HAVE_DYNAMIC
   /* Load core */
   command_event(CMD_EVENT_LOAD_CORE, NULL);

   /* Load content */
   if (!content_info->environ_get)
      content_info->environ_get = menu_content_environment_get;

   if (firmware_update_status(&content_ctx))
      goto end;

   /* Loads content into currently selected core. */
   if (!(ret = content_load(content_info, p_content)))
   {
      if (error_string)
      {
         runloop_msg_queue_push(error_string, 2, 90, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
         free(error_string);
      }

      KingStation_menu_running();
      goto end;
   }

   task_push_to_history_list(p_content, true, false, false);
#else
   command_event_cmd_exec(p_content,
         path_get(KINGSN_PATH_CONTENT), &content_ctx,
         false, &error_string);
   command_event(CMD_EVENT_QUIT, NULL);
#endif

   /* Push quick menu onto menu stack */
   if (type != CORE_TYPE_DUMMY)
      menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);

#ifdef HAVE_DYNAMIC
end:
#endif
   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);

   return ret;
}
#endif

static bool task_load_content_internal(
      content_ctx_info_t *content_info,
      bool loading_from_menu,
      bool loading_from_cli,
      bool loading_from_companion_ui)
{
   content_information_ctx_t content_ctx;

   content_state_t                 *p_content = content_state_get_ptr();
   bool ret                                   = false;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   kingsn_system_info_t *sys_info              = runloop_get_system_info();
   settings_t *settings                       = config_get_ptr();
   bool check_firmware_before_loading         = settings->bools.check_firmware_before_loading;
   bool set_supports_no_game_enable           = settings->bools.set_supports_no_game_enable;
   const char *path_dir_system                = settings->paths.directory_system;
   const char *path_dir_cache                 = settings->paths.directory_cache;

   content_ctx.check_firmware_before_loading  = check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.bios_is_missing                = kingsn_ctl(KINGSN_CTL_IS_MISSING_BIOS, NULL);
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (sys_info)
   {
      struct ks_system_info *system        = runloop_get_libks_system_info();

      content_ctx.set_supports_no_game_enable = set_supports_no_game_enable;

      if (!string_is_empty(path_dir_cache))
         content_ctx.directory_cache          = strdup(path_dir_cache);
      if (!string_is_empty(system->valid_extensions))
         content_ctx.valid_extensions         = strdup(system->valid_extensions);

      content_ctx.block_extract               = system->block_extract;
      content_ctx.need_fullpath               = system->need_fullpath;

      content_ctx.subsystem.data              = sys_info->subsystem.data;
      content_ctx.subsystem.size              = sys_info->subsystem.size;
   }

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);
   }

   if (!string_is_empty(path_dir_system))
      content_ctx.directory_system            = strdup(path_dir_system);

   if (!content_info->environ_get)
      content_info->environ_get = menu_content_environment_get;

   if (firmware_update_status(&content_ctx))
      goto end;

#ifdef HAVE_DISCORD
   if (discord_is_inited)
   {
      discord_userdata_t userdata;
      userdata.status = DISCORD_PRESENCE_NETPLAY_NETPLAY_STOPPED;
      command_event(CMD_EVENT_DISCORD_UPDATE, &userdata);
      userdata.status = DISCORD_PRESENCE_MENU;
      command_event(CMD_EVENT_DISCORD_UPDATE, &userdata);
   }
#endif

   /* Loads content into currently selected core. */
   if ((ret = content_load(content_info, p_content)))
      task_push_to_history_list(p_content,
            true, loading_from_cli, loading_from_companion_ui);

end:
   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);
   if (content_ctx.directory_cache)
      free(content_ctx.directory_cache);
   if (content_ctx.valid_extensions)
      free(content_ctx.valid_extensions);

   if (!ret)
   {
      if (error_string)
      {
         runloop_msg_queue_push(error_string, 2, 90, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
         free(error_string);
      }

      return false;
   }

   return true;
}

bool task_push_load_content_with_new_core_from_companion_ui(
      const char *core_path,
      const char *fullpath,
      const char *label,
      const char *db_name,
      const char *crc32,
      content_ctx_info_t *content_info,
      ks_task_callback_t cb,
      void *user_data)
{
   global_t *global            = global_get_ptr();
   content_state_t  *p_content = content_state_get_ptr();

   path_set(KINGSN_PATH_CONTENT, fullpath);
   path_set(KINGSN_PATH_CORE, core_path);

   p_content->companion_ui_db_name[0] = '\0';
   p_content->companion_ui_crc32[0]   = '\0';

   if (!string_is_empty(db_name))
      strlcpy(p_content->companion_ui_db_name,
            db_name, sizeof(p_content->companion_ui_db_name));

   if (!string_is_empty(crc32))
      strlcpy(p_content->companion_ui_crc32,
            crc32, sizeof(p_content->companion_ui_crc32));

#ifdef HAVE_DYNAMIC
   command_event(CMD_EVENT_LOAD_CORE, NULL);
#endif

   global->launched_from_cli = false;

   if (global)
   {
      if (label)
         strlcpy(global->name.label, label, sizeof(global->name.label));
      else
         global->name.label[0] = '\0';
   }

   /* Load content */
   if (!task_load_content_internal(content_info, true, false, true))
      return false;

#ifdef HAVE_MENU
   /* Push quick menu onto menu stack */
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

   return true;
}

bool task_push_load_content_from_cli(
      const char *core_path,
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   return task_load_content_internal(content_info, true, true, false);
}

bool task_push_start_builtin_core(
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   /* Clear content path */
   path_clear(KINGSN_PATH_CONTENT);

   /* Preliminary stuff that has to be done before we
    * load the actual content. Can differ per mode. */
   KingStation_set_current_core_type(type, true);

   /* Load content */
   if (!task_load_content_internal(content_info, true, false, false))
   {
      KingStation_menu_running();
      return false;
   }

   /* Push quick menu onto menu stack */
#ifdef HAVE_MENU
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

   return true;
}

bool task_push_load_content_with_core_from_menu(
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   path_set(KINGSN_PATH_CONTENT, fullpath);

   /* Load content */
   if (!task_load_content_internal(content_info, true, false, false))
   {
      KingStation_menu_running();
      return false;
   }

#ifdef HAVE_MENU
   /* Push quick menu onto menu stack */
   if (type != CORE_TYPE_DUMMY)
      menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

   return true;
}

bool task_push_load_content_with_current_core_from_companion_ui(
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   content_state_t  *p_content = content_state_get_ptr();

   /* TODO/FIXME: Enable setting of these values
    * via function arguments */
   p_content->companion_ui_db_name[0] = '\0';
   p_content->companion_ui_crc32[0]   = '\0';

   /* Load content
    * > TODO/FIXME: Set loading_from_companion_ui 'false' for
    *   now, until someone can implement the required higher
    *   level functionality in 'win32_common.c' and 'ui_cocoa.m' */
   return task_push_load_content_with_core_from_menu(fullpath,
         content_info, type, cb, user_data);
}


bool task_push_load_subsystem_with_core_from_menu(
      const char *fullpath,
      content_ctx_info_t *content_info,
      enum kingsn_core_type type,
      ks_task_callback_t cb,
      void *user_data)
{
   content_state_t  *p_content = content_state_get_ptr();

   p_content->pending_subsystem_init = true;

   /* Load content */
   if (!task_load_content_internal(content_info, true, false, false))
   {
      KingStation_menu_running();
      return false;
   }

#ifdef HAVE_MENU
   /* Push quick menu onto menu stack */
   if (type != CORE_TYPE_DUMMY)
      menu_driver_ctl(KINGSN_MENU_CTL_SET_PENDING_QUICK_MENU, NULL);
#endif

   return true;
}

void content_get_status(
      bool *contentless,
      bool *is_inited)
{
   content_state_t  *p_content = content_state_get_ptr();

   *contentless = p_content->core_does_not_need_content;
   *is_inited   = p_content->is_inited;
}

/* Clears the pending subsystem rom buffer*/
void content_clear_subsystem(void)
{
   unsigned i;
   content_state_t  *p_content = content_state_get_ptr();

   p_content->pending_subsystem_rom_id    = 0;
   p_content->pending_subsystem_init      = false;

   for (i = 0; i < KINGSN_MAX_SUBSYSTEM_ROMS; i++)
   {
      if (p_content->pending_subsystem_roms[i])
      {
         free(p_content->pending_subsystem_roms[i]);
         p_content->pending_subsystem_roms[i] = NULL;
      }
   }
}

/* Set the current subsystem*/
void content_set_subsystem(unsigned idx)
{
   const struct ks_subsystem_info *subsystem;
   kingsn_system_info_t                  *system = runloop_get_system_info();
   content_state_t  *p_content                  = content_state_get_ptr();

   /* Core fully loaded, use the subsystem data */
   if (system->subsystem.data)
      subsystem = system->subsystem.data + idx;
   /* Core not loaded completely, use the data we peeked on load core */
   else
      subsystem = subsystem_data + idx;

   p_content->pending_subsystem_id = idx;

   if (subsystem && subsystem_current_count > 0)
   {
      strlcpy(p_content->pending_subsystem_ident,
         subsystem->ident, sizeof(p_content->pending_subsystem_ident));

      p_content->pending_subsystem_rom_num = subsystem->num_roms;
   }

   KINGSN_LOG("[Subsystem]: Setting current subsystem to: %d(%s) Content amount: %d\n",
      p_content->pending_subsystem_id,
      p_content->pending_subsystem_ident,
      p_content->pending_subsystem_rom_num);
}

/* Sets the subsystem by name */
bool content_set_subsystem_by_name(const char* subsystem_name)
{
   kingsn_system_info_t                  *system = runloop_get_system_info();
   unsigned i                                   = 0;
   /* Core not loaded completely, use the data we peeked on load core */
   const struct ks_subsystem_info 
      *subsystem                                = subsystem_data;

   /* Core fully loaded, use the subsystem data */
   if (system->subsystem.data)
      subsystem = system->subsystem.data;

   for (i = 0; i < subsystem_current_count; i++, subsystem++)
   {
      if (string_is_equal(subsystem_name, subsystem->ident))
      {
         content_set_subsystem(i);
         return true;
      }
   }

   return false;
}

void content_get_subsystem_friendly_name(const char* subsystem_name, char* subsystem_friendly_name, size_t len)
{
   kingsn_system_info_t                  *system = runloop_get_system_info();
   unsigned i                                   = 0;
   /* Core not loaded completely, use the data we peeked on load core */
   const struct ks_subsystem_info *subsystem = subsystem_data;

   /* Core fully loaded, use the subsystem data */
   if (system->subsystem.data)
      subsystem = system->subsystem.data;

   for (i = 0; i < subsystem_current_count; i++, subsystem++)
   {
      if (string_is_equal(subsystem_name, subsystem->ident))
      {
         strlcpy(subsystem_friendly_name, subsystem->desc, len);
         break;
      }
   }

   return;
}

/* Add a rom to the subsystem ROM buffer */
void content_add_subsystem(const char* path)
{
   content_state_t *p_content = content_state_get_ptr();
   size_t pending_size        = PATH_MAX_LENGTH * sizeof(char);
   p_content->pending_subsystem_roms[p_content->pending_subsystem_rom_id] = (char*)malloc(pending_size);

   strlcpy(p_content->pending_subsystem_roms[
         p_content->pending_subsystem_rom_id],
         path, pending_size);
   KINGSN_LOG("[Subsystem]: Subsystem id: %d Subsystem ident:"
         " %s Content ID: %d, Content Path: %s\n",
         p_content->pending_subsystem_id,
         p_content->pending_subsystem_ident,
         p_content->pending_subsystem_rom_id,
         p_content->pending_subsystem_roms[
         p_content->pending_subsystem_rom_id]);
   p_content->pending_subsystem_rom_id++;
}

void content_set_does_not_need_content(void)
{
   content_state_t *p_content = content_state_get_ptr();
   p_content->core_does_not_need_content = true;
}

void content_unset_does_not_need_content(void)
{
   content_state_t *p_content = content_state_get_ptr();
   p_content->core_does_not_need_content = false;
}

uint32_t content_get_crc(void)
{
   content_state_t *p_content = content_state_get_ptr();
   if (p_content->pending_rom_crc)
   {
      p_content->pending_rom_crc   = false;
      p_content->rom_crc           = file_crc32(0,
            (const char*)p_content->pending_rom_crc_path);
      KINGSN_LOG("[CONTENT LOAD]: CRC32: 0x%x .\n",
            (unsigned)p_content->rom_crc);
   }
   return p_content->rom_crc;
}

char* content_get_subsystem_rom(unsigned index)
{
   content_state_t *p_content = content_state_get_ptr();
   return p_content->pending_subsystem_roms[index];
}

bool content_is_inited(void)
{
   content_state_t *p_content = content_state_get_ptr();
   return p_content->is_inited;
}

void content_deinit(void)
{
   unsigned i;
   content_state_t *p_content = content_state_get_ptr();

   if (p_content->temporary_content)
   {
      for (i = 0; i < p_content->temporary_content->size; i++)
      {
         const char *path = p_content->temporary_content->elems[i].data;

         KINGSN_LOG("[CONTENT LOAD]: %s: %s.\n",
               msg_hash_to_str(MSG_REMOVING_TEMPORARY_CONTENT_FILE), path);
         if (filestream_delete(path) != 0)
            KINGSN_ERR("[CONTENT LOAD]: %s: %s.\n",
                  msg_hash_to_str(MSG_FAILED_TO_REMOVE_TEMPORARY_FILE),
                  path);
      }
      string_list_free(p_content->temporary_content);
   }

   p_content->temporary_content            = NULL;
   p_content->rom_crc                      = 0;
   p_content->is_inited                    = false;
   p_content->core_does_not_need_content   = false;
   p_content->pending_rom_crc              = false;
}

/* Set environment variables before a subsystem load */
void content_set_subsystem_info(void)
{
   content_state_t *p_content = content_state_get_ptr();
   if (!p_content->pending_subsystem_init)
      return;

   path_set(KINGSN_PATH_SUBSYSTEM, p_content->pending_subsystem_ident);
   path_set_special(p_content->pending_subsystem_roms,
         p_content->pending_subsystem_rom_num);
}

/* Initializes and loads a content file for the currently
 * selected libks core. */
bool content_init(void)
{
   struct string_list content;
   content_information_ctx_t content_ctx;
   enum msg_hash_enums error_enum             = MSG_UNKNOWN;
   content_state_t *p_content                 = content_state_get_ptr();

   bool ret                                   = true;
   char *error_string                         = NULL;
   global_t *global                           = global_get_ptr();
   kingsn_system_info_t *sys_info              = runloop_get_system_info();
   settings_t *settings                       = config_get_ptr();
   bool check_firmware_before_loading         = settings->bools.check_firmware_before_loading;
   bool set_supports_no_game_enable           = settings->bools.set_supports_no_game_enable;
   const char *path_dir_system                = settings->paths.directory_system;
   const char *path_dir_cache                 = settings->paths.directory_cache;

   p_content->temporary_content               = string_list_new();

   content_ctx.check_firmware_before_loading  = check_firmware_before_loading;
#ifdef HAVE_PATCH
   content_ctx.is_ips_pref                    = kingsn_ctl(KINGSN_CTL_IS_IPS_PREF, NULL);
   content_ctx.is_bps_pref                    = kingsn_ctl(KINGSN_CTL_IS_BPS_PREF, NULL);
   content_ctx.is_ups_pref                    = kingsn_ctl(KINGSN_CTL_IS_UPS_PREF, NULL);
   content_ctx.patch_is_blocked               = kingsn_ctl(KINGSN_CTL_IS_PATCH_BLOCKED, NULL);
#endif
   content_ctx.temporary_content              = p_content->temporary_content;
   content_ctx.directory_system               = NULL;
   content_ctx.directory_cache                = NULL;
   content_ctx.name_ips                       = NULL;
   content_ctx.name_bps                       = NULL;
   content_ctx.name_ups                       = NULL;
   content_ctx.valid_extensions               = NULL;
   content_ctx.block_extract                  = false;
   content_ctx.need_fullpath                  = false;
   content_ctx.set_supports_no_game_enable    = false;

   content_ctx.subsystem.data                 = NULL;
   content_ctx.subsystem.size                 = 0;

   if (global)
   {
      if (!string_is_empty(global->name.ips))
         content_ctx.name_ips                 = strdup(global->name.ips);
      if (!string_is_empty(global->name.bps))
         content_ctx.name_bps                 = strdup(global->name.bps);
      if (!string_is_empty(global->name.ups))
         content_ctx.name_ups                 = strdup(global->name.ups);
   }

   if (sys_info)
   {
      struct ks_system_info *system        = runloop_get_libks_system_info();

      content_ctx.set_supports_no_game_enable = set_supports_no_game_enable;

      if (!string_is_empty(path_dir_system))
         content_ctx.directory_system         = strdup(path_dir_system);
      if (!string_is_empty(path_dir_cache))
         content_ctx.directory_cache          = strdup(path_dir_cache);
      if (!string_is_empty(system->valid_extensions))
         content_ctx.valid_extensions         = strdup(system->valid_extensions);

      content_ctx.block_extract               = system->block_extract;
      content_ctx.need_fullpath               = system->need_fullpath;

      content_ctx.subsystem.data              = sys_info->subsystem.data;
      content_ctx.subsystem.size              = sys_info->subsystem.size;
   }

   p_content->is_inited = true;

   if (string_list_initialize(&content))
   {
      if (     !p_content->temporary_content
            || !content_file_init(&content_ctx, p_content,
               &content, &error_enum, &error_string))
      {
         content_deinit();

         ret                = false;
      }
      string_list_deinitialize(&content);
   }

   if (content_ctx.name_ips)
      free(content_ctx.name_ips);
   if (content_ctx.name_bps)
      free(content_ctx.name_bps);
   if (content_ctx.name_ups)
      free(content_ctx.name_ups);
   if (content_ctx.directory_system)
      free(content_ctx.directory_system);
   if (content_ctx.directory_cache)
      free(content_ctx.directory_cache);
   if (content_ctx.valid_extensions)
      free(content_ctx.valid_extensions);
   
   if (error_enum != MSG_UNKNOWN)
   {
      switch (error_enum)
      {
         case MSG_ERROR_LIBKS_CORE_REQUIRES_SPECIAL_CONTENT:
         case MSG_ERROR_LIBKS_CORE_REQUIRES_VFS:
         case MSG_FAILED_TO_LOAD_CONTENT:
         case MSG_ERROR_LIBKS_CORE_REQUIRES_CONTENT:
            KINGSN_ERR("[CONTENT LOAD]: %s\n", msg_hash_to_str(error_enum));
            runloop_msg_queue_push(msg_hash_to_str(error_enum), 2, ret ? 1 : 180, false, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
            break;
         case MSG_UNKNOWN:
         default:
            break;
      }
   }

   if (error_string)
   {
      if (ret)
      {
         KINGSN_LOG("[CONTENT LOAD]: %s\n", error_string);
      }
      else
      {
         KINGSN_ERR("[CONTENT LOAD]: %s\n", error_string);
      }
      /* Do not flush the message queue here
       * > This allows any core-generated error messages
       *   to propagate through to the frontend */
      runloop_msg_queue_push(error_string, 2, ret ? 1 : 180, false, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
      free(error_string);
   }

   return ret;
}
