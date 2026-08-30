
macro_rules! max_of {
    ($first:expr) => { $first };
    ($first:expr, $($rest:expr),+) => {
        {
            let rest_max = max_of!($($rest),+);
            if $first > rest_max { $first } else { rest_max }
        }
    };
}

fn main() {
    let m = max_of!(3, 7, 1, 8, 4);
    println!("max: {}", m);
}