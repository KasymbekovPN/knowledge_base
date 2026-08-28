fn main() {
    let n = 7;
    let description = match n {
        1 => "one",
        2 => "two",
        _ => "many",
    };
    println!("description: {}", description);
}