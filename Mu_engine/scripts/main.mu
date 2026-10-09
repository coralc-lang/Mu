#! engine.main

// Demo script (ENGINE.md §8 M1) — compiled by muc at M2.
// Engine built-ins are prelude — always in scope, bare calls, no @
// prefix; the `use Mu::...` lines below are explicit tracking of what
// this script depends on (DESIGN §2/§14). `@extern` stays reserved for
// genuinely foreign functions — C the engine does not provide. Lifecycle
// hooks (§5) run inside the loop's script phase; this script is bound
// to an entity through ScriptBindings (engine side).

use Mu::log::info;
use Mu::timing::frame;
use Mu::scene::{spawn, despawn, setParent};

fn onLoad() {
    let child = spawn();
    if setParent(child, 0) {
        info("main.mu: spawned entity parented under root");
    } else {
        info("main.mu: setParent rejected");
    }
}

fn onUpdate(dt: f32) {
    let f = frame();
    if f == 300 && dt > 0.0 {
        info("main.mu: frame 300 — onUpdate is live");
    }
    if f == 500 {
        let doomed = spawn();
        if despawn(doomed) {
            info("main.mu: spawn/despawn round-trip ok");
        }
    }
}

fn onUnload() {
    info("main.mu: onUnload");
}
