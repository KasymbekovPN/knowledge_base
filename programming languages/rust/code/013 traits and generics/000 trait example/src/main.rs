
trait Shape {
    fn area(&self) -> f64;
    fn name(&self) -> &str {
        // метод с реализацией по умолчанию
        "shape"
    }
}

struct Circle { radius: f64 }
struct Square { side: f64 }

impl Shape for Circle {
    fn area(&self) -> f64 {
        std::f64::consts::PI * self.radius * self.radius
    }
    fn name(&self) -> &str {
        "circle"
    }
}

impl Shape for Square {
    fn area(&self) -> f64 {
        self.side * self.side
    }
}

fn main() {
    let circle = Circle { radius: 3.0 };
    println!("Circle area: {}", circle.area());
    println!("Circle name: {}", circle.name());

    let square = Square { side: 2.0 };
    println!("Square area: {}", square.area());
    println!("Square name: {}", square.name());
}