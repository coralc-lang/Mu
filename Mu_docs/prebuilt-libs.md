# Prebuilt libraries — inventory & build

All real prebuilt archives live in **`Mu_engine/packaged/<platform>/lib/`**
(the same place as the wgpu prebuilt). The old `Mu_engine/prebuilt/` tree —
a scaffold of 44 zero-byte placeholder files — was removed 2026-10-09:
`libmualibs.a` already *contains* the implementations of stb, cgltf, miniz,
tinyexr, dr_libs, FastNoiseLite and MikkTSpace, so those per-library
placeholders were never going to be filled separately.

## Inventory

### `packaged/linux-x86_64/lib/`

| Archive | Size | What it is |
|---|---|---|
| `libwgpu_native.a` | 50 MB | wgpu-native WebGPU backend (prebuilt) |
| `libtcc.a` | 6.4 MB | TinyCC (+ `tcc1/` runtime objects, `win32/` helpers) |
| `libJolt.a` | 85 MB | Jolt physics — downloaded x64 build (GCC 13.3, `-O3 -g -mavx512f/-mavx2 -flto`). *The engine's dev build links its own copy instead, see below* |
| `libmualibs.a` | 1.4 MB | Mu's vendor bundle — built 2026-10-09 from `Mu_engine/native/` |
| `libflecs_static.a` | 4 MB | Reference build from the flecs evaluation — **unused** (Mu uses its own ECS, ENGINE.md §4; two byte-identical copies were deduplicated to one, docs at `Mu_docs/flecs/`) |

### `packaged/linux-arm64/lib/`, `windows-x64/lib/`, `windows-arm64/lib/`

Only `libwgpu_native.a` (and `wgpu_native.lib` on windows) — built upstream,
not on this machine.

## What `libmualibs.a` is

Not a third-party library — Mu's own aggregate archive: the compiled C/C++
implementations behind the Coral extern ports. One archive instead of seven:

| Objects | Backs |
|---|---|
| `mu_stb.c.o`, `stb_vorbis.c.o` | `src/stb` (image decode/encode, truetype, vorbis) |
| `mu_cgltf.c.o` | `src/cgltf` |
| `miniz.c.o`, `miniz_zip.c.o`, `miniz_tinfl.c.o`, `miniz_tdef.c.o` | `src/miniz` |
| `tinyexr.c.o` | `src/tinyexr` |
| `mu_drlibs.c.o` | `src/drlibs` (wav/mp3/flac) |
| `mu_fnl.c.o` | `src/fastnoise` |
| `mikktspace.c.o` | `src/mikktspace` |

## Build / rebuild (linux x64)

The engine links `build/native/` copies (not the packaged ones) — three
archives from one CMake project, per `Mu_engine/coral.crlb`:

1. `build/native/libmualibs.a`
2. `build/native/joltc/libJoltC.a`
3. `build/native/joltc/libJolt.a`

```sh
cd Mu_engine
cmake -S native -B build/native -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build build/native -j2
```

`build/` is git-ignored. Status 2026-10-09: configured fresh (the old cache
pointed at the deleted `Mu/engine` tree) and built with `-j2`; the built
`libmualibs.a` was copied into `packaged/linux-x86_64/lib/`.

## Deliberately not built now

- **SDL3** — linked only from the M3 build phase onward (`src/sdl/sdl.crl`).
- **windows / wasm / arm64 archives** — need cross toolchains or their host
  machines; only upstream prebuilts (wgpu) exist for them.
- **Per-library `libstb.a` & co.** — covered by `libmualibs.a`.

## Build quirk

The vendored miniz ships without miniz's CMake, so its generated
`miniz_export.h` is absent while `miniz.h` includes it unconditionally.
A static-build shim lives at `third_party/miniz/miniz_export.h` (empty
`MINIZ_EXPORT`) — needed for any build that compiles miniz sources.
