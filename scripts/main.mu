// Mu gameplay script for the particle sandbox (ENGINE.md §5/§8).
// The engine pre-resolves these `use` paths by hash at load; at M2 the host
// executes the lifecycle hooks below from the loop's script phase.

use Mu::render::{paint};
use Mu::render;
use Mu::render::draw as blit;
use Mu::physics::step;
use Mu::scene::{spawn, despawn, frame};
use Mu::log::info;
use Mu::timing::{frame, now};

fn onLoad() {
    info("particles.mu: online");
    paint(16, 28, 96, 4, 0x45D6A8FF);
    blit();
}

fn onUpdate(dt: f32) {
    step(dt);

    let tick = Mu::timing::frame();
    let elapsed = now();
    let live = Mu::scene::frame();

    // Recording pulse in the lower-left corner of the HUD.
    let phase = (tick % 60) as i32;
    paint(16, 28, 96, 3, 0x18322EFF);
    paint(16, 28, phase + 2, 3, 0x45D6A8FF);

    if tick % 120 == 0 {
        info("particles.mu: heartbeat");
    }
    if tick == 180 {
        let spare = spawn();
        if despawn(spare) {
            info("particles.mu: spawn/despawn round-trip ok");
        }
    }
    if tick == 240 && elapsed > 3000 && live > 0 {
        info("particles.mu: update loop is live");
    }

    blit();
}

fn onUnload() {
    info("particles.mu: offline");
}
