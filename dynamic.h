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

#ifndef __DYNAMIC_H
#define __DYNAMIC_H

#include <boolean.h>
#include <ks_common_api.h>
#include <libks.h>

#include "core_type.h"

KS_BEGIN_DECLS

/**
 * libks_free_system_info:
 * @info                         : Pointer to system info information.
 *
 * Frees system information.
 **/
void libks_free_system_info(struct ks_system_info *info);

const struct ks_subsystem_info *libks_find_subsystem_info(
      const struct ks_subsystem_info *info,
      unsigned num_info, const char *ident);

/**
 * libks_find_controller_description:
 * @info                         : Pointer to controller info handle.
 * @id                           : Identifier of controller to search
 *                                 for.
 *
 * Search for a controller of type @id in @info.
 *
 * Returns: controller description of found controller on success,
 * otherwise NULL.
 **/
const struct ks_controller_description *
   libks_find_controller_description(
         const struct ks_controller_info *info, unsigned id);

struct ks_core_t
{
   uint64_t serialization_quirks_v;
   void (*ks_init)(void);
   void (*ks_deinit)(void);
   unsigned (*ks_api_version)(void);
   void (*ks_get_system_info)(struct ks_system_info*);
   void (*ks_get_system_av_info)(struct ks_system_av_info*);
   void (*ks_set_environment)(ks_environment_t);
   void (*ks_set_video_refresh)(ks_video_refresh_t);
   void (*ks_set_audio_sample)(ks_audio_sample_t);
   void (*ks_set_audio_sample_batch)(ks_audio_sample_batch_t);
   void (*ks_set_input_poll)(ks_input_poll_t);
   void (*ks_set_input_state)(ks_input_state_t);
   void (*ks_set_controller_port_device)(unsigned, unsigned);
   void (*ks_reset)(void);
   void (*ks_run)(void);
   size_t (*ks_serialize_size)(void);
   bool (*ks_serialize)(void*, size_t);
   bool (*ks_unserialize)(const void*, size_t);
   void (*ks_cheat_reset)(void);
   void (*ks_cheat_set)(unsigned, bool, const char*);
   bool (*ks_load_game)(const struct ks_game_info*);
   bool (*ks_load_game_special)(unsigned,
         const struct ks_game_info*, size_t);
   void (*ks_unload_game)(void);
   unsigned (*ks_get_region)(void);
   void *(*ks_get_memory_data)(unsigned);
   size_t (*ks_get_memory_size)(unsigned);

   unsigned poll_type;
   bool inited;
   bool symbols_inited;
   bool game_loaded;
   bool input_polled;
   bool has_set_subsystems;
   bool has_set_input_descriptors;
};

bool libks_get_shared_context(void);

/* Arbitrary twenty subsystems limite */
#define SUBSYSTEM_MAX_SUBSYSTEMS 20
/* Arbitrary 10 roms for each subsystem limit */
#define SUBSYSTEM_MAX_SUBSYSTEM_ROMS 10

/* TODO/FIXME - globals */
extern struct ks_subsystem_info subsystem_data[SUBSYSTEM_MAX_SUBSYSTEMS];
extern unsigned subsystem_current_count;

KS_END_DECLS

#endif