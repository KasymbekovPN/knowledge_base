// ВАЖНО: именно tests/common/mod.rs, а не tests/common.rs!
// Файл напрямую в tests/ (например, tests/common.rs) cargo считает ОТДЕЛЬНЫМ
// тестовым бинарником и попытается запустить как тесты -- увидишь "running 0 tests"
// для файла, который тестом вообще не является. Вложенность в tests/common/mod.rs
// говорит cargo "это модуль, а не отдельная точка входа теста".

use integration_test_demo::Cart;

pub fn cart_with_items() -> Cart {
    let mut cart = Cart::new();
    cart.add("good a", 1000, 1).unwrap();
    cart.add("good b", 2000, 1).unwrap();

    cart
}

pub fn empty_cat() -> Cart { Cart::new() }
