fn consume<F: FnOnce() -> String>(f: F) -> String {
    f()
}

fn main() {
    let s = String::from("hello world");
    // забирает владение s, возвращает его — второй вызов невозможен
    let closure = move || s;
    println!("result: {}", consume(closure));
}
