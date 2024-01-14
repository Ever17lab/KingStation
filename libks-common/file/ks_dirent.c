/* Copyright  (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (ks_dirent.c).
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ks_common.h>

#include <boolean.h>
#include <ks_dirent.h>
#define VFS_FRONTEND
#include <vfs/vfs_implementation.h>

/* TODO/FIXME - static globals */
static ks_vfs_opendir_t dirent_opendir_cb                 = NULL;
static ks_vfs_readdir_t dirent_readdir_cb                 = NULL;
static ks_vfs_dirent_get_name_t dirent_dirent_get_name_cb = NULL;
static ks_vfs_dirent_is_dir_t dirent_dirent_is_dir_cb     = NULL;
static ks_vfs_closedir_t dirent_closedir_cb               = NULL;

void dirent_vfs_init(const struct ks_vfs_interface_info* vfs_info)
{
   const struct ks_vfs_interface* vfs_iface;

   dirent_opendir_cb         = NULL;
   dirent_readdir_cb         = NULL;
   dirent_dirent_get_name_cb = NULL;
   dirent_dirent_is_dir_cb   = NULL;
   dirent_closedir_cb        = NULL;

   vfs_iface                 = vfs_info->iface;

   if (
         vfs_info->required_interface_version < DIRENT_REQUIRED_VFS_VERSION || 
         !vfs_iface)
      return;

   dirent_opendir_cb         = vfs_iface->opendir;
   dirent_readdir_cb         = vfs_iface->readdir;
   dirent_dirent_get_name_cb = vfs_iface->dirent_get_name;
   dirent_dirent_is_dir_cb   = vfs_iface->dirent_is_dir;
   dirent_closedir_cb        = vfs_iface->closedir;
}

struct RDIR *ks_opendir_include_hidden(
      const char *name, bool include_hidden)
{
   if (dirent_opendir_cb)
      return (struct RDIR *)dirent_opendir_cb(name, include_hidden);
   return (struct RDIR *)ks_vfs_opendir_impl(name, include_hidden);
}

struct RDIR *ks_opendir(const char *name)
{
   return ks_opendir_include_hidden(name, false);
}

bool ks_dirent_error(struct RDIR *rdir)
{
   /* Left for compatibility */
   return false;
}

int ks_readdir(struct RDIR *rdir)
{
   if (dirent_readdir_cb)
      return dirent_readdir_cb((struct ks_vfs_dir_handle *)rdir);
   return ks_vfs_readdir_impl((struct ks_vfs_dir_handle *)rdir);
}

const char *ks_dirent_get_name(struct RDIR *rdir)
{
   if (dirent_dirent_get_name_cb)
      return dirent_dirent_get_name_cb((struct ks_vfs_dir_handle *)rdir);
   return ks_vfs_dirent_get_name_impl((struct ks_vfs_dir_handle *)rdir);
}

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
bool ks_dirent_is_dir(struct RDIR *rdir, const char *unused)
{
   if (dirent_dirent_is_dir_cb)
      return dirent_dirent_is_dir_cb((struct ks_vfs_dir_handle *)rdir);
   return ks_vfs_dirent_is_dir_impl((struct ks_vfs_dir_handle *)rdir);
}

void ks_closedir(struct RDIR *rdir)
{
   if (dirent_closedir_cb)
      dirent_closedir_cb((struct ks_vfs_dir_handle *)rdir);
   else
      ks_vfs_closedir_impl((struct ks_vfs_dir_handle *)rdir);
}
