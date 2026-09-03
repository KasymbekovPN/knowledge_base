use std::rc::{Rc, Weak};
use std::cell::{Ref, RefCell};

struct Node {
    value: i32,
    // Weak -- не владеет
    parent: RefCell<Weak<Node>>,
    // Rc -- владеет
    children: RefCell<Vec<Rc<Node>>>,
}

impl Drop for Node {
    fn drop(&mut self) {
        println!("Drop Node {}", self.value);
    }
}

fn main() {
    let leaf = Rc::new(Node {
        value: 2,
        parent: RefCell::new(Weak::new()),
        children: RefCell::new(vec![]),
    });

    {
        let branch = Rc::new(Node {
            value: 1,
            parent: RefCell::new(Weak::new()),
            children: RefCell::new(vec![Rc::clone(&leaf)]),
        });

        *leaf.parent.borrow_mut() = Rc::downgrade(&branch);

        if let Some(p) = leaf.parent.borrow().upgrade() {
            println!("leaf parent: {}", p.value);
        }
    } // branch выходит из scope -- цикла нет, объект реально уничтожается

    println!("leaf.parent.upgrade(): {}", leaf.parent.borrow().upgrade().is_some());
}