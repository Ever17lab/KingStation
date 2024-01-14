/* KingStation - A frontend for libks.
 * Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 * Copyright (C) 2011-2017 - Daniel De Matteis
 *
 * KingStation is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * KingStation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with KingStation.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <boolean.h>
#include <stddef.h>
#include <string.h>

#include <file/config_file.h>
#include <file/file_path.h>
#include <lists/dir_list.h>
#include <ks_miscellaneous.h>
#include <string/stdstring.h>
#include <compat/strl.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "frontend_driver.h"
#include "../defaults.h"
#include "../verbosity.h"
#include "../file_path_special.h"

struct defaults g_defaults;

/*We need to set libks to the first entry in the cores
 * directory so that it will be saved to the config file
 */
static void find_first_libks_core(char *first_file,
   size_t size_of_first_file, const char *dir,
   const char * ext)
{
   size_t i;
   bool                 ret = false;
   struct string_list *list = dir_list_new(dir, ext, false, true, false, false);

   if (!list)
   {
      KINGSN_ERR("Couldn't read directory."
            " Cannot infer default libks core.\n");
      return;
   }

   KINGSN_LOG("Searching for valid libks implementation in: \"%s\".\n",
         dir);

   for (i = 0; i < list->size && !ret; i++)
   {
      char fname[PATH_MAX_LENGTH]           = {0};
      char salamander_name[PATH_MAX_LENGTH] = {0};
      const char *libks_elem             = (const char*)list->elems[i].data;

      KINGSN_LOG("Checking library: \"%s\".\n", libks_elem);

      if (!libks_elem)
         continue;

      fill_pathname_base(fname, libks_elem, sizeof(fname));

      if (!frontend_driver_get_salamander_basename(
               salamander_name, sizeof(salamander_name)))
         break;

      if (!strncmp(fname, salamander_name, sizeof(fname)))
      {
         if (list->size == (i + 1))
         {
            KINGSN_WARN("Entry is KingStation Salamander itself, "
                  "but is last entry. No choice but to set it.\n");
            strlcpy(first_file, fname, size_of_first_file);
         }

         continue;
      }

      strlcpy(first_file, fname, size_of_first_file);
      KINGSN_LOG("First found libks core is: \"%s\".\n", first_file);
      ret = true;
   }

   dir_list_free(list);
}

/* Last fallback - we'll need to start the first executable file
 * we can find in the KingStation cores directory.
 */
static void find_and_set_first_file(char *s, size_t len,
      const char *ext)
{

   char first_file[PATH_MAX_LENGTH] = {0};
   find_first_libks_core(first_file, sizeof(first_file),
         g_defaults.dirs[DEFAULT_DIR_CORE], ext);

   if (string_is_empty(first_file))
   {
      KINGSN_ERR("Failed last fallback - KingStation Salamander will exit.\n");
      return;
   }

   fill_pathname_join(s, g_defaults.dirs[DEFAULT_DIR_CORE], first_file, len);
   KINGSN_LOG("libks_path now set to: %s.\n", s);
}

static void salamander_init(char *s, size_t len)
{
   /* Normal executable loading path */
   config_file_t *config         = NULL;
   const char *kingsn_config_path = g_defaults.path_config;
   bool config_valid             = false;
   char config_path[PATH_MAX_LENGTH];
   char config_dir[PATH_MAX_LENGTH];

   config_path[0] = '\0';
   config_dir[0]  = '\0';

   /* Get salamander config file path */
   if (!string_is_empty(kingsn_config_path))
      fill_pathname_resolve_relative(config_path,
            kingsn_config_path,
            FILE_PATH_SALAMANDER_CONFIG,
            sizeof(config_path));
   else
      strcpy_literal(config_path, FILE_PATH_SALAMANDER_CONFIG);

   /* Ensure that config directory exists */
   fill_pathname_parent_dir(config_dir, config_path, sizeof(config_dir));
   if (!string_is_empty(config_dir) &&
       !path_is_directory(config_dir))
      path_mkdir(config_dir);

   /* Attempt to open config file */
   config = config_file_new_from_path_to_string(config_path);

   if (config)
   {
      char libks_path[PATH_MAX_LENGTH];

      libks_path[0] = '\0';

      if (config_get_path(config, "libks_path",
            libks_path, sizeof(libks_path)) &&
          !string_is_empty(libks_path) &&
          !string_is_equal(libks_path, "builtin"))
      {
         strlcpy(s, libks_path, len);
         config_valid = true;
      }

      config_file_free(config);
      config = NULL;
   }

   if (!config_valid)
   {
      char executable_name[PATH_MAX_LENGTH];

      executable_name[0] = '\0';

      /* No config file - search filesystem for
       * first available core */
      frontend_driver_get_core_extension(
            executable_name, sizeof(executable_name));
      find_and_set_first_file(s, len, executable_name);

      /* Save result to new config file */
      if (!string_is_empty(s))
      {
         config = config_file_new_alloc();

         if (config)
         {
            config_set_path(config, "libks_path", s);
            config_file_write(config, config_path, false);
            config_file_free(config);
         }
      }
   }
   else
      KINGSN_LOG("Start [%s] found in %s.\n", s,
            FILE_PATH_SALAMANDER_CONFIG);
}

#ifdef HAVE_MAIN
int salamander_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
   char libks_path[PATH_MAX_LENGTH] = {0};
   void *args                          = NULL;
   struct kingsn_main_wrap *wrap_args   = NULL;
   frontend_ctx_driver_t *frontend_ctx = (frontend_ctx_driver_t*)frontend_ctx_init_first();

   if (!frontend_ctx)
      return 0;

   if (frontend_ctx && frontend_ctx->init)
      frontend_ctx->init(args);

   if (frontend_ctx && frontend_ctx->environment_get)
      frontend_ctx->environment_get(&argc, argv, args, wrap_args);

   salamander_init(libks_path, sizeof(libks_path));

   if (frontend_ctx && frontend_ctx->deinit)
      frontend_ctx->deinit(args);

   if (frontend_ctx && frontend_ctx->exitspawn)
      frontend_ctx->exitspawn(libks_path, sizeof(libks_path), NULL);

   return 1;
}
