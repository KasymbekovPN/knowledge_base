use thiserror::Error;

#[derive(Debug, Clone, PartialEq)]
pub struct Product {
    pub name: String,
    pub price_cents: u64,
}

#[derive(Error, Debug)]
pub enum DomainError {
    #[error("price should be more then zero")]
    InvalidPrice,
}

impl Product {
    pub fn new(name: &str, price_cents: u64) -> Result<Self, DomainError> {
        if price_cents == 0 {
            return Err(DomainError::InvalidPrice);
        }
        Ok(Product { name: name.to_string(), price_cents })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn reject_zero_price() {
        assert!(Product::new("", 0).is_err());
    }

    #[test]
    fn accepts_valid_price() {
        assert!(Product::new("", 42).is_ok());
    }
}

