# Mu_engine/prebuilt — placeholder & prebuilt libraries

Status of every archive under `Mu_engine/prebuilt/` (reorganized 2026-10-09;
platform folders use the same naming as `Mu_engine/packaged/`).

## Layout

```
prebuilt/
├── linux-x86_64/
│   ├── libJolt.a                 REAL   89 MB
│   ├── libmualibs.a              REAL   (built 2026-10-09, see below)
│   ├── avx2/lib/libflecs_static.a   REAL   4.2 MB (unused — see flecs)
│   ├── sse2/lib/libflecs_static.a   REAL   4.2 MB (byte-identical to avx2/)
│   ├── libcgltf.a … libtinyexr.a  0-byte placeholders (8 files)
├── linux-arm64/
│   ├── libcgltf.a … libtinyexr.a  0-byte placeholders (9 files)
│   └── neon/lib/                  empty (no arm flecs build was made)
├── windows-x64/                  0-byte .lib placeholders (9 files)
├── windows-arm64/                0-byte .lib placeholders (9 files)
└── wasm/                         0-byte placeholders (9 files)
```

## What each library is, and its status

| Placeholder | What it is | Status |
|---|---|---|
| `libmualibs.a` | Vendor implementations behind the Coral ports: stb, cgltf, miniz, tinyexr, dr_libs, FastNoiseLite, MikkTSpace (one archive, target `mualibs` in `Mu_engine/native/CMakeLists.txt`) | **Built** — `linux-x86_64` filled 2026-10-09 from `build/native/libmualibs.a`; other platforms 0-byte until built there |
| `libJolt.a` | Jolt physics static library | **linux-x86_64 real** (89 MB download: GCC 13.3, `-O3 -g -mavx512f/-mavx2 -flto`). For development the engine links its *own* build, `build/native/joltc/libJolt.a` (clang, Release). No arm64/windows/wasm build exists |
| `libsdl3.a` | SDL3 window/input/audio | Placeholder everywhere — **not needed yet**: "Not linked until the M3 build phase adds SDL3 to `link`" (`src/sdl/sdl.crl`) |
| `libstb.a`, `libcgltf.a`, `libminiz.a`, `libtinyexr.a`, `libdr_libs.a`, `libFastNoiseLite.a`, `libMikkTSpace.a` | Standalone per-library archives | For development **superseded by `libmualibs.a`** — the same implementations are compiled inside it. The placeholders mark the future per-platform prebuilt distribution |
| `libflecs_static.a` (`avx2/`, `sse2/`) | Reference builds from evaluating flecs as Mu's ECS | **Unused** — Mu uses its own ECS (ENGINE.md §4, `src/scene`). The two copies are byte-identical x86-64 builds (the avx2/sse2 names do not reflect their contents). `linux-arm64/neon/lib/` was never filled. Source tree deleted 2026-10-09; docs at `Mu_docs/flecs/` |

## What matters now for development on linux-x86_64

Three archives, all from one CMake project (`Mu_engine/native/`), all linked
by `Mu_engine/coral.crlb`:

1. `build/native/libmualibs.a`
2. `build/native/joltc/libJoltC.a`
3. `build/native/joltc/libJolt.a`

### Build / rebuild (linux x64)

```sh
cd Mu_engine
cmake -S native -B build/native -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build build/native -j2
```

Outputs land in `build/native/` (git-ignored via `.gitignore` `build/`).
After building, copy `libmualibs.a` into `prebuilt/linux-x86_64/` to keep
that placeholder filled.

Status 2026-10-09: configured fresh (the previous `build/native` cache
pointed at the old `Mu/engine` tree) and built with `-j2`.

### Deliberately not built now

- **SDL3** — linked only from the M3 build phase onward.
- **windows / wasm / arm64 archives** — need cross toolchains or their host
  machines; placeholders only.
- **Standalone `libstb.a` & co.** — covered by `libmualibs.a`.
- **flecs** — dropped as the ECS; reference docs only.

### Build quirk

The vendored miniz ships without miniz's CMake, so its generated
`miniz_export.h` is absent while `miniz.h` includes it unconditionally.
A static-build shim lives at `third_party/miniz/miniz_export.h` (empty
`MINIZ_EXPORT`) — needed for any build that compiles miniz sources.
