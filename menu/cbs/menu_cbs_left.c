/*  KingStation - A frontend for libks.
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

#include <compat/strl.h>
#include <file/file_path.h>
#include <string/stdstring.h>
#include <lists/string_list.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../menu_driver.h"
#include "../menu_entries.h"
#include "../menu_cbs.h"
#include "../menu_input.h"
#include "../menu_setting.h"
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
#include "../menu_shader.h"
#endif

#include "../../configuration.h"
#include "../../core.h"
#include "../../core_info.h"
#ifdef HAVE_CHEATS
#include "../../cheat_manager.h"
#endif
#include "../../file_path_special.h"
#include "../../driver.h"
#include "../../KingStation.h"
#include "../../playlist.h"
#include "../../manual_content_scan.h"

#ifndef BIND_ACTION_LEFT
#define BIND_ACTION_LEFT(cbs, name) (cbs)->action_left = (name)
#endif

/* Forward declarations */
int action_ok_core_lock(const char *path, const char *label, unsigned type, size_t idx, size_t entry_idx);

extern struct key_desc key_descriptors[KINGSN_MAX_KEYS];

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
static int generic_shader_action_parameter_left(
      struct video_shader_parameter *param,
      unsigned type, const char *label, bool wraparound)
{
   param->current -= param->step;
   param->current  = MIN(MAX(param->minimum, param->current),
         param->maximum);
   return 0;
}

static int shader_action_parameter_left_internal(unsigned type, const char *label, bool wraparound,
      unsigned offset)
{
   video_shader_ctx_t shader_info;
   struct video_shader *shader          = menu_shader_get();
   struct video_shader_parameter *param_menu = NULL;
   struct video_shader_parameter *param_prev = NULL;

   int ret = 0;

   video_shader_driver_get_current_shader(&shader_info);

   param_prev = &shader_info.data->parameters[type - offset];
   param_menu = shader ? &shader->parameters [type - offset] : NULL;

   if (!param_prev || !param_menu)
      return menu_cbs_exit();
   ret = generic_shader_action_parameter_left(param_prev, type, label, wraparound);

   param_menu->current = param_prev->current;

   shader->modified    = true;

   return ret;
}

static int shader_action_parameter_left(unsigned type, const char *label, bool wraparound)
{
   return shader_action_parameter_left_internal(type, label, wraparound, MENU_SETTINGS_SHADER_PARAMETER_0);
}

static int shader_action_preset_parameter_left(unsigned type, const char *label, bool wraparound)
{
   return shader_action_parameter_left_internal(type, label, wraparound, MENU_SETTINGS_SHADER_PRESET_PARAMETER_0);
}
#endif

#ifdef HAVE_AUDIOMIXER
static int audio_mixer_stream_volume_left(unsigned type, const char *label,
      bool wraparound)
{
   unsigned         offset      = (type - MENU_SETTINGS_AUDIO_MIXER_STREAM_ACTIONS_VOLUME_BEGIN);
   float orig_volume            = 0.0f;

   if (offset >= AUDIO_MIXER_MAX_STREAMS)
      return 0;

   orig_volume                  = audio_driver_mixer_get_stream_volume(offset);
   orig_volume                  = orig_volume - 1.00f;

   audio_driver_mixer_set_stream_volume(offset, orig_volume);

   return 0;
}
#endif

#ifdef HAVE_CHEATS
static int action_left_cheat(unsigned type, const char *label,
      bool wraparound)
{
   size_t idx             = type - MENU_SETTINGS_CHEAT_BEGIN;
   return generic_action_cheat_toggle(idx, type, label,
         wraparound);
}
#endif

static int action_left_input_desc(unsigned type, const char *label,
   bool wraparound)
{
   kingsn_system_info_t *system           = runloop_get_system_info();
   settings_t *settings                  = config_get_ptr();
   unsigned btn_idx, user_idx, remap_idx;

   if (!settings || !system)
      return 0;

   user_idx = (type - MENU_SETTINGS_INPUT_DESC_BEGIN) / (KINGSN_FIRST_CUSTOM_BIND + 8);
   btn_idx  = (type - MENU_SETTINGS_INPUT_DESC_BEGIN) - (KINGSN_FIRST_CUSTOM_BIND + 8) * user_idx;

   if (settings->uints.input_remap_ids[user_idx][btn_idx] == KINGSN_UNMAPPED)
      settings->uints.input_remap_ids[user_idx][btn_idx] = KINGSN_CUSTOM_BIND_LIST_END - 1;

   if (settings->uints.input_remap_ids[user_idx][btn_idx] > 0)
      settings->uints.input_remap_ids[user_idx][btn_idx]--;
   else if (settings->uints.input_remap_ids[user_idx][btn_idx] == 0)
      settings->uints.input_remap_ids[user_idx][btn_idx] = KINGSN_UNMAPPED;
   else
      settings->uints.input_remap_ids[user_idx][btn_idx] = KINGSN_CUSTOM_BIND_LIST_END - 1;

   remap_idx = settings->uints.input_remap_ids[user_idx][btn_idx];

   /* skip the not used buttons (unless they are at the end by calling the right desc function recursively
      also skip all the axes until analog remapping is implemented */
   if (remap_idx != KINGSN_UNMAPPED)
   {
      if ((string_is_empty(system->input_desc_btn[user_idx][remap_idx]) && remap_idx < KINGSN_CUSTOM_BIND_LIST_END) /*||
          (remap_idx >= KINGSN_FIRST_CUSTOM_BIND && remap_idx < KINGSN_CUSTOM_BIND_LIST_END)*/)
         action_left_input_desc(type, label, wraparound);
   }

   return 0;
}

static int action_left_input_desc_kbd(unsigned type, const char *label,
   bool wraparound)
{
   unsigned remap_id;
   unsigned key_id, user_idx, btn_idx;
   settings_t *settings = config_get_ptr();

   if (!settings)
      return 0;

   user_idx = (type - MENU_SETTINGS_INPUT_DESC_KBD_BEGIN) / KINGSN_FIRST_CUSTOM_BIND;
   btn_idx  = (type - MENU_SETTINGS_INPUT_DESC_KBD_BEGIN) - KINGSN_FIRST_CUSTOM_BIND * user_idx;

   remap_id =
      settings->uints.input_keymapper_ids[user_idx][btn_idx];

   for (key_id = 0; key_id < KINGSN_MAX_KEYS; key_id++)
   {
      if (remap_id == key_descriptors[key_id].key)
         break;
   }

   if (key_id > 0)
      key_id--;
   else
      key_id = KINGSN_MAX_KEYS - 1;

   settings->uints.input_keymapper_ids[user_idx][btn_idx] = key_descriptors[key_id].key;

   return 0;
}

static int action_left_scroll(unsigned type, const char *label,
      bool wraparound)
{
   size_t scroll_accel   = 0;
   unsigned scroll_speed = 0, fast_scroll_speed = 0;
   size_t selection      = menu_navigation_get_selection();

   if (!menu_driver_ctl(MENU_NAVIGATION_CTL_GET_SCROLL_ACCEL, &scroll_accel))
      return false;

   scroll_speed          = (unsigned)((MAX(scroll_accel, 2) - 2) / 4 + 1);
   fast_scroll_speed     = 4 + 4 * scroll_speed;

   if (selection > fast_scroll_speed)
   {
      size_t idx  = selection - fast_scroll_speed;
      menu_navigation_set_selection(idx);
      menu_driver_navigation_set(true);
   }
   else
   {
      bool pending_push = false;
      menu_driver_ctl(MENU_NAVIGATION_CTL_CLEAR, &pending_push);
   }

   return 0;
}

static int action_left_goto_tab(void)
{
   menu_ctx_list_t list_info;
   file_list_t *selection_buf = menu_entries_get_selection_buf_ptr(0);
   file_list_t *menu_stack    = menu_entries_get_menu_stack_ptr(0);
   size_t selection           = menu_navigation_get_selection();
   menu_file_list_cbs_t *cbs  = selection_buf ? (menu_file_list_cbs_t*)
      selection_buf->list[selection].actiondata : NULL;

   list_info.type             = MENU_LIST_HORIZONTAL;
   list_info.action           = MENU_ACTION_LEFT;

   menu_driver_list_cache(&list_info);

   if (cbs && cbs->action_content_list_switch)
      return cbs->action_content_list_switch(selection_buf, menu_stack,
            "", "", 0);

   return 0;
}

static int action_left_mainmenu(unsigned type, const char *label,
      bool wraparound)
{
   menu_ctx_list_t list_info;
   settings_t            *settings = config_get_ptr();
   bool menu_nav_wraparound_enable = settings->bools.menu_navigation_wraparound_enable;
   const char *menu_ident          = menu_driver_ident();

   menu_driver_list_get_selection(&list_info);

   list_info.type = MENU_LIST_PLAIN;

   menu_driver_list_get_size(&list_info);

   /* Tab switching functionality only applies
    * to XMB */
   if ((list_info.size == 1) &&
       string_is_equal(menu_ident, "xmb"))
   {
      if ((list_info.selection != 0) || menu_nav_wraparound_enable)
         return action_left_goto_tab();
   }
   else
      action_left_scroll(0, "", false);

   return 0;
}

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
static int action_left_shader_scale_pass(unsigned type, const char *label,
      bool wraparound)
{
   unsigned current_scale, delta;
   unsigned pass                         = type -
      MENU_SETTINGS_SHADER_PASS_SCALE_0;
   struct video_shader *shader           = menu_shader_get();
   struct video_shader_pass *shader_pass = shader ? &shader->pass[pass] : NULL;

   if (!shader_pass)
      return menu_cbs_exit();

   current_scale            = shader_pass->fbo.scale_x;
   delta                    = 5;
   current_scale            = (current_scale + delta) % 6;

   shader_pass->fbo.valid   = current_scale;
   shader_pass->fbo.scale_x = current_scale;
   shader_pass->fbo.scale_y = current_scale;

   shader->modified         = true;

   return 0;
}

static int action_left_shader_filter_pass(unsigned type, const char *label,
      bool wraparound)
{
   unsigned delta = 2;
   unsigned pass                         = type - MENU_SETTINGS_SHADER_PASS_FILTER_0;
   struct video_shader *shader           = menu_shader_get();
   struct video_shader_pass *shader_pass = shader ? &shader->pass[pass] : NULL;

   if (!shader_pass)
      return menu_cbs_exit();

   shader_pass->filter                   = ((shader_pass->filter + delta) % 3);
   shader->modified                      = true;

   return 0;
}

static int action_left_shader_filter_default(unsigned type, const char *label,
      bool wraparound)
{
   kingsn_setting_t *setting = menu_setting_find_enum(
         MENU_ENUM_LABEL_VIDEO_SMOOTH);
   if (!setting)
      return menu_cbs_exit();
   return menu_action_handle_setting(setting,
         setting->type, MENU_ACTION_LEFT, wraparound);
}
#endif

#ifdef HAVE_CHEATS
static int action_left_cheat_num_passes(unsigned type, const char *label,
      bool wraparound)
{
   bool refresh      = false;
   unsigned new_size = 0;

   if (cheat_manager_get_size())
      new_size = cheat_manager_get_size() - 1;
   menu_entries_ctl(MENU_ENTRIES_CTL_SET_REFRESH, &refresh);
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PREVENT_POPULATE, NULL);
   cheat_manager_realloc(new_size, CHEAT_HANDLER_TYPE_EMU);

   return 0;
}
#endif

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
static int action_left_shader_num_passes(unsigned type, const char *label,
      bool wraparound)
{
   bool refresh      = false;
   struct video_shader *shader = menu_shader_get();
   unsigned pass_count         = shader ? shader->passes : 0;

   if (!shader)
      return menu_cbs_exit();

   if (pass_count > 0)
      shader->passes--;

   menu_entries_ctl(MENU_ENTRIES_CTL_SET_REFRESH, &refresh);
   menu_driver_ctl(KINGSN_MENU_CTL_SET_PREVENT_POPULATE, NULL);
   video_shader_resolve_parameters(shader);

   shader->modified                      = true;

   return 0;
}
#endif

static int action_left_video_resolution(unsigned type, const char *label,
      bool wraparound)
{
   video_driver_get_prev_video_out();
   return 0;
}

static int playlist_association_left(unsigned type, const char *label,
      bool wraparound)
{
   char core_filename[PATH_MAX_LENGTH];
   size_t i, current                = 0;
   size_t next                      = 0;
   playlist_t *playlist             = playlist_get_cached();
   const char *default_core_path    = playlist_get_default_core_path(playlist);
   bool default_core_set            = false;
   core_info_list_t *core_info_list = NULL;
   core_info_t *core_info           = NULL;

   core_filename[0] = '\0';

   if (!playlist)
      return -1;

   core_info_get_list(&core_info_list);
   if (!core_info_list)
      return menu_cbs_exit();

   /* Get current core path association */
   if (!string_is_empty(default_core_path) &&
       !string_is_equal(default_core_path, "DETECT"))
   {
      const char *default_core_filename = path_basename(default_core_path);
      if (!string_is_empty(default_core_filename))
      {
         strlcpy(core_filename, default_core_filename, sizeof(core_filename));
         default_core_set = true;
      }
   }

   /* Sort cores alphabetically */
   core_info_qsort(core_info_list, CORE_INFO_LIST_SORT_DISPLAY_NAME);

   /* If a core is currently associated... */
   if (default_core_set)
   {
      /* ...get its index */
      for (i = 0; i < core_info_list->count; i++)
      {
         core_info = NULL;
         core_info = core_info_get(core_info_list, i);
         if (!core_info)
            continue;
         if (string_starts_with(core_filename, core_info->core_file_id.str))
            current = i;
      }

      /* ...then decrement it */
      if (current == 0)
         /* Unset core association (DETECT) */
         default_core_set = false;
      else
         next = current - 1;
   }
   /* If a core is *not* currently associated and
    * wraparound is enabled, select last core in
    * the list */
   else if (wraparound && (core_info_list->count > 1))
   {
      next             = core_info_list->count - 1;
      default_core_set = true;
   }

   /* If a core is now associated, get new core info */
   core_info = NULL;
   if (default_core_set)
      core_info = core_info_get(core_info_list, next);

   /* Update playlist */
   playlist_set_default_core_path(playlist, core_info ? core_info->path         : "DETECT");
   playlist_set_default_core_name(playlist, core_info ? core_info->display_name : "DETECT");
   playlist_write_file(playlist);

   return 0;
}

static int playlist_label_display_mode_left(unsigned type, const char *label,
      bool wraparound)
{
   enum playlist_label_display_mode label_display_mode;
   playlist_t *playlist = playlist_get_cached();

   if (!playlist)
      return -1;

   label_display_mode = playlist_get_label_display_mode(playlist);

   if (label_display_mode != LABEL_DISPLAY_MODE_DEFAULT)
      label_display_mode = (enum playlist_label_display_mode)((int)label_display_mode - 1);
   else if (wraparound)
      label_display_mode = LABEL_DISPLAY_MODE_KEEP_REGION_AND_DISC_INDEX;

   playlist_set_label_display_mode(playlist, label_display_mode);
   playlist_write_file(playlist);

   return 0;
}

static void playlist_thumbnail_mode_left(playlist_t *playlist, enum playlist_thumbnail_id thumbnail_id,
      bool wraparound)
{
   enum playlist_thumbnail_mode thumbnail_mode =
         playlist_get_thumbnail_mode(playlist, thumbnail_id);

   if (thumbnail_mode > PLAYLIST_THUMBNAIL_MODE_DEFAULT)
      thumbnail_mode = (enum playlist_thumbnail_mode)((unsigned)thumbnail_mode - 1);
   else if (wraparound)
      thumbnail_mode = PLAYLIST_THUMBNAIL_MODE_BOXARTS;

   playlist_set_thumbnail_mode(playlist, thumbnail_id, thumbnail_mode);
   playlist_write_file(playlist);
}

static int playlist_right_thumbnail_mode_left(unsigned type, const char *label,
      bool wraparound)
{
   playlist_t *playlist = playlist_get_cached();

   if (!playlist)
      return -1;

   playlist_thumbnail_mode_left(playlist, PLAYLIST_THUMBNAIL_RIGHT, wraparound);

   return 0;
}

static int playlist_left_thumbnail_mode_left(unsigned type, const char *label,
      bool wraparound)
{
   playlist_t *playlist = playlist_get_cached();

   if (!playlist)
      return -1;

   playlist_thumbnail_mode_left(playlist, PLAYLIST_THUMBNAIL_LEFT, wraparound);

   return 0;
}

static int playlist_sort_mode_left(unsigned type, const char *label,
      bool wraparound)
{
   enum playlist_sort_mode sort_mode;
   playlist_t *playlist = playlist_get_cached();

   if (!playlist)
      return -1;

   sort_mode = playlist_get_sort_mode(playlist);

   if (sort_mode > PLAYLIST_SORT_MODE_DEFAULT)
      sort_mode = (enum playlist_sort_mode)((int)sort_mode - 1);
   else if (wraparound)
      sort_mode = PLAYLIST_SORT_MODE_OFF;

   playlist_set_sort_mode(playlist, sort_mode);
   playlist_write_file(playlist);

   return 0;
}

static int manual_content_scan_system_name_left(
      unsigned type, const char *label,
      bool wraparound)
{
   settings_t *settings              = config_get_ptr();
   bool show_hidden_files            = settings->bools.show_hidden_files;
#ifdef HAVE_LIBKSDB
   const char *path_content_database = settings->paths.path_content_database;
   struct string_list *system_name_list                            =
      manual_content_scan_get_menu_system_name_list(path_content_database, show_hidden_files);
#else
   struct string_list *system_name_list                            =
      manual_content_scan_get_menu_system_name_list(NULL, show_hidden_files);
#endif
   const char *current_system_name                                 = NULL;
   enum manual_content_scan_system_name_type next_system_name_type =
         MANUAL_CONTENT_SCAN_SYSTEM_NAME_DATABASE;
   const char *next_system_name                                    = NULL;
   unsigned current_index                                          = 0;
   unsigned next_index                                             = 0;
   unsigned i;

   if (!system_name_list)
      return -1;

   /* Get currently selected system name */
   if (manual_content_scan_get_menu_system_name(&current_system_name))
   {
      /* Get index of currently selected system name */
      for (i = 0; i < system_name_list->size; i++)
      {
         const char *system_name = system_name_list->elems[i].data;

         if (string_is_equal(current_system_name, system_name))
         {
            current_index = i;
            break;
         }
      }

      /* Decrement index */
      if (current_index > 0)
         next_index = current_index - 1;
      else if (wraparound && (system_name_list->size > 1))
         next_index = system_name_list->size - 1;
   }

   /* Get new system name parameters */
   if (next_index == (unsigned)MANUAL_CONTENT_SCAN_SYSTEM_NAME_CONTENT_DIR)
      next_system_name_type = MANUAL_CONTENT_SCAN_SYSTEM_NAME_CONTENT_DIR;
   else if (next_index == (unsigned)MANUAL_CONTENT_SCAN_SYSTEM_NAME_CUSTOM)
      next_system_name_type = MANUAL_CONTENT_SCAN_SYSTEM_NAME_CUSTOM;

   next_system_name = system_name_list->elems[next_index].data;

   /* Set system name */
   manual_content_scan_set_menu_system_name(
         next_system_name_type, next_system_name);

   /* Clean up */
   string_list_free(system_name_list);

   return 0;
}

static int manual_content_scan_core_name_left(unsigned type, const char *label,
      bool wraparound)
{
   struct string_list *core_name_list                =
         manual_content_scan_get_menu_core_name_list();
   const char *current_core_name                     = NULL;
   enum manual_content_scan_core_type next_core_type =
         MANUAL_CONTENT_SCAN_CORE_SET;
   const char *next_core_name                        = NULL;
   unsigned current_index                            = 0;
   unsigned next_index                               = 0;
   unsigned i;

   if (!core_name_list)
      return -1;

   /* Get currently selected core name */
   if (manual_content_scan_get_menu_core_name(&current_core_name))
   {
      /* Get index of currently selected core name */
      for (i = 0; i < core_name_list->size; i++)
      {
         const char *core_name = core_name_list->elems[i].data;

         if (string_is_equal(current_core_name, core_name))
         {
            current_index = i;
            break;
         }
      }

      /* Decrement index */
      if (current_index > 0)
         next_index = current_index - 1;
      else if (wraparound && (core_name_list->size > 1))
         next_index = core_name_list->size - 1;
   }

   /* Get new core name parameters */
   if (next_index == (unsigned)MANUAL_CONTENT_SCAN_CORE_DETECT)
      next_core_type = MANUAL_CONTENT_SCAN_CORE_DETECT;

   next_core_name = core_name_list->elems[next_index].data;

   /* Set core name */
   manual_content_scan_set_menu_core_name(
         next_core_type, next_core_name);

   /* Clean up */
   string_list_free(core_name_list);

   return 0;
}

static int core_setting_left(unsigned type, const char *label,
      bool wraparound)
{
   unsigned idx     = type - MENU_SETTINGS_CORE_OPTION_START;

   kingsn_ctl(KINGSN_CTL_CORE_OPTION_PREV, &idx);

   return 0;
}

static int action_left_core_lock(unsigned type, const char *label,
      bool wraparound)
{
   return action_ok_core_lock(label, label, type, 0, 0);
}

static int disk_options_disk_idx_left(unsigned type, const char *label,
      bool wraparound)
{
   /* Note: Menu itself provides visual feedback - no
    * need to print info message to screen */
   bool print_log = false;

   command_event(CMD_EVENT_DISK_PREV, &print_log);

   return 0;
}

static int action_left_video_gpu_index(unsigned type, const char *label,
      bool wraparound)
{
   enum gfx_ctx_api api = video_context_driver_get_api();

   switch (api)
   {
#ifdef HAVE_VULKAN
      case GFX_CTX_VULKAN_API:
      {
         struct string_list *list = video_driver_get_gpu_api_devices(api);

         if (list)
         {
            settings_t *settings = config_get_ptr();
            int vulkan_gpu_index = settings->ints.vulkan_gpu_index;

            if (vulkan_gpu_index > 0)
            {
               configuration_set_int(settings,
                     settings->ints.vulkan_gpu_index,
                     vulkan_gpu_index - 1);
            }
            else
            {
               configuration_set_int(settings,
                     settings->ints.vulkan_gpu_index,
                     list->size - 1);
            }
         }

         break;
      }
#endif
#ifdef HAVE_D3D10
      case GFX_CTX_DIRECT3D10_API:
      {
         struct string_list *list = video_driver_get_gpu_api_devices(api);

         if (list)
         {
            settings_t *settings = config_get_ptr();
            int d3d10_gpu_index  = settings->ints.d3d10_gpu_index;
            if (d3d10_gpu_index > 0)
            {
               configuration_set_int(settings,
                     settings->ints.d3d10_gpu_index,
                     settings->ints.d3d10_gpu_index - 1);
            }
            else
            {
               configuration_set_int(settings,
                     settings->ints.d3d10_gpu_index,
                     list->size - 1);
            }
         }

         break;
      }
#endif
#ifdef HAVE_D3D11
      case GFX_CTX_DIRECT3D11_API:
      {
         struct string_list *list = video_driver_get_gpu_api_devices(api);

         if (list)
         {
            settings_t *settings = config_get_ptr();
            int d3d11_gpu_index  = settings->ints.d3d11_gpu_index;
            if (d3d11_gpu_index > 0)
            {
               configuration_set_int(settings,
                     settings->ints.d3d11_gpu_index,
                     settings->ints.d3d11_gpu_index - 1);
            }
            else
            {
               configuration_set_int(settings,
                     settings->ints.d3d11_gpu_index,
                     list->size - 1);
            }
         }

         break;
      }
#endif
#ifdef HAVE_D3D12
      case GFX_CTX_DIRECT3D12_API:
      {
         struct string_list *list = video_driver_get_gpu_api_devices(api);

         if (list)
         {
            settings_t *settings = config_get_ptr();
            int d3d12_gpu_index  = settings->ints.d3d12_gpu_index;
            if (d3d12_gpu_index > 0)
            {
               configuration_set_int(settings,
                     settings->ints.d3d12_gpu_index,
                     settings->ints.d3d12_gpu_index - 1);
            }
            else
            {
               configuration_set_int(settings,
                     settings->ints.d3d12_gpu_index,
                     list->size - 1);
            }
         }

         break;
      }
#endif
      default:
         break;
   }

   return 0;
}

static int bind_left_generic(unsigned type, const char *label,
      bool wraparound)
{
   return menu_setting_set(type, MENU_ACTION_LEFT, wraparound);
}

static int menu_cbs_init_bind_left_compare_label(menu_file_list_cbs_t *cbs,
      const char *label, const char *menu_label)
{

   if (cbs->setting)
   {
      const char *parent_group   = cbs->setting->parent_group;

      if (string_is_equal(parent_group,
               msg_hash_to_str(MENU_ENUM_LABEL_MAIN_MENU))
               && (cbs->setting->type == ST_GROUP))
      {
         BIND_ACTION_LEFT(cbs, action_left_mainmenu);
         return 0;
      }
   }

   if (  string_starts_with_size(label, "input_player", STRLEN_CONST("input_player")) && 
         string_ends_with_size(label, "_joypad_index", strlen(label),
            STRLEN_CONST("_joypad_index")))
   {
      unsigned i;
      for (i = 0; i < MAX_USERS; i++)
      {
         char label_setting[128];
         label_setting[0] = '\0';

         snprintf(label_setting,
               sizeof(label_setting), "input_player%d_joypad_index", i + 1);

         if (!string_is_equal(label, label_setting))
            continue;

         BIND_ACTION_LEFT(cbs, bind_left_generic);
         return 0;
      }
   }

   if (string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_PLAYLISTS_TAB)))
   {
      BIND_ACTION_LEFT(cbs, action_left_mainmenu);
      return 0;
   }

   if (strstr(label, "rdb_entry") || string_starts_with_size(label, "content_info", STRLEN_CONST("content_info")))
   {
      BIND_ACTION_LEFT(cbs, action_left_scroll);
   }
   else
   {
      if (cbs->enum_idx != MSG_UNKNOWN)
      {
         switch (cbs->enum_idx)
         {
            case MENU_ENUM_LABEL_SUBSYSTEM_ADD:
            case MENU_ENUM_LABEL_SUBSYSTEM_LOAD:
            case MENU_ENUM_LABEL_CONNECT_NETPLAY_ROOM:
            case MENU_ENUM_LABEL_EXPLORE_ITEM:
            case MENU_ENUM_LABEL_NO_SETTINGS_FOUND:
               BIND_ACTION_LEFT(cbs, action_left_mainmenu);
               break;
            case MENU_ENUM_LABEL_VIDEO_SHADER_SCALE_PASS:
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
               BIND_ACTION_LEFT(cbs, action_left_shader_scale_pass);
#endif
               break;
            case MENU_ENUM_LABEL_VIDEO_SHADER_FILTER_PASS:
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
               BIND_ACTION_LEFT(cbs, action_left_shader_filter_pass);
#endif
               break;
            case MENU_ENUM_LABEL_VIDEO_SHADER_DEFAULT_FILTER:
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
               BIND_ACTION_LEFT(cbs, action_left_shader_filter_default);
#endif
               break;
            case MENU_ENUM_LABEL_VIDEO_SHADER_NUM_PASSES:
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
               BIND_ACTION_LEFT(cbs, action_left_shader_num_passes);
#endif
               break;
            case MENU_ENUM_LABEL_CHEAT_NUM_PASSES:
#ifdef HAVE_CHEATS
               BIND_ACTION_LEFT(cbs, action_left_cheat_num_passes);
#endif
               break;
            case MENU_ENUM_LABEL_SCREEN_RESOLUTION:
               BIND_ACTION_LEFT(cbs, action_left_video_resolution);
               break;
            case MENU_ENUM_LABEL_OPEN_ARCHIVE_DETECT_CORE:
            case MENU_ENUM_LABEL_LOAD_ARCHIVE_DETECT_CORE:
               BIND_ACTION_LEFT(cbs, action_left_scroll);
               break;
            case MENU_ENUM_LABEL_NO_ITEMS:
            case MENU_ENUM_LABEL_NO_PLAYLIST_ENTRIES_AVAILABLE:
               if (
                        string_ends_with_size(menu_label, "_tab",
                           strlen(menu_label),
                           STRLEN_CONST("_tab")
                           )
                     || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_MAIN_MENU))
                     || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_HORIZONTAL_MENU))
                  )
               {
                  BIND_ACTION_LEFT(cbs, action_left_mainmenu);
               }
               else
               {
                  BIND_ACTION_LEFT(cbs, action_left_scroll);
               }
               break;
            case MENU_ENUM_LABEL_START_VIDEO_PROCESSOR:
            case MENU_ENUM_LABEL_TAKE_SCREENSHOT:
               if (
                        string_ends_with_size(menu_label, "_tab",
                           strlen(menu_label),
                           STRLEN_CONST("_tab"))
                     || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_HORIZONTAL_MENU))
                  )
               {
                  BIND_ACTION_LEFT(cbs, action_left_mainmenu);
                  break;
               }
            case MENU_ENUM_LABEL_VIDEO_GPU_INDEX:
               BIND_ACTION_LEFT(cbs, action_left_video_gpu_index);
               break;
            case MENU_ENUM_LABEL_PLAYLIST_MANAGER_DEFAULT_CORE:
               BIND_ACTION_LEFT(cbs, playlist_association_left);
               break;
            case MENU_ENUM_LABEL_PLAYLIST_MANAGER_LABEL_DISPLAY_MODE:
               BIND_ACTION_LEFT(cbs, playlist_label_display_mode_left);
               break;
            case MENU_ENUM_LABEL_PLAYLIST_MANAGER_RIGHT_THUMBNAIL_MODE:
               BIND_ACTION_LEFT(cbs, playlist_right_thumbnail_mode_left);
               break;
            case MENU_ENUM_LABEL_PLAYLIST_MANAGER_LEFT_THUMBNAIL_MODE:
               BIND_ACTION_LEFT(cbs, playlist_left_thumbnail_mode_left);
               break;
            case MENU_ENUM_LABEL_PLAYLIST_MANAGER_SORT_MODE:
               BIND_ACTION_LEFT(cbs, playlist_sort_mode_left);
               break;
            case MENU_ENUM_LABEL_MANUAL_CONTENT_SCAN_SYSTEM_NAME:
               BIND_ACTION_LEFT(cbs, manual_content_scan_system_name_left);
               break;
            case MENU_ENUM_LABEL_MANUAL_CONTENT_SCAN_CORE_NAME:
               BIND_ACTION_LEFT(cbs, manual_content_scan_core_name_left);
               break;
            default:
               return -1;
         }
      }
      else
      {
         return -1;
      }
   }

   return 0;
}

static int menu_cbs_init_bind_left_compare_type(menu_file_list_cbs_t *cbs,
      unsigned type, const char *menu_label)
{
#ifdef HAVE_CHEATS
   if (type >= MENU_SETTINGS_CHEAT_BEGIN
         && type <= MENU_SETTINGS_CHEAT_END)
   {
      BIND_ACTION_LEFT(cbs, action_left_cheat);
   } else
#endif
#ifdef HAVE_AUDIOMIXER
   if (type >= MENU_SETTINGS_AUDIO_MIXER_STREAM_ACTIONS_VOLUME_BEGIN
         && type <= MENU_SETTINGS_AUDIO_MIXER_STREAM_ACTIONS_VOLUME_END)
   {
      BIND_ACTION_LEFT(cbs, audio_mixer_stream_volume_left);
   } else
#endif
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_SLANG) || defined(HAVE_HLSL)
   if (type >= MENU_SETTINGS_SHADER_PARAMETER_0
         && type <= MENU_SETTINGS_SHADER_PARAMETER_LAST)
   {
      BIND_ACTION_LEFT(cbs, shader_action_parameter_left);
   }
   else if (type >= MENU_SETTINGS_SHADER_PRESET_PARAMETER_0
         && type <= MENU_SETTINGS_SHADER_PRESET_PARAMETER_LAST)
   {
      BIND_ACTION_LEFT(cbs, shader_action_preset_parameter_left);
   } else
#endif
   if (type >= MENU_SETTINGS_INPUT_DESC_BEGIN
         && type <= MENU_SETTINGS_INPUT_DESC_END)
   {
      BIND_ACTION_LEFT(cbs, action_left_input_desc);
   }
   else if (type >= MENU_SETTINGS_INPUT_DESC_KBD_BEGIN
      && type <= MENU_SETTINGS_INPUT_DESC_KBD_END)
   {
      BIND_ACTION_LEFT(cbs, action_left_input_desc_kbd);
   }
   else if ((type >= MENU_SETTINGS_CORE_OPTION_START) &&
            (type < MENU_SETTINGS_CHEEVOS_START))
   {
      BIND_ACTION_LEFT(cbs, core_setting_left);
   }
   else
   {
      switch (type)
      {
         case MENU_SETTINGS_CORE_DISK_OPTIONS_DISK_INDEX:
            BIND_ACTION_LEFT(cbs, disk_options_disk_idx_left);
            break;
         case FILE_TYPE_PLAIN:
         case FILE_TYPE_DIRECTORY:
         case FILE_TYPE_CARCHIVE:
         case FILE_TYPE_IN_CARCHIVE:
         case FILE_TYPE_CORE:
         case FILE_TYPE_RDB:
         case FILE_TYPE_RDB_ENTRY:
         case FILE_TYPE_RPL_ENTRY:
         case FILE_TYPE_CURSOR:
         case FILE_TYPE_SHADER:
         case FILE_TYPE_SHADER_PRESET:
         case FILE_TYPE_IMAGE:
         case FILE_TYPE_OVERLAY:
#ifdef HAVE_VIDEO_LAYOUT
         case FILE_TYPE_VIDEO_LAYOUT:
#endif
         case FILE_TYPE_VIDEOFILTER:
         case FILE_TYPE_AUDIOFILTER:
         case FILE_TYPE_CONFIG:
         case FILE_TYPE_USE_DIRECTORY:
         case FILE_TYPE_PLAYLIST_ENTRY:
         case MENU_INFO_MESSAGE:
         case FILE_TYPE_DOWNLOAD_CORE:
         case FILE_TYPE_CHEAT:
         case FILE_TYPE_REMAP:
         case FILE_TYPE_MOVIE:
         case FILE_TYPE_MUSIC:
         case FILE_TYPE_IMAGEVIEWER:
         case FILE_TYPE_PLAYLIST_COLLECTION:
         case FILE_TYPE_DOWNLOAD_CORE_CONTENT:
         case FILE_TYPE_DOWNLOAD_THUMBNAIL_CONTENT:
         case FILE_TYPE_DOWNLOAD_URL:
         case FILE_TYPE_SCAN_DIRECTORY:
         case FILE_TYPE_MANUAL_SCAN_DIRECTORY:
         case FILE_TYPE_FONT:
         case MENU_SETTING_GROUP:
         case MENU_SETTINGS_CORE_INFO_NONE:
            if (  
                  string_ends_with_size(menu_label, "_tab",
                     strlen(menu_label), STRLEN_CONST("_tab"))
                  || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_HORIZONTAL_MENU))
               )
            {
               BIND_ACTION_LEFT(cbs, action_left_mainmenu);
               break;
            }
            BIND_ACTION_LEFT(cbs, action_left_scroll);
            break;
         case MENU_SETTING_ACTION:
         case FILE_TYPE_CONTENTLIST_ENTRY:
            BIND_ACTION_LEFT(cbs, action_left_mainmenu);
            break;
         case MENU_SETTING_ACTION_CORE_LOCK:
            BIND_ACTION_LEFT(cbs, action_left_core_lock);
            break;
         case MENU_SETTING_DROPDOWN_ITEM_INPUT_DESCRIPTION:
         case MENU_SETTING_DROPDOWN_ITEM_INPUT_DESCRIPTION_KBD:
            BIND_ACTION_LEFT(cbs, action_left_scroll);
            break;
         default:
            return -1;
      }
   }

   return 0;
}

int menu_cbs_init_bind_left(menu_file_list_cbs_t *cbs,
      const char *path, const char *label, unsigned type, size_t idx,
      const char *menu_label)
{
   if (!cbs)
      return -1;

   BIND_ACTION_LEFT(cbs, bind_left_generic);

   if (type == MENU_SETTING_NO_ITEM)
   {
      if (  
               string_ends_with_size(menu_label, "_tab",
                  strlen(menu_label), STRLEN_CONST("_tab"))
            || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_MAIN_MENU))
            || string_is_equal(menu_label, msg_hash_to_str(MENU_ENUM_LABEL_HORIZONTAL_MENU))
         )
      {
            BIND_ACTION_LEFT(cbs, action_left_mainmenu);
            return 0;
      }
   }

   if (menu_cbs_init_bind_left_compare_label(cbs, label, menu_label) == 0)
      return 0;

   if (menu_cbs_init_bind_left_compare_type(cbs, type, menu_label) == 0)
      return 0;

   return -1;
}
