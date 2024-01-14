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

#include "../menu_driver.h"
#include "../menu_cbs.h"
#include "../../configuration.h"


#ifndef BIND_ACTION_UP
#define BIND_ACTION_UP(cbs, name) (cbs)->action_up = (name)
#endif

static int action_bind_up_generic(unsigned type, const char *label)
{
   size_t scroll_accel    = 0;
   unsigned scroll_speed  = 0;
#ifdef HAVE_AUDIOMIXER
   settings_t *settings                  = config_get_ptr();
   bool audio_enable_menu                = settings->bools.audio_enable_menu;
   bool audio_enable_menu_ok         = settings->bools.audio_enable_menu_ok;

   if (audio_enable_menu && audio_enable_menu_ok)
      audio_driver_mixer_play_menu_sound(AUDIO_MIXER_SYSTEM_SLOT_OK);
#endif	
   if (!menu_driver_ctl(MENU_NAVIGATION_CTL_GET_SCROLL_ACCEL, &scroll_accel))
      return -1;

   scroll_speed           = (unsigned)((MAX(scroll_accel, 2) - 2) / 4 + 1);

   if (menu_entries_get_size() <= 0)
      return 0;

   menu_driver_ctl(MENU_NAVIGATION_CTL_DECREMENT, &scroll_speed);

   return 0;
}

int menu_cbs_init_bind_up(menu_file_list_cbs_t *cbs,
      const char *path, const char *label, unsigned type, size_t idx)
{
   if (!cbs)
      return -1;

   BIND_ACTION_UP(cbs, action_bind_up_generic);

   return -1;
}
