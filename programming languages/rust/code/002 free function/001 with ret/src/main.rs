

fn abs(x: i32) -> i32 {
    if x < 0 { return -x; }
    x
}

fn main() {
    println!("ans({}) => {}", -10, abs(-10));
    println!("ans({}) => {}", 10, abs(10));
}