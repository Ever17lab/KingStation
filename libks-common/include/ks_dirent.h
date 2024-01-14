/* Copyright  (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (ks_dirent.h).
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

#ifndef __KS_DIRENT_H
#define __KS_DIRENT_H

#include <libks.h>
#include <ks_common_api.h>

#include <boolean.h>

KS_BEGIN_DECLS

#define DIRENT_REQUIRED_VFS_VERSION 3

void dirent_vfs_init(const struct ks_vfs_interface_info* vfs_info);

typedef struct RDIR RDIR;

/**
 *
 * ks_opendir:
 * @name         : path to the directory to open.
 *
 * Opens a directory for reading. Tidy up with ks_closedir.
 *
 * Returns: RDIR pointer on success, NULL if name is not a
 * valid directory, null itself or the empty string.
 */
struct RDIR *ks_opendir(const char *name);

struct RDIR *ks_opendir_include_hidden(const char *name, bool include_hidden);

int ks_readdir(struct RDIR *rdir);

/* Deprecated, returns false, left for compatibility */
bool ks_dirent_error(struct RDIR *rdir);

const char *ks_dirent_get_name(struct RDIR *rdir);

/**
 *
 * ks_dirent_is_dir:
 * @rdir         : pointer to the directory entry.
 * @unused       : deprecated, included for compatibility reasons, pass NULL
 *
 * Is the directory listing entry a directory?
 *
 * Returns: true if directory listing entry is
 * a directory, false if not.
 */
bool ks_dirent_is_dir(struct RDIR *rdir, const char *unused);

void ks_closedir(struct RDIR *rdir);

KS_END_DECLS

#endif
