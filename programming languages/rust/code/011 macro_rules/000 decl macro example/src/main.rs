

macro_rules! greet {
    () => {
        println!("Hello!");
    };
    ($name:expr) => {
        println!("Hello, {}!", $name);
    };
}

fn main() {
    greet!();
    greet!("Pablo");
}