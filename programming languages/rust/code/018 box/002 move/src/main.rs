fn main() {
    let b0 = Box::new(42);
    println!("b0: {:?}", b0);

    let b1 = b0;
    println!("b1: {:?}", b1);
    // println!("b0: {:?}", b0);
}