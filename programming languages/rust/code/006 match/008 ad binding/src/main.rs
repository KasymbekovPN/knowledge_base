
fn classify(n: i32) -> String {
    match n {
        x @ 1..=5 => format!("in range 1..5, value: {x}"),
        x @ 6..=10 => format!("in range 6..10, value: {x}"),
        x => format!("other: {x}"),
    }
}

fn main() {
    for n in [-3, 0, 1, 3, 5, 6, 8, 10, 11, 100] {
        println!("{}", classify(n));
    }
}