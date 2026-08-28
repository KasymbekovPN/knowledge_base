

fn match_it(p: (i32, i32)) {
    match p {
        (x, y) if x == y => println!("equals"),
        (x, y) if x + y == 0 => println!("zero i sum"),
        (x, _) if x % 2 == 0 => println!("first even"),
        _ => println!("nothing special"),
    }
}

fn main() {
    match_it((2, -2));
    match_it((2, 2));
    match_it((16, -1));
    match_it((-1, -2));
}