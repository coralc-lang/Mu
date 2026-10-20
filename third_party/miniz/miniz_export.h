/* Export shim for the vendored miniz — miniz.h unconditionally includes
   "miniz_export.h", which upstream's CMake generate_export_header()s from
   a template; this source drop ships without that build system. Mu compiles
   miniz into a static archive (libmualibs.a via native/CMakeLists.txt), so
   no symbol carries a shared-library export attribute: empty expansion is
   the correct definition for a static build. */
#pragma once

#ifndef MINIZ_EXPORT
#define MINIZ_EXPORT
#endif

#ifndef MINIZ_NO_EXPORT
#define MINIZ_NO_EXPORT
#endif
