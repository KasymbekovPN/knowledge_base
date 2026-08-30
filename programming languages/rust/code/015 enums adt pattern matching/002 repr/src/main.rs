

#[repr(u16)]
#[derive(Debug)]
enum HttpStatus {
    Ok = 200,
    NotFound = 404,
    ServerError = 500,
}

fn main() {
    println!("{}", HttpStatus::Ok as u16);
}