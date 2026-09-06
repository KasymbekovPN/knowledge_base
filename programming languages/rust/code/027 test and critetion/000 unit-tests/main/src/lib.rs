
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
    EmptyName,
    ZeroQuantity,
}

impl Cart {
    pub fn new() -> Self { Self::default() }

    pub fn add(&mut self, name: &str, price_cents: u32, qty: u32) -> Result<(), CartError> {
        if name.trim().is_empty() {
            return Err(CartError::EmptyName);
        }
        if qty == 0 {
            return Err(CartError::ZeroQuantity);
        }
        self.items.push(Item { name: name.to_string(), price_cents, qty });
        Ok(())
    }

    pub fn total_cents(&self) -> u32 {
        self.items.iter().map(line_total).sum()
    }

    pub fn item_count(&self) -> usize {
        self.items.len()
    }

    pub fn total_with_discount(&self, percent_off: u8) -> u32 {
        let total = self.total_cents();
        let discount = (total as u64 * percent_off.min(100) as u64) / 100;
        total - discount as u32
    }
}

// приватная функция -- не видна снаружи крейта, но видна тестам в этом же файле
fn line_total(item: &Item) -> u32 {
    item.price_cents * item.qty
}

#[cfg(test)]
mod tests {
    use super::*;

    // --- базовые проверки ---

    #[test]
    fn new_cart_is_empty() {
        let cart = Cart::new();
        assert_eq!(cart.item_count(), 0);
        assert_eq!(cart.total_cents(), 0);
    }

    #[test]
    fn adding_item_increases_total() {
        let mut cart = Cart::new();
        cart.add("book", 1500, 2).unwrap();
        assert_eq!(cart.item_count(), 1);
        assert_eq!(cart.total_cents(), 3000);
        assert_ne!(cart.total_cents(), 0);
    }

    // --- тест приватной функции напрямую ---

    #[test]
    fn line_total_multiplies_price_by_qty() {
        let item = Item { name: "pen".into(), price_cents: 100, qty: 3 };
        assert_eq!(line_total(&item), 300);
    }

    // --- should_panic ---

    #[test]
    #[should_panic(expected = "attempt to subtract with overflow")]
    fn discount_over_100_percent_would_panic_in_debug() {
        let total: u32 = std::hint::black_box(100);
        let discount: u32 = std::hint::black_box(150);
        // паника из-за overflow-checks в dev
        let _ = total - discount;
    }

    // --- тест, возвращающий Result<(), E>, с ? прямо в теле ---

    #[test]
    fn add_valid_item_returns_ok() -> Result<(), CartError> {
        let mut cart = Cart::new();
        cart.add("milk", 80, 1)?;
        assert_eq!(cart.item_count(), 1);
        Ok(())
    }

    #[test]
    fn add_rejects_empty_name() {
        let mut cart = Cart::new();
        assert_eq!(cart.add("", 100, 1), Err(CartError::EmptyName));
    }

    #[test]
    fn add_rejects_zero_quantity() {
        let mut cart = Cart::new();
        assert_eq!(cart.add("milk", 100, 0), Err(CartError::ZeroQuantity));
    }

    // --- общий helper для нескольких тестов ---

    fn cart_with_items() -> Cart {
        let mut cart = Cart::new();
        cart.add("book", 1000, 1).unwrap();
        cart.add("milk", 2000, 1).unwrap();
        cart
    }

    #[test]
    fn discount_reduces_total() {
        let cart = cart_with_items();
        assert_eq!(cart.total_with_discount(0), 3000);
        assert_eq!(cart.total_with_discount(50), 1500);
        assert_eq!(cart.total_with_discount(100), 0);
    }

    #[test]
    fn discount_clamps_above_100() {
        let cart = cart_with_items();
        assert_eq!(cart.total_with_discount(150), 0);
    }

    // --- табличный тест без сторонних крейтов ---

    #[test]
    fn discount_table() {
        let cases = [
            (0u8, 3000u32),
            (10, 2700),
            (25, 2250),
            (50, 1500),
            (100, 0),
        ];
        let cart = cart_with_items();
        for (percent, expected) in cases {
            assert_eq!(
                cart.total_with_discount(percent),
                expected,
                "mismatched for percent_off = {percent}"
            );
        }
    }

    // --- медленный тест, пропускается по умолчанию ---

    #[test]
    #[ignore = "too long test, run via cargo test -- --ignored"]
    fn large_cart_performance() {
        let mut cart = Cart::new();
        for i in 0..1_000_000 {
            cart.add(&format!("item{i}"), 100, 1).unwrap();
        }
        assert_eq!(cart.item_count(), 1_000_000);
    }
}
