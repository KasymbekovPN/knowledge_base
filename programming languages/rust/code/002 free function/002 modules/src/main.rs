use crate::geometry::area;

mod geometry {

    // pub — видна снаружи модуля
    pub fn area(w: i64, h: i64) -> i64 {
        return mul(w, h);
    }

    // приватная по умолчанию — аналог static-функции в .cpp файле
    fn mul(a: i64, b: i64) -> i64 { a * b }
}

fn main() {
    println!("area: {}", geometry::area(11, 10));
}
