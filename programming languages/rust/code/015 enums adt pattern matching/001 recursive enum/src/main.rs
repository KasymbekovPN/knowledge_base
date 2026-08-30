
// recursive type
// enum List0 {
//     Cons(i32, List0),
//     Nil,
// }

#[derive(Debug)]
enum List {
    // Box — указатель на кучу, размер известен (просто размер указателя)
    Cons(i32, Box<List>),
    Nil
}

use List::{Cons, Nil};

impl List {
    fn sum(&self) -> i32 {
        match self {
            Cons(v, rest) => v + rest.sum(),
            Nil => 0,
        }
    }
}

fn main() {
    let list = Cons(1, Box::new(Cons(2, Box::new(Cons(3, Box::new(Nil))))));
    println!("{:?}", list);
    println!("sum= {}", list.sum());
}