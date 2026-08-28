
fn match_it(x: i32) {
    match x {
        1 | 2 | 3 => println!("1 | 2 | 3"),
        4..=10 => println!("4..=10"),
        _ => println!("Not a match"),
    }
}

fn main() {
    match_it(2);
    match_it(7);
    match_it(100);
}