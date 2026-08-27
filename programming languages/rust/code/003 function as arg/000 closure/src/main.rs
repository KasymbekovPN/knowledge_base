fn main() {
    let x = 10;
    let add_fn = |y: i32| x + y;

    println!("add_fn: {}", add_fn(11));
}