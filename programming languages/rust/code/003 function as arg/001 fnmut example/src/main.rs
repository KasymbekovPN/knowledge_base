
fn call_three_times<F: FnMut()>(mut f: F) {
    f();
    f();
    f();
}

fn main() {
    let mut count = 0;
    call_three_times(|| {
        count += 1;
        println!("count is {}", count);
    });
}