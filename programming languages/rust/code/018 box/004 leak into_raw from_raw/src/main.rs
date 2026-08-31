fn main() {

    // Box::leak -- намеренная утечка, получаем 'static ссылку
    let boxed = Box::new(String::from("forever"));
    let leaked: &'static mut String = Box::leak(boxed);
    leaked.push_str("!");
    println!("leaked: {}", leaked);

    // Box::into_raw / Box::from_raw -- мост в unsafe/FFI мир
    let b = Box::new(100);
    // теперь Rust не отвечает за память
    let raw: *mut i32 = Box::into_raw(b);
    unsafe {
        println!("raw pointer: {}", *raw);
        // забираем обратно под управление Rust
        let reclaimed = Box::from_raw(raw);
        println!("reclaimed: {}", reclaimed);
    } // reclaimed корректно дропается здесь
}