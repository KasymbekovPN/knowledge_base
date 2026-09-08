fn get_first(v: &Vec<i32>) -> i32 {
    return v[0];
}

fn check(x: Option<i32>) -> bool {
    if x == None { true } else { false }
}

fn main() {
    let v = vec![1, 2, 3];
    for i in 0..v.len() {
        println!("{}", v[i]);
    }
}
