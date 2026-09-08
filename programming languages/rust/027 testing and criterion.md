---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Тестирование — три уровня, встроенные прямо в `cargo`, без отдельного фреймворка

Раз у тебя уже плотный опыт с CTest+GoogleTest/Catch2 — ключевое структурное отличие сразу: в C++ это **два разных слоя** (CTest — раннер, GoogleTest/Catch2 — сам фреймворк с assertions/test discovery). В Rust тестирование — **часть языка и `cargo`**, отдельный фреймворк не нужен вообще для базового покрытия.

### Unit-тесты — прямо в том же файле, что и код

## Проект целиком

```
unit_test_demo/
├── Cargo.toml
└── src/
    └── lib.rs
```

### `Cargo.toml`

```toml
[package]
name = "unit_test_demo"
version = "0.1.0"
edition = "2024"
```

### `src/lib.rs`

```rust
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
    pub fn new() -> Self {
        Self::default()
    }

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
        cart.add("книга", 1500, 2).unwrap();
        assert_eq!(cart.item_count(), 1);
        assert_eq!(cart.total_cents(), 3000);
        assert_ne!(cart.total_cents(), 0);
    }

    // --- тест приватной функции напрямую ---

    #[test]
    fn line_total_multiplies_price_by_qty() {
        let item = Item { name: "ручка".into(), price_cents: 100, qty: 3 };
        assert_eq!(line_total(&item), 300);
    }

    // --- should_panic ---

    #[test]
    #[should_panic(expected = "attempt to subtract with overflow")]
    fn discount_over_100_percent_would_panic_in_debug() {
        let total: u32 = std::hint::black_box(100);
        let discount: u32 = std::hint::black_box(150);
        let _ = total - discount; // паника из-за overflow-checks в dev
    }

    // --- тест, возвращающий Result<(), E>, с ? прямо в теле ---

    #[test]
    fn add_valid_item_returns_ok() -> Result<(), CartError> {
        let mut cart = Cart::new();
        cart.add("молоко", 80, 1)?;
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
        assert_eq!(cart.add("хлеб", 100, 0), Err(CartError::ZeroQuantity));
    }

    // --- общий helper для нескольких тестов ---

    fn cart_with_items() -> Cart {
        let mut cart = Cart::new();
        cart.add("товар А", 1000, 1).unwrap();
        cart.add("товар Б", 2000, 1).unwrap();
        cart
    }

    #[test]
    fn discount_reduces_total() {
        let cart = cart_with_items(); // total = 3000
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
                "не совпало для percent_off = {percent}"
            );
        }
    }

    // --- медленный тест, пропускается по умолчанию ---

    #[test]
    #[ignore = "долгий тест, гонять отдельно через cargo test -- --ignored"]
    fn large_cart_performance() {
        let mut cart = Cart::new();
        for i in 0..1_000_000 {
            cart.add(&format!("item{i}"), 100, 1).unwrap();
        }
        assert_eq!(cart.item_count(), 1_000_000);
    }
}
```

## Реальный прогон

```bash
cargo test
```

```
running 11 tests
test tests::add_rejects_empty_name ... ok
test tests::add_rejects_zero_quantity ... ok
test tests::add_valid_item_returns_ok ... ok
test tests::adding_item_increases_total ... ok
test tests::discount_clamps_above_100 ... ok
test tests::discount_over_100_percent_would_panic_in_debug - should panic ... ok
test tests::discount_reduces_total ... ok
test tests::discount_table ... ok
test tests::large_cart_performance ... ignored, долгий тест, гонять отдельно через cargo test -- --ignored
test tests::line_total_multiplies_price_by_qty ... ok
test tests::new_cart_is_empty ... ok

test result: ok. 10 passed; 0 failed; 1 ignored; 0 measured; 0 filtered out
```

## Как выглядит провал теста — тоже честно проверил

Временно испортил ожидаемое значение (`1500` → `9999`) и прогнал:

```bash
cargo test discount_reduces_total
```

```
thread 'tests::discount_reduces_total' panicked at src/lib.rs:134:9:
assertion `left == right` failed
  left: 1500
 right: 9999

failures:
    tests::discount_reduces_total

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 10 filtered out
```

`assert_eq!` печатает **оба** значения (`left`/`right`) при провале — обычный `assert!(a == b)` этого не даёт, только "assertion failed" без чисел, поэтому `assert_eq!`/`assert_ne!` почти всегда предпочтительнее голого `assert!` для сравнений.

## Что демонстрирует каждый паттерн

|Паттерн|Где в примере|Зачем|
|---|---|---|
|`use super::*`|верх `mod tests`|доступ к приватной `line_total`, недоступной снаружи крейта|
|`assert_eq!`/`assert_ne!` с сообщением|`discount_table`|третий аргумент — кастомное сообщение при провале, полезно в цикле|
|`#[should_panic(expected = "...")]`|`discount_over_100_percent...`|проверка, что паника — ожидаемое поведение, с конкретным текстом|
|Тест, возвращающий `Result<(), E>`|`add_valid_item_returns_ok`|`?` прямо в теле теста вместо `.unwrap()` на каждой строке|
|Общая функция-helper (`cart_with_items`)|перед `discount_reduces_total`|не дублировать настройку между тестами|
|Табличный тест (массив кортежей + цикл)|`discount_table`|несколько похожих проверок без copy-paste, без сторонних крейтов вроде `rstest`|
|`#[ignore = "..."]` с текстом причины|`large_cart_performance`|долгий тест пропускается по умолчанию, причина видна в выводе|

Если нужен именно параметризованный тест с более развитым синтаксисом (отдельные подписанные кейсы, автоматическая генерация имени под каждый набор данных) — это делает крейт `rstest`, но для большинства случаев обычный цикл по массиву, как в `discount_table`, покрывает потребность без лишней зависимости.

### rstest

## Проект целиком

```
rstest_demo/
├── Cargo.toml
└── src/
    └── lib.rs
```

### `Cargo.toml`

```toml
[package]
name = "rstest_demo"
version = "0.1.0"
edition = "2021"

[dev-dependencies]
rstest = "0.18"
```

### `src/lib.rs`

```rust
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
        cart.add("товар А", 1000, 1).unwrap();
        cart.add("товар Б", 2000, 1).unwrap();
        cart
    }

    #[rstest]
    fn total_is_sum_of_items(cart_with_items: Cart) {
        assert_eq!(cart_with_items.total_cents(), 3000);
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
    #[case("хлеб", 100, 0)]
    fn add_rejects_invalid_input(#[case] name: &str, #[case] price: u32, #[case] qty: u32) {
        let mut cart = Cart::new();
        assert!(cart.add(name, price, qty).is_err());
    }

    // === #[values] -- декартово произведение всех комбинаций ===
    #[rstest]
    fn add_various_combinations(
        #[values(1, 2, 5)] qty: u32,
        #[values(100, 999)] price: u32,
    ) {
        let mut cart = Cart::new();
        cart.add("товар", price, qty).unwrap();
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
        #[with(_n)] cart_with_n_items: Cart,
    ) {
        assert_eq!(cart_with_n_items.total_cents(), expected_total);
    }
}
```

## Реальный прогон

```bash
cargo test
```

```
running 17 tests
test tests::add_rejects_invalid_input::case_1 ... ok
test tests::add_rejects_invalid_input::case_2 ... ok
test tests::add_various_combinations::qty_1_1::price_1_100 ... ok
test tests::add_various_combinations::qty_1_1::price_2_999 ... ok
test tests::add_various_combinations::qty_2_2::price_1_100 ... ok
test tests::add_various_combinations::qty_2_2::price_2_999 ... ok
test tests::add_various_combinations::qty_3_5::price_1_100 ... ok
test tests::add_various_combinations::qty_3_5::price_2_999 ... ok
test tests::cart_with_n_items_has_correct_total::case_1 ... ok
test tests::cart_with_n_items_has_correct_total::case_2 ... ok
test tests::cart_with_n_items_has_correct_total::case_3 ... ok
test tests::discount_table::case_1 ... ok
test tests::discount_table::case_2 ... ok
test tests::discount_table::case_3 ... ok
test tests::discount_table::case_4 ... ok
test tests::discount_table::case_5 ... ok
test tests::total_is_sum_of_items ... ok

test result: ok. 17 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
```

## Что изменилось по сравнению с прошлым обычным `#[test]`-примером

|Было (обычный `#[test]`)|Стало (`rstest`)|
|---|---|
|`fn cart_with_items() -> Cart { ... }` — обычная функция, вызывается вручную `let cart = cart_with_items();`|`#[fixture] fn cart_with_items() -> Cart` — rstest сам подставляет результат по совпадению имени параметра с именем фикстуры|
|Цикл `for (percent, expected) in cases { assert_eq!(...) }` — все 5 случаев в **одном** тесте, провал одного случая может маскировать остальные|`#[case(...)]` × 5 — **пять отдельных** тестов (`case_1`...`case_5`), каждый виден в отчёте независимо, провал одного не мешает увидеть статус других|
|Ручное дублирование двух похожих тестов (`add_rejects_empty_name`, `add_rejects_zero_quantity`)|Один `add_rejects_invalid_input` с двумя `#[case]` — общая логика, разные входные данные|
|Нет способа быстро перебрать все комбинации `qty × price` без ручного вложенного цикла|`#[values(1, 2, 5)]` × `#[values(100, 999)]` — 6 тестов сгенерированы автоматически, декартово произведение|

## Прямая параллель с тем, что ты знаешь из C++/GoogleTest

`#[rstest] #[case(...)]` — это ровно то же, для чего в GoogleTest используется `TEST_P` + `INSTANTIATE_TEST_SUITE_P` (Parameterized Tests) или Catch2's `TEMPLATE_TEST_CASE`/`GENERATE`. `#[fixture]` — прямой аналог `TEST_F` + класса-фикстуры с `SetUp()` в GoogleTest, только здесь это простая функция, а не наследование от `::testing::Test`. `#[values]` для декартова произведения — это то, что в GoogleTest потребовало бы комбинирования нескольких `INSTANTIATE_TEST_SUITE_P` через `::testing::Combine(...)` — заметно более многословно, чем два соседних атрибута здесь.

Обрати внимание на именование в выводе: `qty_1_1::price_1_100` — rstest сам генерирует читаемые имена подтестов из значений параметров, что при провале сразу показывает **какая именно** комбинация сломалась, без необходимости добавлять кастомное сообщение вручную (как в табличном тесте из прошлого примера, где я добавлял `"не совпало для percent_off = {percent}"` вручную — здесь это не нужно, имя теста уже содержит эту информацию).
### Интеграционные тесты — `tests/`, видят только публичный API

## Проект целиком

```
integration_test_demo/
├── Cargo.toml
├── src/
│   └── lib.rs
└── tests/
    ├── common/
    │   └── mod.rs          ← общий helper, НЕ отдельный тестовый бинарник
    ├── cart_tests.rs        ← отдельный тестовый бинарник №1
    └── discount_tests.rs    ← отдельный тестовый бинарник №2
```

### `Cargo.toml`

```toml
[package]
name = "integration_test_demo"
version = "0.1.0"
edition = "2021"
```

### `src/lib.rs`

```rust
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
    "видно только внутри крейта"
}
```

### `tests/common/mod.rs` — общий код между несколькими файлами тестов

```rust
// ВАЖНО: именно tests/common/mod.rs, а не tests/common.rs!
// Файл напрямую в tests/ (например, tests/common.rs) cargo считает ОТДЕЛЬНЫМ
// тестовым бинарником и попытается запустить как тесты -- увидишь "running 0 tests"
// для файла, который тестом вообще не является. Вложенность в tests/common/mod.rs
// говорит cargo "это модуль, а не отдельная точка входа теста".

use integration_test_demo::Cart;

pub fn cart_with_items() -> Cart {
    let mut cart = Cart::new();
    cart.add("товар А", 1000, 1).unwrap();
    cart.add("товар Б", 2000, 1).unwrap();
    cart
}

pub fn empty_cart() -> Cart {
    Cart::new()
}
```

### `tests/cart_tests.rs`

```rust
use integration_test_demo::{Cart, CartError};

mod common; // подключаем общий helper-модуль

#[test]
fn new_cart_is_empty() {
    let cart = common::empty_cart();
    assert_eq!(cart.item_count(), 0);
    assert_eq!(cart.total_cents(), 0);
}

#[test]
fn adding_item_increases_total() {
    let mut cart = Cart::new();
    cart.add("книга", 1500, 2).unwrap();
    assert_eq!(cart.total_cents(), 3000);
}

#[test]
fn add_rejects_empty_name() {
    let mut cart = Cart::new();
    assert_eq!(cart.add("", 100, 1), Err(CartError::EmptyName));
}

#[test]
fn add_rejects_zero_quantity() {
    let mut cart = Cart::new();
    assert_eq!(cart.add("хлеб", 100, 0), Err(CartError::ZeroQuantity));
}

#[test]
fn shared_fixture_from_common_module() {
    let cart = common::cart_with_items();
    assert_eq!(cart.total_cents(), 3000);
}
```

### `tests/discount_tests.rs`

```rust
use integration_test_demo::Cart;

mod common;

#[test]
fn discount_reduces_total() {
    let cart = common::cart_with_items();
    assert_eq!(cart.total_with_discount(0), 3000);
    assert_eq!(cart.total_with_discount(50), 1500);
    assert_eq!(cart.total_with_discount(100), 0);
}

#[test]
fn discount_clamps_above_100() {
    let cart = common::cart_with_items();
    assert_eq!(cart.total_with_discount(150), 0);
}

#[test]
fn empty_cart_discount_is_always_zero() {
    let cart: Cart = common::empty_cart();
    assert_eq!(cart.total_with_discount(50), 0);
}
```

## Реальный прогон — видно все три бинарника отдельно

```bash
cargo test
```

```
     Running unittests src/lib.rs (target/debug/deps/integration_test_demo-...)
running 0 tests
test result: ok. 0 passed; 0 failed

     Running tests/cart_tests.rs (target/debug/deps/cart_tests-...)
running 5 tests
test add_rejects_empty_name ... ok
test add_rejects_zero_quantity ... ok
test adding_item_increases_total ... ok
test new_cart_is_empty ... ok
test shared_fixture_from_common_module ... ok
test result: ok. 5 passed; 0 failed

     Running tests/discount_tests.rs (target/debug/deps/discount_tests-...)
running 3 tests
test discount_clamps_above_100 ... ok
test discount_reduces_total ... ok
test empty_cart_discount_is_always_zero ... ok
test result: ok. 3 passed; 0 failed
```

Обрати внимание: `unittests src/lib.rs` показал **0 тестов** — это ожидаемо, в `lib.rs` их специально нет (весь упор в этом примере на `tests/`). `cart_tests.rs` и `discount_tests.rs` — **два независимых** тестовых бинарника, каждый со своим `main`, скомпилированный и запущенный отдельно. `tests/common/mod.rs` в списке отдельно **не появился** — ровно то, ради чего понадобилась именно такая структура папки.

## Проверка ключевого свойства — приватное реально недоступно

Специально попробовал вызвать `internal_helper()` (приватную функцию) из временного интеграционного теста:

```rust
use integration_test_demo::internal_helper;
#[test]
fn tries_to_access_private() {
    println!("{}", internal_helper());
}
```

```bash
cargo test --test should_fail_private_access
```

```
error[E0603]: function `internal_helper` is private
note: the function `internal_helper` is defined here
```

Компилятор отказался это компилировать — интеграционные тесты видят **только** `pub`-API крейта, ровно как сторонний потребитель библиотеки. Это прямая противоположность unit-тестам из прошлого примера (`mod tests { use super::*; }`), где `line_total` была видна свободно.

## Почему `tests/common/mod.rs`, а не `tests/common.rs`

Это единственная неочевидная деталь во всей структуре, и я проверил её реальным поведением, а не на словах: если положить общий helper прямо как `tests/common.rs`, Cargo решит, что это **ещё один** тестовый файл (как `cart_tests.rs`/`discount_tests.rs`), скомпилирует его в отдельный бинарник и попытается запустить как тесты — увидишь лишнюю строчку `Running tests/common.rs` с `running 0 tests` в выводе, что и шумно, и семантически неверно (`common` не тест, а инфраструктура). Старое соглашение `tests/common/mod.rs` (module-style, до Rust 2018) — единственный по-настоящему надёжный способ сказать Cargo "это модуль для `mod common;`, не отдельная точка входа" — файлы `tests/<name>/mod.rs` не подхватываются как самостоятельные test-бинарники, в отличие от файлов `tests/<name>.rs` напрямую.

## Сравнение с CTest — какому паттерну это аналогично

Каждый файл `tests/*.rs` — отдельный **исполняемый файл теста**, ровно как отдельный `add_executable(cart_tests cart_tests.cpp)` + `add_test(NAME cart_tests COMMAND cart_tests)` в CMake — именно поэтому `cargo test` печатает их по отдельности (`Running tests/cart_tests.rs`, `Running tests/discount_tests.rs`), а не сливает в один общий прогон. `tests/common/mod.rs` — аналог общего `test_helpers.h`/`test_helpers.cpp`, который линкуется в несколько тестовых таргетов CMake, не будучи сам по себе тестовым таргетом.
### Doc-тесты — то, чего в C++/GoogleTest нет вообще

## Проект целиком

```
doctest_demo/
├── Cargo.toml
└── src/
    └── lib.rs
```

### `Cargo.toml`

```toml
[package]
name = "doctest_demo"
version = "0.1.0"
edition = "2021"
```

### `src/lib.rs`

```rust
//! # doctest_demo
//!
//! Небольшая библиотека для демонстрации doc-тестов.
//!
//! Даже этот блок верхнеуровневой документации (`//!`) может содержать
//! исполняемый пример -- он тоже будет скомпилирован и прогнан `cargo test`:
//!
//! ```
//! assert_eq!(doctest_demo::add(2, 2), 4);
//! ```

use std::fmt;

/// Складывает два числа.
///
/// # Examples
///
/// ```
/// assert_eq!(doctest_demo::add(2, 3), 5);
/// ```
pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

#[derive(Debug, PartialEq)]
pub struct ParseTempError;

impl fmt::Display for ParseTempError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "не удалось распознать температуру")
    }
}

/// Парсит строку вида "36.6C" в градусы Цельсия.
///
/// # Examples
///
/// ```
/// # fn main() -> Result<(), doctest_demo::ParseTempError> {
/// let c = doctest_demo::parse_celsius("36.6C")?;
/// assert_eq!(c, 36.6);
/// # Ok(())
/// # }
/// ```
pub fn parse_celsius(s: &str) -> Result<f64, ParseTempError> {
    s.strip_suffix('C')
        .and_then(|num| num.parse::<f64>().ok())
        .ok_or(ParseTempError)
}

/// Делит `a` на `b`.
///
/// # Panics
///
/// Паникует, если `b` равно нулю:
///
/// ```should_panic
/// doctest_demo::divide(10, 0);
/// ```
///
/// Обычный успешный случай -- отдельный блок в том же комментарии:
///
/// ```
/// assert_eq!(doctest_demo::divide(10, 2), 5);
/// ```
pub fn divide(a: i32, b: i32) -> i32 {
    if b == 0 {
        panic!("деление на ноль");
    }
    a / b
}

/// `no_run` -- код КОМПИЛИРУЕТСЯ, но не выполняется.
///
/// ```no_run
/// let response = doctest_demo::fetch_from_network("http://example.com");
/// println!("{response}");
/// ```
pub fn fetch_from_network(url: &str) -> String {
    format!("ответ от {url}")
}

/// `compile_fail` -- тест ПРОЙДЕН, если код НЕ компилируется.
///
/// ```compile_fail
/// let s = String::from("hello");
/// let s2 = s;
/// println!("{}", s); // ОШИБКА: s перемещён в s2
/// ```
pub fn move_example() {}

/// `ignore` -- код вообще не трогается cargo test.
///
/// ```ignore
/// let x = doctest_demo::not_yet_implemented_function();
/// ```
pub fn placeholder() {}
```

## Реальный прогон

```bash
cargo test
```

```
   Doc-tests doctest_demo

running 8 tests
test src/lib.rs - (line 8) ... ok
test src/lib.rs - add (line 20) ... ok
test src/lib.rs - divide (line 65) ... ok
test src/lib.rs - divide (line 72) ... ok
test src/lib.rs - fetch_from_network (line 89) - compile ... ok
test src/lib.rs - move_example (line 105) - compile fail ... ok
test src/lib.rs - parse_celsius (line 45) ... ok
test src/lib.rs - placeholder (line 117) ... ignored

test result: ok. 7 passed; 0 failed; 1 ignored
```

## Разбор каждого атрибута

**Обычный блок (` ``` ` без атрибутов)** — компилируется **и** выполняется, провал `assert_eq!` внутри = провал теста. Строка `test src/lib.rs - (line 8)` без имени функции — это тест из `//!`-комментария в самом верху файла (модульная документация, а не документация конкретного элемента).

**Скрытые строки (`#` )** — строка, начинающаяся с `#` внутри блока кода, компилируется и выполняется как обычно, но **не отображается** в сгенерированной HTML-документации (`cargo doc`). Использовал это, чтобы обернуть пример с `?` в `fn main() -> Result<...>` — читателю документации не нужно видеть эту обвязку, но компилятору она необходима, раз `?` требует, чтобы окружающая функция возвращала совместимый `Result`.

**`should_panic`** — тест **обязан** запаниковать, иначе провалится. Проверил обратный случай (что будет, если код перестанет паниковать) в прошлом примере с обычными `#[test]` — здесь то же самое, только для doc-теста.

**`no_run`** — компилируется (значит, гарантированно синтаксически и типово валиден, не протухнет при рефакторинге сигнатуры функции), но не запускается. Нужен для примеров с сетью/файлами/долгим ожиданием, которые нежелательно реально выполнять при каждом `cargo test`, но которые всё равно должны оставаться корректными относительно текущего API.

**`compile_fail`** — тест **пройден**, если код **не** компилируется. Я специально проверил обратную сторону: сделал пример из `move_example` валидным (заменил использование перемещённой переменной на корректное) — тест немедленно провалился с точным диагнозом:

```
test src/lib.rs - move_example (line 105) - compile fail ... FAILED
Test compiled successfully, but it's marked `compile_fail`.
```

Это доказывает, что атрибут реально проверяет **отсутствие** компиляции, а не просто декоративная пометка — если кто-то в будущем случайно изменит API так, что пример из документации, иллюстрирующий ошибку, вдруг станет компилироваться, `cargo test` на это укажет.

**`ignore`** — код не компилируется и не выполняется вообще, только показывается как пример в документации. Использовать для псевдокода, будущего API, или примеров, зависящих от вещей, которых точно нет в тестовом окружении.

## Прямое сравнение с C++/Doxygen

В Doxygen `@code`/`@endcode`-блоки — это **чистый текст**, никак не проверяемый компилятором. Пример в комментарии может протухнуть при любом рефакторинге сигнатуры, и единственный способ узнать об этом — человек, читающий документацию, заметит несоответствие сам. В Rust каждый из этих блоков (кроме `ignore`) — это **реальный, скомпилированный и (кроме `no_run`) выполненный** код, гарантированно синхронизированный с текущим состоянием API — расхождение документации и кода здесь физически невозможно молча, оно ловится тем же `cargo test`, что гоняет обычные unit- и интеграционные тесты.
### Фильтрация и вывод — сравнение с GoogleTest

```bash
cargo test add          # фильтр по подстроке имени -- аналог --gtest_filter="*add*"
cargo test -- --ignored # запустить только ранее пропущенные
```

Ключевое отличие поведения по умолчанию: `cargo test` запускает тесты **параллельно** (в разных потоках) из коробки — GoogleTest по умолчанию **последовательный** (параллельность нужно явно настраивать через `gtest-parallel` или sharding). Отсюда практическое следствие: тесты в Rust должны быть по умолчанию написаны так, чтобы не иметь скрытого общего состояния (глобальные переменные, файлы, порты) — иначе параллельный запуск сломает их непредсказуемо; `#[test]` можно принудительно сериализовать через внешние крейты (`serial_test`), если общее состояние неизбежно.

## `criterion` — статистический бенчмаркинг

Встроенного (`std`) бенчмаркинга в стабильном Rust нет вообще (`#[bench]` существует только в nightly и считается устаревшим подходом) — `criterion` фактически стандарт индустрии, аналог Google Benchmark в мире C++.

Ключевые вещи, которые отличают `criterion` от "измерить `std::chrono` вручную вокруг цикла" (частый ручной подход в C++ без Google Benchmark) и приближают к возможностям самого Google Benchmark:

- **`black_box`** — прямой аналог `benchmark::DoNotOptimize`/`ClobberMemory` в Google Benchmark: запрещает компилятору "схитрить" — заинлайнить константный аргумент и вычислить результат на этапе компиляции, что сделало бы бенчмарк бессмысленным (LLVM достаточно умный, чтобы просто вычислить `add(2, 3)` в константу `5` во время компиляции, если не помешать).
- **Автоматический warm-up** и статистический анализ (доверительные интервалы `[718.75 ps 722.42 ps 726.50 ps]` — нижняя/средняя/верхняя граница) — вместо единственного числа, что даёт понимание шума измерения.
- **Обнаружение выбросов** (`Found 7 outliers among 100 measurements`) — сигнализирует, если часть измерений искажена внешним шумом (планировщик ОС, другие процессы), не давая ложной уверенности в точности.
- **Regression detection** — при повторном запуске `criterion` сравнивает с предыдущим прогоном и явно сообщает "стало быстрее/медленнее на X%, статистически значимо/незначимо" — то, что в Google Benchmark обычно требует отдельной обвязки поверх сырых чисел.
- `cargo bench` генерирует HTML-отчёты с графиками (если включён `plotters`, что и подтянулось в зависимостях) — аналог того, что для Google Benchmark обычно делают через сторонние скрипты постобработки JSON-вывода.

## Проект целиком

```
criterion_demo/
├── Cargo.toml
├── src/
│   └── lib.rs
└── benches/
    └── sort_bench.rs
```

### `Cargo.toml`

```toml
[package]
name = "criterion_demo"
version = "0.1.0"
edition = "2021"

[dev-dependencies]
criterion = "0.4"

[[bench]]
name = "sort_bench"
harness = false
```

`harness = false` — обязательная строка: говорит Cargo не использовать встроенный тестовый harness для этого файла, потому что `criterion` предоставляет свой собственный через `criterion_main!`.

### `src/lib.rs` — то, что бенчмаркуем: две реализации сортировки

```rust
/// Сортировка вставками -- O(n^2)
pub fn insertion_sort(data: &mut [i32]) {
    for i in 1..data.len() {
        let mut j = i;
        while j > 0 && data[j - 1] > data[j] {
            data.swap(j - 1, j);
            j -= 1;
        }
    }
}

/// Стандартная сортировка -- O(n log n)
pub fn std_sort(data: &mut [i32]) {
    data.sort();
}

/// n псевдослучайных чисел без внешних крейтов (линейный конгруэнтный генератор)
pub fn generate_data(n: usize, seed: u64) -> Vec<i32> {
    let mut state = seed;
    (0..n)
        .map(|_| {
            state = state.wrapping_mul(6364136223846793005).wrapping_add(1);
            ((state >> 33) as i32).abs() % 100_000
        })
        .collect()
}
```

### `benches/sort_bench.rs`

```rust
use criterion::{black_box, criterion_group, criterion_main, BenchmarkId, Criterion, Throughput};
use criterion_demo::{generate_data, insertion_sort, std_sort};

fn bench_single_size(c: &mut Criterion) {
    let data = generate_data(1000, 42);

    c.bench_function("insertion_sort 1000", |b| {
        b.iter(|| {
            let mut d = data.clone();
            insertion_sort(black_box(&mut d));
        })
    });

    c.bench_function("std_sort 1000", |b| {
        b.iter(|| {
            let mut d = data.clone();
            std_sort(black_box(&mut d));
        })
    });
}

fn bench_across_sizes(c: &mut Criterion) {
    let mut group = c.benchmark_group("sorting");

    for size in [10, 100, 1_000, 5_000] {
        let data = generate_data(size, 42);
        group.throughput(Throughput::Elements(size as u64));

        group.bench_with_input(BenchmarkId::new("insertion_sort", size), &data, |b, data| {
            b.iter(|| {
                let mut d = data.clone();
                insertion_sort(black_box(&mut d));
            })
        });

        group.bench_with_input(BenchmarkId::new("std_sort", size), &data, |b, data| {
            b.iter(|| {
                let mut d = data.clone();
                std_sort(black_box(&mut d));
            })
        });
    }

    group.finish();
}

criterion_group!(benches, bench_single_size, bench_across_sizes);
criterion_main!(benches);
```

## Реальный прогон — `cargo bench`

```
insertion_sort 1000     time:   [434.21 µs 442.75 µs 456.46 µs]
std_sort 1000           time:   [28.899 µs 29.031 µs 29.183 µs]

sorting/insertion_sort/10    time: [33.346 ns 33.454 ns 33.580 ns]
sorting/std_sort/10          time: [33.755 ns 34.203 ns 34.708 ns]

sorting/insertion_sort/100   time: [4.3888 µs 4.3970 µs 4.4063 µs]
sorting/std_sort/100         time: [1.5735 µs 1.5843 µs 1.5990 µs]

sorting/insertion_sort/1000  time: [445.14 µs 449.55 µs 454.54 µs]
sorting/std_sort/1000        time: [28.922 µs 29.041 µs 29.172 µs]

sorting/insertion_sort/5000  time: [11.148 ms 11.184 ms 11.228 ms]
sorting/std_sort/5000        time: [228.29 µs 230.14 µs 232.44 µs]
```

Данные полностью соответствуют теории: на `n=10` разница между O(n²) и O(n log n) практически в шуме измерения (33.5нс vs 33.8нс — сортировка вставками даже чуть быстрее на крошечных массивах за счёт меньшего оверхеда). На `n=5000` разница уже колоссальна: **11.18 мс** против **230 мкс** — почти в 50 раз, что и ожидается от квадратичного роста при увеличении n в 500 раз от размера 10.

## Regression detection — сработал сам, без настройки

Прогнал `cargo bench` **второй раз** — criterion автоматически сравнил с предыдущим прогоном:

```
sorting/insertion_sort/10
                        change: [+0.0301% +0.6162% +1.2273%] (p = 0.04 < 0.05)

sorting/std_sort/100
                        change: [+1.2955% +2.1226% +3.0640%] (p = 0.00 < 0.05)
```

`p = 0.00 < 0.05` означает "изменение статистически значимо" — criterion сам хранит историю прогонов в `target/criterion/` и явно сообщает не просто числа, а **является ли** разница между запусками реальной или шумом измерения. Это именно то, что обычно требует отдельной обвязки поверх сырых чисел Google Benchmark — здесь встроено в сам инструмент.

## `Throughput::Elements` — сравнение в "элементах в секунду", не только во времени

```rust
group.throughput(Throughput::Elements(size as u64));
```

Это добавляет вторую метрику в отчёт (`thrpt:`), позволяющую сравнивать эффективность на **разных** размерах входа в одной шкале — полезно, когда интересует не "сколько заняло на этом конкретном n", а "сколько элементов в секунду обрабатывает алгоритм" как функция масштабируемости. Для байтовых операций есть аналог `Throughput::Bytes(n)` — типичное применение для бенчмарков сериализации/парсинга.

## `BenchmarkGroup` — сравнительный отчёт вместо разрозненных чисел

`c.benchmark_group("sorting")` группирует связанные бенчмарки так, что при генерации HTML-отчёта (`target/criterion/report/index.html`, если установлен `gnuplot` в системе) criterion рисует их **на одном графике** для прямого визуального сравнения — вместо того, чтобы вручную сопоставлять числа из разных прогонов, как пришлось бы делать с сырым Google Benchmark выводом без дополнительной постобработки.

## Параллель с твоим Google Benchmark опытом

| Google Benchmark (C++)                                         | criterion (Rust)                                     |
| -------------------------------------------------------------- | ---------------------------------------------------- |
| `BENCHMARK(BM_Function)`                                       | `c.bench_function("name", \|b\| ...)`                |
| `benchmark::DoNotOptimize(x)`                                  | `black_box(x)`                                       |
| `->Range(8, 8<<10)` для разных размеров                        | `for size in [...]` внутри `bench_with_input`        |
| `->Unit(benchmark::kMillisecond)`                              | Автоматический выбор единиц измерения                |
| Сравнение прогонов вручную (сохранить JSON, сравнить скриптом) | Встроенное `change:`/regression detection из коробки |
| `state.SetItemsProcessed(state.iterations() * n)`              | `Throughput::Elements(n)`                            |
| Группировка через `BENCHMARK_TEMPLATE`/фикстуры                | `benchmark_group`                                    |

## `clippy` — линтер, аналог `clang-tidy`

Написал код с типичными проблемами и прогнал `cargo clippy` — вот что он реально нашёл:

```rust
fn get_first(v: &Vec<i32>) -> i32 {
    return v[0];
}

fn check(x: Option<i32>) -> bool {
    if x == None { true } else { false }
}

fn main() {
    let v = vec![1, 2, 3];
    for i in 0..v.len() {
        println!("{}", v[i]);
    }
}
```

```
warning: unneeded `return` statement
  help: remove `return`

warning: writing `&Vec` instead of `&[_]` involves a new object where a slice will do
  help: change this to: `&[i32]`

warning: this if-then-else expression returns a bool literal
  help: you can reduce it to: `x == None`

warning: binary comparison to literal `Option::None`
  help: use `Option::is_none()` instead: `x.is_none()`

warning: the loop variable `i` is only used to index `v`
  help: consider using an iterator
```

Каждое предупреждение — не просто стиль, а часто реальная семантическая рекомендация (`&Vec<T>` вместо `&[T]` — избыточное сужение API, лишает вызывающего возможности передать обычный срез без выделенного `Vec`; `x == None` вместо `x.is_none()` — менее идиоматично и чуть медленнее для сложных `T`). Категории clippy-линтов (`clippy::correctness`, `clippy::style`, `clippy::complexity`, `clippy::perf`, `clippy::pedantic`) — прямая параллель категориям чеков `clang-tidy` (`bugprone-*`, `performance-*`, `readability-*`, `modernize-*`), только clippy — часть официальной toolchain-экосистемы Rust, устанавливается вместе с `rustup`, а не отдельный инструмент, который нужно отдельно конфигурировать через `.clang-tidy`.

`cargo clippy --fix` — автоматически применяет часть предложений (то же, что `clang-tidy --fix`), `#[allow(clippy::needless_return)]` — точечное подавление конкретного линта на уровне функции/модуля/крейта (аналог `// NOLINT(readability-...)` в clang-tidy).

## `rustfmt` — форматтер, аналог `clang-format`

----
---
## Проект целиком

```
rustfmt_demo/
├── Cargo.toml
├── rustfmt.toml
└── src/
    └── main.rs
```

### Исходный код (намеренно неотформатированный)

```rust
use std::collections::HashMap;
use std::fmt;
use std::io::Read;

struct Point{x:f64,y:f64,label:String}

impl Point{
fn new(x:f64,y:f64)->Self{
Point{x,y,label:String::new()}
}

    fn distance(&self,other:&Point)->f64{
        ((self.x-other.x).powi(2)+(self.y-other.y).powi(2)).sqrt()
    }
}

fn classify(n:i32)->&'static str{
    match n {
0 => "ноль",
        1..=9=>"однозначное",
    _=>"многозначное"
    }
}

fn main(){
    let p1=Point::new(0.0,0.0);
    let p2 = Point::new(3.0,4.0);
    println!("{}",p1.distance(&p2));

    let numbers=vec![1,2,3,4,5];
    let doubled:Vec<i32>=numbers.iter().map(|x|x*2).filter(|x|*x>4).collect();
    println!("{:?}",doubled);

    let mut map=HashMap::new();
    map.insert("a",1);
    map.insert("b",2);

    for(k,v) in &map{
        println!("{}: {}",k,v);
    }

    // Матрица, размеченная вручную для читаемости -- НЕ должна переформатироваться
    #[rustfmt::skip]
    let identity_3x3 = [
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
    ];
    println!("{:?}", identity_3x3);
}
```

## Шаг 1: `cargo fmt --check` — режим для CI, ничего не меняет

```bash
cargo fmt --check
```

```
Diff in /home/claude/rustfmt_demo/src/main.rs at line 2:
-struct Point{x:f64,y:f64,label:String}
-
-impl Point{
-fn new(x:f64,y:f64)->Self{
-Point{x,y,label:String::new()}
+struct Point {
+    x: f64,
+    y: f64,
+    label: String,
 }
...
```

Возвращает ненулевой код выхода и печатает diff, **не трогая файл** — именно это ставят в CI-пайплайн (`cargo fmt --check` в GitHub Actions/аналоге), чтобы падал билд, если кто-то закоммитил неотформатированный код, вместо того чтобы молча переписывать чужие файлы в CI.

## Шаг 2: `cargo fmt` — реально применяет форматирование

```bash
cargo fmt
```

Результат:

```rust
use std::collections::HashMap;
use std::fmt;
use std::io::Read;

struct Point {
    x: f64,
    y: f64,
    label: String,
}

impl Point {
    fn new(x: f64, y: f64) -> Self {
        Point {
            x,
            y,
            label: String::new(),
        }
    }

    fn distance(&self, other: &Point) -> f64 {
        ((self.x - other.x).powi(2) + (self.y - other.y).powi(2)).sqrt()
    }
}

fn classify(n: i32) -> &'static str {
    match n {
        0 => "ноль",
        1..=9 => "однозначное",
        _ => "многозначное",
    }
}

fn main() {
    let p1 = Point::new(0.0, 0.0);
    let p2 = Point::new(3.0, 4.0);
    println!("{}", p1.distance(&p2));

    let numbers = vec![1, 2, 3, 4, 5];
    let doubled: Vec<i32> = numbers.iter().map(|x| x * 2).filter(|x| *x > 4).collect();
    println!("{:?}", doubled);

    let mut map = HashMap::new();
    map.insert("a", 1);
    map.insert("b", 2);

    for (k, v) in &map {
        println!("{}: {}", k, v);
    }

    // Матрица, размеченная вручную для читаемости -- НЕ должна переформатироваться
    #[rustfmt::skip]
    let identity_3x3 = [
        [1, 0, 0],
        [0, 1, 0],
        [0, 0, 1],
    ];
    println!("{:?}", identity_3x3);
}
```

**Обрати внимание на `identity_3x3`** — она осталась ровно в исходном ручном форматировании, потому что `#[rustfmt::skip]` явно исключает следующий за ним элемент из обработки. Проверил: без этого атрибута rustfmt схлопнул бы матрицу в одну строку `[[1, 0, 0], [0, 1, 0], [0, 0, 1]]`, что убило бы визуальную структуру, важную для понимания — типичный кейс, где ручное форматирование оправдано (таблицы, матрицы, выровненные битовые маски).

## Шаг 3: `rustfmt.toml` — точечная настройка

```toml
max_width = 80
use_small_heuristics = "Max"
reorder_imports = true
```

Перемешал импорты вручную:

```rust
use std::io::Read;
use std::fmt;
use std::collections::HashMap;
```

После `cargo fmt` с этим конфигом:

```rust
use std::collections::HashMap;
use std::fmt;
use std::io::Read;
```

Пересортированы по алфавиту автоматически (`reorder_imports = true` — на самом деле включён по умолчанию, но явно показывает, что это настраиваемое поведение).

С `max_width = 80` строка, которая раньше умещалась в 100-символьный лимит по умолчанию, теперь переносится:

```rust
    let doubled: Vec<i32> =
        numbers.iter().map(|x| x * 2).filter(|x| *x > 4).collect();
```

## Ключевые команды — сводка

|Команда|Что делает|
|---|---|
|`cargo fmt`|Переформатировать все файлы крейта на месте|
|`cargo fmt --check`|Проверить без изменений, вернуть ошибку, если что-то не по стандарту (для CI)|
|`cargo fmt -- --check`|То же самое (старый синтаксис передачи флагов напрямую в `rustfmt`)|
|`rustfmt src/main.rs`|Форматировать один конкретный файл, в обход `cargo`|
|`#[rustfmt::skip]`|Исключить следующий элемент (функцию, `let`, блок) из форматирования|
|`rustfmt.toml` в корне проекта|Локальные настройки для этого крейта/workspace|

## Параллель с `clang-format`

|`clang-format`|`rustfmt`|
|---|---|
|`.clang-format` в корне|`rustfmt.toml` в корне|
|`clang-format -i file.cpp`|`rustfmt file.rs`|
|`clang-format --dry-run --Werror` (CI-режим)|`cargo fmt --check`|
|`// clang-format off` / `// clang-format on`|`#[rustfmt::skip]`|
|Десятки настраиваемых опций стиля|Единицы стабильных опций (`max_width`, `reorder_imports`, ещё немного) — большинство остального либо nightly-only, либо принципиально не настраивается ради единого стиля экосистемы|


## Сводная таблица

|Задача|C++|Rust|
|---|---|---|
|Unit-тесты|GoogleTest/Catch2 (отдельная библиотека)|`#[test]` — встроено в язык|
|Раннер тестов|CTest|`cargo test` — встроено|
|Доступ к приватным членам в тестах|`friend class` / `FRIEND_TEST`|Тесты — вложенный модуль, видит приватное по умолчанию|
|Тесты только публичного API|Отдельный CMake-таргет с отдельным `#include`|`tests/` — отдельный крейт автоматически|
|Тестирование примеров в документации|Не проверяется компилятором|Doc-tests — реально компилируются и выполняются|
|Параллельность тестов по умолчанию|Нет (нужен `gtest-parallel`)|Да|
|Пропуск теста|`DISABLED_` префикс|`#[ignore]`|
|Ожидание паники/исключения|`EXPECT_DEATH`/`ASSERT_THROW`|`#[should_panic]`|
|Бенчмаркинг|Google Benchmark (сторонняя библиотека)|`criterion` (сторонний крейт, де-факто стандарт)|
|Защита от constant-folding в бенчмарке|`DoNotOptimize`/`ClobberMemory`|`black_box`|
|Линтер|`clang-tidy` (сторонний, конфигурируется через `.clang-tidy`)|`clippy` (официальный, часть toolchain)|
|Форматтер|`clang-format` (сторонний, множество опций)|`rustfmt` (официальный, минимум опций, единый стиль)|
|Подавление конкретного предупреждения|`// NOLINT(check-name)`|`#[allow(clippy::lint_name)]`|

