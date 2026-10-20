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

`Mu_engine/coral.crlb` does **not** build bgfx. The `link` block points
straight at the prebuilt static archives staged under
`Mu_engine/packaged/<platform>/lib/` (`libbgfx.a`, `libbimg.a`, `libbx.a`),
so `coralc build` never invokes CMake for bgfx. The source tree here is only
needed when (re)generating those archives; CI does that in
`.github/workflows/build-bgfx.yml`.

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
