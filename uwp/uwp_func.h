/*  KingStation - A frontend for libks.
 *  Copyright (C) 2018 - Krzysztof Haładyn
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

#ifndef _UWP_FUNC_H
#define _UWP_FUNC_H

#include <ks_miscellaneous.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char uwp_dir_install[PATH_MAX_LENGTH];
extern char uwp_dir_data[PATH_MAX_LENGTH];
extern char uwp_device_family[128];

void uwp_open_broadfilesystemaccess_settings(void);
bool uwp_drive_exists(const char *path);
char* uwp_trigger_picker(void);

void* uwp_get_corewindow(void);

void uwp_input_next_frame(void *data);
bool uwp_keyboard_pressed(unsigned key);
int16_t uwp_mouse_state(unsigned port, unsigned id, bool screen);
int16_t uwp_pointer_state(unsigned idx, unsigned id, bool screen);
const char* uwp_get_cpu_model_name(void);
enum ks_language uwp_get_language(void);

void uwp_fill_installed_core_packages(struct string_list *list);

extern const struct kingsn_key_map kingsn_key_map_uwp[];

#ifdef __cplusplus
}
#endif

#endif _UWP_FUNC_H
