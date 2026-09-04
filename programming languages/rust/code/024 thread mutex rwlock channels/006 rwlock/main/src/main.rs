use std::sync::{Arc, RwLock};
use std::thread;

fn main() {
    let data = Arc::new(RwLock::new(vec![1, 2, 3]));
    let mut handlers = vec![];

    // несколько читателей одновременно -- разрешено
    for i in 0..3 {
        let data = Arc::clone(&data);
        handlers.push(thread::Builder::new().name(format!("{}", i)).spawn(move || {
            let guard = data.read().unwrap();
            println!("[{:?}] reader {:?}: {:?}", thread::current().name(), i, *guard);
        }));
        // handlers.push(thread::spawn(move || {
        //     let guard = data.read().unwrap();
        //     println!("[{:?}] reader {:?}: {:?}", thread::current().name(), i, *guard);
        // }));
    }

    for result in handlers { result.unwrap().join().unwrap(); }

    // один писатель -- эксклюзивный доступ
    {
        let mut guard = data.write().unwrap();
        guard.push(4);
    }

    println!("[{:?}] after writing: {:?}", thread::current().name(), *data.read().unwrap());
}
