/*  KingStation - A frontend for libks.
 *  Copyright (C) 2011-2020 - Daniel De Matteis
 *  Copyright (C) 2019-2020 - James Leaver
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

#include <SDL/SDL.h>

#include <libks.h>

#include "../input_driver.h"

#include "../../tasks/tasks_internal.h"
#include "../../verbosity.h"

#if defined(HAVE_LIBSHAKE)
#include <shake.h>
#include "../../configuration.h"
#endif

/* Simple joypad driver designed to rationalise
 * the bizarre keyboard/gamepad hybrid setup
 * of OpenDingux devices */

#define SDL_DINGUX_JOYPAD_NAME "Dingux Gamepad"

#if defined(HAVE_LIBSHAKE)
/* 5 ms period == 200 Hz
 * > Meissner's Corpuscle registers this
 *   as 'fast' motion */
#define SDL_DINGUX_RUMBLE_WEAK_PERIOD 5

/* 142 ms period ~= 7 Hz
 * > Merkel's Cells and Ruffini Ending register
 *   this as 'slow' motion */
#define SDL_DINGUX_RUMBLE_STRONG_PERIOD 142

typedef struct
{
   int id;
   uint16_t strength;
   Shake_Effect effect;
   bool active;
} dingux_joypad_rumble_effect_t;

typedef struct
{
   Shake_Device *device;
   dingux_joypad_rumble_effect_t weak;
   dingux_joypad_rumble_effect_t strong;
} dingux_joypad_rumble_t;
#endif

typedef struct
{
   SDL_Joystick *device;
#if defined(HAVE_LIBSHAKE)
   dingux_joypad_rumble_t rumble;
#endif
   unsigned num_axes;
   uint16_t pad_state;
   int16_t analog_state[2][2];
   bool connected;
   bool menu_toggle;
} dingux_joypad_t;

/* TODO/FIXME - global referenced outside */
extern uint64_t lifecycle_state;

static dingux_joypad_t dingux_joypad;

#if defined(HAVE_LIBSHAKE)
static bool sdl_dingux_rumble_init(dingux_joypad_rumble_t *rumble)
{
   settings_t *settings = config_get_ptr();
   unsigned rumble_gain = settings ? settings->uints.input_dingux_rumble_gain : 0;
   bool weak_uploaded   = false;
   bool strong_uploaded = false;

   /* If gain is zero, rumble is disabled
    * > No need to initialise device */
   if (rumble_gain == 0)
      goto error;

   if (Shake_NumOfDevices() < 1)
      goto error;

   /* Open shake device */
   rumble->device = Shake_Open(0);

   if (!rumble->device)
      goto error;

   /* Check whether shake device has the required
    * feature set */
   if (!Shake_QueryEffectSupport(rumble->device, SHAKE_EFFECT_PERIODIC) ||
       !Shake_QueryWaveformSupport(rumble->device, SHAKE_PERIODIC_SINE))
      goto error;

   /* In most cases it is recommended to use SHAKE_EFFECT_PERIODIC
    * instead of SHAKE_EFFECT_RUMBLE. All devices that support
    * SHAKE_EFFECT_RUMBLE support SHAKE_EFFECT_PERIODIC (square,
    * triangle, sine) and vice versa */

   /* Initialise weak rumble effect */
   if (Shake_InitEffect(&rumble->weak.effect, SHAKE_EFFECT_PERIODIC) != SHAKE_OK)
      goto error;

   rumble->weak.effect.u.periodic.waveform  = SHAKE_PERIODIC_SINE;
   rumble->weak.effect.u.periodic.period    = SDL_DINGUX_RUMBLE_WEAK_PERIOD;
   rumble->weak.effect.u.periodic.magnitude = 0;
   rumble->weak.id                          = Shake_UploadEffect(rumble->device, &rumble->weak.effect);

   if (rumble->weak.id == SHAKE_ERROR)
      goto error;
   weak_uploaded = true;

   /* Initialise strong rumble effect */
   if (Shake_InitEffect(&rumble->strong.effect, SHAKE_EFFECT_PERIODIC) != SHAKE_OK)
      goto error;

   rumble->strong.effect.u.periodic.waveform  = SHAKE_PERIODIC_SINE;
   rumble->strong.effect.u.periodic.period    = SDL_DINGUX_RUMBLE_STRONG_PERIOD;
   rumble->strong.effect.u.periodic.magnitude = 0;
   rumble->strong.id                          = Shake_UploadEffect(rumble->device, &rumble->strong.effect);

   if (rumble->strong.id == SHAKE_ERROR)
      goto error;
   strong_uploaded = true;

   /* Set gain, if supported */
   if (Shake_QueryGainSupport(rumble->device))
      if (Shake_SetGain(rumble->device, (int)rumble_gain) != SHAKE_OK)
         goto error;

   return true;

error:
   if (rumble_gain != 0)
      KINGSN_WARN("[libShake]: Input device does not support rumble effects.\n");

   if (rumble->device)
   {
      if (weak_uploaded)
         Shake_EraseEffect(rumble->device, rumble->weak.id);

      if (strong_uploaded)
         Shake_EraseEffect(rumble->device, rumble->strong.id);

      Shake_Close(rumble->device);
      rumble->device = NULL;
   }

   return false;
}

static bool sdl_dingux_rumble_update(Shake_Device *device,
      dingux_joypad_rumble_effect_t *effect,
      uint16_t strength, uint16_t max_strength)
{
   /* If strength is zero, halt rumble effect */
   if (strength == 0)
   {
      if (effect->active)
      {
         if (Shake_Stop(device, effect->id) == SHAKE_OK)
         {
            effect->active = false;
            return true;
         }
         else
            return false;
      }

      return true;
   }

   /* If strength has changed, update effect */
   if (strength != effect->strength)
   {
      int id;

      effect->effect.id                   = effect->id;
      effect->effect.u.periodic.magnitude = (max_strength * strength) / 0xFFFF;
      id                                  = Shake_UploadEffect(device, &effect->effect);

      if (id == SHAKE_ERROR)
         return false;

      effect->id                          = id;
      effect->strength                    = strength;
   }

   /* If effect is currently idle, activate it */
   if (!effect->active)
   {
      if (Shake_Play(device, effect->id) == SHAKE_OK)
      {
         effect->active = true;
         return true;
      }
      else
         return false;
   }

   return true;
}

static bool sdl_dingux_joypad_set_rumble(unsigned pad,
      enum ks_rumble_effect effect, uint16_t strength)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   if (!joypad->rumble.device)
      return false;

   switch (effect)
   {
      case KS_RUMBLE_STRONG:
         return sdl_dingux_rumble_update(joypad->rumble.device,
               &joypad->rumble.strong, strength,
               SHAKE_RUMBLE_STRONG_MAGNITUDE_MAX);
      case KS_RUMBLE_WEAK:
         return sdl_dingux_rumble_update(joypad->rumble.device,
               &joypad->rumble.weak, strength,
               SHAKE_RUMBLE_WEAK_MAGNITUDE_MAX);
      default:
         break;
   }

   return false;
}
#endif

static const char *sdl_dingux_joypad_name(unsigned port)
{
   const char *joypad_name = NULL;

   if (port != 0)
      return NULL;

   return SDL_DINGUX_JOYPAD_NAME;
}

static void sdl_dingux_joypad_connect(void)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   /* Open joypad device */
   if (SDL_NumJoysticks() > 0)
      joypad->device = SDL_JoystickOpen(0);

   /* If joypad exists, get number of axes */
   if (joypad->device)
      joypad->num_axes = SDL_JoystickNumAxes(joypad->device);

#if defined(HAVE_LIBSHAKE)
   /* Configure rumble interface */
   sdl_dingux_rumble_init(&joypad->rumble);
#endif

   /* 'Register' joypad connection via
    * autoconfig task */
   input_autoconfigure_connect(
         sdl_dingux_joypad_name(0), /* name */
         NULL,                      /* display_name */
         sdl_dingux_joypad.ident,   /* driver */
         0,                         /* port */
         0,                         /* vid */
         0);                        /* pid */

   joypad->connected = true;
}

static void sdl_dingux_joypad_disconnect(void)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   if (joypad->device)
      SDL_JoystickClose(joypad->device);

   if (joypad->connected)
      input_autoconfigure_disconnect(0, sdl_dingux_joypad.ident);

#if defined(HAVE_LIBSHAKE)
   if (joypad->rumble.device)
   {
      if (joypad->rumble.weak.active)
         Shake_Stop(joypad->rumble.device, joypad->rumble.weak.id);

      if (joypad->rumble.strong.active)
         Shake_Stop(joypad->rumble.device, joypad->rumble.strong.id);

      Shake_EraseEffect(joypad->rumble.device, joypad->rumble.weak.id);
      Shake_EraseEffect(joypad->rumble.device, joypad->rumble.strong.id);

      Shake_Close(joypad->rumble.device);
   }
#endif

   memset(joypad, 0, sizeof(dingux_joypad_t));
}

static void sdl_dingux_joypad_destroy(void)
{
   SDL_Event event;

   /* Disconnect joypad */
   sdl_dingux_joypad_disconnect();

   /* De-initialise joystick subsystem */
   SDL_QuitSubSystem(SDL_INIT_JOYSTICK);

   /* Flush out all pending events */
   while (SDL_PollEvent(&event));

#if defined(HAVE_LIBSHAKE)
   /* De-initialise rumble interface */
   Shake_Quit();
#endif

   BIT64_CLEAR(lifecycle_state, KINGSN_MENU_TOGGLE);
}

static void *sdl_dingux_joypad_init(void *data)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   memset(joypad, 0, sizeof(dingux_joypad_t));
   BIT64_CLEAR(lifecycle_state, KINGSN_MENU_TOGGLE);

   /* Initialise joystick subsystem */
   if (SDL_WasInit(0) == 0)
   {
      if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
         return NULL;
   }
   else if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
      return NULL;

#if defined(HAVE_LIBSHAKE)
   /* Initialise rumble interface */
   Shake_Init();
#endif

   /* Connect joypad */
   sdl_dingux_joypad_connect();

   return (void*)-1;
}

static bool sdl_dingux_joypad_query_pad(unsigned port)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   return (port == 0) && joypad->connected;
}

static int16_t sdl_dingux_joypad_button(unsigned port, uint16_t joykey)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   if (port != 0)
      return 0;

   return (joypad->pad_state & (1 << joykey));
}

static void sdl_dingux_joypad_get_buttons(unsigned port, input_bits_t *state)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;

   /* Macros require braces here... */
   if (port == 0)
   {
      BITS_COPY16_PTR(state, joypad->pad_state);
   }
   else
   {
      BIT256_CLEAR_ALL_PTR(state);
   }
}

static int16_t sdl_dingux_joypad_axis_state(unsigned port, uint32_t joyaxis)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;
   int val                 = 0;
   int axis                = -1;
   bool is_neg             = false;
   bool is_pos             = false;

   if (port != 0)
      return 0;

   if (AXIS_NEG_GET(joyaxis) < 4)
   {
      axis   = AXIS_NEG_GET(joyaxis);
      is_neg = true;
   }
   else if (AXIS_POS_GET(joyaxis) < 4)
   {
      axis   = AXIS_POS_GET(joyaxis);
      is_pos = true;
   }
   else
      return 0;

   switch (axis)
   {
      case 0:
      case 1:
         val = joypad->analog_state[0][axis];
         break;
      case 2:
      case 3:
         val = joypad->analog_state[1][axis - 2];
         break;
   }

   if (is_neg && val > 0)
      return 0;
   else if (is_pos && val < 0)
      return 0;

   return val;
}

static int16_t sdl_dingux_joypad_axis(unsigned port, uint32_t joyaxis)
{
   if (port != 0)
      return 0;

   return sdl_dingux_joypad_axis_state(port, joyaxis);
}

static int16_t sdl_dingux_joypad_state(
      kingsn_joypad_info_t *joypad_info,
      const struct ks_keybind *binds,
      unsigned port)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;
   uint16_t port_idx       = joypad_info->joy_idx;
   int16_t ret             = 0;
   size_t i;

   if (port_idx != 0)
      return 0;

   for (i = 0; i < KINGSN_FIRST_CUSTOM_BIND; i++)
   {
      /* Auto-binds are per joypad, not per user. */
      const uint64_t joykey  = (binds[i].joykey != NO_BTN)
         ? binds[i].joykey  : joypad_info->auto_binds[i].joykey;
      const uint32_t joyaxis = (binds[i].joyaxis != AXIS_NONE)
         ? binds[i].joyaxis : joypad_info->auto_binds[i].joyaxis;
      
      if ((uint16_t)joykey != NO_BTN && 
            (joypad->pad_state & (1 << (uint16_t)joykey)))
         ret |= (1 << i);
      else if (joyaxis != AXIS_NONE &&
            ((float)abs(sdl_dingux_joypad_axis_state(port_idx, joyaxis)) 
             / 0x8000) > joypad_info->axis_threshold)
         ret |= (1 << i);
   }

   return ret;
}

static void sdl_dingux_joypad_poll(void)
{
   dingux_joypad_t *joypad = (dingux_joypad_t*)&dingux_joypad;
   SDL_Event event;

   /* Note: The menu toggle key is an awkward special
    * case - the press/release events happen almost
    * instantaneously, and since we only sample once
    * per frame the input is often 'missed'.
    * If the toggle key gets pressed, we therefore have
    * to wait until the *next* frame to release it */
   if (joypad->menu_toggle)
   {
      BIT64_CLEAR(lifecycle_state, KINGSN_MENU_TOGGLE);
      joypad->menu_toggle = false;
   }

   /* All digital inputs map to keyboard keys
    * - X:      SDLK_SPACE
    * - A:      SDLK_LCTRL
    * - B:      SDLK_LALT
    * - Y:      SDLK_LSHIFT
    * - L:      SDLK_TAB
    * - R:      SDLK_BACKSPACE
    * - L2:     SDLK_PAGEUP
    * - R2:     SDLK_PAGEDOWN
    * - Select: SDLK_ESCAPE
    * - Start:  SDLK_RETURN
    * - L3:     SDLK_KP_DIVIDE
    * - R3:     SDLK_KP_PERIOD
    * - Up:     SDLK_UP
    * - Right:  SDLK_RIGHT
    * - Down:   SDLK_DOWN
    * - Left:   SDLK_LEFT
    * - Menu:   SDLK_HOME
    */
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
               case SDLK_SPACE:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_X);
                  break;
               case SDLK_LCTRL:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_A);
                  break;
               case SDLK_LALT:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_B);
                  break;
               case SDLK_LSHIFT:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_Y);
                  break;
               case SDLK_TAB:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L);
                  break;
               case SDLK_BACKSPACE:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R);
                  break;
               case SDLK_PAGEUP:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L2);
                  break;
               case SDLK_PAGEDOWN:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R2);
                  break;
               case SDLK_ESCAPE:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_SELECT);
                  break;
               case SDLK_RETURN:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_START);
                  break;
               case SDLK_KP_DIVIDE:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L3);
                  break;
               case SDLK_KP_PERIOD:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R3);
                  break;
               case SDLK_UP:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_UP);
                  break;
               case SDLK_RIGHT:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_RIGHT);
                  break;
               case SDLK_DOWN:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_DOWN);
                  break;
               case SDLK_LEFT:
                  BIT16_SET(joypad->pad_state, KS_DEVICE_ID_JOYPAD_LEFT);
                  break;
               case SDLK_HOME:
                  BIT64_SET(lifecycle_state, KINGSN_MENU_TOGGLE);
                  joypad->menu_toggle = true;
                  break;
               default:
                  break;
            }
            break;
			case SDL_KEYUP:
            switch (event.key.keysym.sym)
            {
               case SDLK_SPACE:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_X);
                  break;
               case SDLK_LCTRL:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_A);
                  break;
               case SDLK_LALT:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_B);
                  break;
               case SDLK_LSHIFT:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_Y);
                  break;
               case SDLK_TAB:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L);
                  break;
               case SDLK_BACKSPACE:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R);
                  break;
               case SDLK_PAGEUP:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L2);
                  break;
               case SDLK_PAGEDOWN:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R2);
                  break;
               case SDLK_ESCAPE:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_SELECT);
                  break;
               case SDLK_RETURN:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_START);
                  break;
               case SDLK_KP_DIVIDE:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_L3);
                  break;
               case SDLK_KP_PERIOD:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_R3);
                  break;
               case SDLK_UP:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_UP);
                  break;
               case SDLK_RIGHT:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_RIGHT);
                  break;
               case SDLK_DOWN:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_DOWN);
                  break;
               case SDLK_LEFT:
                  BIT16_CLEAR(joypad->pad_state, KS_DEVICE_ID_JOYPAD_LEFT);
                  break;
               default:
                  break;
            }
				break;
			default:
				break;
		}
	}

   /* Analog inputs come from the joypad device,
    * if connected */
   if (joypad->device)
   {
      int16_t axis_value;

      SDL_JoystickUpdate();

      if (joypad->num_axes > 0)
      {
         axis_value = SDL_JoystickGetAxis(joypad->device, 0);
         /* -0x8000 can cause trouble if we later abs() it */
         joypad->analog_state[KS_DEVICE_INDEX_ANALOG_LEFT][KS_DEVICE_ID_ANALOG_X] =
               (axis_value < -0x7FFF) ? -0x7FFF : axis_value;
      }

      if (joypad->num_axes > 1)
      {
         axis_value = SDL_JoystickGetAxis(joypad->device, 1);
         joypad->analog_state[KS_DEVICE_INDEX_ANALOG_LEFT][KS_DEVICE_ID_ANALOG_Y] =
               (axis_value < -0x7FFF) ? -0x7FFF : axis_value;
      }

      if (joypad->num_axes > 2)
      {
         axis_value = SDL_JoystickGetAxis(joypad->device, 2);
         joypad->analog_state[KS_DEVICE_INDEX_ANALOG_RIGHT][KS_DEVICE_ID_ANALOG_X] =
               (axis_value < -0x7FFF) ? -0x7FFF : axis_value;
      }

      if (joypad->num_axes > 3)
      {
         axis_value = SDL_JoystickGetAxis(joypad->device, 3);
         joypad->analog_state[KS_DEVICE_INDEX_ANALOG_RIGHT][KS_DEVICE_ID_ANALOG_Y] =
               (axis_value < -0x7FFF) ? -0x7FFF : axis_value;
      }
   }
}

input_device_driver_t sdl_dingux_joypad = {
   sdl_dingux_joypad_init,
   sdl_dingux_joypad_query_pad,
   sdl_dingux_joypad_destroy,
   sdl_dingux_joypad_button,
   sdl_dingux_joypad_state,
   sdl_dingux_joypad_get_buttons,
   sdl_dingux_joypad_axis,
   sdl_dingux_joypad_poll,
#if defined(HAVE_LIBSHAKE)
   sdl_dingux_joypad_set_rumble,
#else
   NULL,
#endif
   sdl_dingux_joypad_name,
   "sdl_dingux",
};
