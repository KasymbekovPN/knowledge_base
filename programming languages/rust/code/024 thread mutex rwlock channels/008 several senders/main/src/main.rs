use std::sync::mpsc;
use std::thread;

fn main() {
    let (tx, rx) = mpsc::channel();

    for producer_id in 0..3 {
        let tx = tx.clone();
        // Sender: Clone -- каждый поток получает свою копию
        thread::spawn(move || {
            tx.send(format!("message from producer: {producer_id}")).unwrap();
        });
    }
    // ВАЖНО: дропнуть оригинальный tx, иначе rx будет ждать вечно
    drop(tx);

    let received: Vec<String> = rx.iter().collect();
    println!("message from received: {:?}", received);
}
