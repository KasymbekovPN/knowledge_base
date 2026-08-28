
struct Point {x: i32, y: i32}
enum Shape {
    Circle { center: Point, radius: f64 },
}

fn match_it(shape: &Shape) {
    match shape {
        Shape::Circle { center: Point {x: 0, y: 0}, radius } => {
            println!("circle in zero coords, radius: {radius}");
        }
        Shape::Circle { center, radius } => {
            println!("circle in ({}, {}), radius: {}", center.x, center.y, radius);
        }
    }
}

fn main() {
    match_it(&Shape::Circle {center: Point{x: 0, y: 0}, radius: 5.0 });
    match_it(&Shape::Circle {center: Point{x: 10, y: 20}, radius: 7.0 });
}