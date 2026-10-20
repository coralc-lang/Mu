/* stb implementation TU — one place for the STB_*_IMPLEMENTATION macros
 * the Coral port (src/stb/) binds against. Compiled into
 * libmualibs.a by native/CMakeLists.txt. */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_truetype.h"
