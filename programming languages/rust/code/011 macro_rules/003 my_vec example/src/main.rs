
macro_rules! my_vec {
    ($($x:expr),* $(,)?) => {
        {
            let mut v = Vec::new();
            $(v.push($x);)*
            v
        }
    };
}

fn main() {
    let v1 = my_vec![1];
    println!("{:?}", v1);

    let v3 = my_vec![1, 2, 3];
    println!("{:?}", v3);
}