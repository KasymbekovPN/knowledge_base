use std::sync::mpsc;
use std::thread;
use std::time::Duration;

fn main() {
    let (tx, rx) = mpsc::channel();

    thread::spawn(move || {
        for i in 1..=5 {
            tx.send(i).unwrap();
            thread::sleep(Duration::from_millis(10));
        }
        // tx выходит из scope здесь -- канал закрывается
    });

    // rx работает как итератор
    for received in rx {
        println!("Got: {}", received);
    }
    println!("Done!");
}
