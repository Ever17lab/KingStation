/* Copyright  (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (vfs_implementation_cdrom.h).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __LIBKS_SDK_VFS_IMPLEMENTATION_CDROM_H
#define __LIBKS_SDK_VFS_IMPLEMENTATION_CDROM_H

#include <vfs/vfs.h>
#include <cdrom/cdrom.h>

KS_BEGIN_DECLS

int64_t ks_vfs_file_seek_cdrom(libks_vfs_implementation_file *stream, int64_t offset, int whence);

void ks_vfs_file_open_cdrom(
      libks_vfs_implementation_file *stream,
      const char *path, unsigned mode, unsigned hints);

int ks_vfs_file_close_cdrom(libks_vfs_implementation_file *stream);

int64_t ks_vfs_file_tell_cdrom(libks_vfs_implementation_file *stream);

int64_t ks_vfs_file_read_cdrom(libks_vfs_implementation_file *stream,
      void *s, uint64_t len);

int ks_vfs_file_error_cdrom(libks_vfs_implementation_file *stream);

const cdrom_toc_t* ks_vfs_file_get_cdrom_toc(void);

const vfs_cdrom_t* ks_vfs_file_get_cdrom_position(const libks_vfs_implementation_file *stream);

KS_END_DECLS

#endif
