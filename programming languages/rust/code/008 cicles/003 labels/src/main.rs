fn main() {

    'outer: for i in 0..5 {
        for j in 0..5 {
            if i * j > 6 { break 'outer; }
            println!("{} {}", i, j);
        }
    }
}