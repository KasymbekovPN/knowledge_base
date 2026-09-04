use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];

    for _ in 0..10 {
        let counter = Arc::clone(&counter);
        handles.push(thread::spawn(move || {
            let mut guard = counter.lock().unwrap();
            *guard += 1;
        })); // guard дропается здесь -- разблокировка автоматическая
    }

    for h in handles { h.join().unwrap(); }
    println!("total: {}", *counter.lock().unwrap());
}
