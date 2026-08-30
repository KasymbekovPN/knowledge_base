use std::fmt::Display;

fn print_it_generic<T: Display>(item: T) {
    println!("generic {item}");
}

fn print_it_impl(item: impl Display) {
    println!("impl {item}");
}

fn main() {
    print_it_generic::<i32>(41);
    print_it_generic(42);
    print_it_impl(43);
    // print_it_impl::<i32>(44);
}