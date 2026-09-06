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
        if name.trim().is_empty() { return Err(CartError::EmptyName); }
        if qty == 0 { return Err(CartError::ZeroQuantity); }
        self.items.push(Item { name: name.to_string(), price_cents, qty });
        Ok(())
    }

    pub fn total_cents(&self) -> u32 {
        self.items.iter().map(|i| i.price_cents * i.qty).sum()
    }

    pub fn total_with_discount(&self, percent_off: u8) -> u32 {
        let total = self.total_cents();
        let discount = (total as u64 * percent_off.min(100) as u64) / 100;
        total - discount as u32
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use rstest::*;

    // === #[fixture] -- замена ручной helper-функции из обычного #[test] ===
    #[fixture]
    fn cart_with_items() -> Cart {
        let mut cart = Cart::new();
        cart.add("good a", 1000, 1).unwrap();
        cart.add("good b", 2000, 1).unwrap();
        cart
    }

    #[rstest]
    fn total_is_sum_of_items(cart_with_items: Cart) {
        assert_eq!(cart_with_items.total_cents(), 3000)
    }

    // === #[case] -- параметризованный тест: каждый case = отдельный именованный тест ===
    #[rstest]
    #[case(0, 3000)]
    #[case(10, 2700)]
    #[case(25, 2250)]
    #[case(50, 1500)]
    #[case(100, 0)]
    fn discount_table(cart_with_items: Cart, #[case] percent: u8, #[case] expected: u32) {
        assert_eq!(cart_with_items.total_with_discount(percent), expected);
    }

    // === фикстура + кейсы вместе ===
    #[rstest]
    #[case("", 100, 1)]
    #[case("bread", 100, 0)]
    fn add_reject_invalid_input(#[case] name: &str, #[case] price: u32, #[case] qty: u32) {
        let mut cart = Cart::new();
        assert!(cart.add(name, price, qty).is_err());
    }

    // === #[values] -- декартово произведение всех комбинаций ===
    #[rstest]
    fn add_various_combinations(
        #[values(1, 2, 5)] qty: u32,
        #[values(100, 999)] price: u32
    ) {
        let mut cart = Cart::new();
        cart.add("good", price, qty).unwrap();
        assert_eq!(cart.total_cents(), price * qty);
    }

    // === параметризованная фикстура ===
    #[fixture]
    fn cart_with_n_items(#[default(1)] n: u32) -> Cart {
        let mut cart = Cart::new();
        for i in 0..n {
            cart.add(&format!("item{i}"), 100, 1).unwrap();
        }
        cart
    }

    #[rstest]
    #[case(1, 100)]
    #[case(3, 300)]
    #[case(10, 1000)]
    fn cart_with_n_items_has_correct_total(
        #[case] _n: u32,
        #[case] expected_total: u32,
        #[with(_n)] cart_with_n_items: Cart
    ) {
        assert_eq!(cart_with_n_items.total_cents(), expected_total);
    }
}
