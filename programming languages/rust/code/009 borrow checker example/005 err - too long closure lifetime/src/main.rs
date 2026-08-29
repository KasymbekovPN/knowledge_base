
// fn make_bad_closure() -> impl Fn() -> String {
//     let s = String::from("hello");
//     // ОШИБКА: closure may outlive the current function, but it borrows `s`
//     || format!("{s}!")
// }

fn make_closure() -> impl Fn() -> String {
    let s = String::from("hello");
    // s перемещена внутрь замыкания, живёт столько же, сколько оно само
    move || format!("{s}!")
}

fn main() {
    println!("{:?}", make_closure()());
}