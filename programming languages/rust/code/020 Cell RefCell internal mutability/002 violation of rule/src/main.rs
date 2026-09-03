use std::cell::RefCell;

fn main() {
    let cell = RefCell::new(5);
    let b1 = cell.borrow_mut();
    println!("{:?}", b1);

    // panic
    let b2 = cell.borrow_mut();
    println!("{:?}", b2);
}