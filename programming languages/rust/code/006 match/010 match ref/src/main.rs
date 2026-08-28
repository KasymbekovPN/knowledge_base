fn main() {

    let v = vec![String::from("A"), String::from("B"), String::from("C")];
    // v.first() возвращает Option<&String>
    match v.first() {
        // s здесь — &String, не String — v не перемещается
        Some(s) => println!("{s}"),
        None => println!("empty"),
    }

    // v всё ещё доступна
    println!("{:?}", v);
}