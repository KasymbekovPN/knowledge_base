
fn print_len(v: &Vec<i32>) {
    println!("len = {}", v.len());
}

fn main() {
    let data = vec![1, 2, 3];
    print_len(&data);
    // data всё ещё доступна — только заимствовали
    println!("data = {:?}", data);
}