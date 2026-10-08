#! string.casing, string.format

use Mu::render;
use Mu::render::draw as blit;
use Mu::physics::step;

pub const GRAVITY: f32 = 9.8;
pub const MAX_ENTITIES: u32 = 4096;

pub struct Entity {
    id: u32;
    hp: u16;
    x: f32;
    y: f32;
}

#! pub extern fn spawn(name: str, x: f32, y: f32) Entity;
#! extern fn despawn(id: u32);
#! extern fn frame() u64;
#! Mu will provide these funtions

pub fn spawn_player(x: f32, y: f32) Entity {
    let player = spawn("player", x, y);
    player.hp = 100;
    return player;
}

pub async fn update(dt: f32, active: Array<Entity>) {
    for entity in active {
        step(entity, dt);
        entity.y = entity.y + GRAVITY * dt;
        if entity.hp == 0 {
            blit(entity.x, entity.y);
            despawn(entity.id);
        }
    }
    await frame();
}
