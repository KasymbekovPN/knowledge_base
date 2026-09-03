
// Мы пытаемся сказать: "f принимает ссылку с временем жизни 'a"
fn process_data<'a, F>(f: F)
where
    F: Fn(&'a str)
{
    let local_string = String::from("hello world");
    // Compilation error
    f(&local_string);
}

fn handle_string(s: &str) {
    println!("str: {s}");
}

fn main() {
    process_data(handle_string);
}