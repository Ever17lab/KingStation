/*  KingStation - A frontend for libks.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
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

#ifndef KINGSN_FILTER_H__
#define KINGSN_FILTER_H__

#include <stddef.h>

#include <libks.h>
#include <ks_common_api.h>

#define KINGSN_SOFTFILTER_THREADS_AUTO 0

KS_BEGIN_DECLS

typedef struct kingsn_softfilter kingsn_softfilter_t;

kingsn_softfilter_t *kingsn_softfilter_new(
      const char *filter_path,
      unsigned threads,
      enum ks_pixel_format in_pixel_format,
      unsigned max_width, unsigned max_height);

void kingsn_softfilter_free(kingsn_softfilter_t *filt);

void kingsn_softfilter_get_max_output_size(kingsn_softfilter_t *filt,
      unsigned *width, unsigned *height);

void kingsn_softfilter_get_output_size(kingsn_softfilter_t *filt,
      unsigned *out_width, unsigned *out_height,
      unsigned width, unsigned height);

enum ks_pixel_format kingsn_softfilter_get_output_format(
      kingsn_softfilter_t *filt);

void kingsn_softfilter_process(kingsn_softfilter_t *filt,
      void *output, size_t output_stride,
      const void *input, unsigned width, unsigned height, size_t input_stride);

const char *kingsn_softfilter_get_name(void *data);

KS_END_DECLS

#endif
