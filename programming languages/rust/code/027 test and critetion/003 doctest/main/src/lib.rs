//! # doctest
//!
//! Little lib for doc-tests demonstration
//!
//! Даже этот блок верхнеуровневой документации (`//!`) может содержать
//! исполняемый пример -- он тоже будет скомпилирован и прогнан `cargo test`:
//!
//! ```
//! assert_eq!(doctest::add(2, 2), 4);
//! ```

use std::fmt;

/// Two number sum
///
/// # Examples
///
/// ```
/// assert_eq!(doctest::add(2, 3), 5)
/// ```
pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

#[derive(Debug, PartialEq)]
pub struct ParseTempError;

impl fmt::Display for ParseTempError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Could not parse temp value")
    }
}

/// Parse a string like "36.6C" into celsius grades
///
/// # Examples
///
/// ```
/// # fn main() -> Result<(), doctest::ParseTempError>
/// {
///     let c = doctest::parse_celsius("36.6C")?;
///     assert_eq!(c, 36.6);
/// # Ok(())
/// # }
/// ```
pub fn parse_celsius(s: &str) -> Result<f64, ParseTempError> {
    s.strip_suffix('C')
        .and_then(|num| num.parse::<f64>().ok())
        .ok_or(ParseTempError)
}

/// Divide a & b
///
/// # Panics
///
/// Panic if b is zero.
///
/// ```should_panic
/// doctest::divide(42, 0);
/// ```
///
/// Successfully
///
/// ```
/// assert_eq!(doctest::divide(10, 2), 5);
/// ```
pub fn divide(a: i32, b: i32) -> i32 {
    if b == 0 {
        panic!("Divide by zero");
    }
    a / b
}

/// `np_run` - code compiles but does not execute
///
/// ```no_run
/// let response = doctest::fetch_from_network("https://example.com");
/// println!("{response}")
/// ```
pub fn fetch_from_network(url: &str) -> String {
    format!("answer from {url}")
}

/// 'compile_fail' -- test is passed if code won't compiled
///
/// ```compile_fail
/// let s = String::from("hello");
/// let s2 = s;
/// println!("{}", s)
/// ```
pub fn move_example() {}

/// 'ignore' -- cargo test does not touch this code
///
/// ```ignore
/// let x = doctest::placeholder();
/// ```
pub fn placeholder() {}
