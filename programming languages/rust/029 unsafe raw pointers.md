---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Структура

```
interop_project/
├── 01_unsafe_basics/            ← raw pointers, unsafe fn, без C/C++
├── 02_cbindgen_rust_to_cpp/     ← Rust-библиотека → C++ вызывает её
├── 03_bindgen_cpp_to_rust/       ← "легаси" C-код → Rust вызывает его
└── 04_cxx_bidirectional/          ← настоящий C++ класс ↔ Rust, в обе стороны
```

---

## 1. `unsafe` и raw pointers — фундамент

```rust
fn main() {
    // Создание raw pointer -- БЕЗОПАСНО
    let x = 42;
    let r1: *const i32 = &x; // аналог const T* в C++
    let mut y = 10;
    let r2: *mut i32 = &mut y; // аналог T*

    // Разыменование -- уже unsafe
    unsafe {
        println!("r1 указывает на: {}", *r1);
        *r2 += 5;
    }

    // Несколько *const на один объект одновременно -- разрешено,
    // borrow checker на raw pointers не действует
    let a = &x as *const i32;
    let b = &x as *const i32;
    unsafe { println!("{} {}", *a, *b); }

    // Указательная арифметика, как в C
    let arr = [10, 20, 30, 40, 50];
    let ptr = arr.as_ptr();
    unsafe {
        for i in 0..arr.len() {
            println!("arr[{i}] = {}", *ptr.add(i));
        }
    }

    // null допустим для raw pointer (недопустим для &T)
    let null_ptr: *const i32 = std::ptr::null();
    println!("is_null: {}", null_ptr.is_null());

    // unsafe fn -- функция с непроверяемым контрактом
    unsafe fn read_at(ptr: *const i32, index: isize) -> i32 {
        *ptr.offset(index)
    }
    unsafe { println!("{}", read_at(arr.as_ptr(), 2)); }
}
```

Вывод:

```
r1 указывает на: 42
r2 указывает на: 15
два const-указателя на одно: 42 42
arr[0] через указатель = 10
...
null_ptr.is_null() = true
read_at(arr, 2) = 30
байты числа 3.14: [31, 133, 235, 81, 184, 30, 9, 64]
```

Ключевая идея: `*const T`/`*mut T` — это буквально C-указатели без гарантий Rust (не проверяются borrow checker'ом, могут быть null, могут быть dangling). `unsafe` — не "выключить проверки типов", а конкретно снять **пять** ограничений: разыменование raw pointer, вызов `unsafe fn`, доступ к `static mut`, реализация `unsafe trait`, доступ к полям `union`. Всё остальное (типы, `Drop`, move-семантика) продолжает работать как обычно даже внутри `unsafe`-блока.

---

## 2. `cbindgen` — Rust-библиотека для C++ монолита

Направление: **Rust → C++**. Пишешь Rust, `cbindgen` сам генерирует заголовок.

### `src/lib.rs`

```rust
#[no_mangle]
pub extern "C" fn rust_add(a: i32, b: i32) -> i32 { a + b }

#[repr(C)] // гарантия layout, совместимого с C-структурой
pub struct Vector3 { pub x: f64, pub y: f64, pub z: f64 }

#[no_mangle]
pub extern "C" fn vector3_length(v: Vector3) -> f64 {
    (v.x * v.x + v.y * v.y + v.z * v.z).sqrt()
}

// Паттерн "непрозрачный указатель" -- владение передаётся через границу FFI
pub struct Accumulator { total: f64 }

#[no_mangle]
pub extern "C" fn accumulator_new() -> *mut Accumulator {
    Box::into_raw(Box::new(Accumulator { total: 0.0 }))
}

#[no_mangle]
pub unsafe extern "C" fn accumulator_add(ptr: *mut Accumulator, value: f64) {
    if ptr.is_null() { return; } // граница FFI -- обязательная защита
    (*ptr).total += value;
}

#[no_mangle]
pub unsafe extern "C" fn accumulator_free(ptr: *mut Accumulator) {
    if !ptr.is_null() { drop(Box::from_raw(ptr)); }
}
```

### `build.rs`

```rust
fn main() {
    cbindgen::Builder::new()
        .with_crate(env!("CARGO_MANIFEST_DIR"))
        .with_language(cbindgen::Language::Cxx)
        .generate().unwrap()
        .write_to_file("include/rust_math_lib.h");
}
```

### Сгенерированный `include/rust_math_lib.h` (автоматически, ни строчки руками)

```cpp
struct Accumulator;

struct Vector3 {
  double x;
  double y;
  double z;
};

extern "C" {
int32_t rust_add(int32_t a, int32_t b);
double vector3_length(Vector3 v);
Vector3 vector3_add(Vector3 a, Vector3 b);
Accumulator *accumulator_new();
void accumulator_add(Accumulator *ptr, double value);
double accumulator_total(const Accumulator *ptr);
void accumulator_free(Accumulator *ptr);
}
```

### `main.cpp` — реальный C++ код монолита

```cpp
#include "include/rust_math_lib.h"
#include <iostream>

int main() {
    std::cout << "rust_add(2, 3) = " << rust_add(2, 3) << "\n";

    Vector3 a{1.0, 2.0, 2.0};
    std::cout << "vector3_length = " << vector3_length(a) << "\n";

    Accumulator* acc = accumulator_new();
    accumulator_add(acc, 10.5);
    accumulator_add(acc, 5.5);
    std::cout << "total = " << accumulator_total(acc) << "\n";
    accumulator_free(acc); // обязательно -- иначе утечка на Rust-стороне

    accumulator_add(nullptr, 100.0); // проверено Rust-стороной, не падает
    return 0;
}
```

### Cargo.toml
```toml
[package]  
name = "main"  
version = "0.1.0"  
edition = "2024"  
build = "build.rs"  
  
[lib]  
crate-type = ["staticlib"]  
  
[build-dependencies]  
cbindgen = "0.29.4"
```

Собрал и слинковал по-настоящему:

```bash
cargo build                                            # → librust_math_lib.a
clang++.exe -std=c++23 main.cpp -L .\target\debug -lmain -lws2_32 -lntdll -luserenv -ladvapi32 -lbcrypt -o cpp_app
```

```
rust_add(2, 3) = 5
vector3_length({1,2,2}) = 3
vector3_add = {4, 2, 2}
accumulator_total = 16
передача nullptr не уронила программу
```

---

## 3. `bindgen` — обратное направление: легаси C-код → Rust

Направление: **C → Rust**. У тебя уже есть C-заголовок монолита, `bindgen` сам генерирует `extern "C"` блоки — то, что мы раньше писали руками.

### "Легаси" код (`vendor/legacy_math.h` + `.c`)

```c
typedef struct { double real; double imag; } Complex;
double legacy_hypotenuse(double a, double b);
Complex legacy_complex_add(Complex a, Complex b);
int legacy_gcd(int a, int b);
```

### `build.rs`

```rust
fn main() {
    cc::Build::new().file("vendor/legacy_math.c").compile("legacy_math");

    let bindings = bindgen::Builder::default()
        .header("vendor/legacy_math.h")
        .generate().unwrap();

    bindings.write_to_file(
        PathBuf::from(env::var("OUT_DIR").unwrap()).join("bindings.rs")
    ).unwrap();
}
```

### Автоматически сгенерированное (фрагмент)

```rust
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct Complex { pub real: f64, pub imag: f64 }

// bindgen САМ добавил тест на layout -- проверяет ABI-совместимость при cargo test
#[test]
fn bindgen_test_layout_Complex() {
    assert_eq!(::std::mem::size_of::<Complex>(), 16usize, ...);
    assert_eq!(::std::mem::align_of::<Complex>(), 8usize, ...);
}

extern "C" {
    pub fn legacy_hypotenuse(a: f64, b: f64) -> f64;
    pub fn legacy_complex_add(a: Complex, b: Complex) -> Complex;
    pub fn legacy_gcd(a: c_int, b: c_int) -> c_int;
}
```

### `src/main.rs`

```rust
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

fn main() {
    unsafe {
        println!("legacy_hypotenuse(3, 4) = {}", legacy_hypotenuse(3.0, 4.0));
        println!("legacy_gcd(48, 18) = {}", legacy_gcd(48, 18));
        let sum = legacy_complex_add(Complex{real:1.0,imag:2.0}, Complex{real:3.0,imag:-1.0});
        println!("legacy_complex_add = {} + {}i", sum.real, sum.imag);
    }
}
```

Вывод:

```
legacy_hypotenuse(3, 4) = 5
legacy_gcd(48, 18) = 6
legacy_complex_add = 4 + 1i
```

Обрати внимание — `bindgen` сам сгенерировал **тест**, проверяющий, что размер и выравнивание `Complex` в Rust точно совпадают с C — если завтра кто-то в C-заголовке поменяет порядок полей, `cargo test` немедленно это заметит.

---

## 4. `cxx` — настоящий двусторонний мост с C++ (не просто C)

Это принципиально другой уровень: `bindgen`/`cbindgen` работают только с C-совместимым ABI (никаких классов, `std::string`, исключений, шаблонов). `cxx` умеет работать **непосредственно** с C++ — с классами, `UniquePtr`, `rust::String`, и, что важнее всего, **проверяет безопасность на границе** в обе стороны на этапе компиляции, а не только предоставляет сырые биндинги.

### `src/main.rs` — мост объявляется декларативно

```rust
#[cxx::bridge]
mod ffi {
    struct RustPoint { x: f64, y: f64 } // тип Rust, виден в C++

    extern "Rust" {
        // функции Rust, вызываемые ИЗ C++
        fn rust_distance(a: RustPoint, b: RustPoint) -> f64;
        fn rust_greet(name: &str) -> String;
    }

    unsafe extern "C++" {
        // существующий класс C++ монолита, вызываемый ИЗ Rust
        include!("cxx_demo/include/legacy_logger.h");
        type LegacyLogger;

        fn new_logger(prefix: &str) -> UniquePtr<LegacyLogger>;
        fn log(self: Pin<&mut LegacyLogger>, message: &str);
        fn entry_count(self: &LegacyLogger) -> i32;
    }
}

fn rust_distance(a: ffi::RustPoint, b: ffi::RustPoint) -> f64 {
    ((a.x - b.x).powi(2) + (a.y - b.y).powi(2)).sqrt()
}
fn rust_greet(name: &str) -> String { format!("Привет, {name}, из Rust!") }

fn main() {
    let mut logger = ffi::new_logger("APP");
    logger.pin_mut().log("первая запись из Rust");
    logger.pin_mut().log("вторая запись из Rust");
    println!("количество записей: {}", logger.entry_count());

    let p1 = ffi::RustPoint { x: 0.0, y: 0.0 };
    let p2 = ffi::RustPoint { x: 3.0, y: 4.0 };
    println!("rust_distance = {}", rust_distance(p1, p2));
}
```

### `include/legacy_logger.h` — обычный, ничего не подозревающий C++ класс

```cpp
#include "rust/cxx.h"
#include <memory>
#include <string>
#include <vector>

class LegacyLogger {
public:
    explicit LegacyLogger(std::string prefix) : prefix_(std::move(prefix)) {}
    void log(rust::Str message) {
        entries_.push_back(prefix_ + ": " + std::string(message));
    }
    int32_t entry_count() const { return static_cast<int32_t>(entries_.size()); }
private:
    std::string prefix_;
    std::vector<std::string> entries_;
};

inline std::unique_ptr<LegacyLogger> new_logger(rust::Str prefix) {
    return std::make_unique<LegacyLogger>(std::string(prefix));
}
```

### `build.rs`

```rust
fn main() {
    cxx_build::bridge("src/main.rs")
        .include("include")
        .std("c++17")
        .compile("cxx_demo");
}
```

Реально собрал и запустил:

```
количество записей в C++ логгере: 2
rust_distance = 5
```

### Почему это лучше сырого `bindgen`/`cbindgen` для настоящего C++

- **`UniquePtr<LegacyLogger>`** — Rust-обёртка над `std::unique_ptr`, с корректным вызовом C++ деструктора при `Drop` — управление временем жизни C++ объекта работает "само", без ручного `accumulator_free`, как в примере 2.
- **`Pin<&mut LegacyLogger>`** — гарантирует, что C++ объект не будет перемещён Rust'ом (C++ объекты часто self-referential/не move-safe, в отличие от Rust-типов) — прямое применение темы `Pin`, которую разбирали в контексте `async`.
- **`rust::String`/`rust::Str`** — безопасный мост строк без ручной работы с сырыми `char*`/длиной, автоматическая конвертация в/из `std::string`.
- **Проверка на этапе компиляции**: если сигнатура в `#[cxx::bridge]` не совпадает с реальным C++ методом (как и было у меня на первой попытке — `const std::string&` вместо `rust::Str`), это **ошибка компиляции**, а не поздний runtime баг.

## Сводная таблица — какой инструмент когда

|Задача|Инструмент|Направление|
|---|---|---|
|Встроить существующий чистый C API в Rust|`bindgen`|C → Rust|
|Дать Rust-библиотеке C-совместимый ABI для потребления из C/C++|`cbindgen`|Rust → C/C++|
|Полноценная работа с C++ классами, `std::string`, `unique_ptr`, обе стороны|`cxx`|Rust ↔ C++|
|Простой, штучный случай без готовых инструментов|Ручной `extern "C"` + `#[repr(C)]` (как в теме про build scripts)|Любое|

Для твоего сценария — постепенного встраивания Rust в существующий C++ монолит — `cxx` почти всегда предпочтительнее голого `bindgen`/`cbindgen`, если код на C++-стороне не тривиален (есть классы, `std::string`, RAII-паттерны): он даёт заметно больше безопасности на границе ценой чуть более декларативного описания моста, вместо ручной осторожности с сырыми указателями на каждом вызове.

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
- [x] тестирование и `criterion` для бенчмарков; `clippy`/`rustfmt`; (2026.08.08)
- [x] управление зависимостями и crates.io. (2026.08.08)

**Фаза 5 — unsafe Rust и интероп с C++** 
- [ ] `unsafe`, raw pointers, `cxx`/`bindgen`/`cbindgen` — это прямо релевантно твоей текущей работе с монолитом на C++: как встраивать Rust-компоненты в существующую C++-кодовую базу и наоборот.

**Фаза 6 — практика** 
- [ ] Итоговый проект, завязанный на Фазы 3–5: например, сетевой сервис на `tokio` или Rust-модуль, подключённый к C++ через FFI, с Docker-сборкой в довесок.
