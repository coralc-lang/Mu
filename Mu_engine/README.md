# Mu Engine

**Mu Engine** — a 3D-first, multiplatform game engine written in **Coral**,
with **Mu** as its scripting language
(Mu scripts : engine = GDScript : Godot). One repo holds the language
(`Mu_coral/`), the specs (`DESIGN.md`, `grammar.md`, `IR.md`), and this
engine.

- **[`../ENGINE.md`](../ENGINE.md)** — the authoritative design document:
  every architecture, feature, and milestone section is marked *Decided*
  unless listed under Open items. Read that first; this README is the
  **state + usage** view.
- **Principles:** headless first (M0–M2 run with no window), scaffold not
  perfection (every subsystem starts as a real interface with a minimal
  implementation), error convention: `Option`/`Result` at failure sites,
  predicates stay `bool`, idempotent no-ops stay `void`.

---

## Status

| Milestone | What | State |
|---|---|---|
| **M0** | Build file, modules, headless loop, scene + system | ✅ done |
| **M1** | Script host API: native registry (`Mu::` library), entity→script bindings, ECS type table | 🟡 scaffold written — pending build verification |
| **M2** | `muc` ↔ host: WallVM IR executed, `main.mu` runs in the loop | ⬜ planned |
| **M3** | SDL3 window + GL 3.3/GLES3 backend, live input | ⬜ planned |
| **M4** | 3D scene basics: camera, meshes, materials, lighting | ⬜ planned |
| **M5** | Assets: handle table, aliases, loaders, hot reload | ⬜ planned |
| **M6** | Audio (SDL3) + immediate-mode UI | ⬜ planned |
| **M7+** | Physics features, editor | ⬜ planned |

---

## Build & run

Prerequisites: `coralc` (this repo's compiler) on `PATH`, `clang`,
`cmake`, a C++ toolchain (Jolt).

```sh
cd Mu_engine

# 1. vendor static libs (once, or when third_party changes)
cmake -S native -B build/native -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build build/native -j2        # -> libmualibs.a + libJolt{C,}.a

# 2. the engine
coralc build                          # -> build/muengine, regenerates .coral/modules.map

# 3. the demo: 600-frame headless run (CI cap, ENGINE.md §6)
./build/muengine
```

- `coral.crlb` also defines a `run` task (`after = ["build"]`,
  `exec = "build/muengine"`) for driving it through the build file.
- Expected demo output: scene summary line + `script: updates=600
  bindings=1 ... natives: 7 component types: 1`, plus `Mu::` library
  probes and the (intentional) negative missing-symbol message.
- **Prebuilt archives** for shipping/cross targets live in
  `packaged/<platform>/lib/` — inventory and rebuild notes:
  [`../Mu_docs/prebuilt-libs.md`](../Mu_docs/prebuilt-libs.md).

---

## Repository layout

```
Mu_engine/
  coral.crlb          build file: modules, link libs, native phase, tasks
  src/                one Coral module per layer (each: lib.crl + file.crl)
  scripts/            Mu gameplay scripts (main.mu first)
  native/             CMake project + vendor implementation TUs (mu_*.c)
  third_party/        jolt, JoltPhysicsC, SDL, stb, cgltf, miniz, tinyexr,
                      dr_libs, FastNoiseLite, MikkTSpace (trimmed vendors)
  packaged/           prebuilt archives per platform (wgpu, tcc, Jolt, mualibs)
  build/              outputs (git-ignored): muengine, native vendor libs
```

### Layers

| Layer | Dir | Status |
|---|---|---|
| entry / main loop demo | `src/engine/` | live (M0) |
| core — `Loop`, logging, `FIXED_DT` | `src/core/` | live |
| math — Vec3/Quat, chainable | `src/math/` | live |
| **scene (ECS)** — entities, hierarchy, components | `src/scene/` | live + type-table scaffold |
| **script** — ScriptHost, `Mu::` natives, bindings | `src/script/` | M1 scaffold (IR at M2) |
| render — fn-ptr interface, null backend | `src/render/` | live interface, stub impl (GL at M3) |
| platform — headless pump/quit | `src/platform/` | live headless (SDL3 at M3) |
| alloc — mimalloc game allocator | `src/alloc/` | live |
| vendor extern ports | `src/{sdl,gl,webgpu,stb,cgltf,miniz,tinyexr,drlibs,fastnoise,mikktspace,physics}/` | ports ready, gated by milestones |

<!-- FUTURE LAYERS (not implemented — ENGINE.md §3/§8):
| input — input state + named action map (§13) | `src/input/` | planned M3 |
| audio — SDL3 streams + bus mixer (§14) | `src/audio/` | planned M6 |
| ui — immediate-mode UI, theme tokens (§9) | `src/ui/` | planned M6 |
| assets — handle table, aliases, hot reload (M5) | `src/assets/` | planned M5 |
-->

---

## ECS — scene model (`src/scene/`)

Composition-only (DESIGN §2 — no inheritance anywhere), keyed by stable
`u32` entity ids:

- **`Entity`** — `{ id, alive, parent, local, world }`. `scene.create()`
  bumps an id (`Option` when storage is full); `destroy()` marks dead and
  detaches children. *Id recycling / generations are an open item
  (ENGINE.md §17).*
- **Transform hierarchy** — `setParent(child, parent)` is cycle-checked
  (`Err` carries why); each `scene.update(dt)` composes world transforms
  top-down. Uniform-scale exact; non-uniform scale under rotation is
  intentionally unsupported at M0.
- **`ComponentStore`** — one store per component type: fixed-stride slots
  indexed by entity id + a `present` flag (`add`/`remove`/`has`/`get`).
  Caller-provided static storage (ENGINE.md §10).
- **`ComponentRegistry`** — the component **type table** (M1 scaffold):
  stores are *named* so everything downstream resolves by name:

```coral
ComponentStore comps = ComponentStore.new(data, present, 4096, 8);
ComponentRegistry types = ComponentRegistry.new(table, 16);
types.add("demo", &comps).unwrap();     // one line per component type
if (types.has(entity, "demo")) { ... }  // storeFor("demo") under the hood
```

  M2 plugs `@component` auto-registration (DESIGN §16) into this table —
  a Mu struct registers itself under its name — and the script-side
  component get/set natives (ENGINE.md §11) dispatch through
  `storeFor`.
- **Systems** are plain Coral functions in fixed order (`scene.update`,
  later `physicsStep`, `scriptUpdate`) — no scheduler until a real need
  (job system is backlog, ENGINE.md §16).
- **Scripts attach to entities** — `ScriptBindings` (M1 scaffold, in
  `src/script/`) pairs an entity with a script path, composition-only and
  multiple per entity:

```coral
ScriptBindings bindings = ScriptBindings.new(&scene, storage, 8);
bindings.bind(entity, "scripts/main.mu").unwrap();  // rejects dead entities
bindings.countFor(entity);                          // M2: instance count
bindings.unbind(entity);                            // detach all scripts
```

<!-- FUTURE ECS (not implemented — ENGINE.md §15/§16/§17):
- Prefab/instance system: entity templates + per-instance overrides.
- Scene/prefab file format: text, diffable, versioned; same grammar for
  runtime loader (M5) and editor writer (M7+).
- Entity id recycling with generation counters (dangling-id safety) —
  DESIGN §14: using a despawned handle panics, never corrupts.
- Archetype storage revisit at first perf need (§17).
-->

---

## Scripting (`src/script/`, `scripts/`)

Scripts are `.mu` files under `scripts/`. Two ways to reach the engine —
**know which is which:**

1. **The `Mu::` library — engine API.** Scripts import symbols with
   `use` (DESIGN §2); the host resolves each one at load against its
   native registry and fails loudly **with the path** if missing:

   ```mu
   use Mu::log::info;
   use Mu::timing::frame;
   use Mu::scene::{spawn, despawn, setParent};   // group import

   fn onLoad() {
       let child = spawn();
       if setParent(child, 0) { info("parented"); }
   }

   fn onUpdate(dt: f32) {
       if frame() == 300 && dt > 0.0 { info("300 frames"); }
   }
   ```

2. **`@extern fn ...;` — genuinely foreign functions only** (C/C++ the
   engine does not provide, DESIGN §14). Engine API **never** goes
   through `@extern`.

**Lifecycle hooks** (any may be absent): `fn onLoad()`,
`fn onUpdate(dt: f32)`, `fn onEvent(...)`, `fn onUnload()` — called from
the loop's script phase between `scene.update` and `render.draw`
(ENGINE.md §6). **Bound scripts** run per entity: the binding
(`ScriptBindings` above) decides which entity a script instance is for;
M2 gives the instance that context.

**Value boundary** (ENGINE.md §5): only `EntityId (u32)`, scalars,
`Vec2/Vec3`, `str` cross. No Coral structs, no pointers, no allocator
access — Mu heap traffic routes through the engine game allocator.

### M1 `Mu::` native library

| Symbol | Signature | Notes |
|---|---|---|
| `Mu::log::info` | `fn(msg: &str)` | prints `[mu:script]` |
| `Mu::log::error` | `fn(msg: &str)` | prints `[mu:script:error]` |
| `Mu::timing::frame` | `fn() u64` | current loop frame count |
| `Mu::scene::spawn` | `fn() u32` | new entity id; `NIL` when full |
| `Mu::scene::despawn` | `fn(u32) bool` | true if the entity was alive |
| `Mu::scene::setParent` | `fn(u32, u32) bool` | cycle-checked reparent |
| `Mu::platform::quit` | `fn()` | requests loop exit |

<!-- FUTURE NATIVES (not implemented — ENGINE.md §11/§13/§14):
| Mu::input::isDown / action     | keyboard/gamepad via named action map (§13) |
| Mu::render::*                  | camera/mesh/material queries once M4 lands |
| Mu::physics::step / raycast    | Jolt-backed (§9, M7)                        |
| Mu::audio::play / bus::volume  | bus mixer voices (§14, M6)                  |
| component get/set              | @component types via ComponentRegistry (§11)|
-->

### Adding a native (the growth path)

Everything is one-line incremental (see the header of
`src/script/script.crl`):

1. write a plain `static` fn in `script.crl`;
2. one `natives->add("Mu::module::name", Native::XxxFn { .f = fn })` line
   — existing signature arms cover most cases; a genuinely new signature
   is one new arm in `variant Native`;
3. `use Mu::module::name;` in the script.

<!-- EXAMPLE (not implemented): adding Mu::input::isDown later
    static bool _nativeIsDown(i32 key) { ... }
    natives->add("Mu::input::isDown", Native::KeyPredFn { .f = _nativeIsDown }).unwrap();
    // main.mu:  use Mu::input::isDown;
-->

The M2 loader keeps the same contract: resolve every `use`d symbol at
load (fail with the path), then execute muc's WallVM IR (`IR.md` is the
contract) with lifecycle hooks inside `load`/`update`/`free`.

---

## Milestones detail + roadmap

M0/M1 above; M2 executes IR, M3 opens the window, M4 makes it 3D, M5
assets, M6 audio+UI, M7 physics features+editor — full definitions in
[ENGINE.md §8](../ENGINE.md).

<!-- FUTURE — NOT IMPLEMENTED (tracked in ENGINE.md §9/§12-§16):
- Input (M3): SDL3 events → down/pressed/released state, mouse, gamepad;
  named rebindable action map (never raw keys); synthetic-event injection
  for headless tests.
- Rendering (M3+): GL 3.3 desktop / GLES3 Web behind the Renderer
  interface; Vulkan later. M4: camera, meshes, materials, lights.
  Backlog: shadows, clustering, culling, LOD, post stack (§16).
- Assets (M5): handle table + aliases, loaders (stb/cgltf/exr/dr_libs),
  hot reload, GUID sidecar metadata, cooked packs (miniz).
- Audio (M6): SDL3 streams, stb_vorbis/dr_libs decode, bus mixer
  (master/music/sfx/ui + user buses) — architecture decided now (§14).
- UI (M6): custom immediate-mode UI, SDF fonts (stb_truetype), theme
  tokens, dark-first; UI quality is a milestone gate (§9).
- Signals (§12): name → subscribers, connect/disconnect by name, queued
  dispatch in the script phase.
- Scene/prefab files (§15): text, human-readable, diffable; prefabs =
  reference + per-instance overrides.
- Physics (M7): Jolt 5.6.0 + JoltC are bound today; character
  controller, collision layers, queries, ragdolls keep their own plan.
- Editor (M7+): own plan first; in-engine Mu script editor (highlighting
  reuses the Mu lexer), split-pane terminal, immediate-mode chrome.
- Debug/profiler (§9): CVars, on-screen stats, CPU/GPU zones behind one
  always-compiled instrumentation API.
- Backlog (§16): job system, animation, navmesh, networking, export/
  inspector annotations, replay stepping.
-->
