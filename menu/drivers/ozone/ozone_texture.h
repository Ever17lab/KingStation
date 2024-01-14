/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2014-2017 - Jean-André Santoni
 *  Copyright (C) 2016-2019 - Brad Parker
 *  Copyright (C) 2018      - Alfredo Monclús
 *  Copyright (C) 2018      - natinusala
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

#ifndef _OZONE_TEXTURE_H
#define _OZONE_TEXTURE_H

#include "ozone.h"

#include "../../menu_driver.h"

enum OZONE_TEXTURE
{
   OZONE_TEXTURE_KingStation = 0,
   OZONE_TEXTURE_CURSOR_BORDER,
#if 0
   OZONE_TEXTURE_DISCORD_OWN_AVATAR,
#endif
   OZONE_TEXTURE_LAST
};

enum OZONE_THEME_TEXTURES
{
   OZONE_THEME_TEXTURE_SWITCH = 0,
   OZONE_THEME_TEXTURE_CHECK,

   OZONE_THEME_TEXTURE_CURSOR_NO_BORDER,
   OZONE_THEME_TEXTURE_CURSOR_STATIC,

   OZONE_THEME_TEXTURE_LAST
};

enum OZONE_TAB_TEXTURES
{
   OZONE_TAB_TEXTURE_MAIN_MENU = 0,
   OZONE_TAB_TEXTURE_SETTINGS,
   OZONE_TAB_TEXTURE_HISTORY,
   OZONE_TAB_TEXTURE_FAVORITES,
   OZONE_TAB_TEXTURE_MUSIC,
   OZONE_TAB_TEXTURE_VIDEO,
   OZONE_TAB_TEXTURE_IMAGE,
   OZONE_TAB_TEXTURE_NETWORK,
   OZONE_TAB_TEXTURE_SCAN_CONTENT,

   OZONE_TAB_TEXTURE_LAST
};

enum
{
   OZONE_ENTRIES_ICONS_TEXTURE_MAIN_MENU = 0,
   OZONE_ENTRIES_ICONS_TEXTURE_SETTINGS,
   OZONE_ENTRIES_ICONS_TEXTURE_HISTORY,
   OZONE_ENTRIES_ICONS_TEXTURE_FAVORITES,
   OZONE_ENTRIES_ICONS_TEXTURE_MUSICS,
#if defined(HAVE_FFMPEG) || defined(HAVE_MPV)
   OZONE_ENTRIES_ICONS_TEXTURE_MOVIES,
#endif
#ifdef HAVE_NETWORKING
   OZONE_ENTRIES_ICONS_TEXTURE_NETPLAY,
   OZONE_ENTRIES_ICONS_TEXTURE_ROOM,
   OZONE_ENTRIES_ICONS_TEXTURE_ROOM_LAN,
   OZONE_ENTRIES_ICONS_TEXTURE_ROOM_RELAY,
#endif
#ifdef HAVE_IMAGEVIEWER
   OZONE_ENTRIES_ICONS_TEXTURE_IMAGES,
#endif
   OZONE_ENTRIES_ICONS_TEXTURE_SETTING,
   OZONE_ENTRIES_ICONS_TEXTURE_SUBSETTING,
   OZONE_ENTRIES_ICONS_TEXTURE_ARROW,
   OZONE_ENTRIES_ICONS_TEXTURE_RUN,
   OZONE_ENTRIES_ICONS_TEXTURE_CLOSE,
   OZONE_ENTRIES_ICONS_TEXTURE_RESUME,
   OZONE_ENTRIES_ICONS_TEXTURE_SAVESTATE,
   OZONE_ENTRIES_ICONS_TEXTURE_LOADSTATE,
   OZONE_ENTRIES_ICONS_TEXTURE_UNDO,
   OZONE_ENTRIES_ICONS_TEXTURE_CORE_INFO,
   OZONE_ENTRIES_ICONS_TEXTURE_BLUETOOTH,
   OZONE_ENTRIES_ICONS_TEXTURE_WIFI,
   OZONE_ENTRIES_ICONS_TEXTURE_CORE_OPTIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_REMAPPING_OPTIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_CHEAT_OPTIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_DISK_OPTIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_SHADER_OPTIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_ACHIEVEMENT_LIST,
   OZONE_ENTRIES_ICONS_TEXTURE_SCREENSHOT,
   OZONE_ENTRIES_ICONS_TEXTURE_RELOAD,
   OZONE_ENTRIES_ICONS_TEXTURE_RENAME,
   OZONE_ENTRIES_ICONS_TEXTURE_FILE,
   OZONE_ENTRIES_ICONS_TEXTURE_FOLDER,
   OZONE_ENTRIES_ICONS_TEXTURE_ZIP,
   OZONE_ENTRIES_ICONS_TEXTURE_FAVORITE,
   OZONE_ENTRIES_ICONS_TEXTURE_ADD_FAVORITE,
   OZONE_ENTRIES_ICONS_TEXTURE_MUSIC,
   OZONE_ENTRIES_ICONS_TEXTURE_IMAGE,
   OZONE_ENTRIES_ICONS_TEXTURE_MOVIE,
   OZONE_ENTRIES_ICONS_TEXTURE_CORE,
   OZONE_ENTRIES_ICONS_TEXTURE_RDB,
   OZONE_ENTRIES_ICONS_TEXTURE_CURSOR,
   OZONE_ENTRIES_ICONS_TEXTURE_SWITCH_ON,
   OZONE_ENTRIES_ICONS_TEXTURE_SWITCH_OFF,
   OZONE_ENTRIES_ICONS_TEXTURE_CLOCK,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_FULL,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_CHARGING,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_80,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_60,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_40,
   OZONE_ENTRIES_ICONS_TEXTURE_BATTERY_20,
   OZONE_ENTRIES_ICONS_TEXTURE_POINTER,
   OZONE_ENTRIES_ICONS_TEXTURE_ADD,
   OZONE_ENTRIES_ICONS_TEXTURE_DISC,
   OZONE_ENTRIES_ICONS_TEXTURE_KEY,
   OZONE_ENTRIES_ICONS_TEXTURE_KEY_HOVER,
   OZONE_ENTRIES_ICONS_TEXTURE_DIALOG_SLICE,
   OZONE_ENTRIES_ICONS_TEXTURE_ACHIEVEMENTS,
   OZONE_ENTRIES_ICONS_TEXTURE_AUDIO,
   OZONE_ENTRIES_ICONS_TEXTURE_EXIT,
   OZONE_ENTRIES_ICONS_TEXTURE_FRAMESKIP,
   OZONE_ENTRIES_ICONS_TEXTURE_INFO,
   OZONE_ENTRIES_ICONS_TEXTURE_HELP,
   OZONE_ENTRIES_ICONS_TEXTURE_NETWORK,
   OZONE_ENTRIES_ICONS_TEXTURE_POWER,
   OZONE_ENTRIES_ICONS_TEXTURE_SAVING,
   OZONE_ENTRIES_ICONS_TEXTURE_UPDATER,
   OZONE_ENTRIES_ICONS_TEXTURE_VIDEO,
   OZONE_ENTRIES_ICONS_TEXTURE_RECORD,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_SETTINGS,
   OZONE_ENTRIES_ICONS_TEXTURE_MIXER,
   OZONE_ENTRIES_ICONS_TEXTURE_LOG,
   OZONE_ENTRIES_ICONS_TEXTURE_OSD,
   OZONE_ENTRIES_ICONS_TEXTURE_UI,
   OZONE_ENTRIES_ICONS_TEXTURE_USER,
   OZONE_ENTRIES_ICONS_TEXTURE_PRIVACY,
   OZONE_ENTRIES_ICONS_TEXTURE_LATENCY,
   OZONE_ENTRIES_ICONS_TEXTURE_DRIVERS,
   OZONE_ENTRIES_ICONS_TEXTURE_PLAYLIST,
   OZONE_ENTRIES_ICONS_TEXTURE_QUICKMENU,
   OZONE_ENTRIES_ICONS_TEXTURE_REWIND,
   OZONE_ENTRIES_ICONS_TEXTURE_OVERLAY,
   OZONE_ENTRIES_ICONS_TEXTURE_OVERRIDE,
   OZONE_ENTRIES_ICONS_TEXTURE_NOTIFICATIONS,
   OZONE_ENTRIES_ICONS_TEXTURE_STREAM,
   OZONE_ENTRIES_ICONS_TEXTURE_SHUTDOWN,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_DPAD_U,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_DPAD_D,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_DPAD_L,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_DPAD_R,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_STCK_U,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_STCK_D,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_STCK_L,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_STCK_R,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_STCK_P,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_SELECT,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_START,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_BTN_U,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_BTN_D,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_BTN_L,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_BTN_R,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_LB,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_RB,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_LT,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_RT,
   OZONE_ENTRIES_ICONS_TEXTURE_CHECKMARK,
   OZONE_ENTRIES_ICONS_TEXTURE_MENU_ADD,
   OZONE_ENTRIES_ICONS_TEXTURE_BRIGHTNESS,
   OZONE_ENTRIES_ICONS_TEXTURE_PAUSE,
   OZONE_ENTRIES_ICONS_TEXTURE_MENU_APPLY_TOGGLE,
   OZONE_ENTRIES_ICONS_TEXTURE_MENU_APPLY_COG,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_ADC,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_BIND_ALL,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_MOUSE,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_LGUN,
   OZONE_ENTRIES_ICONS_TEXTURE_INPUT_TURBO,
   OZONE_ENTRIES_ICONS_TEXTURE_LAST
};

const char *ozone_entries_icon_texture_path(unsigned id);

uintptr_t ozone_entries_icon_get_texture(ozone_handle_t *ozone,
      enum msg_hash_enums enum_idx, unsigned type, bool active);

bool ozone_reset_theme_textures(ozone_handle_t *ozone);
void ozone_unload_theme_textures(ozone_handle_t *ozone);

#endif
