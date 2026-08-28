
fn divide(a: f64, b: f64) -> Option<f64> {
    if b == 0.0 { None } else {Some(a/b)}
}

fn match_it(a: f64, b: f64) -> String {
    match divide(a, b) {
        Some(result) => result.to_string(),
        None => String::from("divide by zero"),
    }
}

fn main() {
    println!("match_it(10.0, 5.0): {}", match_it(10.0, 5.0));
    println!("match_it(10.0, 0.0): {}", match_it(10.0, 0.0));
}