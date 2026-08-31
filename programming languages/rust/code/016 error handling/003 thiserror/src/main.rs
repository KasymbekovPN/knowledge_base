use thiserror::Error;

#[derive(Error, Debug)]
enum ConfigError {
    #[error("could not read file: {0}")]
    Io(#[from] std::io::Error),

    #[error("could not parse timeout: {0}")]
    Parse(#[from] std::num::ParseIntError),

    #[error("out of range {value} (max = {max})")]
    OutOfRange { value: u32, max: u32 },
}

fn read_timeout(path: &str) -> Result<u32, ConfigError> {
    let content = std::fs::read_to_string(path)?;
    let timeout: u32 = content.trim().parse()?;
    if timeout > 3600 {
        return Err(ConfigError::OutOfRange { value: timeout, max: 3600 });
    }
    Ok(timeout)
}

fn main() {
    println!("{:?}", read_timeout("non-exist.txt"));
    println!("{:?}", read_timeout("exist-abc.txt"));
    println!("{:?}", read_timeout("exist-num-too-big.txt"));
    println!("{:?}", read_timeout("exist-num.txt"));
}