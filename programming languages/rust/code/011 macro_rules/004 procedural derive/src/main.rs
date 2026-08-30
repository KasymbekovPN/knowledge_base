
#[derive(Debug, Clone, PartialEq, Default)]
struct Point {
    x: f64,
    y: f64,
}

impl Point {
    fn to_string(&self) -> String {
        return format!("Point[x: {}, y: {}]", self.x, self.y);
    }
}


fn main() {
    let p0 = Point::default();
    println!("p0: {}", p0.to_string());

    let p1 = p0.clone();
    println!("p1: {}", p1.to_string());

    println!("eq: {}", p0.eq(&p1));
}