/*  KingStation - A frontend for libks.
 *  Copyright (C) 2014-2018 - Ali Bouhlel
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

#ifndef __FRAME_SHADER_H_
#define __FRAME_SHADER_H_

#include <wiiu/shader_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   struct
   {
      float x;
      float y;
   }pos;

   struct
   {
      float u;
      float v;
   }coord;
}frame_vertex_t;

extern GX2Shader frame_shader;

#ifdef __cplusplus
}
#endif

#endif /* __FRAME_SHADER_H_ */
