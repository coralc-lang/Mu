# mu_native — the engine's native glue (ENGINE.md §7).
#
#   libmualibs.a — vendor implementations behind the Coral ports: stb
#                  (image/write/truetype/vorbis), cgltf, miniz, tinyexr,
#                  dr_libs (wav/mp3/flac), FastNoiseLite, MikkTSpace.
#   joltc/       — Jolt 5.6.0 + JoltC via add_subdirectory (physics, §9).
#
# Driven by coral.crlb's "jolt" phase (output build/native/joltc/libJoltC.a):
#   cmake -S native -B build/native -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_CXX_COMPILER=clang++ && cmake --build build/native -j1
# Build caps are `-j1` — never run these unbounded (IMPORTANT.md).

cmake_minimum_required(VERSION 3.20)
project(mu_native C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MU_TP ${CMAKE_CURRENT_SOURCE_DIR}/../third_party)

# --- mualibs: implementation TUs for the Coral vendor ports (ENGINE.md §9) ---
# zstd: all four active components (no legacy/, no deprecated/) — runtime
# compression for saves and cooked asset packs.
file(GLOB MU_ZSTD_SOURCES
    ${MU_TP}/zstd/lib/common/*.c
    ${MU_TP}/zstd/lib/compress/*.c
    ${MU_TP}/zstd/lib/decompress/*.c
    ${MU_TP}/zstd/lib/dictBuilder/*.c
)

# tracy: profiler client - one TU (public/TracyClient.cpp pulls in
# client/ + common/); TRACY_ENABLE so the src/tracy port's externs link.
set(MU_TRACY_CLIENT ${MU_TP}/tracy/public/TracyClient.cpp)

# ktx: KTX2/Basis texture reader (port in src/ktx) - own static archive
# (libktx.a + astcenc) built from the vendored, patched CMake: tests/tools/
# GL/VK upload off, static only, zstd supplied by mualibs (the bundled
# basisu zstd.c was removed from its source list to avoid duplicate symbols).
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_VK_UPLOAD OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_DOC OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_JNI OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_PY OFF CACHE BOOL "" FORCE)
add_subdirectory(${MU_TP}/KTX-Software ${CMAKE_BINARY_DIR}/ktx-build)

add_library(mualibs STATIC
    mu_stb.c
    mu_cgltf.c
    mu_drlibs.c
    mu_fnl.c
    ${MU_TP}/stb/stb_vorbis.c
    ${MU_TP}/miniz/miniz.c
    ${MU_TP}/miniz/miniz_zip.c
    ${MU_TP}/miniz/miniz_tinfl.c
    ${MU_TP}/miniz/miniz_tdef.c
    ${MU_TP}/tinyexr/tinyexr.cc
    ${MU_TP}/MikkTSpace/mikktspace.c
    ${MU_ZSTD_SOURCES}
    ${MU_TRACY_CLIENT}
)
target_compile_definitions(mualibs PRIVATE TRACY_ENABLE)
target_include_directories(mualibs PUBLIC
    ${MU_TP}/stb
    ${MU_TP}/cgltf
    ${MU_TP}/miniz          # tinyexr does `#include <miniz.h>` — one shared copy
    ${MU_TP}/tinyexr        # for exr_reader.hh / streamreader.hh next to the header
    ${MU_TP}/dr_libs
    ${MU_TP}/FastNoiseLite/C
    ${MU_TP}/MikkTSpace
    ${MU_TP}/zstd/lib
    ${MU_TP}/tracy/public
)
set_target_properties(mualibs PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# --- JoltC: physics bindings over Jolt 5.6.0 (options mirror build/ cache) ---
set(PHYSICS_REPO_ROOT ${MU_TP}/jolt CACHE PATH "JoltPhysics repo root" FORCE)
set(JOLTC_STATIC ON CACHE BOOL "" FORCE)
set(JOLTC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(JOLTC_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
add_subdirectory(${MU_TP}/JoltPhysicsC/JoltC joltc)
