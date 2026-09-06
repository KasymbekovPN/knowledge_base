include!(concat!(env!("OUT_DIR"), "/build_info.rs"));

unsafe extern "C" {
    fn fast_square(x: i32) -> i32;
    fn fast_factorial(n: i32) -> i64;
}

fn main() {
    println!("Built: {BUILD_PROFILE}");
    println!("Git hash: {GIT_HASH}");
    println!("Target OS: {TARGET_OS}");

    // вызов C-функции требует unsafe -- компилятор не может проверить её контракт
    unsafe {
        println!("fast_square(7): {}", fast_square(7));
        println!("fast_factorial(7): {}", fast_factorial(7));
    }
}
