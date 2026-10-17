// control.flow

use Mu::timing::now;

const EPSILON: f32 = 0.0001;

fn classify(score: u16) string {
    return switch (score) {
        0                        => "none",
        1, 2                     => "tiny",
        3..=9 if score % 2 == 0  => "small even",
        else                     => "big",
    };
}

fn grade(score: u16) string {
let label = if score >= 90 { "A" } else if score >= 80 { "B" } else { "C" };
    return label;
}

fn countdown(from: u32) {
    let n = from;
    while n > 0 {
        n = n - 1;
    }
}

fn checksum(width: u32, height: u32) u32 {
    let total = 0;
    @outer: for y in 0..height {
        for x in 0..width {
            if x * y > 256 {
                continue outer;
            }
            if x + y == 64 {
                break outer;
            }
            total = total + x * y;
        }
    }
    return total;
}

pub fn run() {
    let top = grade(95);
    countdown(3);
    switch (top) {
        "A"   => print("top"),
        else  => print("keep going"),
    }

    let value = 41;
    let hint = switch (value) {
        0     => "zero",
        1, 2  => "small",
        else  => {
            countdown(2);
            "other"
        },
    };

    let at = now();
    if at > EPSILON {
        print(hint);
    }
}
