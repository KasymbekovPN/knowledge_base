

#[derive(Default)]
struct Counter { value: i32 }

impl Counter {
    fn incremented(&mut self) { self.value += 1 }
}

fn main() {
    let mut boxed = Box::new(Counter::default());
    // сахар для (*boxed).increment()
    boxed.incremented();
    boxed.incremented();
    (*boxed).incremented();
    println!("{}", boxed.value);
}