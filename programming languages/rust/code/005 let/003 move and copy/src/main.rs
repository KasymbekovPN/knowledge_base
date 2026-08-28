fn main() {
    let x0 = 5;
    let x1 = x0; // let & copy
    println!("x0 = {}", x0);
    println!("x1 = {}", x1);

    let s0 = String::from("hello");
    let s1 = s0;
    println!("s1 = {}", s1);
    // println!("s0 = {}", s0); // value borrowed here after move
}