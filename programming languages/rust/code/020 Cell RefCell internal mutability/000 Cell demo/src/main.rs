use std::cell::Cell;

#[derive(Default)]
struct Counter { count: Cell<i32>, }

impl Counter {
    // &self, НЕ &mut self!
    fn increment(&self) {
        self.count.set(self.count.get() + 1);
    }
}


fn main() {
    let c = Counter::default();
    c.increment();
    c.increment();
    c.increment();

    println!("{}", c.count.get());
}