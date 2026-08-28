

fn print_range(x: i32) {
    match x {
        0..=10 => println!("0..=10"),
        11..=20 => println!("11..=20"),
        _ => println!(".."),
    }
}

fn main() {
    print_range(5);
    print_range(11);
    print_range(-1);
}