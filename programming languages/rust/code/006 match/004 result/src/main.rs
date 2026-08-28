

fn parse_num(s: &str) -> Result<i32, String> {
    s.parse().map_err(|_| format!("Not number: {s}"))
}

fn match_it(s: &str) -> String {
    match parse_num(s) {
        Ok(n) => format!("Number: {}", n),
        Err(s) => s.to_string(),
    }
}

fn main() {
    println!("match_it(\"10\"): {}", match_it("10"));
    println!("match_it(\"abc\"): {}", match_it("abc"));
}