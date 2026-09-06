// use integration_test_demo::Cart;
//
// mod common;
//
// #[test]
// fn discount_reduces_total() {
//     let cart = common::cart_with_items();
//     assert_eq!(cart.total_with_discount(0), 3000);
//     assert_eq!(cart.total_with_discount(50), 1500);
//     assert_eq!(cart.total_with_discount(100), 0);
// }
//
// #[test]
// fn discount_clamps_above_100() {
//     let cart = common::cart_with_items();
//     assert_eq!(cart.total_with_discount(150), 0);
// }
//
// #[test]
// fn empty_cart_discount_is_always_zero() {
//     let cart: Cart = common::empty_cart();
//     assert_eq!(cart.total_with_discount(50), 0);
// }