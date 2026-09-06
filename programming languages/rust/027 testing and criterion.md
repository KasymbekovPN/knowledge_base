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


---
---

```rust
// tests/integration_test.rs
use test_demo::add;

#[test]
fn test_public_api() {
    assert_eq!(add(10, 20), 30);
}
```

Каждый файл в `tests/` компилируется как **отдельный** крейт, который зависит от твоей библиотеки как внешний потребитель — видит только `pub`-API, ничего приватного. Прямая параллель раздельным CTest-таргетам, тестирующим публичный интерфейс библиотеки через её заголовки, а не внутренности `.cpp`-файлов.

### Doc-тесты — то, чего в C++/GoogleTest нет вообще

```rust
/// Складывает два числа.
///
/// # Examples
///
/// ```
/// assert_eq!(test_demo::add(2, 3), 5);
/// ```
pub fn add(a: i32, b: i32) -> i32 { a + b }
```

Прогнал полный `cargo test` — вот что реально происходит на всех трёх уровнях сразу:

```
     Running unittests src/lib.rs (target/debug/deps/test_demo-...)
running 5 tests
test tests::slow_test ... ignored
test tests::test_add ... ok
test tests::test_add_negative ... ok
test tests::test_add_zero ... ok
test tests::test_divide_by_zero_panics - should panic ... ok
test result: ok. 4 passed; 0 failed; 1 ignored; 0 measured; 0 filtered out

     Running tests/integration_test.rs (target/debug/deps/integration_test-...)
running 1 test
test test_public_api ... ok

   Doc-tests test_demo
running 1 test
test src/lib.rs - add (line 5) ... ok
```

Код внутри тройных обратных кавычек в `///`-комментарии **реально компилируется и выполняется** как тест при каждом `cargo test` — это гарантирует, что примеры в документации никогда не устареют относительно реального API (классическая проблема в C++/Doxygen — комментарии с примерами кода протухают молча, компилятор их не проверяет никак). Прямого аналога в GoogleTest/Catch2 нет — ближайшее по духу, пожалуй, Python `doctest`, но для C++ такой встроенной штуки не существует.

### Фильтрация и вывод — сравнение с GoogleTest

```bash
cargo test add          # фильтр по подстроке имени -- аналог --gtest_filter="*add*"
cargo test -- --ignored # запустить только ранее пропущенные
```

Ключевое отличие поведения по умолчанию: `cargo test` запускает тесты **параллельно** (в разных потоках) из коробки — GoogleTest по умолчанию **последовательный** (параллельность нужно явно настраивать через `gtest-parallel` или sharding). Отсюда практическое следствие: тесты в Rust должны быть по умолчанию написаны так, чтобы не иметь скрытого общего состояния (глобальные переменные, файлы, порты) — иначе параллельный запуск сломает их непредсказуемо; `#[test]` можно принудительно сериализовать через внешние крейты (`serial_test`), если общее состояние неизбежно.

## `criterion` — статистический бенчмаркинг

Встроенного (`std`) бенчмаркинга в стабильном Rust нет вообще (`#[bench]` существует только в nightly и считается устаревшим подходом) — `criterion` фактически стандарт индустрии, аналог Google Benchmark в мире C++.

```toml
[dev-dependencies]
criterion = "0.4"

[[bench]]
name = "add_bench"
harness = false
```

```rust
use criterion::{criterion_group, criterion_main, Criterion, black_box};
use test_demo::add;

fn bench_add(c: &mut Criterion) {
    c.bench_function("add 2+3", |b| {
        b.iter(|| add(black_box(2), black_box(3)))
    });
}

criterion_group!(benches, bench_add);
criterion_main!(benches);
```

Реально прогнал (сборка транзитивных зависимостей заняла время, локальный toolchain старый, но результат настоящий):

```
Benchmarking add 2+3: Warming up for 3.0000 s
Benchmarking add 2+3: Collecting 100 samples in estimated 5.0000 s (7.0B iterations)
Benchmarking add 2+3: Analyzing
add 2+3                 time:   [718.75 ps 722.42 ps 726.50 ps]
Found 7 outliers among 100 measurements (7.00%)
```

Ключевые вещи, которые отличают `criterion` от "измерить `std::chrono` вручную вокруг цикла" (частый ручной подход в C++ без Google Benchmark) и приближают к возможностям самого Google Benchmark:

- **`black_box`** — прямой аналог `benchmark::DoNotOptimize`/`ClobberMemory` в Google Benchmark: запрещает компилятору "схитрить" — заинлайнить константный аргумент и вычислить результат на этапе компиляции, что сделало бы бенчмарк бессмысленным (LLVM достаточно умный, чтобы просто вычислить `add(2, 3)` в константу `5` во время компиляции, если не помешать).
- **Автоматический warm-up** и статистический анализ (доверительные интервалы `[718.75 ps 722.42 ps 726.50 ps]` — нижняя/средняя/верхняя граница) — вместо единственного числа, что даёт понимание шума измерения.
- **Обнаружение выбросов** (`Found 7 outliers among 100 measurements`) — сигнализирует, если часть измерений искажена внешним шумом (планировщик ОС, другие процессы), не давая ложной уверенности в точности.
- **Regression detection** — при повторном запуске `criterion` сравнивает с предыдущим прогоном и явно сообщает "стало быстрее/медленнее на X%, статистически значимо/незначимо" — то, что в Google Benchmark обычно требует отдельной обвязки поверх сырых чисел.
- `cargo bench` генерирует HTML-отчёты с графиками (если включён `plotters`, что и подтянулось в зависимостях) — аналог того, что для Google Benchmark обычно делают через сторонние скрипты постобработки JSON-вывода.

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

```rust
fn add(a:i32,b:i32)->i32{a+b}
fn main(){
        let x=5;
    let    y = 10;
    if x>0{println!("{}",add(x,y));}
}
```

После `cargo fmt`:

```rust
fn add(a: i32, b: i32) -> i32 {
    a + b
}
fn main() {
    let x = 5;
    let y = 10;
    if x > 0 {
        println!("{}", add(x, y));
    }
}
```

Полностью нормализовано за один прогон — прямой аналог `clang-format -i`. Настройка через `rustfmt.toml` в корне проекта — проверил на практике:

```toml
max_width = 60
tab_spaces = 2
```

С этим конфигом отступ реально стал 2 пробела вместо стандартных 4:

```rust
fn add(a: i32, b: i32) -> i32 {
  a + b
}
```

**Важное культурное отличие от `clang-format`:** у `rustfmt` **гораздо меньше** конфигурируемых опций, чем у `clang-format` (там десятки параметров стиля — расстановка скобок, выравнивание, порядок модификаторов). Философия Rust-сообщества здесь осознанно жёстче: "один стандартный стиль для всей экосистемы" — большинство опций `rustfmt` либо unstable (требуют nightly), либо вообще не существуют, потому что цель — минимизировать споры о стиле между проектами, а не дать гибкость настройки под вкус каждой команды, как это принято в C++-мире с его исторически разрозненными style guide'ами (Google Style, LLVM Style, Mozilla Style и т.д., каждый со своим `.clang-format`).

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

Этим закрывается вся Фаза 4 из исходного плана (Cargo, тестирование, `clippy`/`rustfmt`) — остаётся управление зависимостями/crates.io (мы это частично затронули в теме про Cargo, но не разбирали публикацию своих крейтов отдельно) и Фаза 5 — `unsafe`/FFI-интероп с C++, которая прямо связана с твоей текущей работой. Куда двигаемся?

---
---

**Фаза 0 — быстрый рефреш (можно за один присест)** 
- [x] Ownership/borrowing/lifetimes ещё раз, но через призму "как это соотносится с RAII и move-семантикой в C++". Основная цель — не учить с нуля, а закрыть пробелы и зафиксировать терминологию (move, Copy, borrow checker, NLL). (2026.07.27)
- [x] точка входа в rust (2026.07.27)
- [x] свободные функции в rust (2026.07.27)
- [x] функции как аргументы (2026.07.27)
- [x] модификаторы, передача по ссылке, передача по значению (2026.07.28)
- [x] let в rust (2026.07.28)
- [x] match в rust (2026.07.28)
- [x] ветвления в rust (2026.07.28)
- [x] циклы в rust (2026.07.28)
- [x] разобрать конкретно "борьбу с borrow checker" на примерах — типичные ошибки (2026.07.28)
- [x] создание "класса", видимость, поля, конструкторы, деструкторы, методы, статические методы и поля (2026.07.28)
- [x] макросы name!(...) (2026.07.30)
- [x] macro_rules example (2026.07.30)

**Фаза 1 — типовая система и абстракции** 
- [x] Traits и generics vs шаблоны C++ и виртуальные функции; (2026.07.30)
- [x] trait objects (`dyn Trait`) vs vtable; (2026.07.30)
- [x] impl trait (2026.07.30)
- [x] enums как ADT и pattern matching (это то, чего в C++ нет вообще); (2026.07.30)
- [x] обработка ошибок — `Result`/`Option`, `?`, `thiserror`/`anyhow` вместо исключений. (2026.07.31)

**Фаза 2 — продвинутое владение памятью** 
- [x] `Box` в rust, (2026.07.31)
- [x] `Rc`/`Arc` в rust (2026.08.02)
- [x] `RefCell`/`Cell`, interior mutability в rust (2026.08.03)
- [x] продвинутые lifetimes (HRTB) в rust с примерами (2026.08.03)
- [x] продвинутые lifetimes (variance) в rust с примерами (2026.08.03)
- [x] сравнение с `unique_ptr`/`shared_ptr` — где Rust строже, а где придётся обходить borrow checker осознанно. (2026.08.03)

**Фаза 3 — конкурентность и async** 
- [x] Потоки, `Mutex`/`RwLock`, каналы (`mpsc`);  "fearless concurrency" и почему это гарантируется на уровне типов, в отличие от C++ memory model; (2026.08.04)
- [x] `async`/`await`, `tokio`, сравнение с корутинами C++20/Boost.Asio, с которыми ты уже плотно работал. (2026.08.04)

**Фаза 4 — инструментарий и экосистема** 
- [x] Cargo (workspaces, features, build scripts) как аналог CMake/vcpkg; (2026.08.06)
- [ ] тестирование и `criterion` для бенчмарков; `clippy`/`rustfmt`;
- [ ] управление зависимостями и crates.io.

**Фаза 5 — unsafe Rust и интероп с C++** 
- [ ] `unsafe`, raw pointers, `cxx`/`bindgen`/`cbindgen` — это прямо релевантно твоей текущей работе с монолитом на C++: как встраивать Rust-компоненты в существующую C++-кодовую базу и наоборот.

**Фаза 6 — практика** 
- [ ] Итоговый проект, завязанный на Фазы 3–5: например, сетевой сервис на `tokio` или Rust-модуль, подключённый к C++ через FFI, с Docker-сборкой в довесок.
