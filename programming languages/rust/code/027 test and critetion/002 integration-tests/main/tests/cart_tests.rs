use integration_test_demo::{Cart, CartError};

// подключаем общий helper-модуль
mod common;

#[test]
fn new_cart_is_empty() {
    let cart = common::empty_cat();
    assert_eq!(cart.item_count(), 0);
    assert_eq!(cart.total_cents(), 0);
}

#[test]
fn adding_item_increasing_total() {
    let mut cart = Cart::new();
    cart.add("book", 1500, 2).unwrap();
    assert_eq!(cart.total_cents(), 3000);
}

#[test]
fn add_rejects_empty_name() {
    let mut cart = common::empty_cat();
    assert_eq!(cart.add("", 1500, 2), Err(CartError::EmptyName));
}

#[test]
fn reject_zero_quantity() {
    let mut cart = Cart::new();
    assert_eq!(cart.add("bread", 1500, 0), Err(CartError::ZeroQuantity));
}

#[test]
fn shared_fixture_from_common_module() {
    let cart = common::cart_with_items();
    assert_eq!(cart.total_cents(), 3000);
}

