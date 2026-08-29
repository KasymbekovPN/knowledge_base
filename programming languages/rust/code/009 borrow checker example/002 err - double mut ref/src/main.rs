
fn test0() {
    let mut v = vec![1, 2, 3];
    let a = &mut v;
    // ОШИБКА: cannot borrow `v` as mutable more than once at a time
    // let b = &mut v;
    a.push(4);
    // b.push(5);

    println!("{:?}", v);
}

fn test1() {
    let mut v = vec![1, 2, 3];

    {
        let a = &mut v;
        a.push(4);
    } // a умер

    {
        let b = &mut v;
        b.push(5);
    }

    println!("{:?}", v);
}

fn test2() {
    let mut v = vec![1, 2, 3, 4];
    // два независимых &mut [i32] на разные half
    let (left, right) = v.split_at_mut(2);

    left[0] = 100;
    right[0] = 200;

    println!("{:?}", v);
}

fn main() {
    test0();
    test1();
    test2();
}