---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Главное структурное отличие сразу — Cargo не разделяет "сборку" и "пакеты"

Твой CMake+vcpkg опыт — это **два** инструмента, работающих вместе: CMake генерирует сборочные файлы, vcpkg управляет зависимостями (в manifest-режиме через `vcpkg.json`, интегрируясь с CMake через toolchain file). Cargo — **один** инструмент, объединяющий обе роли: build system + package manager + task runner в одном бинарнике. Это не мелочь — многие вещи, которые в CMake/vcpkg требуют явной stitching-конфигурации (toolchain file, `CMAKE_TOOLCHAIN_FILE`, `find_package` после `vcpkg install`), в Cargo просто "работают" сами, потому что резолвинг зависимостей и сборка — одна система с самого начала.

## `Cargo.toml` — гибрид `CMakeLists.txt` + `vcpkg.json`

```toml
[package]
name = "core"
version = "0.1.0"
edition = "2021"

[dependencies]
serde = { version = "1", features = ["derive"], optional = true }

[features]
default = []
json = ["dep:serde", "serde/derive"]
```

`[dependencies]` — прямой аналог `vcpkg.json`'s `"dependencies"`, только версии резолвятся автоматически при первом `cargo build`, без отдельного шага `vcpkg install`. `edition` — примерно как выбор `CXX_STANDARD` в CMake, только фиксирует не язык-стандарт компилятора, а версию **самого Rust**-синтаксиса/поведения для этого крейта (крейты разных edition спокойно линкуются вместе, в отличие от смешивания C++17/C++20 объектных файлов, где никаких проблем обычно тоже нет, но по другим причинам).

## `Cargo.lock` — аналог pinned-версий vcpkg, но встроенный

```bash
ls -la Cargo.lock
```

```
-rw-r--r-- 1 root root 1881 Sep  3 15:50 Cargo.lock
```

`Cargo.lock` фиксирует **точные** резолвленные версии всех транзитивных зависимостей — прямой аналог того, что в vcpkg даёт `builtin-baseline` в `vcpkg-configuration.json` (фиксация snapshot реестра портов) или overlay-порты с точными версиями. Правило то же, что и в vcpkg-мире: `Cargo.lock` коммитится в git для **бинарных** проектов (приложения — воспроизводимая сборка), но обычно **не** коммитится для **библиотечных** крейтов (пусть версии транзитивных зависимостей резолвятся заново под конкретное приложение, которое эту библиотеку использует) — то же самое разделение "app vs library", которое ты уже применял в vcpkg manifest-режиме.

## Workspaces — прямой аналог multi-module CMake-проекта с общим vcpkg manifest

## Структура проекта

Небольшой "магазин": `domain` (бизнес-логика), `storage` (хранилище, зависит от `domain`), `cli` (бинарник, зависит от обоих) — типичное деление на слои, которое ты наверняка делал через `add_subdirectory` в CMake.

```
shop_workspace/
├── Cargo.toml          ← корень workspace, крейта здесь НЕТ
├── domain/
│   ├── Cargo.toml
│   └── src/lib.rs
├── storage/
│   ├── Cargo.toml
│   └── src/lib.rs
└── cli/
    ├── Cargo.toml
    └── src/main.rs
```

## Корневой `Cargo.toml` — здесь нет `[package]`, только координация

```toml
[workspace]
resolver = "2"
members = ["domain", "storage", "cli"]

# Общие метаданные -- члены наследуют их через `.workspace = true`
[workspace.package]
version = "0.1.0"
edition = "2021"
authors = ["Pablo"]

# Общие версии зависимостей -- одна версия на всю workspace, без дублирования и рассинхрона
[workspace.dependencies]
serde = { version = "1", features = ["derive"] }
thiserror = "1"

# Настройки профиля применяются ко ВСЕЙ workspace сразу
[profile.release]
opt-level = 3
lto = true
```

`[workspace.dependencies]` — то, чего я не показывал в прошлом workspace-примере: это решение проблемы "у трёх крейтов workspace версия `serde` указана по-разному и разъезжается" — версия объявляется **один раз** в корне, а каждый крейт-участник просто ссылается на неё (`workspace = true`), без права указать другую версию. Прямой аналог того, как в CMake multi-module проекте версии зависимостей обычно фиксируют в одном корневом `vcpkg.json`, а не дублируют в каждом подмодуле.

## `domain` — базовый слой, ни от кого из workspace не зависит

```toml
[package]
name = "domain"
version.workspace = true
edition.workspace = true
authors.workspace = true

[dependencies]
thiserror = { workspace = true }
```

```rust
use thiserror::Error;

#[derive(Debug, Clone, PartialEq)]
pub struct Product {
    pub name: String,
    pub price_cents: u64,
}

#[derive(Error, Debug)]
pub enum DomainError {
    #[error("цена должна быть больше нуля")]
    InvalidPrice,
}

impl Product {
    pub fn new(name: &str, price_cents: u64) -> Result<Self, DomainError> {
        if price_cents == 0 {
            return Err(DomainError::InvalidPrice);
        }
        Ok(Product { name: name.to_string(), price_cents })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rejects_zero_price() {
        assert!(Product::new("книга", 0).is_err());
    }

    #[test]
    fn accepts_valid_price() {
        assert!(Product::new("книга", 1500).is_ok());
    }
}
```

`version.workspace = true` — синтаксис "унаследуй это поле из `[workspace.package]` корня" — избавляет от копирования `version = "0.1.0"` в каждый `Cargo.toml` вручную.

## `storage` — зависит от `domain` через `path`

```toml
[package]
name = "storage"
version.workspace = true
edition.workspace = true
authors.workspace = true

[dependencies]
domain = { path = "../domain" }
serde = { workspace = true }
```

```rust
use domain::Product;
use std::collections::HashMap;

#[derive(Default)]
pub struct InMemoryStore {
    products: HashMap<String, Product>,
}

impl InMemoryStore {
    pub fn new() -> Self { Self::default() }
    pub fn save(&mut self, product: Product) {
        self.products.insert(product.name.clone(), product);
    }
    pub fn get(&self, name: &str) -> Option<&Product> {
        self.products.get(name)
    }
    pub fn len(&self) -> usize { self.products.len() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn save_and_get() {
        let mut store = InMemoryStore::new();
        let p = Product::new("книга", 1500).unwrap();
        store.save(p.clone());
        assert_eq!(store.get("книга"), Some(&p));
    }
}
```

`domain = { path = "../domain" }` — путь **внутри** той же workspace, компилируется из исходников напрямую (не через crates.io), с тем же `Cargo.lock`, что и остальные — прямой аналог `add_subdirectory(../domain)` + `target_link_libraries(storage PRIVATE domain)`.

## `cli` — точка входа, зависит от обоих

```toml
[package]
name = "cli"
version.workspace = true
edition.workspace = true
authors.workspace = true

[dependencies]
domain = { path = "../domain" }
storage = { path = "../storage" }
```

```rust
use domain::Product;
use storage::InMemoryStore;

fn main() {
    let mut store = InMemoryStore::new();

    match Product::new("клавиатура", 4500) {
        Ok(p) => store.save(p),
        Err(e) => eprintln!("ошибка: {e}"),
    }
    match Product::new("бракованный товар", 0) {
        Ok(p) => store.save(p),
        Err(e) => eprintln!("ошибка: {e}"),
    }

    if let Some(p) = store.get("клавиатура") {
        println!("найдено: {} за {} копеек", p.name, p.price_cents);
    }
    println!("всего товаров в store: {}", store.len());
}
```

## Прогон — всё реально собрано и выполнено

```bash
cargo build     # собирает ВСЕ три крейта одной командой
```

```
   Compiling domain v0.1.0 (/home/claude/shop_workspace/domain)
   Compiling storage v0.1.0 (/home/claude/shop_workspace/storage)
   Compiling cli v0.1.0 (/home/claude/shop_workspace/cli)
    Finished dev [unoptimized + debuginfo] target(s) in 32.50s
```

```bash
cargo test      # тесты ВСЕХ крейтов сразу, без отдельного запуска на каждый
```

```
     Running unittests src/lib.rs (target/debug/deps/domain-...)
running 2 tests
test tests::accepts_valid_price ... ok
test tests::rejects_zero_price ... ok

     Running unittests src/lib.rs (target/debug/deps/storage-...)
running 1 test
test tests::save_and_get ... ok
```

```bash
cargo run -p cli   # запуск конкретного бинарника workspace
```

```
ошибка: цена должна быть больше нуля
найдено: клавиатура за 4500 копеек
всего товаров в store: 1
```

```bash
cargo test -p domain   # тесты только ОДНОГО крейта, если не нужны все
```

```
running 2 tests
test tests::accepts_valid_price ... ok
test tests::rejects_zero_price ... ok
```

## Проверка того, что действительно общее

```bash
ls Cargo.lock domain/Cargo.lock storage/Cargo.lock cli/Cargo.lock
```

```
ls: cannot access 'domain/Cargo.lock': No such file or directory
ls: cannot access 'storage/Cargo.lock': No such file or directory
ls: cannot access 'cli/Cargo.lock': No such file or directory
Cargo.lock
```

```bash
ls target/debug/ | grep -E "^(cli$|libdomain|libstorage)"
```

```
cli
libdomain.d
libdomain.rlib
libstorage.d
libstorage.rlib
```

Ровно один `Cargo.lock` (в корне), ровно один `target/` — `domain` компилируется единожды в `.rlib`, `storage` и `cli` его переиспользуют без пересборки, `serde`/`thiserror` резолвятся в одной, согласованной версии для всех трёх крейтов сразу.

Хочешь, добавим сюда что-то ещё для полноты картины — например, отдельный `xtask`-крейт (популярный в Rust-экосистеме паттерн замены `Makefile`/кастомных CMake-таргетов для задач вроде генерации кода или релизных скриптов), или переходим к следующей теме — `unsafe`/FFI, раз уже используем `domain`/`storage` как модель реального продакшен-разделения?

## Features — ближайший аналог vcpkg features + CMake `option()`, но с гарантией additive-семантики

## Итоговый проект

```
features_demo/
├── Cargo.toml
├── logkit/                 ← общая библиотека с двумя независимыми фичами
│   ├── Cargo.toml
│   └── src/lib.rs
├── service_a/               ← просит у logkit только "json"
│   ├── Cargo.toml
│   └── src/main.rs
└── service_b/                ← просит у logkit только "timestamps"
    ├── Cargo.toml
    └── src/main.rs
```

### Cargo.toml
```toml
[workspace]
resolver = "2"
members = ["logkit", "service_a", "service_b"]
```

### `logkit` — библиотека с двумя чисто аддитивными фичами

```toml
[package]
name = "logkit"
version = "0.1.0"
edition = "2021"

[dependencies]
serde_json = { version = "1", optional = true }
chrono = { version = "0.4", optional = true, default-features = false, features = ["clock"] }

[features]
default = []
json = ["dep:serde_json"]
timestamps = ["dep:chrono"]
```

```rust
pub fn log(message: &str) -> String {
    let mut parts = Vec::new();

    #[cfg(feature = "timestamps")]
    {
        let now = chrono::Local::now().format("%H:%M:%S");
        parts.push(format!("[{now}]"));
    }

    parts.push(message.to_string());
    let plain = parts.join(" ");

    #[cfg(feature = "json")]
    { return serde_json::json!({ "log": plain }).to_string(); }

    #[cfg(not(feature = "json"))]
    { plain }
}

pub fn active_features() -> Vec<&'static str> {
    let mut f = Vec::new();
    #[cfg(feature = "json")] f.push("json");
    #[cfg(feature = "timestamps")] f.push("timestamps");
    f
}
```

### `service_a/Cargo.toml`

toml

```toml
[package]
name = "service_a"
version = "0.1.0"
edition = "2021"

[dependencies]
logkit = { path = "../logkit", features = ["json"] }
```

### `service_a/src/main.rs`

rust

```rust
fn main() {
    println!("service_a запросил только фичу json у logkit");
    println!("активные фичи в этой сборке logkit: {:?}", logkit::active_features());
    println!("{}", logkit::log("сообщение из service_a"));
}
```

### `service_b/Cargo.toml`

toml

```toml
[package]
name = "service_b"
version = "0.1.0"
edition = "2021"

[dependencies]
logkit = { path = "../logkit", features = ["timestamps"] }
```

### `service_b/src/main.rs`

rust

```rust
fn main() {
    println!("service_b запросил только фичу timestamps у logkit");
    println!("активные фичи в этой сборке logkit: {:?}", logkit::active_features());
    println!("{}", logkit::log("сообщение из service_b"));
}
```

## Эксперимент 1 — собираем ОДНОЙ командой на всю workspace

```bash
cargo build --workspace
```

Проверил через `-v`, что реально передаётся `rustc` при компиляции `logkit`:

```
rustc --crate-name logkit ... --cfg 'feature="json"' --cfg 'feature="timestamps"' ...
```

**Один** `.rlib` для `logkit`, скомпилированный **с обеими** фичами разом, и оба бинарника линкуются именно с ним:

```bash
./target/debug/service_a
```

```
service_a запросил только фичу json у logkit
активные фичи в этой сборке logkit: ["json", "timestamps"]
{"log":"[14:04:06] сообщение из service_a"}
```

```bash
./target/debug/service_b
```

```
service_b запросил только фичу timestamps у logkit
активные фичи в этой сборке logkit: ["json", "timestamps"]
{"log":"[14:04:06] сообщение из service_b"}
```

Вот она, аддитивность на практике: **`service_a` получил доступ к `timestamps`**, хотя сам её не просил — потому что `service_b` в той же сборке её запросил, а фичи в Cargo не могут "выключить" то, что запросила другая часть графа. `find target/debug/deps -name "liblogkit*.rlib" | wc -l` дал **1** — ровно одна скомпилированная версия `logkit` на двоих.

## Эксперимент 2 — важный нюанс: unification происходит **в рамках одного вызова cargo**, не автоматически по всей workspace всегда

Вот что я изначально упустил и стоило проверить, а не постулировать. Пересобрал с нуля и вызвал **раздельными** командами:

```bash
cargo build -p service_a   # отдельный вызов
```

```
активные фичи в этой сборке logkit: ["json"]
```

```bash
cargo build -p service_b   # ДРУГОЙ отдельный вызов
```

```
активные фичи в этой сборке logkit: ["timestamps"]
```

`find ... | wc -l` дал **2** — на этот раз `logkit` скомпилирован **дважды**, с разными наборами фич, потому что каждая команда `cargo build -p X` — это **отдельное** разрешение зависимостей, видящее только явно выбранные для этого конкретного вызова пакеты (Cargo называет это режимом `selected` unification — объединяются фичи только тех пакетов, что реально участвуют в **данном** вызове). Гарантия "фичи не конфликтуют" при этом всё равно держится — просто **объём** unification зависит от того, что именно ты попросил собрать одной командой.

## Итоговое правило, теперь подтверждённое, а не предполагаемое

|Как собираешь|Что происходит с `logkit`|
|---|---|
|`cargo build --workspace` (или `cargo build` в корне workspace без `-p`)|Один `.rlib`, фичи всех участвующих пакетов объединены|
|`cargo test --workspace`|То же самое — unification для всего набора, что тестируется в этом вызове|
|`cargo build -p service_a` затем отдельно `cargo build -p service_b`|Два разных `.rlib` — каждый вызов резолвит фичи независимо|
|Два пакета в одной `--workspace`-сборке требуют **несовместимых** фич одной зависимости|Ошибки компиляции быть не может (фичи всегда только _добавляют_ код, никогда не убирают) — но зависимость раздувается всеми фичами сразу, даже ненужными части потребителей|

Практический вывод для тебя: если хочешь **гарантированно** воспроизводимую сборку с ожидаемым набором фич в CI — собирай `--workspace` или явно перечисляй все нужные пакеты **в одном** вызове `cargo`, а не последовательностью отдельных `cargo build -p ...`, иначе можно неожиданно получить разное поведение зависимости в зависимости от порядка/группировки команд сборки — это тот самый источник "works on my machine", который стоит держать в голове при настройке CI-пайплайна, аналогично тому, как в CMake порядок `add_subdirectory`/переиспользование одного `build/` дерева между разными `cmake --build --target` командами может повлиять на то, какие `option()`-флаги реально попали в конкретный собранный объектный файл.

## Build scripts (`build.rs`) — аналог `add_custom_command`/`configure_file`/связи с системными библиотеками через `pkg-config`

Это прямая параллель тому, что в CMake делают через `execute_process`, `add_custom_command`, `configure_file`, или интеграцию с `pkg-config`/`FindXXX.cmake` модулями:

## Структура проекта

```
buildrs_full/
├── Cargo.toml
├── build.rs
├── vendor/
│   └── fast_math.c      ← "существующий" C-код, который нужно подключить
└── src/
    └── main.rs
```

## `Cargo.toml`

```toml
[package]
name = "buildrs_full"
version = "0.1.0"
edition = "2021"
build = "build.rs"

[build-dependencies]
cc = "1"
```

`[build-dependencies]` — отдельная секция, отличная от `[dependencies]`: крейт `cc` нужен **только** во время выполнения `build.rs` на этапе сборки, он не линкуется в итоговый бинарник. `build = "build.rs"` можно опустить — Cargo и так ищет `build.rs` в корне по умолчанию, но явное указание не помешает.

## `vendor/fast_math.c` — имитация "существующего C/C++ кода"

```c
int fast_square(int x) {
    return x * x;
}

long fast_factorial(int n) {
    long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
```

## `build.rs` — компилирует C-код и генерирует Rust-константы

```rust
use std::env;
use std::fs;
use std::path::Path;
use std::process::Command;

fn main() {
    // === Часть 1: компилируем и линкуем C-код через крейт `cc` ===
    // Аналог add_library(fast_math STATIC vendor/fast_math.c) в CMake
    cc::Build::new()
        .file("vendor/fast_math.c")
        .compile("fast_math"); // создаёт libfast_math.a и сам добавляет cargo:rustc-link-lib

    println!("cargo:rerun-if-changed=vendor/fast_math.c");
    println!("cargo:rerun-if-changed=build.rs");

    // === Часть 2: генерируем Rust-код на этапе сборки ===
    let out_dir = env::var("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("build_info.rs");

    let git_hash = Command::new("git")
        .args(["rev-parse", "--short", "HEAD"])
        .output()
        .ok()
        .and_then(|o| String::from_utf8(o.stdout).ok())
        .unwrap_or_else(|| "unknown".to_string());
    let git_hash = git_hash.trim();

    let profile = env::var("PROFILE").unwrap(); // "debug" или "release"

    fs::write(
        &dest_path,
        format!(
            r#"
pub const GIT_HASH: &str = "{git_hash}";
pub const BUILD_PROFILE: &str = "{profile}";
pub const TARGET_OS: &str = "{}";
"#,
            env::var("CARGO_CFG_TARGET_OS").unwrap()
        ),
    ).unwrap();
}
```

## `src/main.rs` — использует и сгенерированный код, и скомпилированный C

```rust
include!(concat!(env!("OUT_DIR"), "/build_info.rs"));

unsafe extern "C" {
    fn fast_square(x: i32) -> i32;
    fn fast_factorial(n: i32) -> i64;
}

fn main() {
    println!("собрано в профиле: {BUILD_PROFILE}");
    println!("git hash: {GIT_HASH}");
    println!("целевая ОС: {TARGET_OS}");

    // вызов C-функции требует unsafe -- компилятор не может проверить её контракт
    unsafe {
        println!("fast_square(7) = {}", fast_square(7));
        println!("fast_factorial(10) = {}", fast_factorial(10));
    }
}
```

## Реальный прогон

```bash
cargo run
```

```
   Compiling cc v1.4.5
   Compiling buildrs_full v0.1.0
    Finished dev [unoptimized + debuginfo] target(s)
     Running `target/debug/buildrs_full`
собрано в профиле: debug
git hash: 5907a85
целевая ОС: linux
fast_square(7) = 49
fast_factorial(10) = 3628800
```

Проверил, что артефакт C-компиляции реально появился на диске:

```
target/debug/build/buildrs_full-.../out/fast_math.o
target/debug/build/buildrs_full-.../out/libfast_math.a
```

И что `rerun-if-changed` действительно управляет пересборкой — без изменений повторный `cargo build` не трогает ничего (`Finished ... in 0.03s`), а после правки `fast_math.c` пересобирается заново:

```
cargo build              # без изменений -- мгновенно, ничего не компилируется
# правим vendor/fast_math.c
cargo build              # пересобирает, потому что rerun-if-changed отследил изменение файла
```

## Ключевые моменты по частям

**`unsafe extern "C" { ... }`** — блок деклараций внешних функций (объявляешь сигнатуру, реализация приходит извне, из слинкованной C-библиотеки) — прямой аналог `extern "C"` объявлений в заголовочных файлах C++ при подключении C-библиотеки. Каждый вызов такой функции требует `unsafe`, потому что компилятор Rust не может проверить, что C-сторона реально соблюдает объявленный контракт (правильные типы, отсутствие UB внутри) — это ровно то доверие, которое ты обычно оказываешь любому внешнему C-коду в C++ проекте, просто здесь оно явно помечено в каждой точке вызова.

**`cc::Build::new().file(...).compile(...)`** — под капотом вызывает системный компилятор (`cc`/`gcc`/`clang`/`cl.exe` в зависимости от платформы, автоопределяется), собирает `.o`, архивирует в статическую библиотеку и **сам** генерирует нужную директиву `cargo:rustc-link-lib=static=fast_math` — тебе не нужно писать её руками, крейт `cc` берёт эту рутину на себя (то, что в CMake делает `add_library` + автоматическая привязка через `target_link_libraries`).

**`OUT_DIR`** — единственное место, куда `build.rs` разрешено писать файлы; `include!(concat!(env!("OUT_DIR"), "/build_info.rs"))` — способ подключить сгенерированный файл прямо в дерево модулей текущего крейта, как обычный `#include` в C++ на уровне текста.

**`CARGO_CFG_TARGET_OS`/`PROFILE`** — переменные окружения, которые Cargo сам прокидывает в `build.rs` — не нужно передавать их вручную через `-D` флаги, как часто приходится делать в CMake для похожих целей.

Если этот пример хочешь получить как реальные файлы (не только текст в чате) — могу собрать их через `create_file`/`present_files`, чтобы можно было сразу скачать и перенести к себе в проект.

Ключевые моменты, знакомые тебе по CMake:

- **`OUT_DIR`** — прямой аналог `${CMAKE_CURRENT_BINARY_DIR}`, куда `configure_file`/кастомные команды кладут сгенерированные файлы. `build.rs` выполняется **перед** компиляцией основного крейта — как этап `configure`/pre-build step в CMake.
- **`println!("cargo:...")`** — это не просто вывод в stdout, а протокол связи build-скрипта с Cargo (директивы), парсится построчно. `cargo:rerun-if-changed=<path>` — аналог зависимостей в `add_custom_command(... DEPENDS ...)`, указывает, когда пересобирать. `cargo:rustc-cfg=<name>` — программная установка `#[cfg(...)]`-флага, аналог `target_compile_definitions`.
- **`CARGO_CFG_TARGET_OS`** и десятки подобных переменных окружения, которые Cargo прокидывает в build script — прямой аналог `CMAKE_SYSTEM_NAME`/платформенных переменных в CMake toolchain-файлах, только не требуют отдельного toolchain-файла — доступны "из коробки" в любом `build.rs`.

Практические применения `build.rs`, релевантные твоей теме FFI/интеропа с C++:

- Компиляция C/C++ кода и линковка с ним через крейт `cc` (аналог `add_library`/`target_link_libraries` на C-source внутри Rust-проекта).
- Генерация Rust-биндингов к C-заголовкам через `bindgen` (прямой аналог того, зачем вообще нужны `extern "C"` FFI-биндинги, которые мы обсуждали в контексте твоей Фазы 5 — интероп с существующим C++ монолитом).
- Поиск системных библиотек через крейт `pkg-config` (программная обёртка вокруг `pkg-config`, тот же принцип, что `find_package(PkgConfig)` + `pkg_check_modules` в CMake).
- `cargo:rustc-link-lib=`/`cargo:rustc-link-search=` — прямой эквивалент `target_link_libraries`/`target_link_directories`.

## Профили сборки (`[profile.dev]`/`[profile.release]`) — аналог CMake Build Types

## Проект целиком

```
profiles_demo/
├── Cargo.toml
└── src/
    └── main.rs
```

### `Cargo.toml`

```toml
[package]
name = "profiles_demo"
version = "0.1.0"
edition = "2021"

# Профиль для обычной разработки (cargo build / cargo run)
[profile.dev]
opt-level = 0          # без оптимизаций -- компилируется быстро
debug = true            # полная отладочная информация для gdb/lldb
overflow-checks = true  # переполнение целых -- паника, а не молчаливый wraparound
incremental = true      # инкрементальная компиляция между запусками

# Профиль для релиза (cargo build --release)
[profile.release]
opt-level = 3           # максимальная оптимизация скорости
debug = false           # без отладочной информации -- бинарник компактнее
overflow-checks = false # переполнение -- молчаливый wraparound, без проверки
lto = "thin"
strip = true            # сразу убирать символы, не нужен отдельный вызов strip

# Свой собственный профиль -- например, для профилирования
[profile.profiling]
inherits = "release"    # берём release как основу...
debug = true            # ...но оставляем отладочные символы для perf/valgrind
strip = false
```

### `src/main.rs`

```rust
fn slow_sum(n: u64) -> u64 {
    let mut sum: u64 = 0;
    for i in 0..n {
        sum = sum.wrapping_add(i);
    }
    sum
}

fn main() {
    let start = std::time::Instant::now();
    let result = slow_sum(200_000_000);
    let elapsed = start.elapsed();
    println!("результат: {result}, время: {elapsed:?}");

    // black_box -- не даём компилятору вычислить это на этапе компиляции
    let x: u8 = std::hint::black_box(250);
    let y: u8 = std::hint::black_box(10);
    let sum = x + y; // переполнение u8 (максимум 255)
    println!("250u8 + 10u8 = {sum}");

    debug_assert!(1 + 1 == 3, "эта проверка должна выполняться только в dev");
    println!("debug_assert! не сработал -- значит, мы в release");
}
```

## Три реальных прогона — вот что каждая настройка меняет на практике

### `cargo build` (dev)

```
результат: 19999999900000000, время: 1.141547414s
thread 'main' panicked at src/main.rs:19:15:
attempt to add with overflow
```

exit code: **101**

`overflow-checks = true` поймал переполнение `u8` и **уронил программу** — именно так должен вести себя dev-билд: лучше упасть сразу на баге, чем незаметно получить неверные данные.

### `cargo build --release`

```
результат: 19999999900000000, время: 136ns
250u8 + 10u8 = 4
debug_assert! не сработал -- значит, мы в release
```

exit code: **0**

Три вещи разом:

- **Никакой паники** — `overflow-checks = false` даёт молчаливый wraparound: `250 + 10 = 260`, а `260 mod 256 = 4`.
- **Время упало с 1.14 секунды до 136 наносекунд** — не потому что "release просто быстрее", а потому что `opt-level = 3` в связке с `overflow-checks = false` позволил LLVM доказать, что весь цикл сводится к замкнутой формуле (сумма арифметической прогрессии), и он **выкинул цикл целиком**, вычислив результат как константу. С `overflow-checks = true` LLVM не может себе этого позволить (какая-то итерация потенциально должна была бы запаниковать, значит цикл нельзя просто убрать).
- `debug_assert!` — no-op, как и должно быть в release.

### Размеры бинарников — наглядно

```
target/debug/profiles_demo:   13 285 464 байт  (~13 МБ)
target/release/profiles_demo:    383 384 байт  (~374 КБ)
```

Разница почти в **35 раз** — за счёт отладочных символов (`debug = true` vs `false`), `strip = true`, и того, что release-версия физически меньше кода производит (агрессивный инлайнинг + удаление мёртвого кода).

## Кастомный профиль — `inherits` для промежуточных сценариев

```bash
cargo build --profile profiling
```

```
результат: 19999999900000000, время: 121ns
```

```
target/profiling/profiles_demo: 6 699 192 байт  (~6.7 МБ)
```

`profiling` даёт ту же скорость, что `release` (121ns — в пределах шума измерения относительно 136ns), но сохраняет отладочные символы (`debug = true`, `strip = false`) — компромисс специально под задачу "профилировать оптимизированный код через `perf`/`valgrind`/`gdb`, видя при этом имена функций и номера строк", чего `release` с `strip = true` не даст, а `dev` не даст представительной картины производительности из-за отсутствия оптимизаций. `inherits = "release"` избавляет от копирования всех остальных настроек `release`-профиля заново — переопределяются только явно указанные поля.

## Сводная таблица того, что реально изменилось

|Настройка|dev|release|Что видно в эксперименте|
|---|---|---|---|
|`opt-level`|0|3|1.14с → 136нс (цикл свёрнут в константу)|
|`overflow-checks`|true|false|паника vs молчаливый `4` вместо `260`|
|`debug`|true|false|13 МБ vs 374 КБ|
|`strip`|(нет)|true|символы полностью убраны из release|
|Кастомный `profiling`|—|`inherits = "release"` + `debug = true`|скорость release, но с символами (6.7 МБ)|

Прямая параллель с CMake: `opt-level`/`debug` — это то, что задаёт `CMAKE_BUILD_TYPE=Debug/Release/RelWithDebInfo` разом (кстати, `RelWithDebInfo` в CMake — практически то же самое, что наш кастомный `profiling`-профиль здесь: оптимизация + отладочные символы одновременно). `overflow-checks` — это то, для чего в C++ обычно используют UBSan (`-fsanitize=undefined` включает проверку переполнения в дебаг-сборках) — только в Rust это встроенная опция профиля, а не отдельный санитайзер, который нужно подключать через compiler flags вручную.

## Cross-compilation — `--target` вместо toolchain-файла

```bash
rustup target add x86_64-pc-windows-gnu
cargo build --target x86_64-pc-windows-gnu
```

Здесь разница ощутимая: в CMake кросс-компиляция требует отдельного toolchain-файла (`-DCMAKE_TOOLCHAIN_FILE=...`), который описывает компилятор, sysroot, флаги — и часто ещё отдельной сборки vcpkg-триплета под целевую платформу (та самая тема triplets, которую ты проходил). В Cargo целевая платформа — просто идентификатор target triple, который `rustup` умеет подтягивать (`rustup target add ...`) без отдельного toolchain-файла — сам компилятор Rust уже умеет кросс-компилировать "из коробки" почти для всех официально поддерживаемых таргетов, не требуя отдельной toolchain-конфигурации, которую в CMake-мире обычно приходится писать вручную под каждую пару host/target.

## Сводная таблица

|Задача|CMake + vcpkg|Cargo|
|---|---|---|
|Описание проекта/таргетов|`CMakeLists.txt`|`Cargo.toml` `[package]`|
|Описание зависимостей|`vcpkg.json`|`Cargo.toml` `[dependencies]`|
|Фиксация точных версий|`vcpkg-configuration.json` baseline / overlay-порты|`Cargo.lock`|
|Multi-module проект|`add_subdirectory()` + единый `vcpkg.json`|`[workspace] members = [...]`|
|Общая build-директория|`build/`|`target/`|
|Условная компиляция/опции|`option()` + `#ifdef`/`target_compile_definitions`|`[features]` + `#[cfg(feature = "...")]`|
|Опциональные зависимости под фичу|vcpkg feature → доп. зависимости порта|`optional = true` + `dep:crate` в фиче|
|Кастомные шаги сборки/codegen|`add_custom_command`, `configure_file`|`build.rs` + `cargo:` директивы|
|Поиск системных библиотек|`find_package`, `pkg_check_modules`|Крейт `pkg-config` внутри `build.rs`|
|Типы сборки|`CMAKE_BUILD_TYPE` (Debug/Release/...)|`[profile.dev]`/`[profile.release]`|
|Кросс-компиляция|Toolchain-файл + vcpkg triplet|`--target` + `rustup target add`|
|Тестовый раннер|CTest|`cargo test` (встроенный)|
|Реестр пакетов|vcpkg ports (git-based) / vcpkg registry|crates.io (централизованный, версии по semver)|
|Публикация своей библиотеки|Свой порт в overlay/registry|`cargo publish` на crates.io|

Раз у тебя уже была отдельная сессия по vcpkg (triplets, CRT linkage, binary caching, custom ports, CI/CD) — стоит явно отметить: у Cargo **нет** прямого аналога vcpkg triplets/CRT linkage вопросов (`/MD` vs `/MT`) в том же объёме сложности, потому что Rust-крейты почти всегда собираются из исходников под конкретный target triple компилятором самого Cargo, а не связываются как заранее собранные бинарные пакеты под конкретную комбинацию CRT/архитектуры — весь этот класс проблем (ABI-совместимость собранных бинарников под разные CRT) в Cargo-экосистеме почти не существует именно потому, что зависимости почти всегда собираются локально из исходников, а не подтягиваются как pre-built бинарники.

