use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let data = Arc::new(Mutex::new(0));
    let d = Arc::clone(&data);

    let handle = thread::spawn(move || {
        let mut guard = d.lock().unwrap();
        *guard += 1;
        panic!("panic is inner thread which holds mutex");
    });

    let join_result = handle.join();
    println!("join returned Err: {}", join_result.is_err());

    let lock_result = data.lock();
    match lock_result {
        Ok(_) => println!("Ok"),
        Err(poisoned) => {
            println!("poisoned mutex but data is reachable");
            let guard = poisoned.into_inner();
            println!("inner value: {guard}");
        }
    }
}
