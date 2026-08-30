
trait Describe {
    fn describe(&self) -> String;
}

impl <T: std::fmt::Debug> Describe for T {
    fn describe(&self) -> String {
        format!("{:?}", self)
    }
}

fn main() {
    println!("{}", 42.describe());
    println!("{}", vec![1, 2, 3].describe());
}