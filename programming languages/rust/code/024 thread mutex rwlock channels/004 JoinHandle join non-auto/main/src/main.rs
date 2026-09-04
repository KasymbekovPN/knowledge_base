use std::thread;
use std::time::Duration;

fn main() {
    let handle = thread::spawn(move || {
        thread::sleep(Duration::from_millis(200));
        println!("Join handle");
    });

    // не вызвали join() -- поток "отсоединяется"
    drop(handle);

    println!("Join handle dropped, main thread continues");
    thread::sleep(Duration::from_millis(400));
}
