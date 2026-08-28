
// заберает владение над v
fn take_value(v: Vec<i32>) {
    println!("{:?}", v);
} // v уничтожается здесь

// передается копия
fn take_copy(mut x: i32) {
    x += 1;
    println!("{}", x);
}

fn main() {
    let n = 5;
    take_copy(n);
    println!("n is {}", n);

    let data = vec![1, 2, 3];
    take_value(data);
    // println!("data = {:?}", data); // Error
}