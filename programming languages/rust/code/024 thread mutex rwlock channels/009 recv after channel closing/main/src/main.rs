use std::sync::mpsc;

fn main() {
    let (tx, rx) = mpsc::channel::<i32>();
    drop(tx);

    match rx.recv() {
        Ok(v) => println!("value: {v}"),
        Err(e) => println!("channel closed: {e}"),
    }
}
