
#[derive(Debug, Clone, PartialEq)]
pub struct Item {
    pub name: String,
    pub price_cents: u32,
    pub qty: u32,
}

#[derive(Debug, Default)]
pub struct Cart {
    items: Vec<Item>,
}

#[derive(Debug, PartialEq)]
pub enum CartError {
    EntryName,
    ZeroQuantity,
}

impl Cart {
    pub fn new() -> Self { Self::default() }

    pub fn add(&mut self, name: &str, price_cents: u32, qty: u32) -> Result<(), CartError> {
        if name.trim().is_empty() { return Err(CartError::ZeroQuantity); }
        if qty == 0 { return Err(CartError::ZeroQuantity); }
        self.items.push(Item { name: name.to_string(), price_cents, qty });
        Ok(())
    }

    pub fn total_cents(&self) -> u32 {
        self.items.iter().map(|i| i.qty * i.price_cents).sum()
    }

    pub fn item_count(&self) -> usize { self.items.len() }

    pub fn total_with_discount(&self, percent_off: u8) -> u32 {
        let total = self.total_cents();
        let discount = (total as u64 * percent_off.min(100) as u64) / 100;
        total - discount as u32
    }
}

// приватная -- интеграционные тесты её не увидят вообще
#[allow(dead_code)]
fn internal_helper() -> &'static str {
    "reached in inner crate only"
}
