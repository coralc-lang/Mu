# Mu Engine Roadmap

Persistent snapshot of the active engineering todo. Session todos are ephemeral;
this file is the source of truth for order and scope.

Last updated: 2026-11-16

## Ground Rules

- 3D is the primary target; 2D is a bonus shipped last.
- bgfx is the committed renderer. SDL3 is window/input/audio only.
- Scripting is intentionally shifted back, after assets.
- Linux is the current build host; Windows, macOS, and Web are first-class goals.
- M0/M1 are complete. bgfx presents on Wayland and X11; the SDL3 presenter
  remains available as a fallback.

## Roadmap

### M3 - Window and bgfx

- [x] M3 renderer: fix bgfx swapchain/presentation so the window maps and animates every frame
- [x] M3 renderer: Linux platform-data matrix (Wayland/X11), renderer type + fallback selection
  - Done: bgfx is the default renderer; `MU_SDL_RENDERER` forces SDL fallback.
  - Done: `MU_BGFX_RENDERER` selects auto/noop/vulkan/opengl/gles/d3d11/d3d12/metal/webgpu;
    `MU_BGFX_FALLBACK=0|false|off` makes the request strict.
  - Done: SDL backend name (`wayland`/`x11`/`win32`/`cocoa`/`android`/`uikit`) is logged with
    the bgfx native type; all six SDL native-handle properties are resolved.
  - Verified: Wayland presents (`wl_surface.attach`/`commit`) and maps; X11 maps.
  - Verified Linux: strict Vulkan initializes on Wayland and X11; a strict D3D11 request fails
    over to the SDL presenter; unknown renderer names fall back to auto.
  - Verified Linux pacing: X11 bgfx held ~60 fps; Wayland bgfx and SDL both showed ~30 fps
    during an unfocused run, pointing at compositor/window-focus pacing rather than the backend.
  - Done: resize/reset handling via `Renderer.resize`, platform size polling, and
    `bgfx_reset` with an updated swap chain.
- [ ] M3 renderer: verify platform-data matrix on Win32, Cocoa, Android, UIKit
- [ ] Build: per-platform coral.crlb targets, link blocks, packaged archive selection; remove Linux-only hardcoded paths
- [x] Core loop: fixed-timestep accumulator, dt clamp, vsync/frame pacing, deterministic headless stepping
  - Done: accumulator in `Loop.frame`, 0.25 s frame-time clamp, and `Loop.run` stepping exact `FIXED_DT` frames.
  - Verified: bgfx Vulkan and `MU_SDL_RENDERER=1` both report ~60 fps on Wayland;
    X11 can still drop to ~30 fps intermittently.
- [x] Core loop: remove the `_nullPhysicsUpdate` placeholder and require a real physics slot; `Loop::new` should not silently run with no simulation
  - Done: `Loop.new` now takes a physics context and update callback; `engine/main` passes `physicsSystem` directly.
  - Done: removed the physics probe path from `engine/main`.
- [x] Core diagnostics: bgfx debug callback, SDL error reporting, startup/frame telemetry
  - Done: bgfx C callback wired for fatal/trace; profiler/cache/capture slots are stubs.
  - Done: SDL window-size and renderer-vsync failures now report `SDL_GetError`.
  - Done: frame stats include sim ticks, average ms, and worst ms; run summary includes ticks.
- [ ] Renderer: diagnose intermittent ~30 fps presentation on Wayland/X11; capture focus/visibility and present timing

### Math and scene

- [x] Math: complete Vec2/Vec3/Vec4/Quat (axis-angle, slerp, matrix conversion, inverse)
  - Done: vector neg/xyz helpers; Quat axis-angle, inverse, normalize, dot, slerp,
    Mat3 conversion, and Mat3 inverse/transpose.
  - Verified with a temporary probe: rotation, inverse, shortest-path slerp,
    axis-angle round trip, and matrix round trip.
- [x] Math: Mat4, perspective, ortho, lookAt, TRS, normal matrix, frustum planes
  - Done: column-major Mat4 with mul/transform/project/transpose, perspective and
    perspectiveGL, ortho and orthoGL, lookAt, trs, normalMatrix, and Frustum planes.
- [x] Scene: engine component types (Camera, MeshRenderer, Light, RigidBody, Collider, Character) registered in ComponentRegistry
  - Done: `scene/components.crl` defines the six component types, per-type stores,
    type constants, and `componentsInit` registration.
- [x] Render extraction: scene entities resolve to render handles/draw list; ECS stays renderer-agnostic
  - Done: `render/extract.crl` turns live MeshRenderer entities into typed mesh/material
    handles plus world transforms in a caller-owned DrawList.
  - Done: the loop rebuilds the draw list before every present and passes it through
    `Renderer.draw`; scene and component code remain renderer-agnostic.
  - Verified with a temporary probe: live/dead filtering, missing-component filtering,
    handle and transform extraction, and draw-list capacity clamping.

### Renderer and materials

- [ ] Render resources: mesh/material/texture/shader handles with explicit ownership and release
- [ ] Render pipeline: vertex layouts, vertex/index buffers, transient buffers
- [ ] Render pipeline: shader/program path (GLSL -> bgfx shaderc, embedded or cooked shaders)
- [ ] Materials: base color, metallic/roughness, normal maps, alpha modes, culling, depth state
- [ ] Camera pass: view/projection uniforms, clear, depth, multiple views, resize handling
- [ ] Primitive meshes: cube, sphere, plane, capsule; mesh component wiring
- [ ] Lighting: directional + point lights, normals/tangents, physically based shading baseline
- [ ] Debug draw: lines, grid, axes, collider wireframes, physics/query visualizers

### First playable loop

- [ ] 3D demo: lit cube on the Jolt ground with a movable camera; validate physics<->render sync
- [ ] Input: keyboard/mouse/gamepad state, just-pressed/released, wheel, focus loss, mouse capture
- [ ] Input: named rebindable action map, runtime rebinding, synthetic events for headless tests
- [ ] Gameplay: camera controllers (fly, orbit, first-person, third-person) and editor camera
- [ ] Gameplay: Jolt character controller movement, jump, stairs/slopes, moving platforms
- [ ] Physics: collision events, triggers/sensors, contact callbacks, layer/query polish, joints, ragdolls

### Scene data

- [ ] Scene format: text diffable scene/prefab grammar, versioned header, runtime loader
- [ ] Scene identity: entity id recycling with generations, dangling-handle safety
- [ ] Scene storage: revisit archetypes vs per-type arrays at first perf need

### Assets (M5)

- [ ] Assets: handle table, aliases, GUID sidecars, dependency graph, refcounted unload
- [ ] Assets: loaders for stb images, cgltf glTF, tinyexr HDR, drlibs/stb_vorbis audio, stb_truetype fonts, FastNoiseLite, MikkTSpace
- [ ] Assets: async/background loading, hot-reload watcher, import metadata
- [ ] Assets: cooked packs with miniz/zstd, container format, shipping build path

### Debug and profiling

- [ ] Debug: CVars, debug console, on-screen stats/log overlay
- [ ] Debug: profiler instrumentation API, CPU/GPU zones, frame marks, plots

### Scripting (M2, shifted back)

- [ ] Script: muc WallVM IR ingestion and execution; main.mu runs in the loop
- [ ] Script: lifecycle hooks onLoad/onUpdate/onEvent/onUnload and per-entity instances via ScriptBindings
- [ ] Script: expand Mu:: natives across log, timing, scene, render, physics, input, audio
- [ ] Script: @component auto-registration and component get/set natives
- [ ] Script: signals/events registry with queued deterministic dispatch
- [ ] Script: game allocator integration, @drop handling, script hot reload

### Renderer depth

- [ ] Render: directional shadow maps with PCF, then cascaded shadows
- [ ] Render: skybox, fog, HDR, tonemap, bloom, FXAA/MSAA, post-processing stack
- [ ] Render: frustum culling, instancing/batching, LOD, occlusion culling
- [ ] Render: forward+/clustered lighting, decals, lightmaps/GI
- [ ] Render: shader/material graph

### Animation, navigation, jobs, networking

- [ ] Animation: skeletal meshes, state machines, blend trees, IK, retargeting
- [ ] Navigation: navmesh generation, path queries, steering/agents
- [ ] Jobs: task system, parallel system scheduling, async asset work
- [ ] Networking: if multiplayer becomes a goal, replication/prediction/RPC layer

### Audio (M6)

- [ ] Audio: SDL3 streams, stb_vorbis/drlibs decode, master/music/sfx/ui bus mixer, voice handles
- [ ] Audio: 3D spatialization and bus effects

### UI (M6)

- [ ] UI: immediate-mode core, layout/containers/widgets, theme tokens, focus states
- [ ] UI: SDF fonts, nine-patch, localization, accessibility, game HUD/menus

### Editor (M7+)

- [ ] Editor: plan, scene viewport, hierarchy, inspector, asset browser, console, profiler, gizmos
- [ ] Editor: in-engine Mu script editor (syntax highlighting, diagnostics, live reload), integrated terminal
- [ ] Editor: play mode, prefab editing, export/cook workflow

### Platforms and testing

- [ ] Platform: Web/Emscripten main-callback loop, threads on/off split, GLES3 feature floor
- [ ] Platform: Windows/macOS/Android build, packaging, CI matrix
- [ ] Testing: deterministic headless CI runs, physics/query probes, replay/frame-step debugging

### 2D bonus

- [ ] 2D bonus: sprites, tilemaps, 2D lights/particles, 2D physics path
- [ ] 2D bonus: 2D tooling/import and UI integration

### Shipping

- [ ] Shipping: export/cook pipeline, per-platform packaging, game project template

## Immediate Next Step

bgfx presentation root cause was a struct-layout mismatch: the packaged
`libbgfx.a` uses an older `bgfx_init_t` layout than upstream master.
`platformData` is at offset 24 and `swapChain` at 48. The previous binding put
them at 40/64, so `nwh` landed in `width`, bgfx saw a null window handle, and
created a headless Vulkan device.

Next:

1. Render resources: mesh/material/texture/shader handles with explicit ownership.
2. Keep `MU_SDL_RENDERER=1` as the temporary SDL presenter fallback.
3. Diagnose intermittent ~30 fps presentation on Wayland/X11; capture
   focus/visibility and present timing.
