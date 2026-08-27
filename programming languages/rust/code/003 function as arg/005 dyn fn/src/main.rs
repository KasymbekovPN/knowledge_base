
fn double_fn(x: i32) -> i32 { x * 2 }

fn main() {
    let callbacks: Vec<Box<dyn Fn(i32) -> i32>> = vec![
        Box::new(|x| x + 1),
        Box::new(|x| x * 2),
        Box::new(double_fn),
    ];

    for cb in &callbacks {
        println!("{}", cb(7));
    }
}