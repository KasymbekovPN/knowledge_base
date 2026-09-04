
async fn step_a() -> i32 { 42 }

async fn step_b() -> i32 {
    let x = step_a().await;
    x + step_a().await
}

fn main() {
    // Future ещё не запущен -- ленивый
    let fut = step_b();
    println!("size_of Future for step_b {} bites", size_of_val(&fut));
}
