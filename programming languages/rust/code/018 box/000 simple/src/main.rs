
struct Huge{ data: [u8; 1_000_000] }

fn main() {
    let b = Box::new(42);
    println!("{}", b);

    println!("size_of::<Box<i32>>(): {}", size_of::<Box<i32>>());
    println!("size_of::<Box<usize>>(): {}", size_of::<Box<usize>>());
    println!("size_of::<Box<Huge>>(): {}", size_of::<Box<Huge>>());
}