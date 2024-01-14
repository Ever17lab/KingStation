/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
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

#ifndef INPUT_KEYMAPS_H__
#define INPUT_KEYMAPS_H__

#include <stdint.h>

#include <ks_common_api.h>
#include <libks.h>

KS_BEGIN_DECLS

struct kingsn_key_map
{
   unsigned sym;
   enum ks_key rk;
};

struct input_key_map
{
   const char *str;
   enum ks_key key;
};

#define KINGSN_KEY_MAP_RWEBINPUT_SIZE 111

extern const struct input_key_map input_config_key_map[];

extern const struct kingsn_key_map kingsn_key_map_x11[];
extern const struct kingsn_key_map kingsn_key_map_sdl[];
extern const struct kingsn_key_map kingsn_key_map_sdl2[];
extern const struct kingsn_key_map kingsn_key_map_dinput[];

 /* is generated at runtime so can't be const */
extern struct kingsn_key_map kingsn_key_map_rwebinput[KINGSN_KEY_MAP_RWEBINPUT_SIZE];

extern const struct kingsn_key_map kingsn_key_map_linux[];
extern const struct kingsn_key_map kingsn_key_map_apple_hid[];
extern const struct kingsn_key_map kingsn_key_map_android[];
extern const struct kingsn_key_map kingsn_key_map_qnx[];
extern const struct kingsn_key_map kingsn_key_map_dos[];
extern const struct kingsn_key_map kingsn_key_map_wiiu[];
extern const struct kingsn_key_map kingsn_key_map_winraw[];
#ifdef HAVE_LIBNX
extern const struct kingsn_key_map kingsn_key_map_switch[];
#endif
#ifdef VITA
extern const struct kingsn_key_map kingsn_key_map_vita[];
#endif

/**
 * input_keymaps_init_keyboard_lut:
 * @map                   : Keyboard map.
 *
 * Initializes and sets the keyboard layout to a keyboard map (@map).
 **/
void input_keymaps_init_keyboard_lut(const struct kingsn_key_map *map);

/**
 * input_keymaps_translate_keysym_to_rk:
 * @sym                   : Key symbol.
 *
 * Translates a key symbol from the keyboard layout table
 * to an associated ks key identifier.
 *
 * Returns: Retro key identifier.
 **/
enum ks_key input_keymaps_translate_keysym_to_rk(unsigned sym);

/**
 * input_keymaps_translate_rk_to_str:
 * @key                   : Retro key identifier.
 * @buf                   : Buffer.
 * @size                  : Size of @buf.
 *
 * Translates a ks key identifier to a human-readable
 * identifier string.
 **/
void input_keymaps_translate_rk_to_str(enum ks_key key, char *buf, size_t size);

extern enum ks_key kingsn_keysym_lut[KSK_LAST];

KS_END_DECLS

#endif
