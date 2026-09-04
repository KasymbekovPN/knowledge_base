use std::thread::sleep;
use std::time::Duration;

async fn task(name: &str, ms: u64) {
    println!("{name} is started");
    sleep(Duration::from_millis(ms));
    println!("{name} is finished");
}

#[tokio::main]
async fn main() {
    let h1 = tokio::spawn(task("A", 300));
    let h2 = tokio::spawn(task("B", 100));
    let h3 = tokio::spawn(task("C", 200));

    let _ = tokio::join!(h1, h2, h3);
}
