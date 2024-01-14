/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2012-2015 - Michael Lelli
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libks.h>

#if defined(HAVE_MENU) && defined(HAVE_RGUI)
#include <string/stdstring.h>
#include "../configuration.h"
#include "../menu/menu_defines.h"
#endif

#include "internal_cores.h"

static uint16_t *dummy_frame_buf;

static uint16_t frame_buf_width;
static uint16_t frame_buf_height;

#if defined(HAVE_LIBNX) && defined(HAVE_STATIC_DUMMY)
void ks_init(void) { libks_dummy_ks_init(); }
void ks_deinit(void) { libks_dummy_ks_deinit(); }
unsigned ks_api_version(void) { return libks_dummy_ks_api_version(); }
void ks_set_controller_port_device(unsigned port, unsigned device) { libks_dummy_ks_set_controller_port_device(port, device); }
void ks_get_system_info(struct ks_system_info *info) { libks_dummy_ks_get_system_info(info); }
void ks_get_system_av_info(struct ks_system_av_info *info) { ks_get_system_av_info(info); }
void ks_set_environment(ks_environment_t cb) { libks_dummy_ks_set_environment(cb); }
void ks_set_audio_sample(ks_audio_sample_t cb) { libks_dummy_ks_set_audio_sample(cb); }
void ks_set_audio_sample_batch(ks_audio_sample_batch_t cb) { libks_dummy_ks_set_audio_sample_batch(cb); }
void ks_set_input_poll(ks_input_poll_t cb) { libks_dummy_ks_set_input_poll(cb); }
void ks_set_input_state(ks_input_state_t cb) { libks_dummy_ks_set_input_state(cb); }
void ks_set_video_refresh(ks_video_refresh_t cb) { libks_dummy_ks_set_video_refresh(cb); }
void ks_reset(void) { libks_dummy_ks_reset(); }
void ks_run(void) { libks_dummy_ks_run(); }
bool ks_load_game(const struct ks_game_info *info) { return libks_dummy_ks_load_game(info); }
void ks_unload_game(void) { libks_dummy_ks_unload_game(); }
unsigned ks_get_region(void) { return libks_dummy_ks_get_region(); }
bool ks_load_game_special(unsigned type, const struct ks_game_info *info, size_t num) { return libks_dummy_ks_load_game_special(type, info, num); }
size_t ks_serialize_size(void) { return libks_dummy_ks_serialize_size(); }
bool ks_serialize(void *data, size_t size) { return libks_dummy_ks_serialize(data, size); }
bool ks_unserialize(const void *data, size_t size) { return libks_dummy_ks_unserialize(data, size); }
void *ks_get_memory_data(unsigned id) { return libks_dummy_ks_get_memory_data(id); }
size_t ks_get_memory_size(unsigned id) { return libks_dummy_ks_get_memory_size(id); }
void ks_cheat_reset(void) { libks_dummy_ks_cheat_reset(); }
void ks_cheat_set(unsigned idx, bool enabled, const char *code) { libks_dummy_ks_cheat_set(idx, enabled, code); }
#endif

void libks_dummy_ks_init(void)
{
#if defined(HAVE_MENU) && defined(HAVE_RGUI)
   settings_t *settings = config_get_ptr();
#endif
   unsigned i;

   /* Sensible defaults */
   frame_buf_width = 320;
   frame_buf_height = 240;

#if defined(HAVE_MENU) && defined(HAVE_RGUI)
   if (string_is_equal(settings->arrays.menu_driver, "rgui"))
   {
      switch (settings->uints.menu_rgui_aspect_ratio)
      {
         case RGUI_ASPECT_RATIO_16_9:
         case RGUI_ASPECT_RATIO_16_9_CENTRE:
            frame_buf_width = 426;
            break;
         case RGUI_ASPECT_RATIO_16_10:
         case RGUI_ASPECT_RATIO_16_10_CENTRE:
            frame_buf_width = 384;
            break;
         default:
            /* 4:3 */
            frame_buf_width = 320;
            break;
      }
   }
#endif

   dummy_frame_buf = (uint16_t*)calloc(frame_buf_width * frame_buf_height, sizeof(uint16_t));
   for (i = 0; i < (unsigned)(frame_buf_width * frame_buf_height); i++)
      dummy_frame_buf[i] = 4 << 5;
}

void libks_dummy_ks_deinit(void)
{
   if (dummy_frame_buf)
      free(dummy_frame_buf);
   dummy_frame_buf = NULL;
}

unsigned libks_dummy_ks_api_version(void)
{
   return KS_API_VERSION;
}

void libks_dummy_ks_set_controller_port_device(
      unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

void libks_dummy_ks_get_system_info(
      struct ks_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "";
   info->library_version  = "";
   info->need_fullpath    = false;
   info->valid_extensions = ""; /* Nothing. */
}

static ks_video_refresh_t dummy_video_cb;
static ks_audio_sample_t dummy_audio_cb;
static ks_audio_sample_batch_t dummy_audio_batch_cb;
static ks_environment_t dummy_environ_cb;
static ks_input_poll_t dummy_input_poll_cb;
static ks_input_state_t dummy_input_state_cb;

/* Doesn't really matter, but need something sane. */
void libks_dummy_ks_get_system_av_info(
      struct ks_system_av_info *info)
{
   float refresh_rate = 0.0;
   if (!dummy_environ_cb(KS_ENVIRONMENT_GET_TARGET_REFRESH_RATE, &refresh_rate))
      refresh_rate    = 60.0;

   info->timing.fps           = refresh_rate;
   info->timing.sample_rate   = 48000.0;

   info->geometry.base_width  = frame_buf_width;
   info->geometry.base_height = frame_buf_height;
   info->geometry.max_width   = frame_buf_width;
   info->geometry.max_height  = frame_buf_height;
   info->geometry.aspect_ratio = (float)frame_buf_width / (float)frame_buf_height;
}

void libks_dummy_ks_set_environment(ks_environment_t cb)
{
   enum ks_pixel_format fmt = KS_PIXEL_FORMAT_RGB565;

   dummy_environ_cb = cb;

   /* We know it's supported, it's internal to KingStation. */
   dummy_environ_cb(KS_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

void libks_dummy_ks_set_audio_sample(ks_audio_sample_t cb)
{
   dummy_audio_cb = cb;
}

void libks_dummy_ks_set_audio_sample_batch(
      ks_audio_sample_batch_t cb)
{
   dummy_audio_batch_cb = cb;
}

void libks_dummy_ks_set_input_poll(ks_input_poll_t cb)
{
   dummy_input_poll_cb = cb;
}

void libks_dummy_ks_set_input_state(ks_input_state_t cb)
{
   dummy_input_state_cb = cb;
}

void libks_dummy_ks_set_video_refresh(ks_video_refresh_t cb)
{
   dummy_video_cb = cb;
}

void libks_dummy_ks_reset(void)
{}

void libks_dummy_ks_run(void)
{
   dummy_input_poll_cb();
   dummy_video_cb(dummy_frame_buf, frame_buf_width, frame_buf_height, 2 * frame_buf_width);
}

/* This should never be called, it's only used as a placeholder. */
bool libks_dummy_ks_load_game(const struct ks_game_info *info)
{
   (void)info;
   return false;
}

void libks_dummy_ks_unload_game(void)
{}

unsigned libks_dummy_ks_get_region(void)
{
   return KS_REGION_NTSC;
}

bool libks_dummy_ks_load_game_special(unsigned type,
      const struct ks_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t libks_dummy_ks_serialize_size(void)
{
   return 0;
}

bool libks_dummy_ks_serialize(void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

bool libks_dummy_ks_unserialize(const void *data,
      size_t size)
{
   (void)data;
   (void)size;
   return false;
}

void *libks_dummy_ks_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t libks_dummy_ks_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void libks_dummy_ks_cheat_reset(void)
{}

void libks_dummy_ks_cheat_set(unsigned idx,
      bool enabled, const char *code)
{
   (void)idx;
   (void)enabled;
   (void)code;
}
