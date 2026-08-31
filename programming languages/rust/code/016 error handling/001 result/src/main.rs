
fn parse_and_double(s: &str) -> Result<i32, std::num::ParseIntError> {
    // если Err — сразу return Err(...) из функции целиком
    let n: i32 = s.parse()?;
    Ok(n * 2)
}

fn main() {
    println!("{:?}", parse_and_double("21"));
    println!("{:?}", parse_and_double("hello"));
}