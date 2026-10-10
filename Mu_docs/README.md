# Mu — external library documentation

Documentation for every third-party library Mu uses, collected in one place.
Source trees stay in `Mu_engine/third_party/`; prebuilt archives live in
`Mu_engine/packaged/<platform>/lib/`. This folder keeps the
READMEs, licenses and guides where they are easy to find. Each subfolder is
a verbatim copy of the documentation that ships with the vendored tree.

| Library | What Mu uses it for | License | Where it lives | Docs here |
|---|---|---|---|---|
| SDL 3.4.18 | Window, input events, GL context, audio (ENGINE.md §2) — not linked until the M3 build phase | zlib | `Mu_engine/third_party/SDL` | `SDL/` (README, LICENSE.txt, docs/) |
| Jolt 5.6.0 | 3D physics simulation (ENGINE.md §9) | MIT | `Mu_engine/third_party/jolt` | `jolt/` |
| JoltPhysicsC | Flat C API over Jolt; `src/physics` rides on it | MIT | `Mu_engine/third_party/JoltPhysicsC` | `JoltPhysicsC/` |
| wgpu-native | WebGPU rendering backend (`src/webgpu`) | BSD-3-Clause | Prebuilt only: `Mu_engine/packaged/{linux-x86_64,linux-arm64,windows-x64,windows-arm64,macos-arm64,android-arm64}/lib` + headers `Mu_engine/include/webgpu/` (v29.0.1.1, no source tree) | — (headers hand-transcribed into `src/webgpu/webgpu.crl`) |
| stb | Image decode/encode, truetype fonts, vorbis (ENGINE.md §9) | Public domain / MIT | `third_party/stb`, compiled into `libmualibs.a` | `stb/` |
| cgltf | glTF parsing | MIT | `third_party/cgltf` → mualibs | `cgltf/` |
| miniz | Deflate / zip / PNG | miniz license (zlib-style, see LICENSE) | `third_party/miniz` → mualibs | `miniz/` |
| tinyexr | OpenEXR read/write | BSD 3-Clause | `third_party/tinyexr` → mualibs | `tinyexr/` |
| dr_libs | wav / mp3 / flac decoding | Public domain | `third_party/dr_libs` → mualibs | `dr_libs/` |
| FastNoiseLite | Procedural noise | MIT | `third_party/FastNoiseLite` → mualibs | `FastNoiseLite/` |
| MikkTSpace | Tangent-space generation | Permissive (Morten M. Mikkelsen — see README/source header) | `third_party/MikkTSpace` → mualibs | `MikkTSpace/` |
| zstd 1.5.7 | Runtime compression — saves, cooked asset packs (ENGINE.md §17) | BSD-3-Clause / GPLv2 dual | `third_party/zstd` → mualibs | `zstd/` (README, LICENSE, COPYING) |
| Tracy 0.14.1 | Frame marks / zones / plots for the engine profiler (`src/tracy`) | BSD-3-Clause | `third_party/tracy` → mualibs (`TracyClient.cpp` with `TRACY_ENABLE`) | `tracy/` (README, LICENSE) |
| KTX-Software 4.4.2 | KTX2 + Basis Universal GPU texture decode (`src/ktx`, M5) | Apache-2.0 | `third_party/KTX-Software` → own static archive `libktx.a` (+ astcenc) | `KTX/` (README, LICENSE, NOTICE) |

Notes:

- The `→ mualibs` entries are compiled together into one static archive
  (`Mu_engine/native/CMakeLists.txt` target `mualibs` → `build/native/libmualibs.a`),
  not as standalone libraries.
- Per-platform prebuilt archive inventory, what `libmualibs.a` contains and
  how to rebuild: see [`prebuilt-libs.md`](prebuilt-libs.md).
