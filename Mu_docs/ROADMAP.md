# Mu Engine Roadmap

Persistent snapshot of the active engineering todo. Session todos are ephemeral;
this file is the source of truth for order and scope.

Last updated: 2026-11-16

## Ground Rules

- 3D is the primary target; 2D is a bonus shipped last.
- bgfx is the committed renderer. SDL3 is window/input/audio only.
- Scripting is intentionally shifted back, after assets.
- Linux is the current build host; Windows, macOS, and Web are first-class goals.
- M0/M1 are complete. A visible window currently exists through the temporary
  SDL3 presenter; the bgfx presentation path still needs to be fixed.

## Roadmap

### M3 - Window and bgfx

- [ ] **IN PROGRESS** M3 renderer: fix bgfx swapchain/presentation so the window maps and animates every frame
- [ ] M3 renderer: bgfx platform-data matrix for Wayland/X11, Win32, Cocoa, Android, UIKit; renderer type + fallback selection
- [ ] Build: per-platform coral.crlb targets, link blocks, packaged archive selection; remove Linux-only hardcoded paths
- [ ] Core loop: fixed-timestep accumulator, dt clamp, vsync/frame pacing, deterministic headless stepping
- [ ] Core diagnostics: bgfx debug callback, SDL error reporting, startup/frame telemetry

### Math and scene

- [ ] Math: complete Vec2/Vec3/Vec4/Quat (axis-angle, slerp, matrix conversion, inverse)
- [ ] Math: Mat4, perspective, ortho, lookAt, TRS, normal matrix, frustum planes
- [ ] Scene: engine component types (Camera, MeshRenderer, Light, RigidBody, Collider, Character) registered in ComponentRegistry
- [ ] Render extraction: scene entities resolve to render handles/draw list; ECS stays renderer-agnostic

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

Fix bgfx presentation on Wayland:

1. Confirm the packaged `libbgfx.a` matches the C99 header layouts in
   `src/thirdparty/bgfx/types.crl`.
2. Verify `bgfx_init_t.swapChain` / `platformData` values reach the Vulkan
   Wayland surface path.
3. Confirm `wl_surface.attach` and `wl_surface.commit` appear under
   `WAYLAND_DEBUG=1` and that the Hyprland window maps.
4. Keep the SDL3 presenter as a temporary fallback until bgfx presents.
