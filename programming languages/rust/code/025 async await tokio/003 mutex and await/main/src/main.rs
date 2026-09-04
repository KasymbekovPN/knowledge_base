use std::sync::Arc;
use std::time::Duration;
use tokio::sync::Mutex as TokioMutex;

async fn bad_task(id: i32, data: Arc<std::sync::Mutex<u32>>) {
    // держим std-мьютекс...
    let mut guard = data.lock().unwrap();
    // ...и ЗАСЫПАЕМ, держа его
    tokio::time::sleep(Duration::from_millis(100)).await;
    *guard += 1;
}

async fn good_task(id: i32, data: Arc<TokioMutex<i32>>) {
    // .await, не .unwrap()
    let mut guard = data.lock().await;
    // безопасно спать, держа tokio::Mutex
    tokio::time::sleep(Duration::from_millis(50)).await;
    *guard += 1;
}


#[tokio::main]
async fn main() {
    let a = bad_task(0, Arc::new(std::sync::Mutex::new(1))).await;
    let b = good_task(1, Arc::new(TokioMutex::new(2))).await;
}
