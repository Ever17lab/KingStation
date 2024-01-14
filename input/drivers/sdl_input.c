/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2014-2015 - Higor Euripedes
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

#include <boolean.h>
#include <string/stdstring.h>
#include <libks.h>

#include "SDL.h"

#include "../input_keymaps.h"

#include "../../configuration.h"
#include "../../KingStation.h"
#include "../../verbosity.h"
#include "../../tasks/tasks_internal.h"

/* TODO/FIXME -
 * fix game focus toggle */

typedef struct sdl_input
{
   int mouse_x;
   int mouse_y;
   int mouse_abs_x;
   int mouse_abs_y;
   int mouse_l;
   int mouse_r;
   int mouse_m;
   int mouse_b4;
   int mouse_b5;
   int mouse_wu;
   int mouse_wd;
   int mouse_wl;
   int mouse_wr;
} sdl_input_t;

static void *sdl_input_init(const char *joypad_driver)
{
   sdl_input_t     *sdl = (sdl_input_t*)calloc(1, sizeof(*sdl));
   if (!sdl)
      return NULL;

   input_keymaps_init_keyboard_lut(kingsn_key_map_sdl);

   return sdl;
}

static bool sdl_key_pressed(int key)
{
   int num_keys;
#ifdef HAVE_SDL2
   const uint8_t *keymap = SDL_GetKeyboardState(&num_keys);
   unsigned sym          = SDL_GetScancodeFromKey(kingsn_keysym_lut[(enum ks_key)key]);
#else
   const uint8_t *keymap = SDL_GetKeyState(&num_keys);
   unsigned sym          = kingsn_keysym_lut[(enum ks_key)key];
#endif

   if (sym >= (unsigned)num_keys)
      return false;

   return keymap[sym];
}

static int16_t sdl_input_state(
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
   sdl_input_t            *sdl = (sdl_input_t*)data;

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
                  if (sdl_key_pressed(binds[port][i].key))
                     ret |= (1 << i);
            }

            return ret;
         }

         if (id < KINGSN_BIND_LIST_END)
         {
            if (binds[port][id].valid)
               if (sdl_key_pressed(binds[port][id].key))
                  return 1;
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
               unsigned sym = kingsn_keysym_lut[(enum ks_key)id_plus_key];
               if (sdl_key_pressed(sym))
                  ret = 0x7fff;
            }
            if (id_minus_valid && id_minus_key < KSK_LAST)
            {
               unsigned sym = kingsn_keysym_lut[(enum ks_key)id_minus_key];
               if (sdl_key_pressed(sym))
                  ret += -0x7fff;
            }

            return ret;
         }
         break;
      case KS_DEVICE_MOUSE:
      case KINGSN_DEVICE_MOUSE_SCREEN:
         if (config_get_ptr()->uints.input_mouse_index[ port ] == 0)
         {
            switch (id)
            {
               case KS_DEVICE_ID_MOUSE_LEFT:
                  return sdl->mouse_l;
               case KS_DEVICE_ID_MOUSE_RIGHT:
                  return sdl->mouse_r;
               case KS_DEVICE_ID_MOUSE_WHEELUP:
                  return sdl->mouse_wu;
               case KS_DEVICE_ID_MOUSE_WHEELDOWN:
                  return sdl->mouse_wd;
               case KS_DEVICE_ID_MOUSE_X:
                  return sdl->mouse_x;
               case KS_DEVICE_ID_MOUSE_Y:
                  return sdl->mouse_y;
               case KS_DEVICE_ID_MOUSE_MIDDLE:
                  return sdl->mouse_m;
               case KS_DEVICE_ID_MOUSE_BUTTON_4:
                  return sdl->mouse_b4;
               case KS_DEVICE_ID_MOUSE_BUTTON_5:
                  return sdl->mouse_b5;
            }
         }
         break;
      case KS_DEVICE_POINTER:
      case KINGSN_DEVICE_POINTER_SCREEN:
         if (idx == 0)
         {
            struct video_viewport vp;
            bool screen                 = device == 
               KINGSN_DEVICE_POINTER_SCREEN;
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

            if (video_driver_translate_coord_viewport_wrap(
                        &vp, sdl->mouse_abs_x, sdl->mouse_abs_y,
                        &res_x, &res_y, &res_screen_x, &res_screen_y))
            {
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
                     return res_x;
                  case KS_DEVICE_ID_POINTER_Y:
                     return res_y;
                  case KS_DEVICE_ID_POINTER_PRESSED:
                     return sdl->mouse_l;
                  case KS_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
                     return !inside;
               }
            }
         }
         break;
      case KS_DEVICE_KEYBOARD:
         return (id < KSK_LAST) && sdl_key_pressed(id);
      case KS_DEVICE_LIGHTGUN:
         switch (id)
         {
            case KS_DEVICE_ID_LIGHTGUN_X:
               return sdl->mouse_x;
            case KS_DEVICE_ID_LIGHTGUN_Y:
               return sdl->mouse_y;
            case KS_DEVICE_ID_LIGHTGUN_TRIGGER:
               return sdl->mouse_l;
            case KS_DEVICE_ID_LIGHTGUN_CURSOR:
               return sdl->mouse_m;
            case KS_DEVICE_ID_LIGHTGUN_TURBO:
               return sdl->mouse_r;
            case KS_DEVICE_ID_LIGHTGUN_START:
               return sdl->mouse_m && sdl->mouse_r;
            case KS_DEVICE_ID_LIGHTGUN_PAUSE:
               return sdl->mouse_m && sdl->mouse_l;
         }
         break;
   }

   return 0;
}

static void sdl_input_free(void *data)
{
#ifndef HAVE_SDL2
   SDL_Event event;
#endif
   sdl_input_t *sdl = (sdl_input_t*)data;

   if (!data)
      return;

   /* Flush out all pending events. */
#ifdef HAVE_SDL2
   SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
#else
   while (SDL_PollEvent(&event));
#endif

   free(data);
}

#ifdef HAVE_SDL2
static void sdl2_grab_mouse(void *data, bool state)
{
   struct temp
   {
      SDL_Window *w;
   };

   if (string_is_not_equal(video_driver_get_ident(), "sdl2"))
      return;

   /* First member of sdl2_video_t is the window */
   SDL_SetWindowGrab(((struct temp*)video_driver_get_ptr(false))->w,
         state ? SDL_TRUE : SDL_FALSE);
}
#endif

static void sdl_poll_mouse(sdl_input_t *sdl)
{
   Uint8 btn = SDL_GetRelativeMouseState(&sdl->mouse_x, &sdl->mouse_y);

   SDL_GetMouseState(&sdl->mouse_abs_x, &sdl->mouse_abs_y);

   sdl->mouse_l  = (SDL_BUTTON(SDL_BUTTON_LEFT)      & btn) ? 1 : 0;
   sdl->mouse_r  = (SDL_BUTTON(SDL_BUTTON_RIGHT)     & btn) ? 1 : 0;
   sdl->mouse_m  = (SDL_BUTTON(SDL_BUTTON_MIDDLE)    & btn) ? 1 : 0;
   sdl->mouse_b4 = (SDL_BUTTON(SDL_BUTTON_X1)        & btn) ? 1 : 0;
   sdl->mouse_b5 = (SDL_BUTTON(SDL_BUTTON_X2)        & btn) ? 1 : 0;
#ifndef HAVE_SDL2
   sdl->mouse_wu = (SDL_BUTTON(SDL_BUTTON_WHEELUP)   & btn) ? 1 : 0;
   sdl->mouse_wd = (SDL_BUTTON(SDL_BUTTON_WHEELDOWN) & btn) ? 1 : 0;
#endif
}

static void sdl_input_poll(void *data)
{
   SDL_Event event;
   sdl_input_t *sdl = (sdl_input_t*)data;

   SDL_PumpEvents();

   sdl_poll_mouse(sdl);

#ifdef HAVE_SDL2
   while (SDL_PeepEvents(&event, 1,
            SDL_GETEVENT, SDL_KEYDOWN, SDL_MOUSEWHEEL) > 0)
#else
   while (SDL_PeepEvents(&event, 1,
            SDL_GETEVENT, SDL_KEYEVENTMASK) > 0)
#endif
   {
      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
         uint16_t mod = 0;
         unsigned code = input_keymaps_translate_keysym_to_rk(
               event.key.keysym.sym);

         if (event.key.keysym.mod & KMOD_SHIFT)
            mod |= KSKMOD_SHIFT;

         if (event.key.keysym.mod & KMOD_CTRL)
            mod |= KSKMOD_CTRL;

         if (event.key.keysym.mod & KMOD_ALT)
            mod |= KSKMOD_ALT;

         if (event.key.keysym.mod & KMOD_NUM)
            mod |= KSKMOD_NUMLOCK;

         if (event.key.keysym.mod & KMOD_CAPS)
            mod |= KSKMOD_CAPSLOCK;

         input_keyboard_event(event.type == SDL_KEYDOWN, code, code, mod,
               KS_DEVICE_KEYBOARD);
      }
#ifdef HAVE_SDL2
      else if (event.type == SDL_MOUSEWHEEL)
      {
         sdl->mouse_wu = event.wheel.y < 0;
         sdl->mouse_wd = event.wheel.y > 0;
         sdl->mouse_wl = event.wheel.x < 0;
         sdl->mouse_wr = event.wheel.x > 0;
         break;
      }
#endif
   }
}

static uint64_t sdl_get_capabilities(void *data)
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

input_driver_t input_sdl = {
   sdl_input_init,
   sdl_input_poll,
   sdl_input_state,
   sdl_input_free,
   NULL,
   NULL,
   sdl_get_capabilities,
#ifdef HAVE_SDL2
   "sdl2",
   sdl2_grab_mouse,
#else
   "sdl",
   NULL,                   /* grab_mouse */
#endif
   NULL
};
