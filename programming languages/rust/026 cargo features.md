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

```rust
use std::env;
use std::fs;
use std::path::Path;

fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("generated.rs");

    fs::write(
        &dest_path,
        "pub fn generated_message() -> &'static str { \"сгенерировано во время сборки, а не запуска\" }",
    ).unwrap();

    println!("cargo:rerun-if-changed=build.rs");

    if env::var("CARGO_CFG_TARGET_OS").unwrap() == "linux" {
        println!("cargo:rustc-cfg=platform_linux");
    }
}
```

```rust
// src/main.rs
include!(concat!(env!("OUT_DIR"), "/generated.rs"));

fn main() {
    println!("{}", generated_message());
    #[cfg(platform_linux)]
    println!("собрано на Linux (через cargo:rustc-cfg из build.rs)");
}
```

Вывод:

```
сгенерировано во время сборки, а не запуска
собрано на Linux (через cargo:rustc-cfg из build.rs)
```

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

Уже использовали `[profile.dev] panic = "abort"` в теме про `panic!`. Общая механика:

```toml
[profile.release]
opt-level = 3
lto = true
codegen-units = 1
panic = "abort"
```

Прямой аналог `CMAKE_BUILD_TYPE=Release` + ручной настройки флагов оптимизации (`-O3`), LTO (`INTERPROCEDURAL_OPTIMIZATION`), но с той разницей, что в Cargo профили — **часть манифеста проекта**, не выбор при вызове `cmake` (`cargo build` = dev-профиль по умолчанию, `cargo build --release` = release-профиль, без отдельного шага "configure" с указанием типа сборки заново).

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

Хочешь разобрать `cargo test`/тестирование (`#[test]`, интеграционные тесты, `criterion` для бенчмарков — прямая параллель CTest/GoogleTest/Catch2, которые ты проходил) — это логичное продолжение темы тулинга, или перейти к следующему пункту Фазы 4 — `clippy`/`rustfmt`?

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
- [ ] Cargo (workspaces, features, build scripts) как аналог CMake/vcpkg;
- [ ] тестирование и `criterion` для бенчмарков; `clippy`/`rustfmt`;
- [ ] управление зависимостями и crates.io.

**Фаза 5 — unsafe Rust и интероп с C++** 
- [ ] `unsafe`, raw pointers, `cxx`/`bindgen`/`cbindgen` — это прямо релевантно твоей текущей работе с монолитом на C++: как встраивать Rust-компоненты в существующую C++-кодовую базу и наоборот.

**Фаза 6 — практика** 
- [ ] Итоговый проект, завязанный на Фазы 3–5: например, сетевой сервис на `tokio` или Rust-модуль, подключённый к C++ через FFI, с Docker-сборкой в довесок.
