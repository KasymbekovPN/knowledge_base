
// fn make_string_bad0() -> &String { // ОШИБКА уже в сигнатуре
//     let s = String::from("hello");
//     &s
// } // s уничтожается здесь — возвращать на неё ссылку некуда

// fn make_string_bad1<'a>()  -> &'a String {
//     let s = String::from("bad");
//     &s // ОШИБКА: cannot return reference to local variable `s`
// }

fn make_string() -> String {
    let s = String::from("hello");
    s // move наружу, никакого clone не нужно
}

fn main() {
    println!("make_string() => {}", make_string());
}