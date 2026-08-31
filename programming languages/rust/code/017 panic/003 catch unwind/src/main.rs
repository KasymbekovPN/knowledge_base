use std::panic;

fn risky(n: i32) -> i32 {
    if n < 0 { panic!("divide by zero: {n}") }
    n * 2
}

fn main() {
    let result = panic::catch_unwind(|| risky(5));
    println!("{:?}", result);

    let result = panic::catch_unwind(|| risky(-10));
    match result {
        Ok(result) => println!("Ok: {:?}", result),
        Err(err) => println!("Err: {:?}", err),
    }
    println!("continues work")
}