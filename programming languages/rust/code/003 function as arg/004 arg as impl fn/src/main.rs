
fn double_fn(x: i32) -> i32 { x * 2 }

fn apply_impl_fn(f: impl Fn(i32) -> i32, x: i32) -> i32 {
    f(x)
}

// это синтаксический сахар для:
fn apply_generic<F: Fn(i32) -> i32>(f: F, x: i32) -> i32 {
    f(x)
}

fn main() {
    let y = 10;
    println!("apply_impl_fn(|x| x + y, 5): {}", apply_impl_fn(|x| x + y, 5));
    println!("apply_impl_fn(double_fn, 6): {}", apply_impl_fn(double_fn, 6));
    println!("apply_generic(double_fn, 7): {}", apply_generic(double_fn, 7));
}