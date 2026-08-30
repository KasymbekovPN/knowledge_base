
fn divide(a: f64, b: f64) -> Option<f64> {
    if b == 0.0 { None } else { Some(a / b) }
}

fn main() {
    let result = divide(100.0, 7.7)
        .map(|x| x * 100.0)
        .unwrap_or(0.0);
    println!("{:?}", result);

    println!("{:?}", divide(1.0, 0.0))
}