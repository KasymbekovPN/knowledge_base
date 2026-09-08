use std::collections::HashMap;
use std::fmt;
use std::io::Read;

struct Point {
    x: f64,
    y: f64,
    label: String,
}

impl Point {
    fn new(x: f64, y: f64) -> Self {
        Point { x, y, label: String::new() }
    }

    fn distance(&self, other: &Point) -> f64 {
        ((self.x - other.x).powi(2) + (self.y - other.y).powi(2)).sqrt()
    }
}

fn classify(n: i32) -> &'static str {
    match n {
        0 => "ноль",
        1..=9 => "однозначное",
        _ => "многозначное",
    }
}

fn main() {
    let p1 = Point::new(0.0, 0.0);
    let p2 = Point::new(3.0, 4.0);
    println!("{}", p1.distance(&p2));

    let numbers = vec![1, 2, 3, 4, 5];
    let doubled: Vec<i32> =
        numbers.iter().map(|x| x * 2).filter(|x| *x > 4).collect();
    println!("{:?}", doubled);

    let mut map = HashMap::new();
    map.insert("a", 1);
    map.insert("b", 2);

    for (k, v) in &map {
        println!("{}: {}", k, v);
    }

    // Матрица, размеченная вручную для читаемости -- НЕ должна переформатироваться
    #[rustfmt::skip]
    let identity_3x3 = [
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
    ];
    println!("{:?}", identity_3x3);
}
