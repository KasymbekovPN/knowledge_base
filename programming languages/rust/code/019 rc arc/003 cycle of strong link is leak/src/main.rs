
use std::rc::Rc;
use std::cell::RefCell;

struct Node {
    value: i32,
    next: RefCell<Option<Rc<Node>>>,
}

impl Drop for Node {
    fn drop(&mut self) {
        println!("Dropping Node {}", self.value);
    }
}

fn main() {
    let a = Rc::new(Node {  value: 5, next: RefCell::new(None)});
    let b = Rc::new(Node {  value: 6, next: RefCell::new(None)});
    let _c = Rc::new(Node {  value: 7, next: RefCell::new(None)});

    *a.next.borrow_mut() = Some(Rc::clone(&b));
    *b.next.borrow_mut() = Some(Rc::clone(&a));

    println!("a strong_count: {}", Rc::strong_count(&a));
    println!("b strong_count: {}", Rc::strong_count(&b));
    println!("--- end ---");
}