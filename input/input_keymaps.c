/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2020      - neil4 (reverse LUT keyboard)
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <compat/strl.h>
#include <ks_assert.h>
#include <ks_miscellaneous.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "../config.def.h"

#ifdef ANDROID
#include <android/keycodes.h>
#include "drivers_keyboard/keyboard_event_android.h"
#endif

#ifdef DJGPP
#include "drivers_keyboard/keyboard_event_dos.h"
#endif

#ifdef __QNX__
#include <sys/keycodes.h>
#endif

#ifdef __PSL1GHT__
#include <io/kb.h>
#endif

#if defined(HAVE_SDL) || defined(HAVE_SDL2)
#include "SDL.h"
#endif

#if defined(__linux__) || defined(HAVE_WAYLAND)
#include <linux/input.h>
#include <linux/kd.h>
#endif

#ifdef HAVE_X11
#include "input/include/xfree86_keycodes.h"
#endif

#ifdef HAVE_DINPUT
#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#endif

#include "input_keymaps.h"

#ifdef __APPLE__
#include "drivers_keyboard/keyboard_event_apple.h"
#endif

const struct input_key_map input_config_key_map[] = {
   { "A", KSK_a },
   { "B", KSK_b },
   { "C", KSK_c },
   { "D", KSK_d },
   { "E", KSK_e },
   { "F", KSK_f },
   { "G", KSK_g },
   { "H", KSK_h },
   { "I", KSK_i },
   { "J", KSK_j },
   { "K", KSK_k },
   { "L", KSK_l },
   { "M", KSK_m },
   { "N", KSK_n },
   { "O", KSK_o },
   { "P", KSK_p },
   { "Q", KSK_q },
   { "R", KSK_r },
   { "S", KSK_s },
   { "T", KSK_t },
   { "U", KSK_u },
   { "V", KSK_v },
   { "W", KSK_w },
   { "X", KSK_x },
   { "Y", KSK_y },
   { "Z", KSK_z },
   { "Left", KSK_LEFT },
   { "Right", KSK_RIGHT },
   { "Up", KSK_UP },
   { "Down", KSK_DOWN },
   { "Enter", KSK_RETURN },
   { "Num Enter", KSK_KP_ENTER },
   { "Tab", KSK_TAB },
   { "Insert", KSK_INSERT },
   { "Del", KSK_DELETE },
   { "End", KSK_END },
   { "Home", KSK_HOME },
   { "Right Shift", KSK_RSHIFT },
   { "Shift", KSK_LSHIFT },
   { "Ctrl", KSK_LCTRL },
   { "Alt", KSK_LALT },
   { "Space", KSK_SPACE },
   { "Esc", KSK_ESCAPE },
   { "Plus", KSK_KP_PLUS },
   { "Minus", KSK_KP_MINUS },
   { "kp_plus", KSK_KP_PLUS },
   { "kp_minus", KSK_KP_MINUS },
   { "F1", KSK_F1 },
   { "F2", KSK_F2 },
   { "F3", KSK_F3 },
   { "F4", KSK_F4 },
   { "F5", KSK_F5 },
   { "F6", KSK_F6 },
   { "F7", KSK_F7 },
   { "F8", KSK_F8 },
   { "F9", KSK_F9 },
   { "F10", KSK_F10 },
   { "F11", KSK_F11 },
   { "F12", KSK_F12 },
   { "0", KSK_0 },
   { "1", KSK_1 },
   { "2", KSK_2 },
   { "3", KSK_3 },
   { "4", KSK_4 },
   { "5", KSK_5 },
   { "6", KSK_6 },
   { "7", KSK_7 },
   { "8", KSK_8 },
   { "9", KSK_9 },
   { "PgUp", KSK_PAGEUP },
   { "PgDn", KSK_PAGEDOWN },
   { "Num0", KSK_KP0 },
   { "Num1", KSK_KP1 },
   { "Num2", KSK_KP2 },
   { "Num3", KSK_KP3 },
   { "Num4", KSK_KP4 },
   { "Num5", KSK_KP5 },
   { "Num6", KSK_KP6 },
   { "Num7", KSK_KP7 },
   { "Num8", KSK_KP8 },
   { "Num9", KSK_KP9 },
   { "period", KSK_PERIOD },
   { "Caps Lock", KSK_CAPSLOCK },
   { "Num Lock", KSK_NUMLOCK },
   { "Backspace", KSK_BACKSPACE },
   { "Multiply", KSK_KP_MULTIPLY },
   { "Divide", KSK_KP_DIVIDE },
   { "Print Screen", KSK_PRINT },
   { "Scroll Lock", KSK_SCROLLOCK },
   { "Tilde", KSK_BACKQUOTE },
   { "Backquote", KSK_BACKQUOTE },
   { "Pause", KSK_PAUSE },

   /* Keys that weren't mappable before */
   { "Quote", KSK_QUOTE },
   { "Comma", KSK_COMMA },
   { "Minus", KSK_MINUS },
   { "Slash", KSK_SLASH },
   { "Semicolon", KSK_SEMICOLON },
   { "Equals", KSK_EQUALS },
   { "Leftbracket", KSK_LEFTBRACKET },
   { "Backslash", KSK_BACKSLASH },
   { "Rightbracket", KSK_RIGHTBRACKET },
   { "kp_period", KSK_KP_PERIOD },
   { "kp_equals", KSK_KP_EQUALS },
   { "RCtrl", KSK_RCTRL },
   { "RAlt", KSK_RALT },

   /* Keys not referenced in any keyboard mapping
    * (except perhaps kingsn_key_map_apple_hid) */
   { "caret", KSK_CARET },
   { "underscore", KSK_UNDERSCORE },
   { "exclaim", KSK_EXCLAIM },
   { "quotedbl", KSK_QUOTEDBL },
   { "hash", KSK_HASH },
   { "dollar", KSK_DOLLAR },
   { "ampersand", KSK_AMPERSAND },
   { "leftparen", KSK_LEFTPAREN },
   { "rightparen", KSK_RIGHTPAREN },
   { "asterisk", KSK_ASTERISK },
   { "plus", KSK_PLUS },
   { "colon", KSK_COLON },
   { "less", KSK_LESS },
   { "greater", KSK_GREATER },
   { "question", KSK_QUESTION },
   { "at", KSK_AT },

   { "F13", KSK_F13 },
   { "F14", KSK_F14 },
   { "F15", KSK_F15 },

   { "rmeta", KSK_RMETA },
   { "lmeta", KSK_LMETA },
   { "lsuper", KSK_LSUPER },
   { "rsuper", KSK_RSUPER },
   { "mode", KSK_MODE },
   { "compose", KSK_COMPOSE },

   { "help", KSK_HELP },
   { "sysreq", KSK_SYSREQ },
   { "break", KSK_BREAK },
   { "menu", KSK_MENU },
   { "power", KSK_POWER },
   { "euro", KSK_EURO },
   { "undo", KSK_UNDO },
   { "clear", KSK_CLEAR },
   { "oem102", KSK_OEM_102 },

   { "nul", KSK_UNKNOWN },
   { NULL, KSK_UNKNOWN },
};

#ifdef HAVE_LIBNX
const struct kingsn_key_map kingsn_key_map_switch[] = {
   { KBD_A, KSK_a },
   { KBD_B, KSK_b },
   { KBD_C, KSK_c },
   { KBD_D, KSK_d },
   { KBD_E, KSK_e },
   { KBD_F, KSK_f },
   { KBD_G, KSK_g },
   { KBD_H, KSK_h },
   { KBD_I, KSK_i },
   { KBD_J, KSK_j },
   { KBD_K, KSK_k },
   { KBD_L, KSK_l },
   { KBD_M, KSK_m },
   { KBD_N, KSK_n },
   { KBD_O, KSK_o },
   { KBD_P, KSK_p },
   { KBD_Q, KSK_q },
   { KBD_R, KSK_r },
   { KBD_S, KSK_s },
   { KBD_T, KSK_t },
   { KBD_U, KSK_u },
   { KBD_V, KSK_v },
   { KBD_W, KSK_w },
   { KBD_X, KSK_x },
   { KBD_Y, KSK_y },
   { KBD_Z, KSK_z },
   { KBD_BACKSPACE, KSK_BACKSPACE },
   { KBD_TAB, KSK_TAB },
   { KBD_ENTER, KSK_RETURN },
   { KBD_PAUSE, KSK_PAUSE },
   { KBD_ESC, KSK_ESCAPE },
   { KBD_SPACE, KSK_SPACE },
   { KBD_HASHTILDE, KSK_HASH },
   { KBD_APOSTROPHE, KSK_QUOTE },
   { KBD_KPLEFTPAREN, KSK_LEFTPAREN },
   { KBD_KPRIGHTPAREN, KSK_RIGHTPAREN },
   { KBD_COMMA, KSK_COMMA },
   { KBD_MINUS, KSK_MINUS },
   { KBD_DOT, KSK_PERIOD },
   { KBD_SLASH, KSK_SLASH },
   { KBD_0, KSK_0 },
   { KBD_1, KSK_1 },
   { KBD_2, KSK_2 },
   { KBD_3, KSK_3 },
   { KBD_4, KSK_4 },
   { KBD_5, KSK_5 },
   { KBD_6, KSK_6 },
   { KBD_7, KSK_7 },
   { KBD_8, KSK_8 },
   { KBD_9, KSK_9 },
   { KBD_SEMICOLON, KSK_SEMICOLON },
   { KBD_EQUAL, KSK_EQUALS },
   { KBD_LEFTBRACE, KSK_LEFTBRACKET },
   { KBD_BACKSLASH, KSK_BACKSLASH },
   { KBD_RIGHTBRACE, KSK_RIGHTBRACKET },
   { KBD_DELETE, KSK_DELETE },
   { KBD_KP0, KSK_KP0 },
   { KBD_KP1, KSK_KP1 },
   { KBD_KP2, KSK_KP2 },
   { KBD_KP3, KSK_KP3 },
   { KBD_KP4, KSK_KP4 },
   { KBD_KP5, KSK_KP5 },
   { KBD_KP6, KSK_KP6 },
   { KBD_KP7, KSK_KP7 },
   { KBD_KP8, KSK_KP8 },
   { KBD_KP9, KSK_KP9 },
   { KBD_KPDOT, KSK_KP_PERIOD },
   { KBD_KPSLASH, KSK_KP_DIVIDE },
   { KBD_KPASTERISK, KSK_KP_MULTIPLY },
   { KBD_KPMINUS, KSK_KP_MINUS },
   { KBD_KPPLUS, KSK_KP_PLUS },
   { KBD_KPENTER, KSK_KP_ENTER },
   { KBD_KPEQUAL, KSK_KP_EQUALS },
   { KBD_UP, KSK_UP },
   { KBD_DOWN, KSK_DOWN },
   { KBD_RIGHT, KSK_RIGHT },
   { KBD_LEFT, KSK_LEFT },
   { KBD_INSERT, KSK_INSERT },
   { KBD_HOME, KSK_HOME },
   { KBD_END, KSK_END },
   { KBD_PAGEUP, KSK_PAGEUP },
   { KBD_PAGEDOWN, KSK_PAGEDOWN },
   { KBD_F1, KSK_F1 },
   { KBD_F2, KSK_F2 },
   { KBD_F3, KSK_F3 },
   { KBD_F4, KSK_F4 },
   { KBD_F5, KSK_F5 },
   { KBD_F6, KSK_F6 },
   { KBD_F7, KSK_F7 },
   { KBD_F8, KSK_F8 },
   { KBD_F9, KSK_F9 },
   { KBD_F10, KSK_F10 },
   { KBD_F11, KSK_F11 },
   { KBD_F12, KSK_F12 },
   { KBD_F13, KSK_F13 },
   { KBD_F14, KSK_F14 },
   { KBD_F15, KSK_F15 },
   { KBD_NUMLOCK, KSK_NUMLOCK },
   { KBD_CAPSLOCK, KSK_CAPSLOCK },
   { KBD_SCROLLLOCK, KSK_SCROLLOCK },
   { KBD_RIGHTSHIFT, KSK_RSHIFT },
   { KBD_LEFTSHIFT, KSK_LSHIFT },
   { KBD_RIGHTCTRL, KSK_RCTRL },
   { KBD_LEFTCTRL, KSK_LCTRL },
   { KBD_RIGHTALT, KSK_RALT },
   { KBD_LEFTALT, KSK_LALT },
   { KBD_LEFTMETA, KSK_LMETA },
   { KBD_RIGHTMETA, KSK_RMETA },
   { KBD_COMPOSE, KSK_COMPOSE },
   { KBD_HELP, KSK_HELP },
   { KBD_PAUSE, KSK_BREAK },
   { KBD_POWER, KSK_POWER },
   { KBD_UNDO, KSK_UNDO },
   { 0, KSK_UNKNOWN }
};
#endif

#ifdef VITA
/* Vita scancodes are identical to USB 2.0 standard, e.g. SDL2 */
const struct kingsn_key_map kingsn_key_map_vita[] = {
   { 0x02A, KSK_BACKSPACE },
   { 0x02B, KSK_TAB },
   { 0x09C, KSK_CLEAR },
   { 0x028, KSK_RETURN },
   { 0x048, KSK_PAUSE },
   { 0x029, KSK_ESCAPE },
   { 0x02C, KSK_SPACE },
   /*{ ?, KSK_EXCLAIM },*/
   /*{ ?, KSK_QUOTEDBL },*/
   /*{ ?, KSK_HASH },*/
   /*{ ?, KSK_DOLLAR },*/
   /*{ ?, KSK_AMPERSAND },*/
   { 0x034, KSK_QUOTE },
   /*{ ?, KSK_LEFTPAREN },*/
   /*{ ?, KSK_RIGHTPAREN },*/
   /*{ ?, KSK_ASTERISK },*/
   /*{ ?, KSK_PLUS },*/
   { 0x036, KSK_COMMA },
   { 0x02D, KSK_MINUS },
   { 0x037, KSK_PERIOD },
   { 0x038, KSK_SLASH },
   { 0x027, KSK_0 },
   { 0x01E, KSK_1 },
   { 0x01F, KSK_2 },
   { 0x020, KSK_3 },
   { 0x021, KSK_4 },
   { 0x022, KSK_5 },
   { 0x023, KSK_6 },
   { 0x024, KSK_7 },
   { 0x025, KSK_8 },
   { 0x026, KSK_9 },
   /*{ ?, KSK_COLON },*/
   { 0x033, KSK_SEMICOLON },
   /*{ ?, KSK_OEM_102 },*/
   { 0x02E, KSK_EQUALS },
   /*{ ?, KSK_GREATER },*/
   /*{ ?, KSK_QUESTION },*/
   /*{ ?, KSK_AT },*/
   { 0x02F, KSK_LEFTBRACKET },
   { 0x031, KSK_BACKSLASH },
   { 0x030, KSK_RIGHTBRACKET },
   /*{ ?, KSK_CARET },*/
   /*{ ?, KSK_UNDERSCORE },*/
   { 0x035, KSK_BACKQUOTE },
   { 0x004, KSK_a },
   { 0x005, KSK_b },
   { 0x006, KSK_c },
   { 0x007, KSK_d },
   { 0x008, KSK_e },
   { 0x009, KSK_f },
   { 0x00A, KSK_g },
   { 0x00B, KSK_h },
   { 0x00C, KSK_i },
   { 0x00D, KSK_j },
   { 0x00E, KSK_k },
   { 0x00F, KSK_l },
   { 0x010, KSK_m },
   { 0x011, KSK_n },
   { 0x012, KSK_o },
   { 0x013, KSK_p },
   { 0x014, KSK_q },
   { 0x015, KSK_r },
   { 0x016, KSK_s },
   { 0x017, KSK_t },
   { 0x018, KSK_u },
   { 0x019, KSK_v },
   { 0x01A, KSK_w },
   { 0x01B, KSK_x },
   { 0x01C, KSK_y },
   { 0x01D, KSK_z },
   { 0x04C, KSK_DELETE },
   { 0x062, KSK_KP0 },
   { 0x059, KSK_KP1 },
   { 0x05A, KSK_KP2 },
   { 0x05B, KSK_KP3 },
   { 0x05C, KSK_KP4 },
   { 0x05D, KSK_KP5 },
   { 0x05E, KSK_KP6 },
   { 0x05F, KSK_KP7 },
   { 0x060, KSK_KP8 },
   { 0x061, KSK_KP9 },
   { 0x063, KSK_KP_PERIOD },
   { 0x054, KSK_KP_DIVIDE },
   { 0x055, KSK_KP_MULTIPLY },
   { 0x056, KSK_KP_MINUS },
   { 0x057, KSK_KP_PLUS },
   { 0x058, KSK_KP_ENTER },
   { 0x067, KSK_KP_EQUALS },
   { 0x052, KSK_UP },
   { 0x051, KSK_DOWN },
   { 0x04F, KSK_RIGHT },
   { 0x050, KSK_LEFT },
   { 0x049, KSK_INSERT },
   { 0x04A, KSK_HOME },
   { 0x04D, KSK_END },
   { 0x04B, KSK_PAGEUP },
   { 0x04E, KSK_PAGEDOWN },
   { 0x03A, KSK_F1 },
   { 0x03B, KSK_F2 },
   { 0x03C, KSK_F3 },
   { 0x03D, KSK_F4 },
   { 0x03E, KSK_F5 },
   { 0x03F, KSK_F6 },
   { 0x040, KSK_F7 },
   { 0x041, KSK_F8 },
   { 0x042, KSK_F9 },
   { 0x043, KSK_F10 },
   { 0x044, KSK_F11 },
   { 0x045, KSK_F12 },
   { 0x068, KSK_F13 },
   { 0x069, KSK_F14 },
   { 0x06A, KSK_F15 },
   { 0x053, KSK_NUMLOCK },
   { 0x039, KSK_CAPSLOCK },
   { 0x047, KSK_SCROLLOCK },
   { 0x0E5, KSK_RSHIFT },
   { 0x0E1, KSK_LSHIFT },
   { 0x0E4, KSK_RCTRL },
   { 0x0E0, KSK_LCTRL },
   { 0x0E6, KSK_RALT },
   { 0x0E2, KSK_LALT },
   /* { ?, KSK_RMETA }, */
   /* { ?, KSK_LMETA }, */
   { 0x0E3, KSK_LSUPER },
   { 0x0E7, KSK_RSUPER },
   /* { ?, KSK_MODE },*/
   { 0x075, KSK_HELP },
   { 0x046, KSK_PRINT },
   { 0x09A, KSK_SYSREQ },
   { 0x048, KSK_BREAK },
   { 0x076, KSK_MENU },
   { 0x066, KSK_POWER },
   /*{ ?, KSK_EURO },*/
   { 0x07A, KSK_UNDO },
   { 0, KSK_UNKNOWN },
};
#endif

#if defined(HAVE_SDL) || defined(HAVE_SDL2)
const struct kingsn_key_map kingsn_key_map_sdl[] = {
   { SDLK_BACKSPACE, KSK_BACKSPACE },
   { SDLK_TAB, KSK_TAB },
   { SDLK_CLEAR, KSK_CLEAR },
   { SDLK_RETURN, KSK_RETURN },
   { SDLK_PAUSE, KSK_PAUSE },
   { SDLK_ESCAPE, KSK_ESCAPE },
   { SDLK_SPACE, KSK_SPACE },
   { SDLK_EXCLAIM, KSK_EXCLAIM },
   { SDLK_QUOTEDBL, KSK_QUOTEDBL },
   { SDLK_HASH, KSK_HASH },
   { SDLK_DOLLAR, KSK_DOLLAR },
   { SDLK_AMPERSAND, KSK_AMPERSAND },
   { SDLK_QUOTE, KSK_QUOTE },
   { SDLK_LEFTPAREN, KSK_LEFTPAREN },
   { SDLK_RIGHTPAREN, KSK_RIGHTPAREN },
   { SDLK_ASTERISK, KSK_ASTERISK },
   { SDLK_PLUS, KSK_PLUS },
   { SDLK_COMMA, KSK_COMMA },
   { SDLK_MINUS, KSK_MINUS },
   { SDLK_PERIOD, KSK_PERIOD },
   { SDLK_SLASH, KSK_SLASH },
   { SDLK_0, KSK_0 },
   { SDLK_1, KSK_1 },
   { SDLK_2, KSK_2 },
   { SDLK_3, KSK_3 },
   { SDLK_4, KSK_4 },
   { SDLK_5, KSK_5 },
   { SDLK_6, KSK_6 },
   { SDLK_7, KSK_7 },
   { SDLK_8, KSK_8 },
   { SDLK_9, KSK_9 },
   { SDLK_COLON, KSK_COLON },
   { SDLK_SEMICOLON, KSK_SEMICOLON },
   { SDLK_LESS, KSK_OEM_102 },
   { SDLK_EQUALS, KSK_EQUALS },
   { SDLK_GREATER, KSK_GREATER },
   { SDLK_QUESTION, KSK_QUESTION },
   { SDLK_AT, KSK_AT },
   { SDLK_LEFTBRACKET, KSK_LEFTBRACKET },
   { SDLK_BACKSLASH, KSK_BACKSLASH },
   { SDLK_RIGHTBRACKET, KSK_RIGHTBRACKET },
   { SDLK_CARET, KSK_CARET },
   { SDLK_UNDERSCORE, KSK_UNDERSCORE },
   { SDLK_BACKQUOTE, KSK_BACKQUOTE },
   { SDLK_a, KSK_a },
   { SDLK_b, KSK_b },
   { SDLK_c, KSK_c },
   { SDLK_d, KSK_d },
   { SDLK_e, KSK_e },
   { SDLK_f, KSK_f },
   { SDLK_g, KSK_g },
   { SDLK_h, KSK_h },
   { SDLK_i, KSK_i },
   { SDLK_j, KSK_j },
   { SDLK_k, KSK_k },
   { SDLK_l, KSK_l },
   { SDLK_m, KSK_m },
   { SDLK_n, KSK_n },
   { SDLK_o, KSK_o },
   { SDLK_p, KSK_p },
   { SDLK_q, KSK_q },
   { SDLK_r, KSK_r },
   { SDLK_s, KSK_s },
   { SDLK_t, KSK_t },
   { SDLK_u, KSK_u },
   { SDLK_v, KSK_v },
   { SDLK_w, KSK_w },
   { SDLK_x, KSK_x },
   { SDLK_y, KSK_y },
   { SDLK_z, KSK_z },
   { SDLK_DELETE, KSK_DELETE },
#ifdef HAVE_SDL2
   { SDLK_KP_0, KSK_KP0 },
   { SDLK_KP_1, KSK_KP1 },
   { SDLK_KP_2, KSK_KP2 },
   { SDLK_KP_3, KSK_KP3 },
   { SDLK_KP_4, KSK_KP4 },
   { SDLK_KP_5, KSK_KP5 },
   { SDLK_KP_6, KSK_KP6 },
   { SDLK_KP_7, KSK_KP7 },
   { SDLK_KP_8, KSK_KP8 },
   { SDLK_KP_9, KSK_KP9 },
#else
   { SDLK_KP0, KSK_KP0 },
   { SDLK_KP1, KSK_KP1 },
   { SDLK_KP2, KSK_KP2 },
   { SDLK_KP3, KSK_KP3 },
   { SDLK_KP4, KSK_KP4 },
   { SDLK_KP5, KSK_KP5 },
   { SDLK_KP6, KSK_KP6 },
   { SDLK_KP7, KSK_KP7 },
   { SDLK_KP8, KSK_KP8 },
   { SDLK_KP9, KSK_KP9 },
#endif
   { SDLK_KP_PERIOD, KSK_KP_PERIOD },
   { SDLK_KP_DIVIDE, KSK_KP_DIVIDE },
   { SDLK_KP_MULTIPLY, KSK_KP_MULTIPLY },
   { SDLK_KP_MINUS, KSK_KP_MINUS },
   { SDLK_KP_PLUS, KSK_KP_PLUS },
   { SDLK_KP_ENTER, KSK_KP_ENTER },
   { SDLK_KP_EQUALS, KSK_KP_EQUALS },
   { SDLK_UP, KSK_UP },
   { SDLK_DOWN, KSK_DOWN },
   { SDLK_RIGHT, KSK_RIGHT },
   { SDLK_LEFT, KSK_LEFT },
   { SDLK_INSERT, KSK_INSERT },
   { SDLK_HOME, KSK_HOME },
   { SDLK_END, KSK_END },
   { SDLK_PAGEUP, KSK_PAGEUP },
   { SDLK_PAGEDOWN, KSK_PAGEDOWN },
   { SDLK_F1, KSK_F1 },
   { SDLK_F2, KSK_F2 },
   { SDLK_F3, KSK_F3 },
   { SDLK_F4, KSK_F4 },
   { SDLK_F5, KSK_F5 },
   { SDLK_F6, KSK_F6 },
   { SDLK_F7, KSK_F7 },
   { SDLK_F8, KSK_F8 },
   { SDLK_F9, KSK_F9 },
   { SDLK_F10, KSK_F10 },
   { SDLK_F11, KSK_F11 },
   { SDLK_F12, KSK_F12 },
   { SDLK_F13, KSK_F13 },
   { SDLK_F14, KSK_F14 },
   { SDLK_F15, KSK_F15 },
#ifdef HAVE_SDL2
   { SDLK_NUMLOCKCLEAR, KSK_NUMLOCK },
#else
   { SDLK_NUMLOCK, KSK_NUMLOCK },
#endif
   { SDLK_CAPSLOCK, KSK_CAPSLOCK },
#ifdef HAVE_SDL2
   { SDLK_SCROLLLOCK, KSK_SCROLLOCK },
#else
   { SDLK_SCROLLOCK, KSK_SCROLLOCK },
#endif
   { SDLK_RSHIFT, KSK_RSHIFT },
   { SDLK_LSHIFT, KSK_LSHIFT },
   { SDLK_RCTRL, KSK_RCTRL },
   { SDLK_LCTRL, KSK_LCTRL },
   { SDLK_RALT, KSK_RALT },
   { SDLK_LALT, KSK_LALT },
#ifdef HAVE_SDL2
   /* { ?, KSK_RMETA }, */
   /* { ?, KSK_LMETA }, */
   { SDLK_LGUI, KSK_LSUPER },
   { SDLK_RGUI, KSK_RSUPER },
#else
   { SDLK_RMETA, KSK_RMETA },
   { SDLK_LMETA, KSK_LMETA },
   { SDLK_LSUPER, KSK_LSUPER },
   { SDLK_RSUPER, KSK_RSUPER },
#endif
   { SDLK_MODE, KSK_MODE },
#ifndef HAVE_SDL2
   { SDLK_COMPOSE, KSK_COMPOSE },
#endif
   { SDLK_HELP, KSK_HELP },
#ifdef HAVE_SDL2
   { SDLK_PRINTSCREEN, KSK_PRINT },
#else
   { SDLK_PRINT, KSK_PRINT },
#endif
   { SDLK_SYSREQ, KSK_SYSREQ },
   { SDLK_PAUSE, KSK_BREAK },
   { SDLK_MENU, KSK_MENU },
   { SDLK_POWER, KSK_POWER },

#ifndef HAVE_SDL2
   { SDLK_EURO, KSK_EURO },
#endif
   { SDLK_UNDO, KSK_UNDO },

   { 0, KSK_UNKNOWN },
};
#endif

#ifdef HAVE_DINPUT
const struct kingsn_key_map kingsn_key_map_dinput[] = {
   { DIK_LEFT, KSK_LEFT },
   { DIK_RIGHT, KSK_RIGHT },
   { DIK_UP, KSK_UP },
   { DIK_DOWN, KSK_DOWN },
   { DIK_RETURN, KSK_RETURN },
   { DIK_TAB, KSK_TAB },
   { DIK_INSERT, KSK_INSERT },
   { DIK_DELETE, KSK_DELETE },
   { DIK_RSHIFT, KSK_RSHIFT },
   { DIK_LSHIFT, KSK_LSHIFT },
   { DIK_RCONTROL, KSK_RCTRL },
   { DIK_LCONTROL, KSK_LCTRL },
   { DIK_RMENU, KSK_RALT },
   { DIK_LALT, KSK_LALT },
   { DIK_LWIN, KSK_LSUPER },
   { DIK_RWIN, KSK_RSUPER },
   { DIK_APPS, KSK_MENU },
   { DIK_END, KSK_END },
   { DIK_HOME, KSK_HOME },
   { DIK_NEXT, KSK_PAGEDOWN },
   { DIK_PRIOR, KSK_PAGEUP },
   { DIK_SPACE, KSK_SPACE },
   { DIK_ESCAPE, KSK_ESCAPE },
   { DIK_BACKSPACE, KSK_BACKSPACE },
   { DIK_NUMPADENTER, KSK_KP_ENTER },
   { DIK_NUMPADPLUS, KSK_KP_PLUS },
   { DIK_NUMPADMINUS, KSK_KP_MINUS },
   { DIK_NUMPADSTAR, KSK_KP_MULTIPLY },
   { DIK_DIVIDE, KSK_KP_DIVIDE },
   { DIK_GRAVE, KSK_BACKQUOTE },
   { DIK_PAUSE, KSK_PAUSE },
   { DIK_NUMPAD0, KSK_KP0 },
   { DIK_NUMPAD1, KSK_KP1 },
   { DIK_NUMPAD2, KSK_KP2 },
   { DIK_NUMPAD3, KSK_KP3 },
   { DIK_NUMPAD4, KSK_KP4 },
   { DIK_NUMPAD5, KSK_KP5 },
   { DIK_NUMPAD6, KSK_KP6 },
   { DIK_NUMPAD7, KSK_KP7 },
   { DIK_NUMPAD8, KSK_KP8 },
   { DIK_NUMPAD9, KSK_KP9 },
   { DIK_0, KSK_0 },
   { DIK_1, KSK_1 },
   { DIK_2, KSK_2 },
   { DIK_3, KSK_3 },
   { DIK_4, KSK_4 },
   { DIK_5, KSK_5 },
   { DIK_6, KSK_6 },
   { DIK_7, KSK_7 },
   { DIK_8, KSK_8 },
   { DIK_9, KSK_9 },
   { DIK_F1, KSK_F1 },
   { DIK_F2, KSK_F2 },
   { DIK_F3, KSK_F3 },
   { DIK_F4, KSK_F4 },
   { DIK_F5, KSK_F5 },
   { DIK_F6, KSK_F6 },
   { DIK_F7, KSK_F7 },
   { DIK_F8, KSK_F8 },
   { DIK_F9, KSK_F9 },
   { DIK_F10, KSK_F10 },
   { DIK_F11, KSK_F11 },
   { DIK_F12, KSK_F12 },
   { DIK_F13, KSK_F13 },
   { DIK_F14, KSK_F14 },
   { DIK_F15, KSK_F15 },
   { DIK_A, KSK_a },
   { DIK_B, KSK_b },
   { DIK_C, KSK_c },
   { DIK_D, KSK_d },
   { DIK_E, KSK_e },
   { DIK_F, KSK_f },
   { DIK_G, KSK_g },
   { DIK_H, KSK_h },
   { DIK_I, KSK_i },
   { DIK_J, KSK_j },
   { DIK_K, KSK_k },
   { DIK_L, KSK_l },
   { DIK_M, KSK_m },
   { DIK_N, KSK_n },
   { DIK_O, KSK_o },
   { DIK_P, KSK_p },
   { DIK_Q, KSK_q },
   { DIK_R, KSK_r },
   { DIK_S, KSK_s },
   { DIK_T, KSK_t },
   { DIK_U, KSK_u },
   { DIK_V, KSK_v },
   { DIK_W, KSK_w },
   { DIK_X, KSK_x },
   { DIK_Y, KSK_y },
   { DIK_Z, KSK_z },
   { DIK_APOSTROPHE, KSK_QUOTE },
   { DIK_COMMA, KSK_COMMA },
   { DIK_MINUS, KSK_MINUS },
   { DIK_SLASH, KSK_SLASH },
   { DIK_SEMICOLON, KSK_SEMICOLON },
   { DIK_EQUALS, KSK_EQUALS },
   { DIK_LBRACKET, KSK_LEFTBRACKET },
   { DIK_BACKSLASH, KSK_BACKSLASH },
   { DIK_RBRACKET, KSK_RIGHTBRACKET },
   { DIK_DECIMAL, KSK_KP_PERIOD },
   { DIK_PERIOD, KSK_PERIOD },
   { DIK_SCROLL, KSK_SCROLLOCK },
   { DIK_CAPSLOCK, KSK_CAPSLOCK },
   { DIK_NUMLOCK, KSK_NUMLOCK },
   { DIK_OEM_102, KSK_OEM_102 },
   { 0, KSK_UNKNOWN },
};
#endif

#ifdef EMSCRIPTEN
/* this is generated at runtime, so it isn't constant */
struct kingsn_key_map kingsn_key_map_rwebinput[KINGSN_KEY_MAP_RWEBINPUT_SIZE];
#endif

#ifdef WIIU
const struct kingsn_key_map kingsn_key_map_wiiu[] = {
   { 4, KSK_a },
   { 5, KSK_b },
   { 6, KSK_c },
   { 7, KSK_d },
   { 8, KSK_e },
   { 9, KSK_f },
   { 10, KSK_g },
   { 11, KSK_h },
   { 12, KSK_i },
   { 13, KSK_j },
   { 14, KSK_k },
   { 15, KSK_l },
   { 16, KSK_m },
   { 17, KSK_n },
   { 18, KSK_o },
   { 19, KSK_p },
   { 20, KSK_q },
   { 21, KSK_r },
   { 22, KSK_s },
   { 23, KSK_t },
   { 24, KSK_u },
   { 25, KSK_v },
   { 26, KSK_w },
   { 27, KSK_x },
   { 28, KSK_y },
   { 29, KSK_z },
   { 30, KSK_1 },
   { 31, KSK_2 },
   { 32, KSK_3 },
   { 33, KSK_4 },
   { 34, KSK_5 },
   { 35, KSK_6 },
   { 36, KSK_7 },
   { 37, KSK_8 },
   { 38, KSK_9 },
   { 39, KSK_0 },
   { 40, KSK_RETURN },
   { 41, KSK_ESCAPE },
   { 42, KSK_BACKSPACE },
   { 43, KSK_TAB },
   { 44, KSK_SPACE },
   { 45, KSK_MINUS },
   { 46, KSK_EQUALS },
   { 47, KSK_LEFTBRACKET },
   { 48, KSK_RIGHTBRACKET },
   { 49, KSK_BACKSLASH },
   { 51, KSK_SEMICOLON },
   { 52, KSK_QUOTE },
   { 53, KSK_BACKQUOTE },
   { 54, KSK_COMMA },
   { 55, KSK_PERIOD },
   { 56, KSK_SLASH },
   { 57, KSK_CAPSLOCK },
   { 58, KSK_F1 },
   { 59, KSK_F2 },
   { 60, KSK_F3 },
   { 61, KSK_F4 },
   { 62, KSK_F5 },
   { 63, KSK_F6 },
   { 64, KSK_F7 },
   { 65, KSK_F8 },
   { 66, KSK_F9 },
   { 67, KSK_F10 },
   { 68, KSK_F11 },
   { 69, KSK_F12 },
   { 71, KSK_SCROLLOCK },
   { 72, KSK_PAUSE },
   { 73, KSK_INSERT },
   { 74, KSK_HOME },
   { 75, KSK_PAGEUP },
   { 76, KSK_DELETE },
   { 77, KSK_END },
   { 78, KSK_PAGEDOWN },
   { 79, KSK_RIGHT },
   { 80, KSK_LEFT },
   { 81, KSK_DOWN },
   { 82, KSK_UP },
   { 83, KSK_NUMLOCK },
   { 84, KSK_KP_DIVIDE },
   { 85, KSK_KP_MULTIPLY },
   { 86, KSK_KP_MINUS },
   { 87, KSK_KP_PLUS },
   { 88, KSK_KP_ENTER },
   { 89, KSK_KP1 },
   { 90, KSK_KP2 },
   { 91, KSK_KP3 },
   { 92, KSK_KP4 },
   { 93, KSK_KP5 },
   { 94, KSK_KP6 },
   { 95, KSK_KP7 },
   { 96, KSK_KP8 },
   { 97, KSK_KP9 },
   { 98, KSK_KP0 },
   { 99, KSK_KP_PERIOD },
   { 224, KSK_LCTRL },
   { 225, KSK_LSHIFT },
   { 226, KSK_LALT },
   { 228, KSK_RCTRL },
   { 229, KSK_RSHIFT },
   { 230, KSK_RALT },
   { 0, KSK_UNKNOWN },
};
#endif

#ifdef HAVE_X11

const struct kingsn_key_map kingsn_key_map_x11[] = {

   { XFVK_ESC, KSK_ESCAPE },
   { XFVK_FK01, KSK_F1 },
   { XFVK_FK02, KSK_F2 },
   { XFVK_FK03, KSK_F3 },
   { XFVK_FK04, KSK_F4 },
   { XFVK_FK05, KSK_F5 },
   { XFVK_FK06, KSK_F6 },
   { XFVK_FK07, KSK_F7 },
   { XFVK_FK08, KSK_F8 },
   { XFVK_FK09, KSK_F9 },
   { XFVK_FK10, KSK_F10 },
   { XFVK_FK11, KSK_F11 },
   { XFVK_FK12, KSK_F12 },

   { XFVK_TLDE, KSK_BACKQUOTE },
   { XFVK_AE01, KSK_1 },
   { XFVK_AE02, KSK_2 },
   { XFVK_AE03, KSK_3 },
   { XFVK_AE04, KSK_4 },
   { XFVK_AE05, KSK_5 },
   { XFVK_AE06, KSK_6 },
   { XFVK_AE07, KSK_7 },
   { XFVK_AE08, KSK_8 },
   { XFVK_AE09, KSK_9 },
   { XFVK_AE10, KSK_0 },
   { XFVK_AE11, KSK_MINUS },
   { XFVK_AE12, KSK_EQUALS },
   { XFVK_BKSP, KSK_BACKSPACE },

   { XFVK_TAB, KSK_TAB },
   { XFVK_AD01, KSK_q },
   { XFVK_AD02, KSK_w },
   { XFVK_AD03, KSK_e },
   { XFVK_AD04, KSK_r },
   { XFVK_AD05, KSK_t },
   { XFVK_AD06, KSK_y },
   { XFVK_AD07, KSK_u },
   { XFVK_AD08, KSK_i },
   { XFVK_AD09, KSK_o },
   { XFVK_AD10, KSK_p },
   { XFVK_AD11, KSK_LEFTBRACKET },
   { XFVK_AD12, KSK_RIGHTBRACKET },
   { XFVK_RTRN, KSK_RETURN },

   { XFVK_CAPS, KSK_CAPSLOCK },
   { XFVK_AC01, KSK_a },
   { XFVK_AC02, KSK_s },
   { XFVK_AC03, KSK_d },
   { XFVK_AC04, KSK_f },
   { XFVK_AC05, KSK_g },
   { XFVK_AC06, KSK_h },
   { XFVK_AC07, KSK_j },
   { XFVK_AC08, KSK_k },
   { XFVK_AC09, KSK_l },
   { XFVK_AC10, KSK_SEMICOLON },
   { XFVK_AC11, KSK_QUOTE },
   { XFVK_AC12, KSK_BACKSLASH },

   { XFVK_LFSH, KSK_LSHIFT },
   { XFVK_AB01, KSK_z },
   { XFVK_AB02, KSK_x },
   { XFVK_AB03, KSK_c },
   { XFVK_AB04, KSK_v },
   { XFVK_AB05, KSK_b },
   { XFVK_AB06, KSK_n },
   { XFVK_AB07, KSK_m },
   { XFVK_AB08, KSK_COMMA },
   { XFVK_AB09, KSK_PERIOD },
   { XFVK_AB10, KSK_SLASH },
   { XFVK_RTSH, KSK_RSHIFT },

   { XFVK_LALT, KSK_LALT },
   { XFVK_LCTL, KSK_LCTRL },
   { XFVK_SPCE, KSK_SPACE },
   { XFVK_RCTL, KSK_RCTRL },
   { XFVK_RALT, KSK_RALT },

   { XFVK_LSGT, KSK_OEM_102 },
   { XFVK_MENU, KSK_MENU },
   { XFVK_LWIN, KSK_LSUPER },
   { XFVK_RWIN, KSK_RSUPER },
   { XFVK_CALC, KSK_HELP },

   { XFVK_PRSC, KSK_PRINT },
   { XFVK_SCLK, KSK_SCROLLOCK },
   { XFVK_PAUS, KSK_PAUSE },
   { XFVK_INS, KSK_INSERT },
   { XFVK_HOME, KSK_HOME },
   { XFVK_PGUP, KSK_PAGEUP },
   { XFVK_DELE, KSK_DELETE },
   { XFVK_END, KSK_END },
   { XFVK_PGDN, KSK_PAGEDOWN },
   { XFVK_UP, KSK_UP },
   { XFVK_LEFT, KSK_LEFT },
   { XFVK_DOWN, KSK_DOWN },
   { XFVK_RGHT, KSK_RIGHT },

   { XFVK_NMLK, KSK_NUMLOCK },
   { XFVK_KPDV, KSK_KP_DIVIDE },
   { XFVK_KPMU, KSK_KP_MULTIPLY },
   { XFVK_KPSU, KSK_KP_MINUS },
   { XFVK_KP7, KSK_KP7 },
   { XFVK_KP8, KSK_KP8 },
   { XFVK_KP9, KSK_KP9 },
   { XFVK_KPAD, KSK_KP_PLUS },
   { XFVK_KP4, KSK_KP4 },
   { XFVK_KP5, KSK_KP5 },
   { XFVK_KP6, KSK_KP6 },
   { XFVK_KP1, KSK_KP1 },
   { XFVK_KP2, KSK_KP2 },
   { XFVK_KP3, KSK_KP3 },
   { XFVK_KPEN, KSK_KP_ENTER },
   { XFVK_KP0, KSK_KP0 },
   { XFVK_KPDL, KSK_KP_PERIOD },
   { XFVK_KPEQ, KSK_KP_EQUALS },

   { 0, KSK_UNKNOWN },
};
#endif

#if defined(__linux__) || defined(HAVE_WAYLAND)
/* Note: Only one input can be mapped to each
 * KSK_* key. If several physical inputs
 * correspond to the same key, these inputs
 * must be merged at the input driver level */
const struct kingsn_key_map kingsn_key_map_linux[] = {
   { KEY_BACKSPACE, KSK_BACKSPACE },
   { KEY_TAB, KSK_TAB },
   { KEY_CLEAR, KSK_CLEAR },
   /* { KEY_EXIT, KSK_CLEAR }, */     /* Duplicate - Skip */
   { KEY_ENTER, KSK_RETURN },
   /* { KEY_OK, KSK_RETURN }, */      /* Duplicate - Skip */
   /* { KEY_SELECT, KSK_RETURN }, */  /* Duplicate - Skip */
   { KEY_PAUSE, KSK_PAUSE },
   { KEY_ESC, KSK_ESCAPE },
   { KEY_SPACE, KSK_SPACE },
   /* { ?, KSK_EXCLAIM }, */
   /* { ?, KSK_QUOTEDBL }, */
   /* { ?, KSK_HASH }, */
#ifndef ANDROID
   { KEY_DOLLAR, KSK_DOLLAR },
#endif
   /* { ?, KSK_AMPERSAND }, */
   { KEY_APOSTROPHE, KSK_QUOTE },
   { KEY_KPLEFTPAREN, KSK_LEFTPAREN },
   { KEY_KPRIGHTPAREN, KSK_RIGHTPAREN },
   { KEY_KPASTERISK, KSK_ASTERISK },
   { KEY_KPPLUS, KSK_PLUS },
   { KEY_COMMA, KSK_COMMA },
   { KEY_MINUS, KSK_MINUS },
   { KEY_DOT, KSK_PERIOD },
   { KEY_SLASH, KSK_SLASH },
   { KEY_0, KSK_0 },
   { KEY_1, KSK_1 },
   { KEY_2, KSK_2 },
   { KEY_3, KSK_3 },
   { KEY_4, KSK_4 },
   { KEY_5, KSK_5 },
   { KEY_6, KSK_6 },
   { KEY_7, KSK_7 },
   { KEY_8, KSK_8 },
   { KEY_9, KSK_9 },
   /* { KEY_COLON, KSK_COLON }, */
   { KEY_SEMICOLON, KSK_SEMICOLON },
   /* { KEY_LESS, KSK_LESS }, */
   { KEY_EQUAL, KSK_EQUALS },
   /* { KEY_GREATER, KSK_GREATER }, */
   { KEY_QUESTION, KSK_QUESTION },
   /* { KEY_AT, KSK_AT }, */
   { KEY_LEFTBRACE, KSK_LEFTBRACKET },
   { KEY_BACKSLASH, KSK_BACKSLASH },
   { KEY_RIGHTBRACE, KSK_RIGHTBRACKET },
   /* { ?, KSK_CARET }, */
   /* { ?, KSK_UNDERSCORE }, */
   { KEY_GRAVE, KSK_BACKQUOTE },
   { KEY_A, KSK_a },
   { KEY_B, KSK_b },
   { KEY_C, KSK_c },
   { KEY_D, KSK_d },
   { KEY_E, KSK_e },
   { KEY_F, KSK_f },
   { KEY_G, KSK_g },
   { KEY_H, KSK_h },
   { KEY_I, KSK_i },
   { KEY_J, KSK_j },
   { KEY_K, KSK_k },
   { KEY_L, KSK_l },
   { KEY_M, KSK_m },
   { KEY_N, KSK_n },
   { KEY_O, KSK_o },
   { KEY_P, KSK_p },
   { KEY_Q, KSK_q },
   { KEY_R, KSK_r },
   { KEY_S, KSK_s },
   { KEY_T, KSK_t },
   { KEY_U, KSK_u },
   { KEY_V, KSK_v },
   { KEY_W, KSK_w },
   { KEY_X, KSK_x },
   { KEY_Y, KSK_y },
   { KEY_Z, KSK_z },
   { KEY_DELETE, KSK_DELETE },
   { KEY_KP0, KSK_KP0 },
   { KEY_KP1, KSK_KP1 },
   { KEY_KP2, KSK_KP2 },
   { KEY_KP3, KSK_KP3 },
   { KEY_KP4, KSK_KP4 },
   { KEY_KP5, KSK_KP5 },
   { KEY_KP6, KSK_KP6 },
   { KEY_KP7, KSK_KP7 },
   { KEY_KP8, KSK_KP8 },
   { KEY_KP9, KSK_KP9 },
   { KEY_KPDOT, KSK_KP_PERIOD },
   { KEY_KPSLASH, KSK_KP_DIVIDE },
   { KEY_KPASTERISK, KSK_KP_MULTIPLY },
   { KEY_KPMINUS, KSK_KP_MINUS },
   { KEY_KPPLUS, KSK_KP_PLUS },
   { KEY_KPENTER, KSK_KP_ENTER },
   { KEY_KPEQUAL, KSK_KP_EQUALS },
   { KEY_UP, KSK_UP },
   { KEY_DOWN, KSK_DOWN },
   { KEY_RIGHT, KSK_RIGHT },
   { KEY_LEFT, KSK_LEFT },
   { KEY_INSERT, KSK_INSERT },
   { KEY_HOME, KSK_HOME },
   { KEY_END, KSK_END },
   { KEY_PAGEUP, KSK_PAGEUP },
   { KEY_PAGEDOWN, KSK_PAGEDOWN },
   { KEY_F1, KSK_F1 },
   { KEY_F2, KSK_F2 },
   { KEY_F3, KSK_F3 },
   { KEY_F4, KSK_F4 },
   { KEY_F5, KSK_F5 },
   { KEY_F6, KSK_F6 },
   { KEY_F7, KSK_F7 },
   { KEY_F8, KSK_F8 },
   { KEY_F9, KSK_F9 },
   { KEY_F10, KSK_F10 },
   { KEY_F11, KSK_F11 },
   { KEY_F12, KSK_F12 },
   { KEY_F13, KSK_F13 },
   { KEY_F14, KSK_F14 },
   { KEY_F15, KSK_F15 },
   { KEY_NUMLOCK, KSK_NUMLOCK },
   { KEY_CAPSLOCK, KSK_CAPSLOCK },
   { KEY_SCROLLLOCK, KSK_SCROLLOCK },
   { KEY_RIGHTSHIFT, KSK_RSHIFT },
   { KEY_LEFTSHIFT, KSK_LSHIFT },
   { KEY_RIGHTCTRL, KSK_RCTRL },
   { KEY_LEFTCTRL, KSK_LCTRL },
   { KEY_RIGHTALT, KSK_RALT },
   { KEY_LEFTALT, KSK_LALT },
   /* { ?, KSK_RMETA }, */
   /* { ?, KSK_LMETA }, */
   { KEY_LEFTMETA, KSK_LSUPER },
   { KEY_RIGHTMETA, KSK_RSUPER },
   { KEY_MODE, KSK_MODE },
   { KEY_COMPOSE, KSK_COMPOSE },
   { KEY_HELP, KSK_HELP },
   { KEY_PRINT, KSK_PRINT },
   { KEY_SYSRQ, KSK_SYSREQ },
   { KEY_BREAK, KSK_BREAK },
   { KEY_MENU, KSK_MENU },
   { KEY_POWER, KSK_POWER },
#ifndef ANDROID
   { KEY_EURO, KSK_EURO },
#endif
   { KEY_UNDO, KSK_UNDO },
   { KEY_102ND, KSK_OEM_102 },
   { 0, KSK_UNKNOWN },
};
#endif

#ifdef ANDROID
const struct kingsn_key_map kingsn_key_map_android[] = {
   { AKEYCODE_DEL, KSK_BACKSPACE },
   { AKEYCODE_TAB, KSK_TAB },
   { AKEYCODE_CLEAR, KSK_CLEAR },
   { AKEYCODE_ENTER, KSK_RETURN },
   { AKEYCODE_BREAK, KSK_PAUSE },
   { AKEYCODE_ESCAPE, KSK_ESCAPE },
   { AKEYCODE_SPACE, KSK_SPACE },
   { AKEYCODE_APOSTROPHE, KSK_QUOTE },
   { AKEYCODE_NUMPAD_LEFT_PAREN, KSK_LEFTPAREN },
   { AKEYCODE_NUMPAD_RIGHT_PAREN, KSK_RIGHTPAREN },
   { AKEYCODE_NUMPAD_MULTIPLY, KSK_ASTERISK },
   { AKEYCODE_NUMPAD_ADD, KSK_PLUS },
   { AKEYCODE_COMMA, KSK_COMMA },
   { AKEYCODE_MINUS, KSK_MINUS },
   { AKEYCODE_PERIOD, KSK_PERIOD },
   { AKEYCODE_SLASH, KSK_SLASH },
   { AKEYCODE_0, KSK_0 },
   { AKEYCODE_1, KSK_1 },
   { AKEYCODE_2, KSK_2 },
   { AKEYCODE_3, KSK_3 },
   { AKEYCODE_4, KSK_4 },
   { AKEYCODE_5, KSK_5 },
   { AKEYCODE_6, KSK_6 },
   { AKEYCODE_7, KSK_7 },
   { AKEYCODE_8, KSK_8 },
   { AKEYCODE_9, KSK_9 },
   { AKEYCODE_SEMICOLON, KSK_SEMICOLON },
   { AKEYCODE_EQUALS, KSK_EQUALS },
   { AKEYCODE_LEFT_BRACKET, KSK_LEFTBRACKET },
   { AKEYCODE_BACKSLASH, KSK_BACKSLASH },
   { AKEYCODE_RIGHT_BRACKET, KSK_RIGHTBRACKET },
   { AKEYCODE_GRAVE, KSK_BACKQUOTE },
   { AKEYCODE_A, KSK_a },
   { AKEYCODE_B, KSK_b },
   { AKEYCODE_C, KSK_c },
   { AKEYCODE_D, KSK_d },
   { AKEYCODE_E, KSK_e },
   { AKEYCODE_F, KSK_f },
   { AKEYCODE_G, KSK_g },
   { AKEYCODE_H, KSK_h },
   { AKEYCODE_I, KSK_i },
   { AKEYCODE_J, KSK_j },
   { AKEYCODE_K, KSK_k },
   { AKEYCODE_L, KSK_l },
   { AKEYCODE_M, KSK_m },
   { AKEYCODE_N, KSK_n },
   { AKEYCODE_O, KSK_o },
   { AKEYCODE_P, KSK_p },
   { AKEYCODE_Q, KSK_q },
   { AKEYCODE_R, KSK_r },
   { AKEYCODE_S, KSK_s },
   { AKEYCODE_T, KSK_t },
   { AKEYCODE_U, KSK_u },
   { AKEYCODE_V, KSK_v },
   { AKEYCODE_W, KSK_w },
   { AKEYCODE_X, KSK_x },
   { AKEYCODE_Y, KSK_y },
   { AKEYCODE_Z, KSK_z },
   { AKEYCODE_DEL, KSK_DELETE },
   { AKEYCODE_NUMPAD_0, KSK_KP0 },
   { AKEYCODE_NUMPAD_1, KSK_KP1 },
   { AKEYCODE_NUMPAD_2, KSK_KP2 },
   { AKEYCODE_NUMPAD_3, KSK_KP3 },
   { AKEYCODE_NUMPAD_4, KSK_KP4 },
   { AKEYCODE_NUMPAD_5, KSK_KP5 },
   { AKEYCODE_NUMPAD_6, KSK_KP6 },
   { AKEYCODE_NUMPAD_7, KSK_KP7 },
   { AKEYCODE_NUMPAD_8, KSK_KP8 },
   { AKEYCODE_NUMPAD_9, KSK_KP9 },
   { AKEYCODE_NUMPAD_DOT, KSK_KP_PERIOD },
   { AKEYCODE_NUMPAD_DIVIDE, KSK_KP_DIVIDE },
   { AKEYCODE_NUMPAD_MULTIPLY, KSK_KP_MULTIPLY },
   { AKEYCODE_NUMPAD_SUBTRACT, KSK_KP_MINUS },
   { AKEYCODE_NUMPAD_ADD, KSK_KP_PLUS },
   { AKEYCODE_NUMPAD_ENTER, KSK_KP_ENTER },
   { AKEYCODE_NUMPAD_EQUALS, KSK_KP_EQUALS },
   { AKEYCODE_DPAD_UP, KSK_UP },
   { AKEYCODE_DPAD_DOWN, KSK_DOWN },
   { AKEYCODE_DPAD_RIGHT, KSK_RIGHT },
   { AKEYCODE_DPAD_LEFT, KSK_LEFT },
   { AKEYCODE_INSERT, KSK_INSERT },
   { AKEYCODE_MOVE_HOME, KSK_HOME },
   { AKEYCODE_MOVE_END, KSK_END },
   { AKEYCODE_PAGE_UP, KSK_PAGEUP },
   { AKEYCODE_PAGE_DOWN, KSK_PAGEDOWN },
   { AKEYCODE_F1, KSK_F1 },
   { AKEYCODE_F2, KSK_F2 },
   { AKEYCODE_F3, KSK_F3 },
   { AKEYCODE_F4, KSK_F4 },
   { AKEYCODE_F5, KSK_F5 },
   { AKEYCODE_F6, KSK_F6 },
   { AKEYCODE_F7, KSK_F7 },
   { AKEYCODE_F8, KSK_F8 },
   { AKEYCODE_F9, KSK_F9 },
   { AKEYCODE_F10, KSK_F10 },
   { AKEYCODE_F11, KSK_F11 },
   { AKEYCODE_F12, KSK_F12 },
   { AKEYCODE_NUM_LOCK, KSK_NUMLOCK },
   { AKEYCODE_CAPS_LOCK, KSK_CAPSLOCK },
   { AKEYCODE_SCROLL_LOCK, KSK_SCROLLOCK },
   { AKEYCODE_SHIFT_LEFT, KSK_RSHIFT },
   { AKEYCODE_SHIFT_RIGHT, KSK_LSHIFT },
   { AKEYCODE_CTRL_RIGHT, KSK_RCTRL },
   { AKEYCODE_CTRL_LEFT, KSK_LCTRL },
   { AKEYCODE_ALT_RIGHT, KSK_RALT },
   { AKEYCODE_ALT_LEFT, KSK_LALT },
   { 0, KSK_UNKNOWN },
};
#endif

#ifdef __QNX__
const struct kingsn_key_map kingsn_key_map_qnx[] = {
   { KEYCODE_BACKSPACE, KSK_BACKSPACE },
   { KEYCODE_RETURN, KSK_RETURN },
   { KEYCODE_SPACE, KSK_SPACE },
   { KEYCODE_UP, KSK_UP },
   { KEYCODE_DOWN, KSK_DOWN },
   { KEYCODE_LEFT, KSK_LEFT },
   { KEYCODE_RIGHT, KSK_RIGHT },
   { KEYCODE_A, KSK_a },
   { KEYCODE_B, KSK_b },
   { KEYCODE_C, KSK_c },
   { KEYCODE_D, KSK_d },
   { KEYCODE_E, KSK_e },
   { KEYCODE_F, KSK_f },
   { KEYCODE_G, KSK_g },
   { KEYCODE_H, KSK_h },
   { KEYCODE_I, KSK_i },
   { KEYCODE_J, KSK_j },
   { KEYCODE_K, KSK_k },
   { KEYCODE_L, KSK_l },
   { KEYCODE_M, KSK_m },
   { KEYCODE_N, KSK_n },
   { KEYCODE_O, KSK_o },
   { KEYCODE_P, KSK_p },
   { KEYCODE_Q, KSK_q },
   { KEYCODE_R, KSK_r },
   { KEYCODE_S, KSK_s },
   { KEYCODE_T, KSK_t },
   { KEYCODE_U, KSK_u },
   { KEYCODE_V, KSK_v },
   { KEYCODE_W, KSK_w },
   { KEYCODE_X, KSK_x },
   { KEYCODE_Y, KSK_y },
   { KEYCODE_Z, KSK_z },
   { KEYCODE_ZERO, KSK_0 },
   { KEYCODE_ONE, KSK_1 },
   { KEYCODE_TWO, KSK_2 },
   { KEYCODE_THREE, KSK_3 },
   { KEYCODE_FOUR, KSK_4 },
   { KEYCODE_FIVE, KSK_5 },
   { KEYCODE_SIX, KSK_6 },
   { KEYCODE_SEVEN, KSK_7 },
   { KEYCODE_EIGHT, KSK_8 },
   { KEYCODE_NINE, KSK_9 },
   { KEYCODE_INSERT, KSK_INSERT },
   { KEYCODE_HOME, KSK_HOME },
   { KEYCODE_END, KSK_END },
   { KEYCODE_PG_UP, KSK_PAGEUP },
   { KEYCODE_PG_DOWN, KSK_PAGEDOWN },
   { KEYCODE_F1, KSK_F1 },
   { KEYCODE_F2, KSK_F2 },
   { KEYCODE_F3, KSK_F3 },
   { KEYCODE_F4, KSK_F4 },
   { KEYCODE_F5, KSK_F5 },
   { KEYCODE_F6, KSK_F6 },
   { KEYCODE_F7, KSK_F7 },
   { KEYCODE_F8, KSK_F8 },
   { KEYCODE_F9, KSK_F9 },
   { KEYCODE_F10, KSK_F10 },
   { KEYCODE_F11, KSK_F11 },
   { KEYCODE_F12, KSK_F12 },
   { KEYCODE_LEFT_SHIFT, KSK_LSHIFT },
   { KEYCODE_RIGHT_SHIFT, KSK_RSHIFT },
   { KEYCODE_LEFT_CTRL, KSK_LCTRL },
   { KEYCODE_RIGHT_CTRL, KSK_RCTRL },
   { KEYCODE_LEFT_ALT, KSK_LALT },
   { KEYCODE_RIGHT_ALT, KSK_RALT },
   // TODO/FIXME: Code for 'sym' key on BB keyboards. Figure out which sys/keycodes.h define this maps to.
   { 61651, KSK_RSUPER },
   { KEYCODE_DOLLAR, KSK_DOLLAR },
   { KEYCODE_MENU, KSK_MENU },
   { 0, KSK_UNKNOWN },
};
#endif

#ifdef __APPLE__
const struct kingsn_key_map kingsn_key_map_apple_hid[] = {
   { KEY_Delete, KSK_BACKSPACE },
   { KEY_Tab, KSK_TAB },
   /* { ?, KSK_CLEAR }, */
   { KEY_Enter, KSK_RETURN },
   { KEY_Pause, KSK_PAUSE },
   { KEY_Escape, KSK_ESCAPE },
   { KEY_Space, KSK_SPACE },
   /* { ?, KSK_EXCLAIM }, */
   /* { ?, KSK_QUOTEDBL }, */
   /* { ?, KSK_HASH }, */
   /* { ?, KSK_DOLLAR }, */
   /* { ?, KSK_AMPERSAND }, */
   { KEY_Quote, KSK_QUOTE },
   /* { ?, KSK_LEFTPAREN }, */
   /* { ?, KSK_RIGHTPAREN }, */
   /* { ?, KSK_ASTERISK }, */
   /* { ?, KSK_PLUS }, */
   { KEY_Comma, KSK_COMMA },
   { KEY_Minus, KSK_MINUS },
   { KEY_Period, KSK_PERIOD },
   { KEY_Slash, KSK_SLASH },
   { KEY_0, KSK_0 },
   { KEY_1, KSK_1 },
   { KEY_2, KSK_2 },
   { KEY_3, KSK_3 },
   { KEY_4, KSK_4 },
   { KEY_5, KSK_5 },
   { KEY_6, KSK_6 },
   { KEY_7, KSK_7 },
   { KEY_8, KSK_8 },
   { KEY_9, KSK_9 },
   /* { ?, KSK_COLON }, */
   { KEY_Semicolon, KSK_SEMICOLON },
   /* { ?, KSK_LESS }, */
   { KEY_Equals, KSK_EQUALS },
   /* { ?, KSK_GREATER }, */
   /* { ?, KSK_QUESTION }, */
   /* { ?, KSK_AT }, */
   { KEY_LeftBracket, KSK_LEFTBRACKET },
   { KEY_Backslash, KSK_BACKSLASH },
   { KEY_RightBracket, KSK_RIGHTBRACKET },
   /* { ?, KSK_CARET }, */
   /* { ?, KSK_UNDERSCORE }, */
   { KEY_Grave, KSK_BACKQUOTE },
   { KEY_A, KSK_a },
   { KEY_B, KSK_b },
   { KEY_C, KSK_c },
   { KEY_D, KSK_d },
   { KEY_E, KSK_e },
   { KEY_F, KSK_f },
   { KEY_G, KSK_g },
   { KEY_H, KSK_h },
   { KEY_I, KSK_i },
   { KEY_J, KSK_j },
   { KEY_K, KSK_k },
   { KEY_L, KSK_l },
   { KEY_M, KSK_m },
   { KEY_N, KSK_n },
   { KEY_O, KSK_o },
   { KEY_P, KSK_p },
   { KEY_Q, KSK_q },
   { KEY_R, KSK_r },
   { KEY_S, KSK_s },
   { KEY_T, KSK_t },
   { KEY_U, KSK_u },
   { KEY_V, KSK_v },
   { KEY_W, KSK_w },
   { KEY_X, KSK_x },
   { KEY_Y, KSK_y },
   { KEY_Z, KSK_z },
   { KEY_DeleteForward, KSK_DELETE },

   { KP_0, KSK_KP0 },
   { KP_1, KSK_KP1 },
   { KP_2, KSK_KP2 },
   { KP_3, KSK_KP3 },
   { KP_4, KSK_KP4 },
   { KP_5, KSK_KP5 },
   { KP_6, KSK_KP6 },
   { KP_7, KSK_KP7 },
   { KP_8, KSK_KP8 },
   { KP_9, KSK_KP9 },
   { KP_Point, KSK_KP_PERIOD },
   { KP_Divide, KSK_KP_DIVIDE },
   { KP_Multiply, KSK_KP_MULTIPLY },
   { KP_Subtract, KSK_KP_MINUS },
   { KP_Add, KSK_KP_PLUS },
   { KP_Enter, KSK_KP_ENTER },
   { KP_Equals, KSK_KP_EQUALS },

   { KEY_Up, KSK_UP },
   { KEY_Down, KSK_DOWN },
   { KEY_Right, KSK_RIGHT },
   { KEY_Left, KSK_LEFT },
   { KEY_Insert, KSK_INSERT },
   { KEY_Home, KSK_HOME },
   { KEY_End, KSK_END },
   { KEY_PageUp, KSK_PAGEUP },
   { KEY_PageDown, KSK_PAGEDOWN },

   { KEY_F1, KSK_F1 },
   { KEY_F2, KSK_F2 },
   { KEY_F3, KSK_F3 },
   { KEY_F4, KSK_F4 },
   { KEY_F5, KSK_F5 },
   { KEY_F6, KSK_F6 },
   { KEY_F7, KSK_F7 },
   { KEY_F8, KSK_F8 },
   { KEY_F9, KSK_F9 },
   { KEY_F10, KSK_F10 },
   { KEY_F11, KSK_F11 },
   { KEY_F12, KSK_F12 },
   { KEY_F13, KSK_F13 },
   { KEY_F14, KSK_F14 },
   { KEY_F15, KSK_F15 },

   /* { ?, KSK_NUMLOCK }, */
   { KEY_CapsLock, KSK_CAPSLOCK },
   /* { ?, KSK_SCROLLOCK }, */
   { KEY_RightShift, KSK_RSHIFT },
   { KEY_LeftShift, KSK_LSHIFT },
   { KEY_RightControl, KSK_RCTRL },
   { KEY_LeftControl, KSK_LCTRL },
   { KEY_RightAlt, KSK_RALT },
   { KEY_LeftAlt, KSK_LALT },
   { KEY_RightGUI, KSK_RMETA },
   { KEY_LeftGUI, KSK_RMETA },
   /* { ?, KSK_LSUPER }, */
   /* { ?, KSK_RSUPER }, */
   /* { ?, KSK_MODE }, */
   /* { ?, KSK_COMPOSE }, */

   /* { ?, KSK_HELP }, */
   { KEY_PrintScreen, KSK_PRINT },
   /* { ?, KSK_SYSREQ }, */
   /* { ?, KSK_BREAK }, */
   { KEY_Menu, KSK_MENU },
   /* { ?, KSK_POWER }, */
   /* { ?, KSK_EURO }, */
   /* { ?, KSK_UNDO }, */
   { KEY_NonUSBackslash, KSK_OEM_102 },
   { 0, KSK_UNKNOWN }
};
#endif

#ifdef DJGPP
const struct kingsn_key_map kingsn_key_map_dos[] = {
   { DOSKEY_ESCAPE, KSK_ESCAPE },
   { DOSKEY_F1, KSK_F1 },
   { DOSKEY_F2, KSK_F2 },
   { DOSKEY_F3, KSK_F3 },
   { DOSKEY_F4, KSK_F4 },
   { DOSKEY_F5, KSK_F5 },
   { DOSKEY_F6, KSK_F6 },
   { DOSKEY_F7, KSK_F7 },
   { DOSKEY_F8, KSK_F8 },
   { DOSKEY_F9, KSK_F9 },
   { DOSKEY_F10, KSK_F10 },

   { DOSKEY_BACKQUOTE, KSK_BACKQUOTE },
   { DOSKEY_1, KSK_1 },
   { DOSKEY_2, KSK_2 },
   { DOSKEY_3, KSK_3 },
   { DOSKEY_4, KSK_4 },
   { DOSKEY_5, KSK_5 },
   { DOSKEY_6, KSK_6 },
   { DOSKEY_7, KSK_7 },
   { DOSKEY_8, KSK_8 },
   { DOSKEY_9, KSK_9 },
   { DOSKEY_0, KSK_0 },
   { DOSKEY_MINUS, KSK_MINUS },
   { DOSKEY_EQUAL, KSK_EQUALS },
   { DOSKEY_BACKSPACE, KSK_BACKSPACE },

   { DOSKEY_TAB, KSK_TAB },
   { DOSKEY_q, KSK_q },
   { DOSKEY_w, KSK_w },
   { DOSKEY_e, KSK_e },
   { DOSKEY_r, KSK_r },
   { DOSKEY_t, KSK_t },
   { DOSKEY_y, KSK_y },
   { DOSKEY_u, KSK_u },
   { DOSKEY_i, KSK_i },
   { DOSKEY_o, KSK_o },
   { DOSKEY_p, KSK_p },
   { DOSKEY_LBRACKET, KSK_LEFTBRACKET },
   { DOSKEY_RBRACKET, KSK_RIGHTBRACKET },
   { DOSKEY_BACKSLASH, KSK_BACKSLASH },

   { DOSKEY_CAPSLOCK, KSK_CAPSLOCK },
   { DOSKEY_a, KSK_a },
   { DOSKEY_s, KSK_s },
   { DOSKEY_d, KSK_d },
   { DOSKEY_f, KSK_f },
   { DOSKEY_g, KSK_g },
   { DOSKEY_h, KSK_h },
   { DOSKEY_j, KSK_j },
   { DOSKEY_k, KSK_k },
   { DOSKEY_l, KSK_l },
   { DOSKEY_SEMICOLON, KSK_SEMICOLON },
   { DOSKEY_QUOTE, KSK_QUOTE },
   { DOSKEY_RETURN, KSK_RETURN },

   { DOSKEY_LSHIFT, KSK_LSHIFT },
   { DOSKEY_z, KSK_z },
   { DOSKEY_x, KSK_x },
   { DOSKEY_c, KSK_c },
   { DOSKEY_v, KSK_v },
   { DOSKEY_b, KSK_b },
   { DOSKEY_n, KSK_n },
   { DOSKEY_m, KSK_m },
   { DOSKEY_COMMA, KSK_COMMA },
   { DOSKEY_PERIOD, KSK_PERIOD },
   { DOSKEY_SLASH, KSK_SLASH },
   { DOSKEY_RSHIFT, KSK_RSHIFT },

   { DOSKEY_LCTRL, KSK_LCTRL },
   { DOSKEY_LSUPER, KSK_LSUPER },
   { DOSKEY_LALT, KSK_LALT },
   { DOSKEY_SPACE, KSK_SPACE },
   { DOSKEY_RALT, KSK_RALT },
   { DOSKEY_RSUPER, KSK_RSUPER },
   { DOSKEY_MENU, KSK_MENU },
   { DOSKEY_RCTRL, KSK_RCTRL },

   { DOSKEY_UP, KSK_UP },
   { DOSKEY_DOWN, KSK_DOWN },
   { DOSKEY_LEFT, KSK_LEFT },
   { DOSKEY_RIGHT, KSK_RIGHT },

   { DOSKEY_HOME, KSK_HOME },
   { DOSKEY_END, KSK_END },
   { DOSKEY_PGUP, KSK_PAGEUP },
   { DOSKEY_PGDN, KSK_PAGEDOWN },

   { 0, KSK_UNKNOWN }
};
#endif

#ifdef __PSL1GHT__
const struct kingsn_key_map kingsn_key_map_psl1ght[] = {
   { KB_RAWKEY_A, KSK_a },
   { KB_RAWKEY_B, KSK_b },
   { KB_RAWKEY_C, KSK_c },
   { KB_RAWKEY_D, KSK_d },
   { KB_RAWKEY_E, KSK_e },
   { KB_RAWKEY_F, KSK_f },
   { KB_RAWKEY_G, KSK_g },
   { KB_RAWKEY_H, KSK_h },
   { KB_RAWKEY_I, KSK_i },
   { KB_RAWKEY_J, KSK_j },
   { KB_RAWKEY_K, KSK_k },
   { KB_RAWKEY_L, KSK_l },
   { KB_RAWKEY_M, KSK_m },
   { KB_RAWKEY_N, KSK_n },
   { KB_RAWKEY_O, KSK_o },
   { KB_RAWKEY_P, KSK_p },
   { KB_RAWKEY_Q, KSK_q },
   { KB_RAWKEY_R, KSK_r },
   { KB_RAWKEY_S, KSK_s },
   { KB_RAWKEY_T, KSK_t },
   { KB_RAWKEY_U, KSK_u },
   { KB_RAWKEY_V, KSK_v },
   { KB_RAWKEY_W, KSK_w },
   { KB_RAWKEY_X, KSK_x },
   { KB_RAWKEY_Y, KSK_y },
   { KB_RAWKEY_Z, KSK_z },
   { KB_RAWKEY_BS, KSK_BACKSPACE },
   { KB_RAWKEY_TAB, KSK_TAB },
   { KB_RAWKEY_ENTER, KSK_RETURN },
   { KB_RAWKEY_PAUSE, KSK_PAUSE },
   { KB_RAWKEY_ESCAPE, KSK_ESCAPE },
   { KB_RAWKEY_SPACE, KSK_SPACE },
   { KB_RAWKEY_QUOTATION_101, KSK_QUOTE },
   { KB_RAWKEY_COMMA, KSK_COMMA },
   { KB_RAWKEY_MINUS, KSK_MINUS },
   { KB_RAWKEY_PERIOD, KSK_PERIOD },
   { KB_RAWKEY_SLASH, KSK_SLASH },
   { KB_RAWKEY_0, KSK_0 },
   { KB_RAWKEY_1, KSK_1 },
   { KB_RAWKEY_2, KSK_2 },
   { KB_RAWKEY_3, KSK_3 },
   { KB_RAWKEY_4, KSK_4 },
   { KB_RAWKEY_5, KSK_5 },
   { KB_RAWKEY_6, KSK_6 },
   { KB_RAWKEY_7, KSK_7 },
   { KB_RAWKEY_8, KSK_8 },
   { KB_RAWKEY_9, KSK_9 },
   { KB_RAWKEY_SEMICOLON, KSK_SEMICOLON },
   { KB_RAWKEY_EQUAL_101, KSK_EQUALS },
   { KB_RAWKEY_LEFT_BRACKET_101, KSK_LEFTBRACKET },
   { KB_RAWKEY_BACKSLASH_101, KSK_BACKSLASH },
   { KB_RAWKEY_RIGHT_BRACKET_101, KSK_RIGHTBRACKET },
   { KB_RAWKEY_DELETE, KSK_DELETE },
   { KB_RAWKEY_KPAD_0, KSK_KP0 },
   { KB_RAWKEY_KPAD_1, KSK_KP1 },
   { KB_RAWKEY_KPAD_2, KSK_KP2 },
   { KB_RAWKEY_KPAD_3, KSK_KP3 },
   { KB_RAWKEY_KPAD_4, KSK_KP4 },
   { KB_RAWKEY_KPAD_5, KSK_KP5 },
   { KB_RAWKEY_KPAD_6, KSK_KP6 },
   { KB_RAWKEY_KPAD_7, KSK_KP7 },
   { KB_RAWKEY_KPAD_8, KSK_KP8 },
   { KB_RAWKEY_KPAD_9, KSK_KP9 },
   { KB_RAWKEY_KPAD_PERIOD, KSK_KP_PERIOD },
   { KB_RAWKEY_KPAD_SLASH, KSK_KP_DIVIDE },
   { KB_RAWKEY_KPAD_ASTERISK, KSK_KP_MULTIPLY },
   { KB_RAWKEY_KPAD_MINUS, KSK_KP_MINUS },
   { KB_RAWKEY_KPAD_PLUS, KSK_KP_PLUS },
   { KB_RAWKEY_KPAD_ENTER, KSK_KP_ENTER },
   { KB_RAWKEY_UP_ARROW, KSK_UP },
   { KB_RAWKEY_DOWN_ARROW, KSK_DOWN },
   { KB_RAWKEY_RIGHT_ARROW, KSK_RIGHT },
   { KB_RAWKEY_LEFT_ARROW, KSK_LEFT },
   { KB_RAWKEY_INSERT, KSK_INSERT },
   { KB_RAWKEY_HOME, KSK_HOME },
   { KB_RAWKEY_END, KSK_END },
   { KB_RAWKEY_PAGE_UP, KSK_PAGEUP },
   { KB_RAWKEY_PAGE_DOWN, KSK_PAGEDOWN },
   { KB_RAWKEY_F1, KSK_F1 },
   { KB_RAWKEY_F2, KSK_F2 },
   { KB_RAWKEY_F3, KSK_F3 },
   { KB_RAWKEY_F4, KSK_F4 },
   { KB_RAWKEY_F5, KSK_F5 },
   { KB_RAWKEY_F6, KSK_F6 },
   { KB_RAWKEY_F7, KSK_F7 },
   { KB_RAWKEY_F8, KSK_F8 },
   { KB_RAWKEY_F9, KSK_F9 },
   { KB_RAWKEY_F10, KSK_F10 },
   { KB_RAWKEY_F11, KSK_F11 },
   { KB_RAWKEY_F12, KSK_F12 },
   { KB_RAWKEY_KPAD_NUMLOCK, KSK_NUMLOCK },
   { KB_RAWKEY_CAPS_LOCK, KSK_CAPSLOCK },
   { KB_RAWKEY_SCROLL_LOCK, KSK_SCROLLOCK },
   { KB_RAWKEY_PAUSE, KSK_BREAK },

   /* 
   { KB_RAWKEY_HASHTILDE, KSK_HASH },
   { KB_RAWKEY_KPLEFTPAREN, KSK_LEFTPAREN },
   { KB_RAWKEY_KPRIGHTPAREN, KSK_RIGHTPAREN },
   { KB_RAWKEY_LEFTMETA, KSK_LMETA },
   { KB_RAWKEY_RIGHTMETA, KSK_RMETA },
   { KB_RAWKEY_COMPOSE, KSK_COMPOSE },
   { KB_RAWKEY_HELP, KSK_HELP },
   { KB_RAWKEY_POWER, KSK_POWER },
   { KB_RAWKEY_UNDO, KSK_UNDO },
   { KB_RAWKEY_KPAD_EQUAL, KSK_KP_EQUALS },

 KB_RAWKEY_PRINTSCREEN
 KB_RAWKEY_APPLICATION

 KB_RAWKEY_106_KANJI
 KB_RAWKEY_KANA
 KB_RAWKEY_HENKAN
 KB_RAWKEY_MUHENKAN
 KB_RAWKEY_ACCENT_CIRCONFLEX_106
 KB_RAWKEY_ATMARK_106
 KB_RAWKEY_LEFT_BRACKET_106
 KB_RAWKEY_RIGHT_BRACKET_106
 KB_RAWKEY_COLON_106
 KB_RAWKEY_BACKSLASH_106
 KB_RAWKEY_YEN_106 */

   { 0, KSK_UNKNOWN }
};
#endif

#if defined(_WIN32) && _WIN32_WINNT >= 0x0501 && !defined(__WINRT__)
const struct kingsn_key_map kingsn_key_map_winraw[] = {
   { VK_BACK, KSK_BACKSPACE },
   { VK_TAB, KSK_TAB },
   { VK_CLEAR, KSK_CLEAR },
   { VK_RETURN, KSK_RETURN },
   { VK_PAUSE, KSK_PAUSE },
   { VK_ESCAPE, KSK_ESCAPE },
   { VK_MODECHANGE, KSK_MODE },
   { VK_SPACE, KSK_SPACE },
   { VK_PRIOR, KSK_PAGEUP },
   { VK_NEXT, KSK_PAGEDOWN },
   { VK_END, KSK_END },
   { VK_HOME, KSK_HOME },
   { VK_LEFT, KSK_LEFT },
   { VK_UP, KSK_UP },
   { VK_RIGHT, KSK_RIGHT },
   { VK_DOWN, KSK_DOWN },
   { VK_PRINT, KSK_PRINT },
   { VK_INSERT, KSK_INSERT },
   { VK_DELETE, KSK_DELETE },
   { VK_HELP, KSK_HELP },
   { 0x30, KSK_0 },
   { 0x31, KSK_1 },
   { 0x32, KSK_2 },
   { 0x33, KSK_3 },
   { 0x34, KSK_4 },
   { 0x35, KSK_5 },
   { 0x36, KSK_6 },
   { 0x37, KSK_7 },
   { 0x38, KSK_8 },
   { 0x39, KSK_9 },
   { 0x41, KSK_a },
   { 0x42, KSK_b },
   { 0x43, KSK_c },
   { 0x44, KSK_d },
   { 0x45, KSK_e },
   { 0x46, KSK_f },
   { 0x47, KSK_g },
   { 0x48, KSK_h },
   { 0x49, KSK_i },
   { 0x4A, KSK_j },
   { 0x4B, KSK_k },
   { 0x4C, KSK_l },
   { 0x4D, KSK_m },
   { 0x4E, KSK_n },
   { 0x4F, KSK_o },
   { 0x50, KSK_p },
   { 0x51, KSK_q },
   { 0x52, KSK_r },
   { 0x53, KSK_s },
   { 0x54, KSK_t },
   { 0x55, KSK_u },
   { 0x56, KSK_v },
   { 0x57, KSK_w },
   { 0x58, KSK_x },
   { 0x59, KSK_y },
   { 0x5A, KSK_z },
   { VK_LWIN, KSK_LSUPER },
   { VK_RWIN, KSK_RSUPER },
   { VK_APPS, KSK_MENU },
   { VK_NUMPAD0, KSK_KP0 },
   { VK_NUMPAD1, KSK_KP1 },
   { VK_NUMPAD2, KSK_KP2 },
   { VK_NUMPAD3, KSK_KP3 },
   { VK_NUMPAD4, KSK_KP4 },
   { VK_NUMPAD5, KSK_KP5 },
   { VK_NUMPAD6, KSK_KP6 },
   { VK_NUMPAD7, KSK_KP7 },
   { VK_NUMPAD8, KSK_KP8 },
   { VK_NUMPAD9, KSK_KP9 },
   { VK_MULTIPLY, KSK_KP_MULTIPLY },
   { VK_ADD, KSK_KP_PLUS },
   { VK_SUBTRACT, KSK_KP_MINUS },
   { VK_DECIMAL, KSK_KP_PERIOD },
   { VK_DIVIDE, KSK_KP_DIVIDE },
   { VK_F1, KSK_F1 },
   { VK_F2, KSK_F2 },
   { VK_F3, KSK_F3 },
   { VK_F4, KSK_F4 },
   { VK_F5, KSK_F5 },
   { VK_F6, KSK_F6 },
   { VK_F7, KSK_F7 },
   { VK_F8, KSK_F8 },
   { VK_F9, KSK_F9 },
   { VK_F10, KSK_F10 },
   { VK_F11, KSK_F11 },
   { VK_F12, KSK_F12 },
   { VK_F13, KSK_F13 },
   { VK_F14, KSK_F14 },
   { VK_F15, KSK_F15 },
   { VK_NUMLOCK, KSK_NUMLOCK },
   { VK_SCROLL, KSK_SCROLLOCK },
   { VK_LSHIFT, KSK_LSHIFT },
   { VK_RSHIFT, KSK_RSHIFT },
   { VK_LCONTROL, KSK_LCTRL },
   { VK_RCONTROL, KSK_RCTRL },
   { VK_LMENU, KSK_LALT },
   { VK_RMENU, KSK_RALT },
   { VK_RETURN, KSK_KP_ENTER },
   { VK_CAPITAL, KSK_CAPSLOCK },
   { VK_OEM_1, KSK_SEMICOLON },
   { VK_OEM_PLUS, KSK_EQUALS },
   { VK_OEM_COMMA, KSK_COMMA },
   { VK_OEM_MINUS, KSK_MINUS },
   { VK_OEM_PERIOD, KSK_PERIOD },
   { VK_OEM_2, KSK_SLASH },
   { VK_OEM_3, KSK_BACKQUOTE },
   { VK_OEM_4, KSK_LEFTBRACKET },
   { VK_OEM_5, KSK_BACKSLASH },
   { VK_OEM_6, KSK_RIGHTBRACKET },
   { VK_OEM_7, KSK_QUOTE },
   { 0, KSK_UNKNOWN }
};
#endif

#ifdef __WINRT__
/* Refer to uwp_main.cpp - on WinRT these constants are defined as C++ enum classes
 * so they can't be placed in a C source file */
#endif

/* TODO/FIXME - global */
enum ks_key kingsn_keysym_lut[KSK_LAST];

/* TODO/FIXME - static globals */
static unsigned *kingsn_keysym_rlut           = NULL;
static unsigned kingsn_keysym_rlut_size       = 0;

/**
 * input_keymaps_init_keyboard_lut:
 * @map                   : Keyboard map.
 *
 * Initializes and sets the keyboard layout to a keyboard map (@map).
 **/
void input_keymaps_init_keyboard_lut(const struct kingsn_key_map *map)
{
   const struct kingsn_key_map *map_start = map;
   memset(kingsn_keysym_lut, 0, sizeof(kingsn_keysym_lut));
   kingsn_keysym_rlut_size = 0;

   for (; map->rk != KSK_UNKNOWN; map++)
   {
      kingsn_keysym_lut[map->rk] = (enum ks_key)map->sym;
      if (map->sym > kingsn_keysym_rlut_size)
         kingsn_keysym_rlut_size = map->sym;
   }

   if (kingsn_keysym_rlut_size < 65536)
   {
      if (kingsn_keysym_rlut)
         free(kingsn_keysym_rlut);

      kingsn_keysym_rlut = (unsigned*)calloc(++kingsn_keysym_rlut_size, sizeof(unsigned));

      for (map = map_start; map->rk != KSK_UNKNOWN; map++)
         kingsn_keysym_rlut[map->sym] = (enum ks_key)map->rk;
   }
   else
      kingsn_keysym_rlut_size = 0;
}

/**
 * input_keymaps_translate_keysym_to_rk:
 * @sym                   : Key symbol.
 *
 * Translates a key symbol from the keyboard layout table
 * to an associated ks key identifier.
 *
 * Returns: Retro key identifier.
 **/
enum ks_key input_keymaps_translate_keysym_to_rk(unsigned sym)
{
   unsigned i;

   /* Fast path */
   if (kingsn_keysym_rlut && sym < kingsn_keysym_rlut_size)
      return (enum ks_key)kingsn_keysym_rlut[sym];

   /* Slow path */
   for (i = 0; i < ARRAY_SIZE(kingsn_keysym_lut); i++)
   {
      if (kingsn_keysym_lut[i] != sym)
         continue;

      return (enum ks_key)i;
   }

   return KSK_UNKNOWN;
}

/**
 * input_keymaps_translate_rk_to_str:
 * @key                   : Retro key identifier.
 * @buf                   : Buffer.
 * @size                  : Size of @buf.
 *
 * Translates a ks key identifier to a human-readable
 * identifier string.
 **/
void input_keymaps_translate_rk_to_str(enum ks_key key, char *buf, size_t size)
{
   unsigned i;

   ks_assert(size >= 2);
   *buf = '\0';


   for (i = 0; input_config_key_map[i].str; i++)
   {
      if (input_config_key_map[i].key != key)
         continue;

      strlcpy(buf, input_config_key_map[i].str, size);
      break;
   }
}
