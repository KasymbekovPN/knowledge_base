
fn make_adder(x: i32) -> impl Fn(i32) -> i32 {
    move |y| x + y
}

fn events_up_to(n: i32) -> impl Iterator<Item = i32> {
    (0..n).filter(|x| x % 2 == 0)
}

fn main() {
    let add5 = make_adder(5);
    println!("add5(10): {:?}", add5(10));

    let v: Vec<i32> = events_up_to(5).collect();
    println!("v: {:?}", v);
}