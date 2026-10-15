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
    seen: Set<EntityId>;
    pending: Queue<EntityId>;
    bodies: Pool<Vec2>;
    mask: Bitset;
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

// Mu has no user-defined generics: one function per concrete type.
fn max(a: i32, b: i32) i32 {
    if a > b {
        return a;
    }
    return b;
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
    let picked = max(sizes[0], 2);
    let pair = (1, "one");
    let (first, second) = pair;
}

// Built-in containers: Set<T>, Queue<T>, Pool<T>, Bitset (non-generic).
fn containers() {
    var live: Set<EntityId> = Set.new();
    live.add(7);
    live.add(7);
    live.add(9);
    let unique = live.len();
    let has_seven = live.has(7);
    let has_five = live.has(5);
    live.remove(7);
    let after_remove = live.has(7);

    var pending: Queue<EntityId> = Queue.new();
    pending.push(1);
    pending.push(2);
    pending.push(3);
    let queued = pending.len();
    let next = pending.pop();
    let rest = pending.len();

    var total: u32 = 0;
    for id in live {
        total = total + id;
    }
    
    for id in pending {
        total = total + id;
    }

    var bodies: Pool<Vec2> = Pool.new(64);
    let slot = bodies.create();
    bodies.set(slot, Vec2 { x: 1.0, y: 2.0 });
    let reused = bodies.create();
    let capacity = bodies.cap();
    let in_use = bodies.len();
    bodies.release(slot);
    let still_live = bodies.alive(slot);

    var mask: Bitset = Bitset.new(128);
    mask.set(3);
    mask.set(70);
    let on = mask.test(3);
    let off = mask.test(4);
    let bits = mask.count();
    mask.clear(3);
    let after_clear = mask.test(3);
    let any_set = mask.any();
    mask.toggle(70);
    let after_toggle = mask.test(70);
}
