use std::sync::Arc;
use std::thread;

fn main() {
    let data = Arc::new(vec![1, 2, 3, 4, 5]);
    let mut handles = vec![];

    for i in 0..3 {
        let data_clone = Arc::clone(&data);
        let handle = thread::spawn(move || {
            let sum: i32 = data_clone.iter().sum();
            println!("thread {i}, sum: {sum}, strong_count = {}", Arc::strong_count(&data_clone));
        });
        handles.push(handle);
    }

    for h in handles { h.join().unwrap(); }
    println!("strong_count = {}", Arc::strong_count(&data));
}