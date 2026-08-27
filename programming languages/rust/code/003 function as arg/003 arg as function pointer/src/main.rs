
fn apply_fn_pointer(f: fn(i32) -> i32, x: i32) -> i32 {
    f(x)
}

fn double_fn(x: i32) -> i32 { x * 2 }

fn main() {
    println!("apply_fn_pointer(double, 5): {}", apply_fn_pointer(double_fn, 5));
    println!("apply_fn_pointer(|x| x + 1, 5): {}", apply_fn_pointer(|x| x + 1, 5))
}