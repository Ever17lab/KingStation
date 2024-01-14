/* Copyright (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------------
 * The following license statement only applies to this libks API header (libks_d3d.h)
 * ---------------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the
 * "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIBKS_DIRECT3D_H__
#define LIBKS_DIRECT3D_H__

#include "libks.h"

#ifdef HAVE_D3D11

#include <d3d11.h>
#include <d3dcompiler.h>

#define KS_HW_RENDER_INTERFACE_D3D11_VERSION 1

struct ks_hw_render_interface_d3d11
{
  /* Must be set to KS_HW_RENDER_INTERFACE_D3D11. */
  enum ks_hw_render_interface_type interface_type;
  /* Must be set to KS_HW_RENDER_INTERFACE_D3D11_VERSION. */
  unsigned interface_version;

  /* Opaque handle to the d3d11 backend in the frontend
   * which must be passed along to all function pointers
   * in this interface.
   */
  void* handle;
  ID3D11Device *device;
  ID3D11DeviceContext *context;
  D3D_FEATURE_LEVEL featureLevel;
  pD3DCompile D3DCompile;
};

#endif

#endif /* LIBKS_DIRECT3D_H__ */
