/* dr_libs implementation TU — WAV / MP3 / FLAC decoders for the audio
 * layer (src/drlibs/drlibs.crl, ENGINE.md §14). One file implements all
 * three headers. Compiled into libmualibs.a. */

#define DR_WAV_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION

#include "dr_wav.h"
#include "dr_mp3.h"
#include "dr_flac.h"
