#[tokio::main]
async fn main() {
    tokio::select! {
        _ = tokio::time::sleep(std::time::Duration::from_millis(10)) => {
            println!("10");
        },
        _ = tokio::time::sleep(std::time::Duration::from_millis(5)) => {
            println!("5");
        }
    }
}
