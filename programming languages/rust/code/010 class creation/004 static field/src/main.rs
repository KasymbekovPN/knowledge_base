use std::sync::atomic::{ AtomicU32, Ordering };

struct Circle { radius: f64 }

static INSTANCE_COUNT: AtomicU32 = AtomicU32::new(0);

impl Circle {
    fn new(radius: f64) -> Self {
        INSTANCE_COUNT.fetch_add(1, Ordering::SeqCst);
        Circle { radius }
    }
}

fn main() {
    let _c0 = Circle::new(1.0);
    let _c1 = Circle::new(2.0);
    let _c2 = Circle::new(3.0);

    println!("count: {}", INSTANCE_COUNT.load(Ordering::SeqCst));
}