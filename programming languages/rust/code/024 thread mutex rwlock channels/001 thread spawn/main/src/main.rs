use std::thread;

fn main() {
    let handle = thread::spawn(|| {
        let mut sum = 0;
        for i in 1..=100 { sum += i; }
        sum
    });

    println!("main thread continues work");
    let result = handle.join().unwrap();
    println!("result is {}", result);
}
