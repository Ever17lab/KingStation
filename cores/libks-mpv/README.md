# libks-mpv

mpv media player as a libks core. A proof of concept release is now
available.

Aims to use features already established in mpv that are not currently
available in KingStation movieplayer.

I want to be able to use KingStation as my movie player on my embedded devices
(Raspberry Pi) and desktop using hardware acceleration without having to use
Kodi or mpv directly. Thus allowing for a more integrated experience, and
smaller root filesystem.

## Compiling

### Overview

KingStation must be compiled with `--disable-ffmpeg` to stop the integrated
movieplayer from playing the input file.

FFmpeg (preferably master branch) must be compiled with `--enable-shared`.

mpv must be compiled with `--enable-libmpv-shared`.

Then run `make` in the mpv-libks folder.

### Compiling with Mingw-w64 on Windows

KingStation must be compiled with `--disable-ffmpeg` and have OpenGL or
OpenGLES enabled. Compiling KingStation is not described here.

1. Open `Minwg-w64 64 bit` (not MSYS2 shell).
2. Install ffmpeg using `pacman -S mingw64/mingw-w64-x86_64-ffmpeg`.
3. Clone the ao_cb branch of https://github.com/deltabeard/mpv.git . This fork has the audio
callback patches required for libks-mpv.
4. Follow the instructions at https://github.com/mpv-player/mpv/blob/master/DOCS/compile-windows.md#native-compilation-with-msys2 to compile mpv.
5. Download libks-mpv release 0.3.alpha by either checking out the
   `audio-cb-new` branch of `https://github.com/libks/libks-mpv.git` or
   by downloading
   https://github.com/libks/libks-mpv/archive/0.3.alpha.tar.gz .
6. Run `make` in the libks-mpv folder. If using OpenGLES, run `make
   platform=gles` instead.

Overall, the dependencies required to build libks-mpv are:
- ffmpeg 4.0 (provided by mingw64/mingw-w64-x86_64-ffmpeg)
- mpv from https://github.com/deltabeard/mpv.git fork.
