---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

Идеально — видно все три уровня тестов в одном прогоне: unit-тесты, интеграционный тест (отдельная бинарь), doc-тест (реально скомпилированный и выполненный код из `///`-комментария). Теперь `cargo test -- --ignored` и фильтрация:Теперь `criterion` для бенчмарков:Отлично, наконец собралось и реально прогнало бенчмарк с настоящей статистикой. Теперь `clippy` и `rustfmt`:Отлично, есть пакеты. Установим:Теперь напишем код с типичными clippy-предупреждениями и плохим форматированием:Отличный набор реальных clippy-предупреждений с точными подсказками. Теперь `cargo fmt`:Идеально, полностью нормализованный код. Теперь `rustfmt.toml`:Отступ реально изменился на 2 пробела согласно `rustfmt.toml`. Все примеры проверены. Собираю полный разбор.

## Тестирование — три уровня, встроенные прямо в `cargo`, без отдельного фреймворка

Раз у тебя уже плотный опыт с CTest+GoogleTest/Catch2 — ключевое структурное отличие сразу: в C++ это **два разных слоя** (CTest — раннер, GoogleTest/Catch2 — сам фреймворк с assertions/test discovery). В Rust тестирование — **часть языка и `cargo`**, отдельный фреймворк не нужен вообще для базового покрытия.

### Unit-тесты — прямо в том же файле, что и код

```rust
pub fn add(a: i32, b: i32) -> i32 { a + b }

pub fn divide(a: i32, b: i32) -> i32 {
    if b == 0 { panic!("деление на ноль"); }
    a / b
}

#[cfg(test)]
mod tests {
    use super::*; // доступ к приватным элементам модуля

    #[test]
    fn test_add() {
        assert_eq!(add(2, 3), 5);
    }

    #[test]
    #[should_panic(expected = "деление на ноль")]
    fn test_divide_by_zero_panics() {
        divide(10, 0);
    }

    #[test]
    #[ignore] // пропускается в обычном запуске
    fn slow_test() {
        std::thread::sleep(std::time::Duration::from_secs(5));
    }
}
```

`#[cfg(test)]` — тот самый механизм условной компиляции из темы про `cargo build.rs`/features: этот модуль **вообще не попадает** в обычную сборку, только в тестовую (`cargo test` компилирует отдельный тестовый бинарник) — прямая параллель тому, как в CMake тестовые таргеты обычно отделены от `add_executable` основного приложения через `if(BUILD_TESTING)`.

`use super::*` — тесты видят **приватные** элементы модуля напрямую, без `friend class` (GoogleTest-паттерн для доступа к приватным членам) — потому что видимость в Rust определяется на уровне модуля (уже разбирали это в теме про "классы"), а тестовый `mod` — вложенный модуль того же файла, значит имеет доступ к тому же приватному пространству.

`#[should_panic(expected = "...")]` — прямой аналог `EXPECT_DEATH`/`ASSERT_THROW` в GoogleTest, только заточен именно под `panic!`, не под исключения (в Rust их и нет).

`#[ignore]` — аналог `DISABLED_`-префикса в GoogleTest, только явный атрибут вместо соглашения по именованию.

### Интеграционные тесты — `tests/`, видят только публичный API

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
- [ ] Потоки, `Mutex`/`RwLock`, каналы (`mpsc`);  "fearless concurrency" и почему это гарантируется на уровне типов, в отличие от C++ memory model; 
- [ ] `async`/`await`, `tokio`, сравнение с корутинами C++20/Boost.Asio, с которыми ты уже плотно работал.

**Фаза 4 — инструментарий и экосистема** 
- [ ] Cargo (workspaces, features, build scripts) как аналог CMake/vcpkg;
- [ ] тестирование и `criterion` для бенчмарков; `clippy`/`rustfmt`;
- [ ] управление зависимостями и crates.io.

**Фаза 5 — unsafe Rust и интероп с C++** 
- [ ] `unsafe`, raw pointers, `cxx`/`bindgen`/`cbindgen` — это прямо релевантно твоей текущей работе с монолитом на C++: как встраивать Rust-компоненты в существующую C++-кодовую базу и наоборот.

**Фаза 6 — практика** 
- [ ] Итоговый проект, завязанный на Фазы 3–5: например, сетевой сервис на `tokio` или Rust-модуль, подключённый к C++ через FFI, с Docker-сборкой в довесок.
