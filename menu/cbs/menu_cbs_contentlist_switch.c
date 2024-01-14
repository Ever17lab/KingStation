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

#ifndef BIND_ACTION_CONTENT_LIST_SWITCH
#define BIND_ACTION_CONTENT_LIST_SWITCH(cbs, name) (cbs)->action_content_list_switch = (name)
#endif

static int deferred_push_content_list(void *data, void *userdata,
      const char *path,
      const char *label, unsigned type)
{
   file_list_t *selection_buf = menu_entries_get_selection_buf_ptr(0);

   /* Must clear any existing menu search terms
    * when switching 'tabs', since doing so
    * bypasses standard backwards navigation
    * (i.e. 'cancel' actions would normally
    * pop the search stack - this will not
    * happen if we jump to a new list directly) */
   menu_driver_search_clear();

   menu_navigation_set_selection(0);
   return action_refresh_default((file_list_t*)data, selection_buf);
}

int menu_cbs_init_bind_content_list_switch(menu_file_list_cbs_t *cbs,
      const char *path, const char *label, unsigned type, size_t idx)
{
   if (!cbs)
      return -1;

   BIND_ACTION_CONTENT_LIST_SWITCH(cbs, deferred_push_content_list);

   return -1;
}
