fn slow_sum(n: u64) -> u64 {
    let mut sum: u64 = 0;
    for i in 0..n {
        sum = sum.wrapping_add(i);
    }
    sum
}

fn main() {
    let start = std::time::Instant::now();
    let result = slow_sum(200_000_000);
    let elapsed = start.elapsed();
    println!("result {result:?}, duration: {elapsed:?}");

    // black_box -- не даём компилятору вычислить это на этапе компиляции
    let x: u8 = std::hint::black_box(250);
    let y: u8 = std::hint::black_box(10);
    // переполнение u8
    let sum = x + y;
    println!("sum: {}, x: {}, y: {}", sum, x, y);

    debug_assert!(1 + 1 == 3, "this check should be executed for dev-profile only");
    println!("debug_assert! does not work -> it is release-profile")
}
