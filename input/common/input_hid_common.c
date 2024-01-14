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

#include "../include/gamepad.h"

enum gamepad_pad_axes
{
   AXIS_LEFT_ANALOG_X = 0,
   AXIS_LEFT_ANALOG_Y,
   AXIS_RIGHT_ANALOG_X,
   AXIS_RIGHT_ANALOG_Y,
   AXIS_TOUCH_X,
   AXIS_TOUCH_Y,
   AXIS_INVALID
};

void gamepad_read_axis_data(uint32_t axis, axis_data *data)
{
   if (!data)
      return;

   data->axis           = AXIS_POS_GET(axis);
   data->is_negative    = false;

   if (data->axis >= AXIS_INVALID)
   {
      data->axis        = AXIS_NEG_GET(axis);
      data->is_negative = true;
   }
}

int16_t gamepad_get_axis_value(int16_t state[3][2], axis_data *data)
{
   int16_t value = 0;

   if (!data)
      return 0;

   switch (data->axis)
   {
      case AXIS_LEFT_ANALOG_X:
      case AXIS_LEFT_ANALOG_Y:
         value = state[KS_DEVICE_INDEX_ANALOG_LEFT][data->axis];
         break;
      case AXIS_RIGHT_ANALOG_X:
      case AXIS_RIGHT_ANALOG_Y:
         value = state[KS_DEVICE_INDEX_ANALOG_RIGHT][data->axis];
         break;
   }

   if (data->is_negative && value > 0)
      return 0;
   if (!data->is_negative && value < 0)
      return 0;

   return value;
}
