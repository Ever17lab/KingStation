/*  KingStation - A frontend for libks.
 *  Copyright (C) 2013-2014 - Jason Fetters
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

#ifndef _JOYPAD_CONNECTION_H
#define _JOYPAD_CONNECTION_H

#include <stdint.h>
#include <stddef.h>

#include <libks.h>
#include <ks_miscellaneous.h>
#include <ks_endianness.h>
#include "../input_driver.h"

/* Wii have PID/VID already swapped by USB_GetDescriptors from libogc */
#ifdef GEKKO
#define SWAP_IF_BIG(val) (val)
#else
#define SWAP_IF_BIG(val) swap_if_big16(val)
#endif

#define VID_NONE          0x0000
#define VID_NINTENDO      SWAP_IF_BIG(0x057e)
#define VID_SONY          SWAP_IF_BIG(0x054c)
#define VID_MICRONTEK     SWAP_IF_BIG(0x0079)
#define VID_PCS           SWAP_IF_BIG(0x0810)
#define VID_PS3_CLONE     SWAP_IF_BIG(0x0313)
#define VID_SNES_CLONE    SWAP_IF_BIG(0x081f)
#define VID_KSDE       SWAP_IF_BIG(0x0403)
#define VID_HORI_1        SWAP_IF_BIG(0x0f0d)

#define PID_NONE          0x0000
#define PID_NINTENDO_PRO  SWAP_IF_BIG(0x0330)
#define PID_SONY_DS3      SWAP_IF_BIG(0x0268)
#define PID_SONY_DS4      SWAP_IF_BIG(0x05c4)
#define PID_DS3_CLONE     SWAP_IF_BIG(0x20d6)
#define PID_SNES_CLONE    SWAP_IF_BIG(0xe401)
#define PID_MICRONTEK_NES SWAP_IF_BIG(0x0011)
#define PID_NINTENDO_GCA  SWAP_IF_BIG(0x0337)
#define PID_PCS_PS2PSX    SWAP_IF_BIG(0x0001)
#define PID_PCS_PSX2PS3   SWAP_IF_BIG(0x0003)
#define PID_KSDE       SWAP_IF_BIG(0x97c1)
#define PID_HORI_MINI_WIRED_PS4 SWAP_IF_BIG(0x00ee)

struct joypad_connection
{
    struct pad_connection_interface *iface;
    void* data;
    bool connected;
};

typedef struct pad_connection_interface
{
   void*    	(*init)(void *data, uint32_t slot, hid_driver_t *driver);
   void     	(*deinit)(void* device);
   void     	(*packet_handler)(void* device, uint8_t *packet, uint16_t size);
   void     	(*set_rumble)(void* device, enum ks_rumble_effect effect,
					uint16_t strength);
   void			(*get_buttons)(void *data, input_bits_t *state);
   int16_t  	(*get_axis)(void *data, unsigned axis);
   const char*	(*get_name)(void *data);
   int16_t      (*button)(void *data, uint16_t joykey);
} pad_connection_interface_t;

extern pad_connection_interface_t pad_connection_wii;
extern pad_connection_interface_t pad_connection_wiiupro;
extern pad_connection_interface_t pad_connection_ps3;
extern pad_connection_interface_t pad_connection_ps4;
extern pad_connection_interface_t pad_connection_snesusb;
extern pad_connection_interface_t pad_connection_nesusb;
extern pad_connection_interface_t pad_connection_wiiugca;
extern pad_connection_interface_t pad_connection_ps2adapter;
extern pad_connection_interface_t pad_connection_psxadapter;
extern pad_connection_interface_t pad_connection_ksde;
extern pad_connection_interface_t pad_connection_ps4_hori_mini;

int32_t pad_connection_pad_init(joypad_connection_t *joyconn,
   const char* name, uint16_t vid, uint16_t pid,
   void *data, hid_driver_t *driver);

joypad_connection_t *pad_connection_init(unsigned pads);

void pad_connection_destroy(joypad_connection_t *joyconn);

void pad_connection_pad_deinit(joypad_connection_t *joyconn,
   uint32_t idx);

void pad_connection_packet(joypad_connection_t *joyconn,
   uint32_t idx, uint8_t* data, uint32_t length);

void pad_connection_get_buttons(joypad_connection_t *joyconn,
   unsigned idx, input_bits_t* state);

int16_t pad_connection_get_axis(joypad_connection_t *joyconn,
   unsigned idx, unsigned i);

/* Determine if connected joypad is a hidpad backed device.
 * If false, pad_connection_packet cannot be used */

bool pad_connection_has_interface(joypad_connection_t *joyconn,
   unsigned idx);

int pad_connection_find_vacant_pad(joypad_connection_t *joyconn);

bool pad_connection_rumble(joypad_connection_t *s,
   unsigned pad, enum ks_rumble_effect effect, uint16_t strength);

const char* pad_connection_get_name(joypad_connection_t *joyconn,
   unsigned idx);

#endif
