
#[derive(Default)]
struct Counter { value: i32 }

impl Counter {
    fn incremented(&mut self) { self.value += 1 }
}

impl Drop for Counter {
    fn drop(&mut self) {
        println!("Drop counter with value: {}", self.value);
    }
}

fn main() {
    let mut boxed = Box::new(Counter::default());
    boxed.incremented();
    boxed.incremented();
}