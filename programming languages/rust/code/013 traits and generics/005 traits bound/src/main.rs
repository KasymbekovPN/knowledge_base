use std::fmt::{Display, Formatter};

#[derive(Clone, Default)]
struct Point { x: i32, y: i32 }

impl Display for Point {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

fn describe<T>(item: T) -> String
where T: Display + Clone {
    format!("{} (clone: {})", item, item.clone())
}

fn main() {
    let p = Point::default();
    println!("{}", describe(&p));
}
