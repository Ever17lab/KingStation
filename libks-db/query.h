/* Copyright  (C) 2010-2017 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (query.h).
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

#ifndef __LIBKSDB_QUERY_H__
#define __LIBKSDB_QUERY_H__

#include <ks_common_api.h>

#include "libksdb.h"
#include "rmsgpack_dom.h"

KS_BEGIN_DECLS

typedef struct libksdb_query libksdb_query_t;

void libksdb_query_inc_ref(libksdb_query_t *q);

void libksdb_query_dec_ref(libksdb_query_t *q);

int libksdb_query_filter(libksdb_query_t *q, struct rmsgpack_dom_value *v);

KS_END_DECLS

#endif
