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

#ifndef __INPUT_DEFINES__H
#define __INPUT_DEFINES__H

#include <stdint.h>
#include <string.h>

#include <ks_common_api.h>

KS_BEGIN_DECLS

#define MAX_USERS                      16

#define MAX_INPUT_DEVICES              16

#define KINGSN_MAX_KEYS 137

#define KINGSN_FIRST_CUSTOM_BIND        16
#define KINGSN_FIRST_LIGHTGUN_BIND      KINGSN_ANALOG_BIND_LIST_END
#define KINGSN_FIRST_MISC_CUSTOM_BIND   KINGSN_LIGHTGUN_BIND_LIST_END
#define KINGSN_FIRST_META_KEY           KINGSN_CUSTOM_BIND_LIST_END

#define KINGSN_UNMAPPED                 1024

/* KingStation specific bind IDs. */
enum
{
   /* Custom binds that extend the scope of KS_DEVICE_JOYPAD for
    * KingStation specifically.
    * Analogs (KS_DEVICE_ANALOG) */
   KINGSN_ANALOG_LEFT_X_PLUS = KINGSN_FIRST_CUSTOM_BIND,
   KINGSN_ANALOG_LEFT_X_MINUS,
   KINGSN_ANALOG_LEFT_Y_PLUS,
   KINGSN_ANALOG_LEFT_Y_MINUS,
   KINGSN_ANALOG_RIGHT_X_PLUS,
   KINGSN_ANALOG_RIGHT_X_MINUS,
   KINGSN_ANALOG_RIGHT_Y_PLUS,
   KINGSN_ANALOG_RIGHT_Y_MINUS,
   KINGSN_ANALOG_BIND_LIST_END,

   /* Lightgun */
   KINGSN_LIGHTGUN_TRIGGER = KINGSN_FIRST_LIGHTGUN_BIND,
   KINGSN_LIGHTGUN_RELOAD,
   KINGSN_LIGHTGUN_AUX_A,
   KINGSN_LIGHTGUN_AUX_B,
   KINGSN_LIGHTGUN_AUX_C,
   KINGSN_LIGHTGUN_START,
   KINGSN_LIGHTGUN_SELECT,
   KINGSN_LIGHTGUN_DPAD_UP,
   KINGSN_LIGHTGUN_DPAD_DOWN,
   KINGSN_LIGHTGUN_DPAD_LEFT,
   KINGSN_LIGHTGUN_DPAD_RIGHT,
   KINGSN_LIGHTGUN_BIND_LIST_END,

   /* Turbo */
   KINGSN_TURBO_ENABLE = KINGSN_FIRST_MISC_CUSTOM_BIND,

   KINGSN_CUSTOM_BIND_LIST_END,

   /* Command binds. Not related to game input,
    * only usable for port 0. */
   KINGSN_FAST_FORWARD_KEY = KINGSN_FIRST_META_KEY,
   KINGSN_FAST_FORWARD_HOLD_KEY,
   KINGSN_VOLUME_UP,
   KINGSN_VOLUME_DOWN,
   KINGSN_SLOWMOTION_KEY,
   KINGSN_SLOWMOTION_HOLD_KEY,
   KINGSN_FULLSCREEN_TOGGLE_KEY,
   KINGSN_CLOSE_CONTENT_KEY,
   KINGSN_QUIT_KEY,
   KINGSN_STATE_SLOT_PLUS,
   KINGSN_STATE_SLOT_MINUS,
   KINGSN_REWIND,
   KINGSN_BSV_RECORD_TOGGLE,
   KINGSN_PAUSE_TOGGLE,
   KINGSN_FRAMEADVANCE,
   KINGSN_RESET,
   KINGSN_SHADER_NEXT,
   KINGSN_SHADER_PREV,
   KINGSN_CHEAT_INDEX_PLUS,
   KINGSN_CHEAT_INDEX_MINUS,
   KINGSN_CHEAT_TOGGLE,
   KINGSN_SCREENSHOT,
   KINGSN_MUTE,
   KINGSN_OSK,
   KINGSN_FPS_TOGGLE,
   KINGSN_SEND_DEBUG_INFO,
   KINGSN_NETPLAY_HOST_TOGGLE,
   KINGSN_NETPLAY_GAME_WATCH,
   KINGSN_ENABLE_HOTKEY,
   KINGSN_OVERLAY_NEXT,
   KINGSN_DISK_EJECT_TOGGLE,
   KINGSN_DISK_NEXT,
   KINGSN_DISK_PREV,
   KINGSN_GRAB_MOUSE_TOGGLE,
   KINGSN_GAME_FOCUS_TOGGLE,
   KINGSN_UI_COMPANION_TOGGLE,

   KINGSN_MENU_TOGGLE,

   KINGSN_RECORDING_TOGGLE,
   KINGSN_STREAMING_TOGGLE,
   KINGSN_RUNAHEAD_TOGGLE,

   KINGSN_AI_SERVICE,

   KINGSN_LOAD_STATE_KEY,
   KINGSN_SAVE_STATE_KEY,
   KINGSN_BIND_LIST_END,
   KINGSN_BIND_LIST_END_NULL
};

enum analog_dpad_mode
{
   ANALOG_DPAD_NONE = 0,
   ANALOG_DPAD_LSTICK,
   ANALOG_DPAD_RSTICK,
   ANALOG_DPAD_LAST
};

/* Specialized _MOUSE that targets the full screen regardless of viewport.
 */
#define KINGSN_DEVICE_MOUSE_SCREEN      (KS_DEVICE_MOUSE | 0x10000)

/* Specialized _POINTER that targets the full screen regardless of viewport.
 * Should not be used by a libks implementation as coordinates returned
 * make no sense.
 *
 * It is only used internally for overlays. */
#define KINGSN_DEVICE_POINTER_SCREEN    (KS_DEVICE_POINTER | 0x10000)

#define KINGSN_DEVICE_ID_POINTER_BACK   (KS_DEVICE_ID_POINTER_PRESSED | 0x10000)

/* libks has 16 buttons from 0-15 (libks.h)
 * Analog binds use KS_DEVICE_ANALOG, but we follow the same scheme
 * internally in KingStation for simplicity, so they are mapped into [16, 23].
 */

#define AXIS_NEG(x)        (((uint32_t)(x) << 16) | 0xFFFFU)
#define AXIS_POS(x)        ((uint32_t)(x) | 0xFFFF0000UL)
#define AXIS_NONE          0xFFFFFFFFUL
#define AXIS_DIR_NONE      0xFFFFU

#define AXIS_NEG_GET(x)    (((uint32_t)(x) >> 16) & 0xFFFFU)
#define AXIS_POS_GET(x)    ((uint32_t)(x) & 0xFFFFU)

#define NO_BTN             0xFFFFU

#define HAT_UP_SHIFT       15
#define HAT_DOWN_SHIFT     14
#define HAT_LEFT_SHIFT     13
#define HAT_RIGHT_SHIFT    12
#define HAT_UP_MASK        (1 << HAT_UP_SHIFT)
#define HAT_DOWN_MASK      (1 << HAT_DOWN_SHIFT)
#define HAT_LEFT_MASK      (1 << HAT_LEFT_SHIFT)
#define HAT_RIGHT_MASK     (1 << HAT_RIGHT_SHIFT)
#define HAT_MAP(x, hat)    ((x & ((1 << 12) - 1)) | hat)

#define HAT_MASK           (HAT_UP_MASK | HAT_DOWN_MASK | HAT_LEFT_MASK | HAT_RIGHT_MASK)
#define GET_HAT_DIR(x)     (x & HAT_MASK)
#define GET_HAT(x)         (x & (~HAT_MASK))

KS_END_DECLS

#endif
