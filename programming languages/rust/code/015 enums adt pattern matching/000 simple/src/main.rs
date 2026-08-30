
#[derive(Debug)]
enum Message {
    // unit-вариант — без данных
    Quit,
    // struct-вариант — именованные поля
    Move { x: i32, y: i32 },
    // tuple-вариант — одно поле
    Write(String),
    // tuple-вариант — три поля
    ChangeColor(i32, i32, i32),
}

fn process(msg: &Message) -> String {
    match msg {
        Message::Quit => String::from("quit"),
        Message::Move { x, y } => format!("movement: {} -> {}", x, y),
        Message::Write(text) => format!("write: {}", text),
        Message::ChangeColor(r, g, b) => format!("color: #{r:02x}{g:02x}{b:02x}"),
    }
}

fn main() {
    println!("{}", process(&Message::Quit));
    println!("{}", process(&Message::Move { x: 10, y: 20 }));
    println!("{}", process(&Message::Write("Hello, world!".to_string())));
    println!("{}", process(&Message::ChangeColor(0, 0, 255)));
}