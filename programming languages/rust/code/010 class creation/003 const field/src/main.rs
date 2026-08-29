
struct Circle {
    radius: f64,
}

impl Circle {
    // ассоциированная константа — аналог static constexpr
    const PI: f64 = 3.141592653589793;

    fn area(&self) -> f64 {
        Self::PI * self.radius * self.radius
    }
}

fn main() {
    println!("PI: {}", Circle::PI);

    let circle = Circle { radius: 1.0 };
    println!("area: {}", circle.area());
}