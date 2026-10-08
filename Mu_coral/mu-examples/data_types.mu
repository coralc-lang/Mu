#! types.demo

type Meters = f32;
type EntityId = u32;

variant Shape {
    Circle(f64),
    Rect(f64, f64),
    Point,
}

struct Vec2 {
    x: f32;
    y: f32;

    fn length(self) f32 {
        return self.x * self.x + self.y * self.y;
    }

    fn new(x: f32, y: f32) Vec2 {
        return Vec2 { x: x, y: y };
    }
};

struct Scene {
    name: string;
    origin: Vec2;
    tags: string[];
    weights: Array<f32>;
    lookup: Map<string, i32>;
    bounds: i32[8];
    flags: (bool, u16);
}

const ORIGIN = Vec2 { x: 0.0, y: 0.0 };
const LIMITS: i32[3] = [1, 2, 3];

fn areas(shape: Shape) f64 {
    return switch (shape) {
        Circle(r)  => 3.14159 * r * r,
        Rect(w, h) => w * h,
        Point      => 0.0,
    };
}

fn literals() {
    let sizes = [1, 2, 3];
    let grid = [[1, 2], [3, 4]];
    let lookup = ["hp": 100, "mp": 50];
    let spread = [10, ...sizes];
    let span = 0..=10;
    let head = sizes[0];
    let window = sizes[1..3];
    let big: i64 = 1_000_000;
    let small = 42i8;
    let pi = 3.14159f32;
    let masked = 0xFFu8;
    let shifted = small as i64;
    let glyphs = ['a', 'μ', '\u{1F600}'];
    let grid_of: Array<Array<i32>> = [[1], [2]];
    let picked = max::<i32>(sizes[0], 2);
    let pair = (1, "one");
    let (first, second) = pair;
}
