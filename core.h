/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2016 - Daniel De Matteis
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

#ifndef _LIBKS_CORE_IMPL_H
#define _LIBKS_CORE_IMPL_H

#include <boolean.h>
#include <libks.h>

#include <ks_common_api.h>

#include "core_type.h"
#include "input/input_defines.h"
#include "disk_control_interface.h"

KS_BEGIN_DECLS

enum
{
   /* Polling is performed before
    * call to ks_run. */
   POLL_TYPE_EARLY = 0,

   /* Polling is performed when requested. */
   POLL_TYPE_NORMAL,

   /* Polling is performed on first call to
    * ks_input_state per frame. */
   POLL_TYPE_LATE
};

typedef struct kingsn_memory_descriptor
{
   struct ks_memory_descriptor core;        /* uint64_t alignment */
   size_t disconnect_mask;
} kingsn_memory_descriptor_t;

typedef struct kingsn_memory_map
{
   kingsn_memory_descriptor_t *descriptors;
   unsigned num_descriptors;
} kingsn_memory_map_t;

typedef struct kingsn_system_info
{
   struct ks_location_callback location_cb; /* ptr alignment */
   disk_control_interface_t disk_control;      /* ptr alignment */
   struct ks_system_info info;              /* ptr alignment */
   kingsn_memory_map_t mmaps;                   /* ptr alignment */
   const char *input_desc_btn[MAX_USERS][KINGSN_FIRST_META_KEY];
   struct
   {
      struct ks_subsystem_info *data;
      unsigned size;
   } subsystem;
   struct
   {
      struct ks_controller_info *data;
      unsigned size;
   } ports;
   unsigned rotation;
   unsigned performance_level;
   char valid_extensions[255];
   bool load_no_content;
   bool supports_vfs;
} kingsn_system_info_t;

typedef struct ks_ctx_input_state_info
{
   ks_input_state_t cb;
} ks_ctx_input_state_info_t;

typedef struct ks_ctx_cheat_info
{
   const char *code;
   unsigned index;
   bool enabled;
} ks_ctx_cheat_info_t;

typedef struct ks_ctx_api_info
{
   unsigned version;
} ks_ctx_api_info_t;

typedef struct ks_ctx_region_info
{
  unsigned region;
} ks_ctx_region_info_t;

typedef struct ks_ctx_controller_info
{
   unsigned port;
   unsigned device;
} ks_ctx_controller_info_t;

typedef struct ks_ctx_memory_info
{
   void *data;
   size_t size;
   unsigned id;
} ks_ctx_memory_info_t;

typedef struct ks_ctx_load_content_info
{
   struct ks_game_info *info;
   const struct string_list *content;
   const struct ks_subsystem_info *special;
} ks_ctx_load_content_info_t;

typedef struct ks_ctx_serialize_info
{
   const void *data_const;
   void *data;
   size_t size;
} ks_ctx_serialize_info_t;

typedef struct ks_ctx_size_info
{
   size_t size;
} ks_ctx_size_info_t;

typedef struct ks_ctx_environ_info
{
   ks_environment_t env;
} ks_ctx_environ_info_t;

typedef struct ks_callbacks
{
   ks_video_refresh_t frame_cb;
   ks_audio_sample_t sample_cb;
   ks_audio_sample_batch_t sample_batch_cb;
   ks_input_state_t state_cb;
   ks_input_poll_t poll_cb;
} ks_callbacks_t;

bool core_set_default_callbacks(struct ks_callbacks *cbs);

#ifdef HAVE_REWIND
bool core_set_rewind_callbacks(void);
#endif

#ifdef HAVE_NETWORKING
bool core_set_netplay_callbacks(void);

bool core_unset_netplay_callbacks(void);
#endif

bool core_set_poll_type(unsigned type);

/* Runs the core for one frame. */
bool core_run(void);

bool core_reset(void);

bool core_serialize_size(ks_ctx_size_info_t *info);

uint64_t core_serialization_quirks(void);

bool core_serialize(ks_ctx_serialize_info_t *info);

bool core_unserialize(ks_ctx_serialize_info_t *info);

bool core_set_cheat(ks_ctx_cheat_info_t *info);

bool core_reset_cheat(void);

bool core_get_memory(ks_ctx_memory_info_t *info);

/* Get system A/V information. */
bool core_get_system_info(struct ks_system_info *system);

bool core_load_game(ks_ctx_load_content_info_t *load_info);

bool core_set_controller_port_device(ks_ctx_controller_info_t *pad);

bool core_has_set_input_descriptor(void);

KS_END_DECLS

#endif
