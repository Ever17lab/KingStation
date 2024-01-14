#ifndef FFMPEG_FFT_H_
#define FFMPEG_FFT_H_

#include <glsym/glsym.h>

#include <ks_common_api.h>

KS_BEGIN_DECLS

typedef struct GLFFT fft_t;

fft_t *fft_new(unsigned fft_steps, rglgen_proc_address_t proc);

void fft_free(fft_t *fft);

void fft_init_multisample(fft_t *fft, unsigned width, unsigned height, unsigned samples);

void fft_step_fft(fft_t *fft, const GLshort *buffer, unsigned frames);

void fft_render(fft_t *fft, GLuint backbuffer, unsigned width, unsigned height);

KS_END_DECLS

#endif
