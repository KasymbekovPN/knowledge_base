
#[derive(Debug)]
enum ConfigError {
    Io(std::io::Error),
    Parse(std::num::ParseIntError),
}

impl From<std::io::Error> for ConfigError {
    fn from(err: std::io::Error) -> Self { ConfigError::Io(err) }
}

impl From<std::num::ParseIntError> for ConfigError {
    fn from(err: std::num::ParseIntError) -> Self { ConfigError::Parse(err) }
}

fn read_timeout(path: &str) -> Result<u32, ConfigError> {
    let content = std::fs::read_to_string(path)?;
    let timeout: u32 = content.trim().parse()?;
    Ok(timeout)
}

fn main() {
    println!("{:?}", read_timeout("non-exist.txt"));
    println!("{:?}", read_timeout("exist-abc.txt"));
    println!("{:?}", read_timeout("exist-num.txt"));
}