
enum MyOption<T> {
    MySome(T),
    MyNone,
}

fn main() {
    let x: MyOption<i32> = MyOption::MySome(42);
    let is_some = matches!(x, MyOption::MySome(_));
    println!("is_some {is_some}");
}