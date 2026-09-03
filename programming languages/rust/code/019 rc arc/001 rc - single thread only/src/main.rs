use std::rc::Rc;

struct NotShareable;

fn requires_send<T: Send>(_: T) {}


fn main() {
    let data = Rc::new(NotShareable);
    // requires_send(data); // Error
}