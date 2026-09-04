
#[tokio::main]
async fn main() {
    // с ограниченной ёмкостью буфера
    let (tx, mut rx) = tokio::sync::mpsc::channel(8);
    tokio::spawn(async move {
        for i in 1..=3 {
            // .await -- если буфер полон, ждём асинхронно
            tx.send(i).await.unwrap();
            tokio::time::sleep(std::time::Duration::from_millis(20)).await;
        }
    });

    while let Some(v) = rx.recv().await {
        println!("gotten from channel {:?}", v);
    }
}
