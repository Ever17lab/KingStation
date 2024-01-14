/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *  Copyright (C) 2014-2017 - Jean-André Santoni
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

#ifndef __WIFI_DRIVER__H
#define __WIFI_DRIVER__H

#include <stdint.h>

#include <boolean.h>
#include <ks_common_api.h>

KS_BEGIN_DECLS

enum kingsn_wifi_ctl_state
{
   KINGSN_WIFI_CTL_NONE = 0,
   KINGSN_WIFI_CTL_DESTROY,
   KINGSN_WIFI_CTL_DEINIT,
   KINGSN_WIFI_CTL_SET_ACTIVE,
   KINGSN_WIFI_CTL_UNSET_ACTIVE,
   KINGSN_WIFI_CTL_IS_ACTIVE,
   KINGSN_WIFI_CTL_FIND_DRIVER,
   KINGSN_WIFI_CTL_SET_CB,
   KINGSN_WIFI_CTL_STOP,
   KINGSN_WIFI_CTL_START,
   KINGSN_WIFI_CTL_INIT
};

typedef struct wifi_network_info
{
   char ssid[33];
   char passphrase[33];
   bool connected;
   bool saved_password;
   char netid[160];   /* Do not use, internal */
   /* TODO Add signal strength & other info */
} wifi_network_info_t;

typedef struct wifi_network_scan
{
   time_t scan_time;
   wifi_network_info_t *net_list;   /* This is an rbuf array */
} wifi_network_scan_t;

typedef struct wifi_driver
{
   void *(*init)(void);

   void (*free)(void *data);

   bool (*start)(void *data);
   void (*stop)(void *data);

   bool (*enable)(void *data, bool enabled);
   bool (*connection_info)(void *data, wifi_network_info_t *ssid);
   void (*scan)(void *data);
   wifi_network_scan_t* (*get_ssids)(void *data);
   bool (*ssid_is_online)(void *data, unsigned i);
   bool (*connect_ssid)(void *data, const wifi_network_info_t *netinfo);
   bool (*disconnect_ssid)(void *data, const wifi_network_info_t *netinfo);
   void (*tether_start_stop)(void *data, bool start, char* configfile);

   const char *ident;
} wifi_driver_t;

extern wifi_driver_t wifi_connmanctl;

/**
 * config_get_wifi_driver_options:
 *
 * Get an enumerated list of all wifi driver names,
 * separated by '|'.
 *
 * Returns: string listing of all wifi driver names,
 * separated by '|'.
 **/
const char* config_get_wifi_driver_options(void);

void driver_wifi_stop(void);

bool driver_wifi_start(void);

bool driver_wifi_enable(bool);

bool driver_wifi_connection_info(wifi_network_info_t *network);

void driver_wifi_scan(void);

wifi_network_scan_t* driver_wifi_get_ssids(void);

bool driver_wifi_ssid_is_online(unsigned i);

bool driver_wifi_connect_ssid(const wifi_network_info_t *netinfo);

bool driver_wifi_disconnect_ssid(const wifi_network_info_t* netinfo);

void driver_wifi_tether_start_stop(bool start, char* configfile);

bool wifi_driver_ctl(enum kingsn_wifi_ctl_state state, void *data);

KS_END_DECLS

#endif
