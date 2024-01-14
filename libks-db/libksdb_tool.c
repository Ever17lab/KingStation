/* Copyright  (C) 2010-2017 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (libksdb_tool.c).
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
#include <string.h>

#include <string/stdstring.h>

#include "libksdb.h"
#include "rmsgpack_dom.h"

int main(int argc, char ** argv)
{
   int rv;
   struct rmsgpack_dom_value item;
   const char *command                            = NULL;
   const char *path                               = NULL;
   const char *query_exp                          = NULL;
   const char *error                              = NULL;
   libksdb_t *db                               = NULL;
   libksdb_cursor_t *cur                       = NULL;
   libksdb_query_t *q                          = NULL;

   if (argc < 3)
   {
      printf("Usage: %s <db file> <command> [extra args...]\n", argv[0]);
      printf("Available Commands:\n");
      printf("\tlist\n");
      printf("\tcreate-index <index name> <field name>\n");
      printf("\tfind <query expression>\n");
      printf("\tget-names <query expression>\n");
      return 1;
   }

   command = argv[2];
   path    = argv[1];

   db      = libksdb_new();
   cur     = libksdb_cursor_new();

   if (!db || !cur)
      goto error;

   if ((rv = libksdb_open(path, db)) != 0)
   {
      printf("Could not open db file '%s': %s\n", path, strerror(-rv));
      goto error;
   }
   else if (memcmp(command, "list", 4) == 0)
   {
      if ((rv = libksdb_cursor_open(db, cur, NULL)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }

      if (argc != 3)
      {
         printf("Usage: %s <db file> list\n", argv[0]);
         goto error;
      }

      while (libksdb_cursor_read_item(cur, &item) == 0)
      {
         rmsgpack_dom_value_print(&item);
         printf("\n");
         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "find", 4) == 0)
   {
      if (argc != 4)
      {
         printf("Usage: %s <db file> find <query expression>\n", argv[0]);
         goto error;
      }

      query_exp = argv[3];
      error = NULL;
      q = libksdb_query_compile(db, query_exp, strlen(query_exp), &error);

      if (error)
      {
         printf("%s\n", error);
         goto error;
      }

      if ((rv = libksdb_cursor_open(db, cur, q)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }

      while (libksdb_cursor_read_item(cur, &item) == 0)
      {
         rmsgpack_dom_value_print(&item);
         printf("\n");
         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "get-names", 9) == 0)
   {
      if (argc != 4)
      {
         printf("Usage: %s <db file> get-names <query expression>\n", argv[0]);
         goto error;
      }

      query_exp = argv[3];
      error = NULL;
      q = libksdb_query_compile(db, query_exp, strlen(query_exp), &error);

      if (error)
      {
         printf("%s\n", error);
         goto error;
      }

      if ((rv = libksdb_cursor_open(db, cur, q)) != 0)
      {
         printf("Could not open cursor: %s\n", strerror(-rv));
         goto error;
      }

      while (libksdb_cursor_read_item(cur, &item) == 0)
      {
         if (item.type == RDT_MAP) //should always be true, but if false the program would segfault
         {
            unsigned i;
            for (i = 0; i < item.val.map.len; i++)
            {
               if (item.val.map.items[i].key.type == RDT_STRING && (strncmp(item.val.map.items[i].key.val.string.buff, "name", item.val.map.items[i].key.val.string.len) == 0))
               {
                  rmsgpack_dom_value_print(&item.val.map.items[i].value);
                  printf("\n");
               }
            }
         }

         rmsgpack_dom_value_free(&item);
      }
   }
   else if (memcmp(command, "create-index", 12) == 0)
   {
      const char * index_name, * field_name;

      if (argc != 5)
      {
         printf("Usage: %s <db file> create-index <index name> <field name>\n", argv[0]);
         goto error;
      }

      index_name = argv[3];
      field_name = argv[4];

      libksdb_create_index(db, index_name, field_name);
   }
   else
   {
      printf("Unknown command %s\n", argv[2]);
      goto error;
   }

   libksdb_cursor_close(cur);
   libksdb_close(db);

error:
   if (db)
      libksdb_free(db);
   if (cur)
      libksdb_cursor_free(cur);
   if (q)
      libksdb_query_free(q);
   return 1;
}
