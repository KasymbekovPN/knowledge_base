
fn get_count(s: &str) -> u32 {
    let Ok(count) = s.parse::<u32>() else {
        return 0;
    };
    count * 2
}

fn get_count_match(s: &str) -> u32 {
    match s.parse::<u32>() {
        Ok(count) => count * 2,
        Err(_) => return 0,
    }
}

fn main() {
    println!("get_count(\"hello\") => {}", get_count("hello"));
    println!("get_count(\"42\") => {}", get_count("42"));
    println!("get_count_match(\"hello\") => {}", get_count_match("hello"));
    println!("get_count_match(\"42\") => {}", get_count_match("42"));
}