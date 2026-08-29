

macro_rules! make_function {
    ($fn_name:ident, $ret_type:ty, $value:expr) => {
        fn $fn_name() -> $ret_type {
            $value
        }
    };
}

make_function!(get_answer, i32, 42);

fn main() {
    println!("answer: {}", get_answer())
}