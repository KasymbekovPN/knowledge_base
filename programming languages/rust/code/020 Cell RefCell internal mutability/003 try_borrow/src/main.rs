use std::cell::RefCell;

fn main() {
    let cell = RefCell::new(5);
    let b1 = cell.borrow_mut();

    match cell.try_borrow_mut() {
        Ok(_) => println!("[unexpected] second borrow_mut took"),
        Err(e) => println!("could not take: {e}"),
    }

    println!("b1 is still alive: {b1}");
}