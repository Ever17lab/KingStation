/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2017 - Higor Euripedes
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
#ifndef TASKS_HANDLER_AUDIO_MIXER_H
#define TASKS_HANDLER_AUDIO_MIXER_H

#include <stdint.h>

#include <boolean.h>
#include <ks_common_api.h>
#include <ks_miscellaneous.h>

#include <queues/task_queue.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "../KingStation.h"

KS_BEGIN_DECLS

bool task_audio_mixer_load_handler(ks_task_t *task);

bool task_push_audio_mixer_load_and_play(
      const char *fullpath, ks_task_callback_t cb, void *user_data,
      bool system,
      enum audio_mixer_slot_selection_type slot_selection_type,
      int slot_selection_idx);

bool task_push_audio_mixer_load(
      const char *fullpath, ks_task_callback_t cb, void *user_data,
      bool system,
      enum audio_mixer_slot_selection_type slot_selection_type,
      int slot_selection_idx);

KS_END_DECLS

#endif
