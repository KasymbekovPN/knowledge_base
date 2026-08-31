
fn get_result(i: i32) -> Result<u32, String> {
    if i < 0 { Err(format!("{} is negative", i)) } else { Ok(i as u32) }
}

fn main() {
    let _r = get_result(1).expect("oops 1");
    let _r = get_result(-1).expect("oops 2");
}