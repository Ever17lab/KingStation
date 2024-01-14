/* Copyright  (C) 2010-2017 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (libksdb.h).
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

#ifndef __LIBKSDB_H__
#define __LIBKSDB_H__

#include <stdint.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ks_common_api.h>

#include "query.h"
#include "rmsgpack_dom.h"

KS_BEGIN_DECLS

typedef struct libksdb libksdb_t;

typedef struct libksdb_cursor libksdb_cursor_t;

typedef struct libksdb_index libksdb_index_t;

typedef int (*libksdb_value_provider)(void *ctx, struct rmsgpack_dom_value *out);

int libksdb_create(RFILE *fd, libksdb_value_provider value_provider, void *ctx);

void libksdb_close(libksdb_t *db);

int libksdb_open(const char *path, libksdb_t *db);

int libksdb_create_index(libksdb_t *db, const char *name,
      const char *field_name);

int libksdb_find_entry(libksdb_t *db, const char *index_name,
        const void *key, struct rmsgpack_dom_value *out);

libksdb_t *libksdb_new(void);

void libksdb_free(libksdb_t *db);

libksdb_cursor_t *libksdb_cursor_new(void);

void libksdb_cursor_free(libksdb_cursor_t *dbc);

/**
 * libksdb_cursor_open:
 * @db                  : Handle to database.
 * @cursor              : Handle to database cursor.
 * @q                   : Query to execute.
 *
 * Opens cursor to database based on query @q.
 *
 * Returns: 0 if successful, otherwise negative.
 **/
int libksdb_cursor_open(libksdb_t *db,
      libksdb_cursor_t *cursor,
      libksdb_query_t *query);

/**
 * libksdb_cursor_reset:
 * @cursor              : Handle to database cursor.
 *
 * Resets cursor.
 *
 * Returns: ???.
 **/
int libksdb_cursor_reset(libksdb_cursor_t *cursor);

/**
 * libksdb_cursor_close:
 * @cursor              : Handle to database cursor.
 *
 * Closes cursor and frees up allocated memory.
 **/
void libksdb_cursor_close(libksdb_cursor_t *cursor);

void *libksdb_query_compile(libksdb_t *db, const char *query,
        size_t buff_len, const char **error);

void libksdb_query_free(void *q);

int libksdb_cursor_read_item(libksdb_cursor_t *cursor,
      struct rmsgpack_dom_value *out);

KS_END_DECLS

#endif
