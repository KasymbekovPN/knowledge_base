
fn test0() {
    let mut v = vec![1, 2, 3, 4, 5];
    for x in &v {
        if *x == 3 {
            // ОШИБКА: cannot borrow `v` as mutable
            // v.push(100);
        }
    }

    println!("{:?}", v);
}

fn test1() {
    let mut v = vec![1, 2, 3, 4, 5];
    let mut to_add = Vec::new();
    for x in &v {
        if *x == 3 {
            to_add.push(1);
        }
    }
    v.extend(to_add);

    println!("{:?}", v);
}

fn test2() {
    let mut v = vec![1, 2, 3, 4, 5];
    // убрать все тройки — без ручной борьбы с итератором вообще
    v.retain(|&x| x != 3);

    println!("{:?}", v);
}

fn main() {
    test0();
    test1();
    test2();
}