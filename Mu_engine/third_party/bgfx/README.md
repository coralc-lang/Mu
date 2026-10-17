# third_party/bgfx

Vendored **bgfx** render backend (ENGINE.md §2). bgfx is graphics-API
agnostic: one build selects GLES3 / OpenGL / Vulkan / Metal / D3D11 / D3D12 /
WebGPU at runtime, which is exactly the "any backend" requirement.

## What lives here

The tree is the [bgfx.cmake](https://github.com/bkaradzic/bgfx.cmake)
checkout (CC0 build scripts) with its three submodules checked out as
siblings:

```
third_party/bgfx/
  CMakeLists.txt     from bgfx.cmake
  bx/                bkaradzic/bx    (base library)
  bimg/              bkaradzic/bimg  (image/decode + shader tooling)
  bgfx/              bkaradzic/bgfx  (BSD-2-Clause)
```

`Mu_engine/coral.crlb` builds it via the `bgfx` phase into `build/bgfx/`
and links `libbgfx.a`, `libbimg.a`, `libbx.a`.

## Fetch / refresh

```sh
git clone --recursive https://github.com/bkaradzic/bgfx.cmake.git third_party/bgfx
# or, if submodules were not pulled:
cd third_party/bgfx && git submodule update --init --recursive
```

CI (`.github/workflows/build-bgfx.yml`) builds static archives per platform
and stages them under `Mu_engine/packaged/<platform>/lib/`, so a developer
who only consumes prebuilts does not need this tree checked out.

## License

bgfx is BSD-2-Clause; bx and bimg are the same. See `bgfx/LICENSE`.
