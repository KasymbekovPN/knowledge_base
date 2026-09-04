use std::sync::{Arc, Mutex};

fn is_send<T: Send>() {}
fn is_sync<T: Sync>() {}


fn main() {
    is_send::<i32>();
    is_send::<Arc<i32>>();
    is_send::<Mutex<i32>>();
    // ОШИБКА -- Rc не Send
    // is_send::<Rc<i32>>();

    is_sync::<Mutex<i32>>();
    // ОШИБКА -- RefCell не Sync
    // is_sync::<RefCell<i32>>();
}
