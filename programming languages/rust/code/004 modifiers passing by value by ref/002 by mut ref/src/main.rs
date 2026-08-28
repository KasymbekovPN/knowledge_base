
fn add_element(v: &mut Vec<i32>, x: i32) {
    v.push(x);
}

fn main() {
    let mut data = vec![1, 2, 3];
    add_element(&mut data, 4);
    println!("data = {:?}", data);
}