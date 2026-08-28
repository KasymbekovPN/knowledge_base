
fn test0() {
    let mut v = vec![1, 2, 3];
    let first = &v[0];
    // ОШИБКА: cannot borrow `v` as mutable because it is also borrowed as immutable
    // v.push(4);
    println!("{first}");
}

fn test1() {
    let mut v = vec![1, 2, 3];
    {
        let first = &v[0];
        println!("{first}");
    }
    v.push(4);
}

fn main() {
    test0();
    test1();
}
