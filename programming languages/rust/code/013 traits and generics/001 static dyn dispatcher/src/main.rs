
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

fn print_area_static<T: Shape>(s: &T) {
    println!("[STATIC] {}: {}", s.name(), s.area());
}

fn print_area_dyn(s: &dyn Shape) {
    println!("[DYN] {}: {}", s.name(), s.area());
}

fn main() {
    let circle = Circle { radius: 1.0 };
    let square = Square { side: 2.0 };
    print_area_static(&circle);
    print_area_static(&square);

    let shapes: Vec<Box<dyn Shape>> = vec![
        Box::new(Circle { radius: 3.0 }),
        Box::new(Square { side: 4.0 }),
    ];

    for s in shapes {
        print_area_dyn(s.as_ref());
    }
}
