
enum Shape {
    Circle(f64),
    Rectangle { wight: f64, height: f64 },
    Triangle(f64, f64, f64),
}

fn area(shape: &Shape) -> f64 {
    match shape {
        Shape::Circle(radius) => std::f64::consts::PI * radius * radius,
        Shape::Rectangle { wight, height } => wight * height,
        Shape::Triangle(a, b, c) => {
            let s = (a + b + c) / 2.0;
            (s * (s - a) * (s - b) * (s - c)).sqrt()
        }
    }
}

fn main() {
    println!("area(&Shape::Circle(10.0)): {}", area(&Shape::Circle(10.0)));
    println!("area(&Shape::Rectangle[wight: 3.0, height: 4.0]): {}", area(&Shape::Rectangle { wight: 3.0, height: 4.0 }));
    println!("area(&Shape::Triangle(3.0, 4.0, 5.0)): {}", area(&Shape::Triangle(3.0, 4.0, 5.0)));
}