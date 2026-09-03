---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]


Подтверждено: одна `Cargo.lock`, один `target/` на всю workspace, feature реально включает условную компиляцию (`#[cfg(feature = ...)]`). Теперь — build script:Работает чисто, без предупреждений (кэш от предыдущего запуска). Все примеры проверены. Собираю разбор.

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

```toml
[workspace]
resolver = "2"
members = ["core", "cli"]
```

Я собрал реальную workspace из двух крейтов (`core` — библиотека, `cli` — бинарник, зависящий от неё через `path`) и проверил два ключевых свойства:

```
--- один Cargo.lock на всю workspace ---
-rw-r--r-- 1 root root 1881 Sep  3 15:50 Cargo.lock
--- один target/ на всю workspace ---
cli
cli.d
libcore.d
libcore.rlib
```

**Один `Cargo.lock`** на всю workspace (не по одному на крейт) — прямой аналог того, что в CMake-проекте с `add_subdirectory()` + единым `vcpkg.json` в корне все подмодули резолвят зависимости из одного и того же дерева версий, без риска конфликта версий одной и той же библиотеки в разных модулях.

**Один `target/`** (аналог единой `build/` директории CMake) — общий кэш скомпилированных артефактов между всеми членами workspace: если `core` уже скомпилирован, `cli` просто линкуется с готовым `.rlib`, не пересобирая заново — то же самое, что происходит с общей `build/` в CMake multi-target проекте.

`cargo build`/`cargo run` из корня workspace без `-p` собирает/выбирает **все** члены (или требует явного `-p <crate>` для конкретного, как я сделал в примере через `cargo run -p cli`) — концептуально похоже на выбор конкретного таргета в CMake (`cmake --build . --target cli`), только имена членов workspace определяются структурой `Cargo.toml`, а не `add_executable`/`add_library` вызовами.

## Features — ближайший аналог vcpkg features + CMake `option()`, но с гарантией additive-семантики

```toml
[dependencies]
serde = { version = "1", features = ["derive"], optional = true }

[features]
default = []
json = ["dep:serde", "serde/derive"]
```

```rust
#[cfg(feature = "json")]
pub fn describe() -> &'static str { "core собран С фичей json" }

#[cfg(not(feature = "json"))]
pub fn describe() -> &'static str { "core собран БЕЗ фичи json" }
```

Прогнал обе сборки:

```bash
cargo run -p cli                          # → "core собран БЕЗ фичи json"
cargo run -p cli --features core/json     # → "core собран С фичей json"
```

Механически `--features` включает `#[cfg(feature = "...")]`-код на этапе компиляции — прямой аналог `target_compile_definitions` в CMake + `#ifdef` в коде, или `#cmakedefine` в `.h.in`-шаблонах, которые ты наверняка использовал. `optional = true` у зависимости означает "эта зависимость подтягивается **только** если включена фича, которая её требует" (`dep:serde` в списке фичи) — прямой аналог того, как vcpkg-порты объявляют собственные `"features"` в `vcpkg.json`, каждая из которых может тянуть дополнительные зависимости порта.

**Критическое отличие от `#define`/CMake options, о котором стоит явно знать:** Cargo features **аддитивны и не могут конфликтовать** — если в одной workspace два крейта требуют одну и ту же зависимость, но с разным набором фич, Cargo **объединяет** фичи (feature unification) — зависимость собирается один раз с **объединением** всех запрошенных фич от всех крейтов workspace. Это принципиально отличается от CMake `option()`, где два разных таргета в одном build tree технически **могут** линковаться с библиотекой, собранной по-разному (что часто и есть источник ODR-violation багов при неаккуратной конфигурации) — Cargo такую ситуацию architecturally исключает: фичи не могут "выключать" код, который использует другая часть той же зависимости, только **добавлять**.

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
