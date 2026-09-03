
fn process_string<'a>(line: &'a str) {
    println!("{}", line);
}

fn main() {
    let str: &'static str = "This is a string";
    process_string(str);
}
