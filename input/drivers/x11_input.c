/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2015 - Hans-Kristian Arntzen
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

#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <boolean.h>
#include <compat/strl.h>
#include <ks_inline.h>

#include "../input_keymaps.h"

#include "../common/input_x11_common.h"

#include "../../configuration.h"
#include "../../KingStation.h"
#include "../../verbosity.h"

typedef struct x11_input
{
   Display *display;
   Window win;

   int mouse_x;
   int mouse_y;
   int mouse_delta_x;
   int mouse_delta_y;
   bool mouse_grabbed;
   char state[32];
   bool mouse_l;
   bool mouse_r;
   bool mouse_m;
} x11_input_t;

/* Public global variable */
extern bool g_x11_entered;

static void *x_input_init(const char *joypad_driver)
{
   x11_input_t *x11;

   if (video_driver_display_type_get() != KINGSN_DISPLAY_X11)
   {
      KINGSN_ERR("Currently active window is not an X11 window. Cannot use this driver.\n");
      return NULL;
   }

   x11 = (x11_input_t*)calloc(1, sizeof(*x11));
   if (!x11)
      return NULL;

   /* Borrow the active X window ... */
   x11->display = (Display*)video_driver_display_get();
   x11->win     = (Window)video_driver_window_get();

   input_keymaps_init_keyboard_lut(kingsn_key_map_x11);

   return x11;
}

static bool x_keyboard_pressed(x11_input_t *x11, unsigned key)
{
   int keycode = kingsn_keysym_lut[(enum ks_key)key];
   return x11->state[keycode >> 3] & (1 << (keycode & 7));
}

static bool x_mouse_button_pressed(
      x11_input_t *x11, unsigned port, unsigned key)
{
   switch (key)
   {
      case KS_DEVICE_ID_MOUSE_LEFT:
         return x11->mouse_l;
      case KS_DEVICE_ID_MOUSE_RIGHT:
         return x11->mouse_r;
      case KS_DEVICE_ID_MOUSE_MIDDLE:
         return x11->mouse_m;
#if 0
      case KS_DEVICE_ID_MOUSE_BUTTON_4:
         return x11->mouse_b4;
      case KS_DEVICE_ID_MOUSE_BUTTON_5:
         return x11->mouse_b5;
#endif
      case KS_DEVICE_ID_MOUSE_WHEELUP:
      case KS_DEVICE_ID_MOUSE_WHEELDOWN:
      case KS_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
      case KS_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
         return x_mouse_state_wheel(key);
   }

   return false;
}

static int16_t x_input_state(
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
   x11_input_t *x11     = (x11_input_t*)data;
   settings_t *settings = config_get_ptr();

   if (port >= MAX_USERS)
      return 0;

   switch (device)
   {
      case KS_DEVICE_JOYPAD:
         if (id == KS_DEVICE_ID_JOYPAD_MASK)
         {
            unsigned i;
            int16_t ret = 0;

            if (settings->uints.input_mouse_index[port] == 0)
            {
               for (i = 0; i < KINGSN_FIRST_CUSTOM_BIND; i++)
               {
                  if (binds[port][i].valid)
                  {
                     if (x_mouse_button_pressed(x11, port,
                              binds[port][i].mbutton))
                        ret |= (1 << i);
                  }
               }
            }
            if (!keyboard_mapping_blocked)
            {
               for (i = 0; i < KINGSN_FIRST_CUSTOM_BIND; i++)
               {
                  if (binds[port][i].valid)
                  {
                     if ((binds[port][i].key < KSK_LAST) &&
                           x_keyboard_pressed(x11, binds[port][i].key))
                        ret |= (1 << i);
                  }
               }
            }

            return ret;
         }

         if (id < KINGSN_BIND_LIST_END)
         {
            if (binds[port][id].valid)
            {
               if (
                     ((binds[port][id].key < KSK_LAST) && 
                      x_keyboard_pressed(x11, binds[port][id].key)) 
                     && ((    id == KINGSN_GAME_FOCUS_TOGGLE) 
                        || !keyboard_mapping_blocked)
                     )
                  return 1;
               else if (settings->uints.input_mouse_index[port] == 0)
               {
                  if (x_mouse_button_pressed(x11, port,
                           binds[port][id].mbutton))
                     return 1;
               }
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
            int16_t pressed_minus = 0;
            int16_t pressed_plus  = 0;
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
               unsigned sym = kingsn_keysym_lut[(enum ks_key)id_plus_key];
               if (x11->state[sym >> 3] & (1 << (sym & 7)))
                  ret = 0x7fff;
            }
            if (id_minus_valid && id_minus_key < KSK_LAST)
            {
               unsigned sym = kingsn_keysym_lut[(enum ks_key)id_minus_key];
               if (x11->state[sym >> 3] & (1 << (sym & 7)))
                  ret += -0x7fff;
            }

            return ret;
         }
         break;
      case KS_DEVICE_KEYBOARD:
         return (id < KSK_LAST) && x_keyboard_pressed(x11, id);
      case KS_DEVICE_MOUSE:
      case KINGSN_DEVICE_MOUSE_SCREEN:
         switch (id)
         {
            case KS_DEVICE_ID_MOUSE_X:
               if (device == KINGSN_DEVICE_MOUSE_SCREEN)
                  return x11->mouse_x;
               return x11->mouse_delta_x;
            case KS_DEVICE_ID_MOUSE_Y:
               if (device == KINGSN_DEVICE_MOUSE_SCREEN)
                  return x11->mouse_y;
               return x11->mouse_delta_y;
            case KS_DEVICE_ID_MOUSE_LEFT:
               return x11->mouse_l;
            case KS_DEVICE_ID_MOUSE_RIGHT:
               return x11->mouse_r;
            case KS_DEVICE_ID_MOUSE_WHEELUP:
            case KS_DEVICE_ID_MOUSE_WHEELDOWN:
            case KS_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
            case KS_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
               return x_mouse_state_wheel(id);
            case KS_DEVICE_ID_MOUSE_MIDDLE:
               return x11->mouse_m;
         }
         break;
      case KS_DEVICE_POINTER:
      case KINGSN_DEVICE_POINTER_SCREEN:
         if (idx == 0)
         {
            struct video_viewport vp;
            bool screen                 = device == KINGSN_DEVICE_POINTER_SCREEN;
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

            if (video_driver_translate_coord_viewport_wrap(
                     &vp, x11->mouse_x, x11->mouse_y,
                     &res_x, &res_y, &res_screen_x, &res_screen_y))
            {
               if (screen)
               {
                  res_x = res_screen_x;
                  res_y = res_screen_y;
               }

               inside = (res_x >= -0x7fff) && (res_y >= -0x7fff);

               if (!inside)
                  return 0;

               switch (id)
               {
                  case KS_DEVICE_ID_POINTER_X:
                     return res_x;
                  case KS_DEVICE_ID_POINTER_Y:
                     return res_y;
                  case KS_DEVICE_ID_POINTER_PRESSED:
                     return x11->mouse_l;
               }
            }
         }
         break;
      case KS_DEVICE_LIGHTGUN:
         switch ( id )
         {
            /*aiming*/
            case KS_DEVICE_ID_LIGHTGUN_SCREEN_X:
            case KS_DEVICE_ID_LIGHTGUN_SCREEN_Y:
            case KS_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
               {
                  struct video_viewport vp;
                  const int edge_detect       = 32700;
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

                  if (video_driver_translate_coord_viewport_wrap(&vp,
                           x11->mouse_x, x11->mouse_y,
                           &res_x, &res_y, &res_screen_x, &res_screen_y))
                  {
                     inside =    (res_x >= -edge_detect) 
                        && (res_y >= -edge_detect)
                        && (res_x <= edge_detect)
                        && (res_y <= edge_detect);

                     switch ( id )
                     {
                        case KS_DEVICE_ID_LIGHTGUN_SCREEN_X:
                           if (inside)
                              return res_x;
                           break;
                        case KS_DEVICE_ID_LIGHTGUN_SCREEN_Y:
                           if (inside)
                              return res_y;
                           break;
                        case KS_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
                           return !inside;
                        default:
                           break;
                     }
                  }
               }
               break;
            /*buttons*/
            case KS_DEVICE_ID_LIGHTGUN_TRIGGER:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_TRIGGER;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_RELOAD:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_RELOAD;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_AUX_A:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_AUX_A;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_AUX_B:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_AUX_B;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_AUX_C:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_AUX_C;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_START:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_START;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_SELECT:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_SELECT;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_DPAD_UP:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_DPAD_UP;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_DPAD_DOWN:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_DPAD_DOWN;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_DPAD_LEFT:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_DPAD_LEFT;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            case KS_DEVICE_ID_LIGHTGUN_DPAD_RIGHT:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_DPAD_RIGHT;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
            /*deprecated*/
            case KS_DEVICE_ID_LIGHTGUN_X:
               return x11->mouse_delta_x;
            case KS_DEVICE_ID_LIGHTGUN_Y:
               return x11->mouse_delta_y;
            case KS_DEVICE_ID_LIGHTGUN_PAUSE:
               {
                  unsigned new_id = KINGSN_LIGHTGUN_START;
                  if (!keyboard_mapping_blocked)
                     if ((binds[port][new_id].key < KSK_LAST) 
                           && x_keyboard_pressed(x11, binds[port]
                              [new_id].key) )
                        return 1;
                  if (binds[port][new_id].valid)
                  {
                     if (button_is_pressed(joypad,
                           joypad_info, binds[port],
                           port, new_id))
                        return 1;
                     else if (settings->uints.input_mouse_index[port] == 0)
                     {
                        if (x_mouse_button_pressed(x11, port,
                                 binds[port][new_id].mbutton))
                           return 1;
                     }
                  }
               }
               break;
         }
         break;
   }

   return 0;
}

static void x_input_free(void *data)
{
   x11_input_t *x11 = (x11_input_t*)data;

   if (!x11)
      return;

   free(x11);
}

static void x_input_poll(void *data)
{
   Window root_win;
   Window child_win;
   x11_input_t *x11     = (x11_input_t*)data;
   bool video_has_focus = video_driver_has_focus();
   int root_x           = 0;
   int root_y           = 0;
   int win_x            = 0;
   int win_y            = 0;
   unsigned mask        = 0;

   /* If window loses focus, 'reset' keyboard
    * and ignore mouse input */
   if (!video_has_focus)
   {
      memset(x11->state, 0, sizeof(x11->state));
      x11->mouse_delta_x = 0;
      x11->mouse_delta_y = 0;
      x11->mouse_l       = 0;
      x11->mouse_m       = 0;
      x11->mouse_r       = 0;
      return;
   }

   /* Process keyboard */
   XQueryKeymap(x11->display, x11->state);

   /* If pointer is not inside the application
    * window, ignore mouse input */
   if (!g_x11_entered)
   {
      x11->mouse_delta_x = 0;
      x11->mouse_delta_y = 0;
      x11->mouse_l       = 0;
      x11->mouse_m       = 0;
      x11->mouse_r       = 0;
      return;
   }

   /* Process mouse */
   if (!XQueryPointer(x11->display,
         x11->win,
         &root_win, &child_win,
         &root_x, &root_y,
         &win_x, &win_y,
         &mask))
      return;

   /* > Mouse buttons */
   x11->mouse_l = mask & Button1Mask;
   x11->mouse_m = mask & Button2Mask;
   x11->mouse_r = mask & Button3Mask;

   /* > Mouse pointer */
   if (!x11->mouse_grabbed)
   {
      /* Mouse is not grabbed - this corresponds
       * to 'conventional' pointer input, using
       * absolute screen coordinates */
      int mouse_last_x = x11->mouse_x;
      int mouse_last_y = x11->mouse_y;

      x11->mouse_x = win_x;
      x11->mouse_y = win_y;

      x11->mouse_delta_x = x11->mouse_x - mouse_last_x;
      x11->mouse_delta_y = x11->mouse_y - mouse_last_y;
   }
   else
   {
      /* Mouse is grabbed - all pointer movement
       * must be considered 'relative' */
      XWindowAttributes win_attr;
      int centre_x;
      int centre_y;
      int warp_x   = win_x;
      int warp_y   = win_y;
      bool do_warp = false;

      /* Get dimensions/centre coordinates of
       * application window */
      if (!XGetWindowAttributes(x11->display, x11->win, &win_attr))
      {
         x11->mouse_delta_x = 0;
         x11->mouse_delta_y = 0;
         return;
      }

      centre_x = win_attr.width  >> 1;
      centre_y = win_attr.height >> 1;

      /* Get relative movement delta since last
       * poll event */
      x11->mouse_delta_x = win_x - centre_x;
      x11->mouse_delta_y = win_y - centre_y;

      /* Get effective 'absolute' pointer location
       * (last position + delta, bounded by current
       * application window dimensions) */
      x11->mouse_x += x11->mouse_delta_x;
      x11->mouse_x = (x11->mouse_x < 0)                ? 0                     : x11->mouse_x;
      x11->mouse_x = (x11->mouse_x >= win_attr.width)  ? (win_attr.width - 1)  : x11->mouse_x;

      x11->mouse_y += x11->mouse_delta_y;
      x11->mouse_y = (x11->mouse_y < 0)                ? 0                     : x11->mouse_y;
      x11->mouse_y = (x11->mouse_y >= win_attr.height) ? (win_attr.height - 1) : x11->mouse_y;

      /* Hack/workaround:
       * - X11 gives absolute pointer coordinates
       * - Once the pointer reaches a screen edge
       *   it cannot go any further
       * - To achieve 'relative' motion, we therefore
       *   have to reset the hardware cursor to the
       *   centre of the screen after polling each
       *   movement delta, such that it is always
       *   free to move in all directions during the
       *   time interval until the next poll event */
      if (win_x != centre_x)
      {
         warp_x  = centre_x;
         do_warp = true;
      }

      if (win_y != centre_y)
      {
         warp_y  = centre_y;
         do_warp = true;
      }

      if (do_warp)
      {
         XWarpPointer(x11->display, None,
               x11->win, 0, 0, 0, 0,
               warp_x, warp_y);
         XSync(x11->display, False);
      }
   }
}

static void x_grab_mouse(void *data, bool state)
{
   x11_input_t *x11 = (x11_input_t*)data;
   if (!x11)
      return;

   x11->mouse_grabbed = state;

   if (state)
   {
      XGrabPointer(x11->display, x11->win, False,
                   ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                   GrabModeAsync, GrabModeAsync, x11->win, None, CurrentTime);
   }
   else
      XUngrabPointer(x11->display, CurrentTime);
}

static uint64_t x_input_get_capabilities(void *data)
{
   uint64_t caps = 0;

   caps |= (1 << KS_DEVICE_JOYPAD);
   caps |= (1 << KS_DEVICE_MOUSE);
   caps |= (1 << KS_DEVICE_KEYBOARD);
   caps |= (1 << KS_DEVICE_LIGHTGUN);
   caps |= (1 << KS_DEVICE_POINTER);
   caps |= (1 << KS_DEVICE_ANALOG);

   return caps;
}

input_driver_t input_x = {
   x_input_init,
   x_input_poll,
   x_input_state,
   x_input_free,
   NULL,
   NULL,
   x_input_get_capabilities,
   "x",
   x_grab_mouse,
   NULL
};
