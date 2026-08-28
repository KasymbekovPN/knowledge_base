
struct Point {x: i32, y: i32}

fn main() {
    // деструктуризация кортежа
    let (a, b) = (1, 2);
    println!("a = {}", a);
    println!("b = {}", b);

    // деструктуризация массива
    let [first, second, third] = [3, 4, 5];
    println!("first = {}", first);
    println!("second = {}", second);
    println!("third = {}", third);

    // деструктуризация struct
    let Point {x, y} = Point{x: 6, y: 7};
    println!("x = {}", x);
    println!("y = {}", y);

    // средний элемент не нужен
    let (x0, _, z0) = (8, 9, 10);
    println!("x0 = {}", x0);
    println!("z0 = {}", z0);

    // остальные поля struct игнорируем
    let Point {x, ..} = Point{x: 11, y: 12};
    println!("x = {}", x);
}