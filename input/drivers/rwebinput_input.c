/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2018 - Michael Lelli
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <boolean.h>
#include <ks_assert.h>
#include <ks_miscellaneous.h>
#include <encodings/crc32.h>
#include <encodings/utf.h>

#include <emscripten/html5.h>

#include "../input_keymaps.h"

#include "../../tasks/tasks_internal.h"
#include "../../configuration.h"
#include "../../KingStation.h"
#include "../../verbosity.h"

/* https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button */
#define RWEBINPUT_MOUSE_BTNL 0
#define RWEBINPUT_MOUSE_BTNM 1
#define RWEBINPUT_MOUSE_BTNR 2
#define RWEBINPUT_MOUSE_BTN4 3
#define RWEBINPUT_MOUSE_BTN5 4

typedef struct rwebinput_key_to_code_map_entry
{
   const char *key;
   enum ks_key rk;
} rwebinput_key_to_code_map_entry_t;

typedef struct rwebinput_keyboard_event
{
   int type;
   EmscriptenKeyboardEvent event;
} rwebinput_keyboard_event_t;

typedef struct rwebinput_keyboard_event_queue
{
   rwebinput_keyboard_event_t *events;
   size_t count;
   size_t max_size;
} rwebinput_keyboard_event_queue_t;

typedef struct rwebinput_mouse_states
{
   double pending_scroll_x;
   double pending_scroll_y;
   double scroll_x;
   double scroll_y;
   signed x;
   signed y;
   signed pending_delta_x;
   signed pending_delta_y;
   signed delta_x;
   signed delta_y;
   uint8_t buttons;
} rwebinput_mouse_state_t;

typedef struct rwebinput_input
{
   rwebinput_mouse_state_t mouse;             /* double alignment */
   rwebinput_keyboard_event_queue_t keyboard; /* ptr alignment */
   bool keys[KSK_LAST];
} rwebinput_input_t;

/* KeyboardEvent.keyCode has been deprecated for a while and doesn't have
 * separate left/right modifer codes, so we have to map string labels from
 * KeyboardEvent.code to ks keys */
static const rwebinput_key_to_code_map_entry_t rwebinput_key_to_code_map[] =
{
   { "KeyA", KSK_a },
   { "KeyB", KSK_b },
   { "KeyC", KSK_c },
   { "KeyD", KSK_d },
   { "KeyE", KSK_e },
   { "KeyF", KSK_f },
   { "KeyG", KSK_g },
   { "KeyH", KSK_h },
   { "KeyI", KSK_i },
   { "KeyJ", KSK_j },
   { "KeyK", KSK_k },
   { "KeyL", KSK_l },
   { "KeyM", KSK_m },
   { "KeyN", KSK_n },
   { "KeyO", KSK_o },
   { "KeyP", KSK_p },
   { "KeyQ", KSK_q },
   { "KeyR", KSK_r },
   { "KeyS", KSK_s },
   { "KeyT", KSK_t },
   { "KeyU", KSK_u },
   { "KeyV", KSK_v },
   { "KeyW", KSK_w },
   { "KeyX", KSK_x },
   { "KeyY", KSK_y },
   { "KeyZ", KSK_z },
   { "ArrowLeft", KSK_LEFT },
   { "ArrowRight", KSK_RIGHT },
   { "ArrowUp", KSK_UP },
   { "ArrowDown", KSK_DOWN },
   { "Enter", KSK_RETURN },
   { "NumpadEnter", KSK_KP_ENTER },
   { "Tab", KSK_TAB },
   { "Insert", KSK_INSERT },
   { "Delete", KSK_DELETE },
   { "End", KSK_END },
   { "Home", KSK_HOME },
   { "ShiftRight", KSK_RSHIFT },
   { "ShiftLeft", KSK_LSHIFT },
   { "ControlLeft", KSK_LCTRL },
   { "AltLeft", KSK_LALT },
   { "Space", KSK_SPACE },
   { "Escape", KSK_ESCAPE },
   { "NumpadAdd", KSK_KP_PLUS },
   { "NumpadSubtract", KSK_KP_MINUS },
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
   { "Digit0", KSK_0 },
   { "Digit1", KSK_1 },
   { "Digit2", KSK_2 },
   { "Digit3", KSK_3 },
   { "Digit4", KSK_4 },
   { "Digit5", KSK_5 },
   { "Digit6", KSK_6 },
   { "Digit7", KSK_7 },
   { "Digit8", KSK_8 },
   { "Digit9", KSK_9 },
   { "PageUp", KSK_PAGEUP },
   { "PageDown", KSK_PAGEDOWN },
   { "Numpad0", KSK_KP0 },
   { "Numpad1", KSK_KP1 },
   { "Numpad2", KSK_KP2 },
   { "Numpad3", KSK_KP3 },
   { "Numpad4", KSK_KP4 },
   { "Numpad5", KSK_KP5 },
   { "Numpad6", KSK_KP6 },
   { "Numpad7", KSK_KP7 },
   { "Numpad8", KSK_KP8 },
   { "Numpad9", KSK_KP9 },
   { "Period", KSK_PERIOD },
   { "CapsLock", KSK_CAPSLOCK },
   { "NumLock", KSK_NUMLOCK },
   { "Backspace", KSK_BACKSPACE },
   { "NumpadMultiply", KSK_KP_MULTIPLY },
   { "NumpadDivide", KSK_KP_DIVIDE },
   { "PrintScreen", KSK_PRINT },
   { "ScrollLock", KSK_SCROLLOCK },
   { "Backquote", KSK_BACKQUOTE },
   { "Pause", KSK_PAUSE },
   { "Quote", KSK_QUOTE },
   { "Comma", KSK_COMMA },
   { "Minus", KSK_MINUS },
   { "Slash", KSK_SLASH },
   { "Semicolon", KSK_SEMICOLON },
   { "Equal", KSK_EQUALS },
   { "BracketLeft", KSK_LEFTBRACKET },
   { "Backslash", KSK_BACKSLASH },
   { "BracketRight", KSK_RIGHTBRACKET },
   { "NumpadDecimal", KSK_KP_PERIOD },
   { "NumpadEqual", KSK_KP_EQUALS },
   { "ControlRight", KSK_RCTRL },
   { "AltRight", KSK_RALT },
   { "F13", KSK_F13 },
   { "F14", KSK_F14 },
   { "F15", KSK_F15 },
   { "MetaRight", KSK_RMETA },
   { "MetaLeft", KSK_LMETA },
   { "Help", KSK_HELP },
   { "ContextMenu", KSK_MENU },
   { "Power", KSK_POWER },
};

/* to make the string labels for codes from JavaScript work, we convert them
 * to CRC32 hashes for the LUT */
static void rwebinput_generate_lut(void)
{
   int i;
   struct kingsn_key_map *key_map;

   ks_assert(ARRAY_SIZE(kingsn_key_map_rwebinput) ==
      ARRAY_SIZE(rwebinput_key_to_code_map) + 1);

   for (i = 0; i < ARRAY_SIZE(rwebinput_key_to_code_map); i++)
   {
      int j;
      uint32_t crc;
      const rwebinput_key_to_code_map_entry_t *key_to_code =
         &rwebinput_key_to_code_map[i];
      key_map = &kingsn_key_map_rwebinput[i];
      crc = encoding_crc32(0, (const uint8_t *)key_to_code->key,
         strlen(key_to_code->key));

      /* sanity check: make sure there's no collisions */
      for (j = 0; j < i; j++)
         ks_assert(kingsn_key_map_rwebinput[j].sym != crc);

      key_map->rk  = key_to_code->rk;
      key_map->sym = crc;
   }

   /* set terminating entry */
   key_map      = &kingsn_key_map_rwebinput[
      ARRAY_SIZE(kingsn_key_map_rwebinput) - 1];
   key_map->rk  = KSK_UNKNOWN;
   key_map->sym = 0;
}

static EM_BOOL rwebinput_keyboard_cb(int event_type,
   const EmscriptenKeyboardEvent *key_event, void *user_data)
{
   rwebinput_input_t *rwebinput = (rwebinput_input_t*)user_data;

   if (event_type == EMSCRIPTEN_EVENT_KEYPRESS)
      return EM_TRUE;

   if (rwebinput->keyboard.count >= rwebinput->keyboard.max_size)
   {
      size_t new_max = MAX(1, rwebinput->keyboard.max_size << 1);
      rwebinput->keyboard.events = realloc(rwebinput->keyboard.events,
         new_max * sizeof(rwebinput->keyboard.events[0]));
      ks_assert(rwebinput->keyboard.events != NULL);
      rwebinput->keyboard.max_size = new_max;
   }

   rwebinput->keyboard.events[rwebinput->keyboard.count].type = event_type;
   memcpy(&rwebinput->keyboard.events[rwebinput->keyboard.count].event,
      key_event, sizeof(*key_event));
   rwebinput->keyboard.count++;

   return EM_TRUE;
}

static EM_BOOL rwebinput_mouse_cb(int event_type,
   const EmscriptenMouseEvent *mouse_event, void *user_data)
{
   rwebinput_input_t *rwebinput      = (rwebinput_input_t*)user_data;

   uint8_t mask                      = 1 << mouse_event->button;

   rwebinput->mouse.x                = mouse_event->targetX;
   rwebinput->mouse.y                = mouse_event->targetY;
   rwebinput->mouse.pending_delta_x += mouse_event->movementX;
   rwebinput->mouse.pending_delta_y += mouse_event->movementY;

   if (event_type ==  EMSCRIPTEN_EVENT_MOUSEDOWN)
      rwebinput->mouse.buttons |= mask;
   else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP)
      rwebinput->mouse.buttons &= ~mask;

   return EM_TRUE;
}

static EM_BOOL rwebinput_wheel_cb(int event_type,
   const EmscriptenWheelEvent *wheel_event, void *user_data)
{
   rwebinput_input_t       *rwebinput = (rwebinput_input_t*)user_data;

   rwebinput->mouse.pending_scroll_x += wheel_event->deltaX;
   rwebinput->mouse.pending_scroll_y += wheel_event->deltaY;

   return EM_TRUE;
}

static void *rwebinput_input_init(const char *joypad_driver)
{
   EMSCRIPTEN_RESULT r;
   rwebinput_input_t *rwebinput =
      (rwebinput_input_t*)calloc(1, sizeof(*rwebinput));

   if (!rwebinput)
      return NULL;

   rwebinput_generate_lut();

   r = emscripten_set_keydown_callback(
         EMSCRIPTEN_EVENT_TARGET_DOCUMENT, rwebinput, false,
         rwebinput_keyboard_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create keydown callback: %d\n", r);
   }

   r = emscripten_set_keyup_callback(
         EMSCRIPTEN_EVENT_TARGET_DOCUMENT, rwebinput, false,
         rwebinput_keyboard_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create keydown callback: %d\n", r);
   }

   r = emscripten_set_keypress_callback(
         EMSCRIPTEN_EVENT_TARGET_DOCUMENT, rwebinput, false,
         rwebinput_keyboard_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create keypress callback: %d\n", r);
   }

   r = emscripten_set_mousedown_callback("#canvas", rwebinput, false,
         rwebinput_mouse_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create mousedown callback: %d\n", r);
   }

   r = emscripten_set_mouseup_callback("#canvas", rwebinput, false,
         rwebinput_mouse_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create mouseup callback: %d\n", r);
   }

   r = emscripten_set_mousemove_callback("#canvas", rwebinput, false,
         rwebinput_mouse_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create mousemove callback: %d\n", r);
   }

   r = emscripten_set_wheel_callback(
         EMSCRIPTEN_EVENT_TARGET_DOCUMENT, rwebinput, false,
         rwebinput_wheel_cb);
   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      KINGSN_ERR(
         "[EMSCRIPTEN/INPUT] failed to create wheel callback: %d\n", r);
   }

   input_keymaps_init_keyboard_lut(kingsn_key_map_rwebinput);

   return rwebinput;
}

static bool rwebinput_key_pressed(rwebinput_input_t *rwebinput, int key)
{
   if (key >= KSK_LAST)
      return false;

   return rwebinput->keys[key];
}

static int16_t rwebinput_mouse_state(
      rwebinput_mouse_state_t *mouse,
      unsigned id, bool screen)
{
   switch (id)
   {
      case KS_DEVICE_ID_MOUSE_X:
         return (int16_t)(screen ? mouse->x : mouse->delta_x);
      case KS_DEVICE_ID_MOUSE_Y:
         return (int16_t)(screen ? mouse->y : mouse->delta_y);
      case KS_DEVICE_ID_MOUSE_LEFT:
         return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTNL));
      case KS_DEVICE_ID_MOUSE_RIGHT:
         return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTNR));
      case KS_DEVICE_ID_MOUSE_MIDDLE:
         return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTNM));
      case KS_DEVICE_ID_MOUSE_BUTTON_4:
         return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTN4));
      case KS_DEVICE_ID_MOUSE_BUTTON_5:
         return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTN5));
      case KS_DEVICE_ID_MOUSE_WHEELUP:
         return mouse->scroll_y < 0.0;
      case KS_DEVICE_ID_MOUSE_WHEELDOWN:
         return mouse->scroll_y > 0.0;
      case KS_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
         return mouse->scroll_x < 0.0;
      case KS_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
         return mouse->scroll_x > 0.0;
   }

   return 0;
}

static int16_t rwebinput_is_pressed(
      rwebinput_input_t *rwebinput,
      const struct ks_keybind *binds,
      unsigned port, unsigned id,
      bool keyboard_mapping_blocked)
{
   const struct ks_keybind *bind = &binds[id];
   int key                          = bind->key;

   if ((key < KSK_LAST) && rwebinput_key_pressed(rwebinput, key))
      if ((id == KINGSN_GAME_FOCUS_TOGGLE) || !keyboard_mapping_blocked)
         return 1;
   if (port == 0 && !!rwebinput_mouse_state(&rwebinput->mouse,
            bind->mbutton, false))
      return 1;
   return 0;
}

static int16_t rwebinput_input_state(
      void *data,
      const input_device_driver_t *joypad,
      const input_device_driver_t *sec_joypad,
      kingsn_joypad_info_t *joypad_info,
      const struct ks_keybind **binds,
      bool keyboard_mapping_blocked,
      unsigned port,
      unsigned device,
      unsigned idx,
      unsigned id)
{
   rwebinput_input_t *rwebinput = (rwebinput_input_t*)data;

   switch (device)
   {
      case KS_DEVICE_JOYPAD:
         if (id == KS_DEVICE_ID_JOYPAD_MASK)
         {
            unsigned i;
            int16_t ret = 0;
            for (i = 0; i < KINGSN_FIRST_CUSTOM_BIND; i++)
            {
               if (binds[port][i].valid)
               {
                  if (rwebinput_is_pressed(
                           rwebinput, binds[port], port, i,
                           keyboard_mapping_blocked))
                     ret |= (1 << i);
               }
            }

            return ret;
         }

         if (id < KINGSN_BIND_LIST_END)
         {
            if (binds[port][id].valid)
            {
               if (rwebinput_is_pressed(rwebinput,
                        binds[port],
                        port, id,
                        keyboard_mapping_blocked))
                  return 1;
            }
         }
         break;
      case KS_DEVICE_ANALOG:
         if (binds[port])
         {
            int id_minus_key      = 0;
            int id_plus_key       = 0;
            unsigned id_minus     = 0;
            unsigned id_plus      = 0;
            int16_t ret           = 0;
            bool id_plus_valid    = false;
            bool id_minus_valid   = false;

            input_conv_analog_id_to_bind_id(idx, id, id_minus, id_plus);

            id_minus_valid        = binds[port][id_minus].valid;
            id_plus_valid         = binds[port][id_plus].valid;
            id_minus_key          = binds[port][id_minus].key;
            id_plus_key           = binds[port][id_plus].key;

            if (id_plus_valid && id_plus_key < KSK_LAST)
            {
               if (rwebinput_is_pressed(rwebinput,
                        binds[port], idx, id_plus,
                        keyboard_mapping_blocked))
                  ret = 0x7fff;
            }
            if (id_minus_valid && id_minus_key < KSK_LAST)
            {
               if (rwebinput_is_pressed(rwebinput, 
                        binds[port], idx, id_minus,
                        keyboard_mapping_blocked))
                  ret += -0x7fff;
            }

            return ret;
         }
         break;
      case KS_DEVICE_KEYBOARD:
         return ((id < KSK_LAST) && rwebinput->keys[id]);
      case KS_DEVICE_MOUSE:
      case KINGSN_DEVICE_MOUSE_SCREEN:
         return rwebinput_mouse_state(&rwebinput->mouse, id,
               device == KINGSN_DEVICE_MOUSE_SCREEN);
      case KS_DEVICE_POINTER:
      case KINGSN_DEVICE_POINTER_SCREEN:
         {
            struct video_viewport vp;
            rwebinput_mouse_state_t 
               *mouse                   = &rwebinput->mouse;
            const int edge_detect       = 32700;
            bool screen                 = device == 
               KINGSN_DEVICE_POINTER_SCREEN;
            bool inside                 = false;
            int16_t res_x               = 0;
            int16_t res_y               = 0;
            int16_t res_screen_x        = 0;
            int16_t res_screen_y        = 0;

            vp.x                        = 0;
            vp.y                        = 0;
            vp.width                    = 0;
            vp.height                   = 0;
            vp.full_width               = 0;
            vp.full_height              = 0;

            if (!(video_driver_translate_coord_viewport_wrap(
                        &vp, mouse->x, mouse->x,
                        &res_x, &res_y, &res_screen_x, &res_screen_y)))
               return 0;

            if (screen)
            {
               res_x = res_screen_x;
               res_y = res_screen_y;
            }

            inside =    (res_x >= -edge_detect) 
               && (res_y >= -edge_detect)
               && (res_x <= edge_detect)
               && (res_y <= edge_detect);

            switch (id)
            {
               case KS_DEVICE_ID_POINTER_X:
                  if (inside)
                     return res_x;
                  break;
               case KS_DEVICE_ID_POINTER_Y:
                  if (inside)
                     return res_y;
                  break;
               case KS_DEVICE_ID_POINTER_PRESSED:
                  return !!(mouse->buttons & (1 << RWEBINPUT_MOUSE_BTNL));
               case KS_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
                  return !inside;
               default:
                  break;
            }
         }
         break;
   }

   return 0;
}

static void rwebinput_input_free(void *data)
{
   rwebinput_input_t *rwebinput = (rwebinput_input_t*)data;

   emscripten_html5_remove_all_event_listeners();

   free(rwebinput->keyboard.events);

   free(data);
}

static void rwebinput_process_keyboard_events(
      rwebinput_input_t *rwebinput,
      rwebinput_keyboard_event_t *event)
{
   uint32_t keycode;
   unsigned translated_keycode;
   uint32_t character                       = 0;
   uint16_t mod                             = 0;
   const EmscriptenKeyboardEvent *key_event = &event->event;
   bool keydown                             = 
      event->type == EMSCRIPTEN_EVENT_KEYDOWN;

   /* a printable key: populate character field */
   if (utf8len(key_event->key) == 1)
   {
      const char *key_ptr = &key_event->key[0];
      character           = utf8_walk(&key_ptr);
   }

   if (key_event->ctrlKey)
      mod |= KSKMOD_CTRL;
   if (key_event->altKey)
      mod |= KSKMOD_ALT;
   if (key_event->shiftKey)
      mod |= KSKMOD_SHIFT;
   if (key_event->metaKey)
      mod |= KSKMOD_META;

   keycode = encoding_crc32(0, (const uint8_t *)key_event->code,
      strnlen(key_event->code, sizeof(key_event->code)));
   translated_keycode = input_keymaps_translate_keysym_to_rk(keycode);

   if (     translated_keycode == KSK_BACKSPACE)
      character = '\b';
   else if (translated_keycode == KSK_RETURN ||
            translated_keycode == KSK_KP_ENTER)
      character = '\n';
   else if (translated_keycode == KSK_TAB)
      character = '\t';

   if (translated_keycode != KSK_UNKNOWN)
      input_keyboard_event(keydown, translated_keycode, character, mod,
         KS_DEVICE_KEYBOARD);
   
   if (     translated_keycode  < KSK_LAST 
         && translated_keycode != KSK_UNKNOWN)
      rwebinput->keys[translated_keycode] = keydown;
}

static void rwebinput_input_poll(void *data)
{
   size_t i;
   rwebinput_input_t *rwebinput      = (rwebinput_input_t*)data;

   for (i = 0; i < rwebinput->keyboard.count; i++)
      rwebinput_process_keyboard_events(rwebinput,
         &rwebinput->keyboard.events[i]);

   rwebinput->keyboard.count         = 0;

   rwebinput->mouse.delta_x          = rwebinput->mouse.pending_delta_x;
   rwebinput->mouse.delta_y          = rwebinput->mouse.pending_delta_y;
   rwebinput->mouse.pending_delta_x  = 0;
   rwebinput->mouse.pending_delta_y  = 0;

   rwebinput->mouse.scroll_x         = rwebinput->mouse.pending_scroll_x;
   rwebinput->mouse.scroll_y         = rwebinput->mouse.pending_scroll_y;
   rwebinput->mouse.pending_scroll_x = 0;
   rwebinput->mouse.pending_scroll_y = 0;
}

static void rwebinput_grab_mouse(void *data, bool state)
{
   if (state)
      emscripten_request_pointerlock("#canvas", EM_TRUE);
   else
      emscripten_exit_pointerlock();
}

static uint64_t rwebinput_get_capabilities(void *data)
{
   uint64_t caps = 0;

   caps |= (1 << KS_DEVICE_JOYPAD);
   caps |= (1 << KS_DEVICE_ANALOG);
   caps |= (1 << KS_DEVICE_KEYBOARD);
   caps |= (1 << KS_DEVICE_MOUSE);
   caps |= (1 << KS_DEVICE_POINTER);

   return caps;
}

input_driver_t input_rwebinput = {
   rwebinput_input_init,
   rwebinput_input_poll,
   rwebinput_input_state,
   rwebinput_input_free,
   NULL,
   NULL,
   rwebinput_get_capabilities,
   "rwebinput",
   rwebinput_grab_mouse,
   NULL
};
