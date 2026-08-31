
fn divide(a: i32, b: i32) -> i32 {
    if b == 0 {
        panic!("divide by zero: {a} / {b}")
    }
    a / b
}

fn main() {
    println!("until");
    let r = divide(10, 0);
    println!("after: {r}");
}