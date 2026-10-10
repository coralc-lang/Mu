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
| `libmualibs.a` | 2.4 MB | Mu's vendor bundle (incl. zstd) — CI build 2026-10-10 from `Mu_engine/native/`, see "CI builds" below |

### `packaged/linux-arm64/lib/`, `windows-x64/lib/`, `windows-arm64/lib/`

`wgpu_native.lib` on windows (windows-x64's archive was renamed from
`libwgpu_native.a` — same MSVC content, wrong extension), `libwgpu_native.a`
on linux — built upstream, not on this machine. `mualibs` archives here
are full **canonical-set** builds produced by CI (2026-10-10), replacing
the former partials (8-object linux-arm64, 2-object windows-x64) and the
empty windows-arm64 stub.

### `packaged/android-arm64/lib/`, `macos-arm64/lib/`, `wasm/lib/`

`libmualibs.a` — full canonical-set CI builds (2026-10-10); the android
archive previously held only 5 objects, and the macos/wasm directories
did not exist (their former placeholders were 46-byte stubs removed in
the reorganization below). `libwgpu_native.a` was added to
`android-arm64/` and `macos-arm64/` from the upstream wgpu-native
v29.0.1.1 release (matching headers now live at
`Mu_engine/include/webgpu/`).

### SIMD variants — removed 2026-10-10

The stale 8-object alternate builds under
`packaged/{linux-x86_64,windows-x64}/lib/mualibs/{sse,avx2,avx512}/`
were dropped; CI produces the canonical archive per platform.

## Reorganization 2026-10-10

The stray nested `packaged/packaged/` tree was merged into
`packaged/<platform>/lib/` (ISA variants filed under
`lib/mualibs/<variant>/`). Removed as stubs/empties: `wasm/libmualibs.a`
and `windows-arm64/mualibs.lib` (both 46-byte empty archives), the empty
`macos-arm64/` placeholder directory, and the incomplete 2-object
`linux-x86_64` duplicate `libmualibs.a` (the full 11-object build
already lives at `linux-x86_64/lib/libmualibs.a`). The removed
platforms came back later the same day as real CI builds — see
"CI builds" below.

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
| zstd objects (`lib/{common,compress,decompress,dictBuilder}`) | runtime compression — saves, cooked packs |
| `TracyClient.o` (+ `client/`, `common/` includes) | `src/tracy` — profiler zones, frame marks, plots |

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
`libmualibs.a` was copied into `packaged/linux-x86_64/lib/`. All
`packaged/*/lib` mualibs copies were refreshed from CI archives on
2026-10-10 (now the full set including zstd).

## CI builds (GitHub Actions)

Two workflows push to `coralc-lang/Mu` (both also dispatchable):

- **`.github/workflows/build-shared-libs.yml`** — JoltC + SDL3 shared
  libraries for `linux-x64`, `linux-arm64`, `windows-x64`,
  `windows-arm64`, `macos-arm64` and `android-arm64` (build-time -dev
  headers only; runners have no GPU drivers, so no runtime GPU tests;
  wasm deliberately left out — link SDL/Jolt statically there).
  Artifacts: `shared-libs-<platform>-<arch>`. On a fully green run the
  final `commit-packaged` job copies the `.so`/`.dll`/`.dylib` files
  into `packaged/<platform>/lib/` and pushes with `[skip ci]` — the
  shared runtime libs in the tree are refreshed automatically.
- **`.github/workflows/build-mualibs.yml`** — `mualibs` **and** `ktx`
  targets of `Mu_engine/native/` (vendored JoltC not compiled) for
  `linux-x64`, `linux-arm64`, `windows-x64`, `windows-arm64`,
  `macos-arm64`, `android-arm64` (NDK toolchain) and `wasm32`
  (Emscripten). Artifacts: `mualibs-<platform>-<arch>` — each verified
  to hold the full canonical object set (stb/cgltf/miniz/tinyexr/
  dr_libs/FNL/MikkTSpace + zstd + Tracy) plus `libktx.a` and its
  `astcenc` dependency (merged into `libktx.a` on macOS).

Fetch a run's outputs with `gh run download <run-id> -D <dir>`, or from
the run's Artifacts panel on the Actions tab.

## Deliberately not built now

- **SDL3 static / Jolt static on non-linux hosts** — the packaged
  statics are prebuilt upstream or linux-local; SDL3 links from the M3
  build phase onward (`src/sdl/sdl.crl`).
- **Per-library `libstb.a` & co.** — covered by `libmualibs.a`.

## Build quirk

The vendored miniz ships without miniz's CMake, so its generated
`miniz_export.h` is absent while `miniz.h` includes it unconditionally.
A static-build shim lives at `third_party/miniz/miniz_export.h` (empty
`MINIZ_EXPORT`) — needed for any build that compiles miniz sources.
