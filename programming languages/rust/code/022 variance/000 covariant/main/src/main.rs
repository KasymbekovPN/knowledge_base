
fn take_short_lifetime<'a>(text: &'a str) {
    println!("short ref: {}", text);
}

fn main() {
    // 'static живет до конца программы (очень длинное)
    let static_str: &'static str = "forever";

    // Передаем 'static туда, где ожидается обычное локальное 'a.
    // Это работает, потому что &'a T ковариантен по 'a (сжатие разрешено).
    take_short_lifetime(static_str);
}
