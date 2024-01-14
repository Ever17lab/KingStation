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

#ifndef INTERNAL_CORES_H__
#define INTERNAL_CORES_H__

#include <boolean.h>
#include <libks.h>
#include <ks_common_api.h>
#include <ks_environment.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

KS_BEGIN_DECLS

void libks_dummy_ks_init(void);

void libks_dummy_ks_deinit(void);

unsigned libks_dummy_ks_api_version(void);

void libks_dummy_ks_get_system_info(struct ks_system_info *info);

void libks_dummy_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_dummy_ks_set_environment(ks_environment_t cb);

void libks_dummy_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_dummy_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_dummy_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_dummy_ks_set_input_poll(ks_input_poll_t cb);

void libks_dummy_ks_set_input_state(ks_input_state_t cb);

void libks_dummy_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_dummy_ks_reset(void);

void libks_dummy_ks_run(void);

size_t libks_dummy_ks_serialize_size(void);

bool libks_dummy_ks_serialize(void *data, size_t size);

bool libks_dummy_ks_unserialize(const void *data, size_t size);

void libks_dummy_ks_cheat_reset(void);

void libks_dummy_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_dummy_ks_load_game(const struct ks_game_info *game);

bool libks_dummy_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_dummy_ks_unload_game(void);

unsigned libks_dummy_ks_get_region(void);

void *libks_dummy_ks_get_memory_data(unsigned id);

size_t libks_dummy_ks_get_memory_size(unsigned id);

#ifdef HAVE_FFMPEG
/* Internal ffmpeg core. */

void libks_ffmpeg_ks_init(void);

void libks_ffmpeg_ks_deinit(void);

unsigned libks_ffmpeg_ks_api_version(void);

void libks_ffmpeg_ks_get_system_info(struct ks_system_info *info);

void libks_ffmpeg_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_ffmpeg_ks_set_environment(ks_environment_t cb);

void libks_ffmpeg_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_ffmpeg_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_ffmpeg_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_ffmpeg_ks_set_input_poll(ks_input_poll_t cb);

void libks_ffmpeg_ks_set_input_state(ks_input_state_t cb);

void libks_ffmpeg_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_ffmpeg_ks_reset(void);

void libks_ffmpeg_ks_run(void);

size_t libks_ffmpeg_ks_serialize_size(void);

bool libks_ffmpeg_ks_serialize(void *data, size_t size);

bool libks_ffmpeg_ks_unserialize(const void *data, size_t size);

void libks_ffmpeg_ks_cheat_reset(void);

void libks_ffmpeg_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_ffmpeg_ks_load_game(const struct ks_game_info *game);

bool libks_ffmpeg_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_ffmpeg_ks_unload_game(void);

unsigned libks_ffmpeg_ks_get_region(void);

void *libks_ffmpeg_ks_get_memory_data(unsigned id);

size_t libks_ffmpeg_ks_get_memory_size(unsigned id);

#endif

#ifdef HAVE_MPV
/* Internal mpv core. */

void libks_mpv_ks_init(void);

void libks_mpv_ks_deinit(void);

unsigned libks_mpv_ks_api_version(void);

void libks_mpv_ks_get_system_info(struct ks_system_info *info);

void libks_mpv_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_mpv_ks_set_environment(ks_environment_t cb);

void libks_mpv_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_mpv_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_mpv_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_mpv_ks_set_input_poll(ks_input_poll_t cb);

void libks_mpv_ks_set_input_state(ks_input_state_t cb);

void libks_mpv_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_mpv_ks_reset(void);

void libks_mpv_ks_run(void);

size_t libks_mpv_ks_serialize_size(void);

bool libks_mpv_ks_serialize(void *data, size_t size);

bool libks_mpv_ks_unserialize(const void *data, size_t size);

void libks_mpv_ks_cheat_reset(void);

void libks_mpv_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_mpv_ks_load_game(const struct ks_game_info *game);

bool libks_mpv_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_mpv_ks_unload_game(void);

unsigned libks_mpv_ks_get_region(void);

void *libks_mpv_ks_get_memory_data(unsigned id);

size_t libks_mpv_ks_get_memory_size(unsigned id);

#endif

#ifdef HAVE_IMAGEVIEWER
/* Internal image viewer core. */

void libks_imageviewer_ks_init(void);

void libks_imageviewer_ks_deinit(void);

unsigned libks_imageviewer_ks_api_version(void);

void libks_imageviewer_ks_get_system_info(struct ks_system_info *info);

void libks_imageviewer_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_imageviewer_ks_set_environment(ks_environment_t cb);

void libks_imageviewer_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_imageviewer_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_imageviewer_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_imageviewer_ks_set_input_poll(ks_input_poll_t cb);

void libks_imageviewer_ks_set_input_state(ks_input_state_t cb);

void libks_imageviewer_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_imageviewer_ks_reset(void);

void libks_imageviewer_ks_run(void);

size_t libks_imageviewer_ks_serialize_size(void);

bool libks_imageviewer_ks_serialize(void *data, size_t size);

bool libks_imageviewer_ks_unserialize(const void *data, size_t size);

void libks_imageviewer_ks_cheat_reset(void);

void libks_imageviewer_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_imageviewer_ks_load_game(const struct ks_game_info *game);

bool libks_imageviewer_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_imageviewer_ks_unload_game(void);

unsigned libks_imageviewer_ks_get_region(void);

void *libks_imageviewer_ks_get_memory_data(unsigned id);

size_t libks_imageviewer_ks_get_memory_size(unsigned id);

#endif

#if defined(HAVE_NETWORKGAMEPAD) && defined(HAVE_NETWORKING)
/* Internal networked kspad core. */

void libks_netkspad_ks_init(void);

void libks_netkspad_ks_deinit(void);

unsigned libks_netkspad_ks_api_version(void);

void libks_netkspad_ks_get_system_info(struct ks_system_info *info);

void libks_netkspad_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_netkspad_ks_set_environment(ks_environment_t cb);

void libks_netkspad_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_netkspad_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_netkspad_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_netkspad_ks_set_input_poll(ks_input_poll_t cb);

void libks_netkspad_ks_set_input_state(ks_input_state_t cb);

void libks_netkspad_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_netkspad_ks_reset(void);

void libks_netkspad_ks_run(void);

size_t libks_netkspad_ks_serialize_size(void);

bool libks_netkspad_ks_serialize(void *data, size_t size);

bool libks_netkspad_ks_unserialize(const void *data, size_t size);

void libks_netkspad_ks_cheat_reset(void);

void libks_netkspad_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_netkspad_ks_load_game(const struct ks_game_info *game);

bool libks_netkspad_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_netkspad_ks_unload_game(void);

unsigned libks_netkspad_ks_get_region(void);

void *libks_netkspad_ks_get_memory_data(unsigned id);

size_t libks_netkspad_ks_get_memory_size(unsigned id);

#endif

#if defined(HAVE_V4L2)
/* Internal video processor core. */

void libks_videoprocessor_ks_init(void);

void libks_videoprocessor_ks_deinit(void);

unsigned libks_videoprocessor_ks_api_version(void);

void libks_videoprocessor_ks_get_system_info(struct ks_system_info *info);

void libks_videoprocessor_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_videoprocessor_ks_set_environment(ks_environment_t cb);

void libks_videoprocessor_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_videoprocessor_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_videoprocessor_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_videoprocessor_ks_set_input_poll(ks_input_poll_t cb);

void libks_videoprocessor_ks_set_input_state(ks_input_state_t cb);

void libks_videoprocessor_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_videoprocessor_ks_reset(void);

void libks_videoprocessor_ks_run(void);

size_t libks_videoprocessor_ks_serialize_size(void);

bool libks_videoprocessor_ks_serialize(void *data, size_t size);

bool libks_videoprocessor_ks_unserialize(const void *data, size_t size);

void libks_videoprocessor_ks_cheat_reset(void);

void libks_videoprocessor_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_videoprocessor_ks_load_game(const struct ks_game_info *game);

bool libks_videoprocessor_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_videoprocessor_ks_unload_game(void);

unsigned libks_videoprocessor_ks_get_region(void);

void *libks_videoprocessor_ks_get_memory_data(unsigned id);

size_t libks_videoprocessor_ks_get_memory_size(unsigned id);

#endif

#ifdef HAVE_GONG
/* Internal gong core. */

void libks_gong_ks_init(void);

void libks_gong_ks_deinit(void);

unsigned libks_gong_ks_api_version(void);

void libks_gong_ks_get_system_info(struct ks_system_info *info);

void libks_gong_ks_get_system_av_info(struct ks_system_av_info *info);

void libks_gong_ks_set_environment(ks_environment_t cb);

void libks_gong_ks_set_video_refresh(ks_video_refresh_t cb);

void libks_gong_ks_set_audio_sample(ks_audio_sample_t cb);

void libks_gong_ks_set_audio_sample_batch(ks_audio_sample_batch_t cb);

void libks_gong_ks_set_input_poll(ks_input_poll_t cb);

void libks_gong_ks_set_input_state(ks_input_state_t cb);

void libks_gong_ks_set_controller_port_device(unsigned port, unsigned device);

void libks_gong_ks_reset(void);

void libks_gong_ks_run(void);

size_t libks_gong_ks_serialize_size(void);

bool libks_gong_ks_serialize(void *data, size_t size);

bool libks_gong_ks_unserialize(const void *data, size_t size);

void libks_gong_ks_cheat_reset(void);

void libks_gong_ks_cheat_set(unsigned index, bool enabled, const char *code);

bool libks_gong_ks_load_game(const struct ks_game_info *game);

bool libks_gong_ks_load_game_special(unsigned game_type,
      const struct ks_game_info *info, size_t num_info);

void libks_gong_ks_unload_game(void);

unsigned libks_gong_ks_get_region(void);

void *libks_gong_ks_get_memory_data(unsigned id);

size_t libks_gong_ks_get_memory_size(unsigned id);

#endif

KS_END_DECLS

#endif
