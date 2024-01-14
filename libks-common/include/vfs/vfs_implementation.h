/* Copyright  (C) 2010-2020 The KingStation team
*
* ---------------------------------------------------------------------------------------
* The following license statement only applies to this file (vfs_implementation.h).
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

#ifndef __LIBKS_SDK_VFS_IMPLEMENTATION_H
#define __LIBKS_SDK_VFS_IMPLEMENTATION_H

#include <stdio.h>
#include <stdint.h>
#include <libks.h>
#include <ks_environment.h>
#include <vfs/vfs.h>

KS_BEGIN_DECLS

libks_vfs_implementation_file *ks_vfs_file_open_impl(const char *path, unsigned mode, unsigned hints);

int ks_vfs_file_close_impl(libks_vfs_implementation_file *stream);

int ks_vfs_file_error_impl(libks_vfs_implementation_file *stream);

int64_t ks_vfs_file_size_impl(libks_vfs_implementation_file *stream);

int64_t ks_vfs_file_truncate_impl(libks_vfs_implementation_file *stream, int64_t length);

int64_t ks_vfs_file_tell_impl(libks_vfs_implementation_file *stream);

int64_t ks_vfs_file_seek_impl(libks_vfs_implementation_file *stream, int64_t offset, int seek_position);

int64_t ks_vfs_file_read_impl(libks_vfs_implementation_file *stream, void *s, uint64_t len);

int64_t ks_vfs_file_write_impl(libks_vfs_implementation_file *stream, const void *s, uint64_t len);

int ks_vfs_file_flush_impl(libks_vfs_implementation_file *stream);

int ks_vfs_file_remove_impl(const char *path);

int ks_vfs_file_rename_impl(const char *old_path, const char *new_path);

const char *ks_vfs_file_get_path_impl(libks_vfs_implementation_file *stream);

int ks_vfs_stat_impl(const char *path, int32_t *size);

int ks_vfs_mkdir_impl(const char *dir);

libks_vfs_implementation_dir *ks_vfs_opendir_impl(const char *dir, bool include_hidden);

bool ks_vfs_readdir_impl(libks_vfs_implementation_dir *dirstream);

const char *ks_vfs_dirent_get_name_impl(libks_vfs_implementation_dir *dirstream);

bool ks_vfs_dirent_is_dir_impl(libks_vfs_implementation_dir *dirstream);

int ks_vfs_closedir_impl(libks_vfs_implementation_dir *dirstream);

KS_END_DECLS

#endif
