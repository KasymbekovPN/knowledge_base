
fn add(a: i32, b: i32) -> i32 {
    // последнее выражение без `;` — неявный return
    a + b
}

fn greet(name: &str) {
    println!("Hello, {}!", name);
}

fn main() {
    println!("Result: {}", add(1, 2));
    greet("Free function");
}