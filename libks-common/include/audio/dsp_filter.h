/* Copyright  (C) 2010-2020 The KingStation team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (dsp_filter.h).
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

#ifndef __LIBKS_SDK_AUDIO_DSP_FILTER_H
#define __LIBKS_SDK_AUDIO_DSP_FILTER_H

#include <ks_common_api.h>

KS_BEGIN_DECLS

typedef struct ks_dsp_filter ks_dsp_filter_t;

ks_dsp_filter_t *ks_dsp_filter_new(const char *filter_config,
      void *string_data, float sample_rate);

void ks_dsp_filter_free(ks_dsp_filter_t *dsp);

struct ks_dsp_data
{
   float *input;
   unsigned input_frames;

   /* Set by ks_dsp_filter_process(). */
   float *output;
   unsigned output_frames;
};

void ks_dsp_filter_process(ks_dsp_filter_t *dsp,
      struct ks_dsp_data *data);

KS_END_DECLS

#endif
