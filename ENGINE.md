# Mu Engine — Design Notes

**Mu Engine** — a game engine written in **Coral**, with **Mu** as its
scripting language (Mu scripts : engine = GDScript : Godot). Lives in this
repo alongside the Mu compiler (`Mu_coral/`), the language spec (`DESIGN.md`),
and the copied Coral std (`std/`). Everything below is **Decided** unless
listed under Open items.

---

## 1. Goals — **Decided**

- One repo = language + engine. The Mu compiler (`muc`) is the toolchain; the
  engine is its first and primary client. **`muc` emits WallVM IR**
  (see `IR.md`); the script host consumes it.
- Engine core in Coral (compiled natively), gameplay in Mu (compiled by
  `muc`).
- **Multiplatform, everywhere:** Linux, Windows, macOS, **and Web** — all
  four are first-class targets, not afterthoughts. Every subsystem chosen
  below is judged against that bar.
- **3D-first:** meshes, cameras, lighting, 3D physics are in scope. The
  engine is for small/mid-scale 3D games (indie scale complexity, honest
  scope).
- **Headless first.** M0–M2 run with no window; the renderer is a stub
  interface from day one so real backends slot in later without touching
  gameplay or scene code.
- Scaffold, not perfection: every subsystem starts as a real interface with a
  minimal implementation, growing milestone by milestone.

## 2. Technology choices — **Decided**

| Concern | Choice | Why |
|---|---|---|
| Windowing / input / gamepad | **SDL3** | Only option with first-party support for all four targets (Linux X11+Wayland, Windows, macOS, Web/Emscripten), plus dialogs/clipboard; GLFW has no official web platform, sokol is X11-only and single-window |
| Audio | **SDL3 audio** | Comes free with the windowing choice; works on all four targets incl. Web |
| Rendering | **GL 3.3 desktop / GLES3 (Web)** | Broad support incl. WebGL2 on the web; behind the renderer interface so a Vulkan backend can come later |
| Script output | **WallVM IR** (`muc` → host) | The compiler's decided backend |
| Assets | **Handle-based + hot reload** | Stable id/alias per asset; source formats in dev with hot reload, cooked packs for shipping |
| UI style | **Custom immediate-mode UI** | One paradigm for game HUD/menus and the future editor, drawn by the renderer (Dear ImGui-style) |

## 3. Architecture layers — **Decided**

```
scripts/            Mu gameplay scripts (*.mu)
Mu_engine/
  script/           script host: loads WallVM IR, exposes natives, drives lifecycleA
  scene/            entities, components, transform hierarchy
  render/           renderer interface (stub → GL backend at M3+)
  input/            input abstraction over SDL3 events
  audio/            audio over SDL3
  ui/               immediate-mode UI (game UI + future editor)
  assets/           handle table, aliases, loaders, hot-reload watcher
  core/             game loop, time, logging, math glue
  platform/         window/event loop over SDL3 (headless mode until M3)
```

- Upper layers only call lower ones. Nothing below `script/` knows Mu exists
  except the script host itself.
- The renderer/input/audio/ui are **interfaces with a null implementation**
  until their milestone — the engine runs headless, but call sites are
  already written against the final API.
- **Error convention:** failure sites return std `Option`/`Result` — never
  bool or status codes. `Option` when absence needs no message (lookup miss,
  full storage, decode failure with no vendor reason); `Result<T, E>` when
  the caller may report why — `E = str` for engine-written literals,
  `E = String` when the message is copied out of a vendor buffer before it
  is freed. Predicates (`has`, `alive`) stay `bool`; idempotent no-ops stay
  `void`. Vendor ports expose raw externs; engine-facing wrappers
  (`stb::loadImage`, `tinyexr::loadEXR`, `drlibs::loadWavF32`, ...) do the
  wrapping.

## 4. Scene model — **Decided**

- `Scene` owns `Entity` records (stable `u32` id, alive flag) and per-type
  component storage (struct arrays keyed by component type, appended by id).
- Transform hierarchy: parented entities (3D transform + children).
- No inheritance anywhere — matches Mu's composition-only rule (DESIGN §2).
- **Scaffold (2026-10-09):**
  - `ComponentRegistry` (in `scene/scene.crl`) — the component **type
    table**: `add(name, store)` once per type (duplicate/full → `Err`),
    then `storeFor(name)` / `has(entity, name)` resolve by name. The M0
    demo registers `demo`; @component auto-registration (DESIGN §16) and
    the M2 component get/set natives (§11) feed/dispatch through this
    same table — one add line per component type.
  - `ScriptBindings` (in `script/script.crl`) — entity → script path:
    composition-only attachment, multiple scripts per entity,
    `bind` rejects dead entities, `unbind` / `countFor`. M2 instantiates
    one script instance per binding (§5).
  - Still open (§17): id recycling/generations, archetypes — per-type
    arrays stay until a perf need forces a revisit.
- Systems are plain Coral functions run in a fixed order by the loop:
  `sceneUpdate(scene, dt)`, later `physicsStep`, `scriptUpdate`, …
- Mu scripts receive entities by id and access components through engine
  natives — scripts never see Coral memory directly.

## 5. Mu script integration — **Decided**

- Scripts are `.mu` files under `Mu_engine/scripts/` (demo: `main.mu`).
- The engine exposes its API as a **Mu library**: scripts pull symbols
  with `use Mu::<module>::<fn>;` (DESIGN §2 — e.g.
  `use Mu::timing::frame;`), and the **native registry** is keyed by those
  full library paths (`Mu::...` → function pointer). The script host
  resolves each `use`d symbol at load time and errors with the path if it
  is missing. **`@extern` is reserved for genuinely foreign functions** —
  C/C++ the engine does not provide (DESIGN §14) — engine API never goes
  through `@extern`.
- **Scripts bind to entities:** `ScriptBindings` pairs an entity id with a
  script path (§4 scaffold), alive-checked at `bind` time. M2 runs one
  script instance per binding with the entity as its context.
- Script lifecycle hooks the engine calls each frame:
  `fn onLoad()`, `fn onUpdate(dt: f32)`, `fn onEvent(...)`, `fn onUnload()`.
  Any of these may be absent.
- Scripts are compiled by `muc` ahead of time to **WallVM IR** (asset-pipeline
  step); the script host runs it. The host API is stubbed behind
  `ScriptHost.load()` until M2.
- Value boundary: only Mu's engine-known types cross it — `EntityId (u32)`,
  scalars, `Vec2/Vec3` (DESIGN §3 built-ins), `str`. No Coral structs leak
  into Mu.
- Memory: script code never allocates or frees by hand — the compiler
  injects alloc/free (DESIGN §6, engine side §10); all Mu heap traffic
  routes through the engine game allocator.

## 6. Main loop — **Decided**

```
while running:
    dt = clock.tick()          # fixed max, clamped
    platform.pump()            # SDL3 events → input (no-op headless)
    scene.update(dt)
    script.update(dt)
    ui.frame()                 # immediate-mode build pass
    render.draw(scene, ui)     # null renderer: no-op until M3
    frame++
```

- Fixed logical timestep (1/60) with the loop capped for tests (e.g. 600
  frames in CI), variable dt allowed later.
- Loop is drivable headless and deterministically (no wall-clock dependence
  in tests). On Web, the same loop is driven by SDL's main-callbacks /
  requestAnimationFrame — structure the loop body as a callable `frame()`
  from day one.

## 7. Directory layout — **Decided**

```
Mu/
  DESIGN.md  IR.md  IMPORTANT.md  grammar.md   # language docs
  ENGINE.md                                     # this file
  Mu_coral/          # muc sources (Coral)
  Mu_examples/       # language examples
  std/               # copied Coral std
  Mu_engine/
    coral.crlb       # build file (modules, tasks, deps, phases)
    src/
      engine/        # entry module: lib.crl + main.crl (the `main()` entry)
      math/          # Vec3/Quat/RVec3 — methods, chainable
      scene/         # entities, transform hierarchy, ComponentStore
      render/        # Renderer fn-ptr interface (+ null backend)
      physics/       # JoltC v5.6.0 extern port
      sdl/           # SDL 3.4.18 extern port (window/input/audio, ready before M3)
      gl/            # GL 3.3 core/GLES3 extern port (backend API, ready before M3/M4)
      stb/           # stb_image/write/truetype port (assets, SDF fonts)
      cgltf/         # cgltf port (glTF 2.0 mesh import)
      miniz/         # miniz port (deflate/zip for cooked packs)
      tinyexr/       # EXR decode/encode port (HDR, lightmaps)
      drlibs/        # WAV/MP3/FLAC decode port (dr_libs)
      fastnoise/     # FastNoiseLite port (procedural noise)
      mikktspace/    # MikkTSpace port (tangent generation)
      alloc/         # mimalloc game allocator (std::allocator surface)
      core/          # logging, FIXED_DT, the Loop
      platform/      # headless pump/quit (SDL3 lands at M3)
      script/        # script host: ScriptHost + native registry (M1); IR execution M2
    native/          # CMakeLists, jolt_smoke.c, mu_{stb,cgltf,drlibs,fnl}.c impl TUs
    third_party/     # jolt (5.6.0), JoltPhysicsC, SDL (3.4.18), stb, cgltf, miniz,
                     # tinyexr, dr_libs, FastNoiseLite, MikkTSpace — trimmed, .git-stripped
    scripts/         # Mu scripts (main.mu first)
```

- One Coral module per layer directory (`math`, `scene`, ...), each with a
  `lib.crl` surface; cross-layer imports use the Mu_coral form
  `import <module>::<file> { ... }` (e.g. `import scene::scene { Scene }`).
  The entry module is `engine` (coral.crlb `entry = "engine"`), holding
  `main()`.

## 8. Milestones — **Decided**

- **M0 — Scaffold:** build file, module skeleton, all layer stubs, headless
  main loop running N frames, scene with entities + a simple system.
- **M1 — Script host API:** `ScriptHost` with load/update/free stubs, native
  registry with a handful of natives (`log`, `frame`, spawn/despawn), demo
  `main.mu` calling into it (compilation of `main.mu` still external).
- **M2 — muc ↔ host:** `muc`'s WallVM IR output consumed and executed by the
  script host; `main.mu`'s `onUpdate` actually runs inside the loop.
- **M3 — Window + render:** SDL3 window + event pump, GL 3.3/GLES3 backend
  behind the renderer interface, input events live.
- **M4 —** 3D scene basics: camera, meshes, materials, lighting.
- **M5 —** assets: handle table, aliases, loaders, hot reload.
- **M6 —** audio (SDL3) + immediate-mode UI.
- **M7+ —** physics, editor (each gets its own plan section first).

## 9. Features — **Decided**

Core features adopted now (cheap to design early, painful to retrofit):

- **Signals/events** — decoupled engine↔script and script↔script callbacks
  (Godot-style); scripts connect by name, no direct references.
- **Prefab/instance system** — reusable entity templates with per-instance
  overrides (Godot scenes / Unity prefab variants).
- **Scene file format** — text, human-readable, diffable serialization of
  scenes + prefabs; designed before the editor exists.
- **`@component` binding** — Mu structs marked `@component` (DESIGN §16)
  auto-register with the scene; the script side of the scene model.
- **Input action mapping** — gameplay binds to named, rebindable actions,
  never raw keys (Godot InputMap).
- **Audio buses** — per-bus volume/effects mixer from day one; the mixer
  architecture is decided now even though audio code lands at M6.
- **Asset import metadata (GUID sidecars)** — every source asset gets a
  stable id; renames/moves never break references. This is the foundation
  the handle/alias system (M5) sits on, so the format is decided early.
- **Debug console + overlay** — on-screen stats, log, and **CVars**
  (Valve/Unreal style) tunable at runtime; headless mode prints them.
- **Profiler** — CPU frame zones + GPU timers behind a single instrumentation
  API, always compiled, costs ~nothing when unused.
- **Mu script editor, in-engine** — the editor embeds a script editor for
  `.mu` files like Godot's: **syntax highlighting**, code folding, auto-indent,
  error underlines from `muc` diagnostics, and live reload. The highlighter
  reuses the Mu lexer (`Mu_coral` mu-lexer) so editor highlighting can never
  drift from the compiler's view of the language.
- **Integrated terminal** — an editor terminal (split-pane capable) for
  builds, `muc`, git, and any shell command; Mu/`muc`-aware shortcuts.
- **UI ambition: one of the most beautiful UIs.** Not an afterthought — the
  immediate-mode UI layer is a design target in its own right: strong
  typography (SDF fonts), coherent theme tokens (spacing/radius/palette),
  subtle motion/easing, focus states, dark-first design, and editor chrome
  that looks considered. UI quality is a milestone gate, not a polish item.
- **Physics engine: Jolt 5.6.0 + JoltC.** Jolt is pinned at **v5.6.0**;
  its C API is **EvergineTeam/JoltPhysicsC** (targets v5.6.0, MIT, 259-test
  suite), ported 1:1 to Coral as `Mu_engine/src/physics/physics.crl`
  (`extern` decls, unmangled, ABI-checked typedefs: ObjectLayer = u16,
  BroadPhaseLayer = u8, BodyID = u32). Native side builds static
  `libJoltC.a` + `libJolt.a` from `native/CMakeLists.txt`. Character
  controller, collision layers, queries, ragdolls keep their own plan (M7).
- **Game allocator: mimalloc (Coral std).** Not a top-level std file — it
  lives recursively at `std/x86_64/{linux,windows}/memory/allocators/
  mimalloc.crl`, exported through `std/lib.crl` as `std::allocator`
  (submodules `mimalloc`, `allocator`, `tlsf`); the platform manifests pick
  the copy. The engine wraps it in `src/alloc` (`gameAllocator()` →
  `std::allocator { mimalloc::MiAlloc, allocator::Allocator }`). Jolt keeps
  its default malloc path for now (hookable later if profiling says so).
- **Asset codecs: vendored now, ports ready.**
  `stb_image`/`stb_image_write` (PNG, JPEG, BMP, TGA, GIF, HDR, ... — the
  whole common-image decode/encode path, so no separate libpng/libjpeg),
  `stb_image_resize2` (scaling/mips), `stb_truetype` (SDF font baking for
  the UI), `stb_vorbis` (Ogg Vorbis decode for audio — SDL3 outputs, stb
  decodes), `tinyexr` (OpenEXR decode/encode for HDR/lightmaps — shares
  our single miniz), `dr_libs` (dr_wav/dr_mp3/dr_flac — WAV/MP3/FLAC
  whole-file decode), `cgltf` (glTF 2.0 — the mesh/scene import format),
  `miniz` (deflate + zip for cooked packs), `MikkTSpace` (reference
  tangent generator for normal-mapped glTF), `FastNoiseLite` (procedural
  noise). Structured data — JSON/TOML/CSV/INI/XML/UUID — comes from std
  (`std::parser::*`), no vendored parser. All repos are vendored under
  `third_party/` (trimmed, .git-stripped), bound in
  `src/{stb,cgltf,miniz,tinyexr,drlibs,fastnoise,mikktspace}`, and their
  implementations compile through `native/mu_*.c`, tinyexr.cc,
  stb_vorbis.c, miniz's own sources and mikktspace.c into
  `build/native/libmualibs.a`.

## 10. Memory model (engine side) — **Decided**

- Mu is AOT (muc → WallVM IR). Memory decisions are made **at compile
  time**: the compiler's smart analysis injects alloc/free where needed —
  scope-based free + escape analysis (DESIGN §6). **No GC, no reference
  counting, no manual alloc/free in script code.**
- Every Mu heap allocation (escaping objects, arrays, strings) goes
  through **the engine game allocator** — mimalloc behind std's `Allocator`
  (`src/alloc`, §9). The script host hands `gameAllocator()` to the
  generated code at load time; scripts never see or choose an allocator.
- Engine-side Coral follows the same rule as layers land: static
  pre-allocated storage where the shape is fixed (M0 scene storage),
  `gameAllocator()` everywhere else.
- Jolt stays on its default malloc path for now (§9) — profiling decides
  later.
- Consequence for CI: allocation is deterministic and bounded — capped
  headless runs (§6) never depend on GC pauses or runtime RC decisions.

## 11. Script host & native registry (M1/M2) — **Scaffolded (M1) / Planned (M2)**

- **Scaffold (2026-10-09, `src/script/`):**
  - `NativeRegistry` — name → function-pointer table; values are
    signature-tagged (`variant Native`, **one arm per signature**, not per
    native); caller-provided static storage like Scene (§10); `resolve`
    for lookups, `lookup` for the M2 dispatch path (pointer into the table).
  - `ScriptHost` — `load` / `update` / `free` stubs (M2 replaces bodies
    with IR execution) plus `require(name)`, which resolves a script's
    @extern list up front and fails with the missing name (DESIGN §14).
  - Engine-library natives (Mu:: paths, DESIGN §2 `use`):
    `Mu::log::info`, `Mu::log::error`, `Mu::timing::frame`,
    `Mu::scene::spawn`, `Mu::scene::despawn`, `Mu::scene::setParent`,
    `Mu::platform::quit` (the cross-layer request `platform/` always
    anticipated). Context (scene, frame counter) is bound host-side —
    plain fn pointers, no closures cross the boundary (§5); `@extern`
    is not involved (foreign-only). Entity→script binding lives here too
    (`ScriptBindings`, §4/§5).
  - `scriptSystem` — ticked by `Loop` through a fn-ptr slot in `frame()`
    at the §6 position; `engine/main` wires ctx+fn, so core never imports
    script (same pattern as `Renderer.draw`).
  - Demo: `Mu_engine/scripts/main.mu` (written, compiled by muc at M2);
    `engine/main` exercises require/resolve/load, the negative
    missing-native probe, and reports `script updates: 600` from the run.
- **Growth path (incremental by construction):** a native that fits an
  existing signature = one static fn + one `natives->add(...)` line; a new
  boundary signature = one new `Native` arm + the add line. Registry,
  host, and loop call sites never change — lifecycle hook bodies (§5),
  `@component` get/set natives (below), and further natives slot in behind
  the same API. Boundary types only (§5): `EntityId (u32)`, scalars,
  `Vec2/Vec3`, `str`.

- `ScriptHost` with `load()`, `update(dt)`, `free()` — stubs at M1, real
  execution at M2 (§8).
- **Native registry:** library path → function-pointer table owned by the
  host. Scripts `use Mu::...;` (DESIGN §2); at load the host resolves each
  used symbol against the table and fails with the path. `@extern fn ...;`
  stays for foreign C functions only (DESIGN §14).
- Initial natives (the `Mu::` library): `Mu::log::info`, `Mu::log::error`,
  `Mu::timing::frame`, `Mu::scene::spawn` / `Mu::scene::despawn`,
  `Mu::scene::setParent`, `Mu::platform::quit`, plus component get/set
  for `@component` types (§9).
- Boundary types only (§5): `EntityId (u32)`, scalars, `Vec2/Vec3`, `str`.
  No Coral structs, no pointers cross.
- Lifecycle hooks driven by the loop (§6): `onLoad`, `onUpdate(dt)`,
  `onEvent(...)`, `onUnload` — any may be absent.
- M2: muc's WallVM IR output is consumed and executed; `main.mu`'s
  `onUpdate` runs inside the loop. `IR.md` is the contract.

## 12. Signals — **Planned**

- One engine-owned registry: signal name → subscriber list (engine
  systems + scripts addressed by id).
- `connect(name, target)` / `disconnect(name, target)` **by name only**
  (§9) — no direct references, nothing to dangle (Godot-style).
- Payload restricted to boundary types (§5): `EntityId`, scalars, `str`.
- Deterministic order (§6): registration order; emission is queued and
  dispatched in the script phase — a signal never re-enters the frame
  mid-dispatch.
- Names are interned strings; unknown-signal checks happen at connect
  time, not per emit.

## 13. Input (M3) — **Planned**

- SDL3 events (`src/sdl`) → input layer state: down / up / just-pressed /
  just-released per key and button, mouse position/delta/wheel, gamepad
  axes and buttons via `SDL_EVENT_GAMEPAD_*`.
- Just-pressed/released bits are frame-local: cleared when the event pump
  runs, read by `script.update` — matching §6 order.
- **Action mapping (§9):** gameplay binds to named rebindable actions
  (`jump`, `fire`), never raw keys; the action table is data, rebindable
  at runtime (debug console, §9).
- Headless: no events, all action state idle; tests inject synthetic
  events straight into the input layer (deterministic, §6).

## 14. Audio (M6) — **Planned**

- Output: SDL3 audio streams (`src/sdl` — `SDL_OpenAudioDeviceStream`,
  `SDL_PutAudioStreamData`).
- Decode: `stb_vorbis` for Ogg plus `dr_libs` — dr_wav / dr_mp3 / dr_flac
  — for WAV/MP3/FLAC (`src/drlibs`: Option-returning whole-file loaders,
  §9 codecs).
- **Bus mixer decided now (§9):** engine-owned mix graph — `master`,
  `music`, `sfx`, `ui`, plus user buses, each with volume/mute; voices
  submit PCM, the mixer applies bus levels into the device stream. The
  architecture lands before any sounds do.
- Voice model: one-shot (auto-freed) + persistent handles (loops, music);
  handles are `u32` ids like entities — scripts never see SDL types.
- 3D spatialization stays in the feature backlog (§16).

## 15. Scene & prefab file format — **Planned**

- Text, human-readable, diffable (§9). One file = one scene or one
  prefab — same grammar, versioned header.
- Entity blocks reference parents by name; components are typed key/value
  blocks keyed by `@component` name (§9). Files never contain array
  indices — runtime ids are assigned on load.
- GUID sidecars (§9) sit next to source assets; renames/moves update the
  sidecar, never the references.
- Prefab instance = prefab reference + per-instance overrides, written
  with the same fields the scene writer emits; instantiation = spawn tree,
  then apply overrides.
- Loader lands with assets (M5), writer with the editor (M7+) — the
  format is designed now so runtime and editor share one grammar.

## 16. Feature backlog — **stubs (decide when milestone approaches)**

Deferred features, kept here so they are not forgotten:

- **Shader/material graph** — visual node editor producing GLSL; before any
  material tooling (M4+ materials start with plain GLSL files).
- **Navmesh + pathfinding agents** — generation, queries, steering; own plan
  before implementation.
- **Job/task system** — parallel system scheduling + async loading; revisit
  when a system actually needs it (probably post-M6).
- **Animation system** — skeletal meshes, state machines, blend trees, IK,
  retargeting; needed for real 3D characters (post-M4).
- **Networking** — replication, prediction, or RPC-style high-level layer;
  only if multiplayer is a goal.
- **Export/inspector annotations** — expose script fields to the editor
  inspector (Godot `@export`); pairs with the editor milestone.
- **Rendering extras** — forward+ clustering, shadow cascades, occlusion
  culling, LOD, decals, lightmaps/GI, post stack (bloom/tonemap/SSAO),
  instancing/batching, skybox.
- **Physics** — character controller, collision layers, triggers,
  ray/shape queries, ragdolls (own plan; M7+; engine decided in §9).
- **Background asset loading** — dependency graph, refcounted unload,
  bundles/addressables for shipping.
- **UI toolkit depth** — anchors/containers, theming, nine-patch, SDF text.
- **Loc. & save data** — string tables, font fallback; save-game API.
- **Replay/frame-step debugging** — deterministic record & re-simulate
  (our deterministic loop makes this viable).

## 17. Open items

- **Component storage details:** archetypes vs per-type arrays (start with
  per-type arrays; revisit at first perf need).
- **Entity id policy:** id recycling, generations (dangling-id safety).
- **Cooked asset format:** the pack/container layout for shipping builds.
- **Editor:** not scaffolded until a plan section exists (M7+).
- **Web constraints:** threads-on/off build split, GLES3 feature floor —
  revisit at M3.
