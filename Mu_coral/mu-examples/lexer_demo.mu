#! string.casing, string.format

use Mu::Out::log; // provides fn log(msg: str);
use Mu::render as r;

type Meters = f32;


pub const MAX_SPEED: u16 = 0xFFu16;
pub const GREETING: string = "hello\nworld";

variant Shape {
    Circle(f64),
    Rect(f64, f64),
}

struct Vec2 {
    x: f32;
    y: f32;

    fn length(self) f32 {
        return self.x * self.x + self.y * self.y;
    }
}

fn area(shape: Shape) f64 {
    return switch (shape) {
        Circle(r) => 3.14159 * r * r,
        Rect(w, h) => w * h,
    };
}

async fn tick(dt: f32) {
    @outer: while true {
        if dt >= 0.016f32 && dt != 0.0 {
            break outer;
        }
    }
    let values = [1, 2, 3];
    let lookup = ["hp": 100, "mp": 50];
    let span = 0..=10;
    let ch = 'μ';
    let big = 1000000i64;
    let picked = max(values[0], 2);
}

fn max(a: i32, b: i32) i32 {
    if a > b {
        return a;
    }
    return b;
}
