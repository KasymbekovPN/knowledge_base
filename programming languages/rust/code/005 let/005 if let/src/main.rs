
fn main() {
    // let maybe_value: Option<i32> = Some(42);
    let maybe_value: Option<i32> = None;

    if let Some(x) = maybe_value {
        println!("value = {x}");
    } else {
        println!("empty");
    }

    match maybe_value {
        Some(x) => println!("value = {x}"),
        None => println!("empty"),
    }

    let x: i32 = if let Some(x) = maybe_value { x } else { -1 };
    println!("x = {}", x);
}