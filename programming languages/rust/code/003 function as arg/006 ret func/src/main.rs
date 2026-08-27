
fn make_addr_f(x: i32) -> impl Fn(i32) -> i32 {
    move |y| x + y
}

fn make_addr_df(x: i32) -> Box<dyn Fn(i32) -> i32> {
    Box::new(move |y| x + y)
}

fn main() {
    println!("make_addr_f(10)(11) = {}", make_addr_f(10)(11));
    println!("make_addr_df(12)(13) = {}", make_addr_df(12)(13));
}