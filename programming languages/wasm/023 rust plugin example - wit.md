---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]


**Плагин** (`.../src/lib.rs`) — реализует `init/info/process/shutdown` через макрос `wit_bindgen::generate!` (то же самое, что делает консольная `wit-bindgen rust plugin.wit`, только на этапе `cargo build`, без отдельного шага кодогенерации). Реализация — тот же uppercase-плагин, что и во всех предыдущих версиях, но с честной разницей: `input.to_uppercase()` в Rust — полноценно юникодный (`"привет"` → `"ПРИВЕТ"`), в отличие от C-версии (`impl.c`), которая руками поднимает только байты `'a'..'z'`, то есть только ASCII.

Рецепт для сборки настоящего `.wasm`, когда таргет будет доступен (по аналогии с уже проверенной в этой сессии C-сборкой, тем же адаптером `wasi_snapshot_preview1.reactor.wasm`, лежащим в `wit_plugin/`):

```
rustup target add wasm32-wasip1
cargo build --release --target wasm32-wasip1
wasm-tools component new target/wasm32-wasip1/release/rust_component_plugin.wasm \
  --adapt wasi_snapshot_preview1=wasi_snapshot_preview1.reactor.wasm \
  -o plugin_component_rust.wasm
```

Нужен ли адаптер — по аналогии с C++-плагином этой сессии (std Rust на wasm32-wasip1 тоже тянет WASI preview1 пути), но именно этот шаг не проверен вживую — стоит подтвердить `wasm-tools validate`/`wasm-tools component wit` перед запуском, ровно как это делалось для C++-версии. Альтернатива — `cargo install cargo-component` и `cargo component build --release`: он оборачивает весь этот пайплайн (таргет, адаптер, `component new`) сам, ценой немного другого способа объявления биндингов в `Cargo.toml`.

**Хост** — `host_component.cc` (День 9) отправлен как есть, без изменений: он берёт путь к `.wasm` первым аргументом (`./host_component plugin_component_rust.wasm`), и весь код, который находит `example:plugin/transform@0.1.0` → `init/info/process/shutdown` и печатает результат, ему совершенно всё равно, на чём собран гость. Это и есть главный практический вывод по сравнению с ручным `plugin_abi.h`-ABI — один и тот же хост без правок уже прогнан на C и C++, и по контракту обязан так же взять и Rust-версию, как только появится реальный `.wasm`.

## Пример

### vcpkg.json
```json
{
    "name": "demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_component CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(WASMTIME_VERSION "v47.0.3")
set(WASMTIME_ARCHIVE "wasmtime-${WASMTIME_VERSION}-x86_64-windows-c-api.zip")

FetchContent_Declare(
        wasmtime_c_api
        URL "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/${WASMTIME_ARCHIVE}"
)
FetchContent_MakeAvailable(wasmtime_c_api)

# В архиве нет своего CMakeLists.txt -- это просто заголовки + готовая
# библиотека, поэтому объявляем IMPORTED-таргет вручную. Дальше он
# используется в проекте как обычная зависимость через target_link_libraries.
# Windows-архив содержит только shared-вариант (wasmtime.dll + .dll.lib),
# в отличие от Linux/macOS, где есть статическая libwasmtime.a.
if(WIN32)
    add_library(wasmtime SHARED IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            IMPORTED_IMPLIB "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
else()
    add_library(wasmtime STATIC IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/libwasmtime.a"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
endif()

add_executable(host_component host_component.cpp)
if(WIN32)
    target_link_libraries(host_component PRIVATE wasmtime)
    add_custom_command(TARGET host_component POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_component>"
    )
else()
    target_link_libraries(host_component PRIVATE wasmtime pthread dl m)
endif()


#add_library(ide OBJECT upper.cpp)

```

### host_component.cpp
```cpp
// Хост поверх РЕАЛЬНОГО Component Model API Wasmtime (wasmtime/component/*.hh),
// а не Core-модуля через Module/Instance, как во всех предыдущих днях.
// Вызывает init -> info -> process -> process(без init была бы ошибка,
// поэтому здесь порядок правильный) -> shutdown на одном и том же
// инстансе -- то, что через `wasmtime run --invoke` сделать нельзя,
// т.к. каждый его вызов -- это новый инстанс.

#include <fstream>
#include <iostream>
#include <format>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>

namespace {

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    wasmtime::component::ExportIndex must_get_index(wasmtime::component::Instance& instance,
                                                    wasmtime::Store::Context& ctx,
                                                    const wasmtime::component::ExportIndex* parent,
                                                    const char* name) {
        const auto idx{instance.get_export_index(ctx, parent, name)};
        if (!idx) {
            std::cerr << std::format("Export '{}' not found\n", name);
            std::exit(1);
        }

        return *idx;
    }

    wasmtime::component::Func must_get_func(wasmtime::component::Instance& instance,
                                            wasmtime::Store::Context& ctx,
                                            const wasmtime::component::ExportIndex& idx) {
        const auto f{instance.get_func(ctx, idx)};
        if (!f) {
            std::cerr << "Export found, but it is not function\n";
            std::exit(1);
        }
        return *f;
    }

    template<typename T>
    bool check_bad_result(const wasmtime::Result<T>& result, const std::string& label) {
        if (result) return false;
        std::cerr << std::format("{}: {}\n", label, result.err().message());

        return true;
    }

}

int main(const int argc, char *argv[]) {
    wasmtime::Engine engine;
    const char* path{argc > 1 ? argv[1] : "plugin_component_rust.wasm"};
    std::cout << std::format("Loading: {}\n", path);
    auto bytes{read_file(path)};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Component compilation error: {}\n", component_result.err().message());
        return 1;
    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::component::Linker linker{engine};;
    (void)linker.add_wasip2();
    // На случай если модуль всё же попросит что-то ещё не покрытое --
    // не падаем при инстанцировании, падаем только если реально позовут.
    (void)linker.define_unknown_imports_as_traps(component);

    auto ctx{store.context()};
    auto instance_result{linker.instantiate(ctx, component)};
    if (check_bad_result(instance_result, "Instantiation error")) return 1;

    wasmtime::component::Instance instance{instance_result.unwrap()};

    // Сначала находим индекс интерфейса-экспорта "example:plugin/transform@0.1.0",
    // затем внутри него -- индексы отдельных функций.
    wasmtime::component::ExportIndex transform_idx{must_get_index(
        instance,
        ctx,
        nullptr,
        "example:plugin/transform@0.1.0")};

    const auto export_func = [&instance, &ctx, &transform_idx](const char* name) {
        return must_get_func( instance, ctx, must_get_index(
            instance,
            ctx,
            &transform_idx,
            name));
    };
    wasmtime::component::Func init_fn{export_func("init")};
    wasmtime::component::Func info_fn{export_func("info")};
    wasmtime::component::Func process_fn{export_func("process")};
    wasmtime::component::Func shutdown_fn{export_func("shutdown")};

    // 1) init() -- без аргументов, без результата.
    std::vector<wasmtime::component::Val> no_args, no_results;
    if (auto init_result{init_fn.call(ctx, no_args, no_results)};
        check_bad_result(init_result, "init() failed")) return 1;
    std::cout << "init() executed\n";

    // 2) info() -> record { name: string, abi-version: u32 }
    std::vector<wasmtime::component::Val> info_results{wasmtime::component::Val{false}};
    std::vector<wasmtime::component::Val> info_args{};
    if (auto info_result{info_fn.call(ctx, info_args, info_results)};
        check_bad_result(info_result, "info() failed")) return 1;

    const wasmtime::component::Record& record{info_results[0].get_record()};
    std::string delimiter;
    std::cout << "info() {";
    for (const wasmtime::component::RecordField& field: record) {
        std::cout << std::format("{}{}=", delimiter, field.name());
        delimiter = ", ";
        if (const auto& value{field.value()}; value.is_string()) {
            std::cout << std::format("'{}'", value.get_string());
        } else if (value.is_u32()) {
            std::cout << value.get_u32();
        }
    }
    std::cout << "}\n";

    // 3) process(input: string) -> result<string, process-error>
    const auto call_process = [&](const std::string& text) {
        std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(text)};
        std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};
        if (const auto result{process_fn.call(ctx, args, results)};
            check_bad_result(result, "  TRAP")) return;

        if (const wasmtime::component::WitResult& wr{results[0].get_result()};
            wr.is_ok()) {
            std::cout << std::format("  Ok('{}')\n", wr.payload()->get_string());
        } else {
            std::cout << std::format("  Err({})\n", wr.payload()->get_variant().discriminant());
        }
    };

    std::cout << "process(\"hello wit component\"):\n";
    call_process("hello wit component");

    // 4) shutdown()
    std::vector<wasmtime::component::Val> shutdown_args, shutdown_results;
    if (const auto shutdown_result{shutdown_fn.call(ctx, shutdown_args, shutdown_results)};
        check_bad_result(shutdown_result, "shutdown() failed")) return 1;
    std::cout << "shutdown() executed\n";

    // 5) Вызов после shutdown -- у нас plugin_shutdown() просто сбрасывает
    // g_initialized, так что process() после него снова должен вернуть
    // Err(not-initialized) -- проверим, что состояние действительно общее
    // на весь инстанс (тот самый плюс к ручному ABI: здесь это staticная
    // переменная в WASM-памяти инстанса, а не что-то, что хост обязан
    // сериализовать сам).
    std::cout << "process(\"after shutdown\"):\n";
    call_process("after shutdown");

    return 0;
}

```

### wit/plugin.wit
```wit
// Простой WIT-интерфейс плагина. В отличие от plugin_abi.h (День 6),
// где pointer+length и упаковка (ptr<<32)|len были ручной работой хоста
// и плагина, здесь строки, ошибки и записи -- это ЧАСТЬ языка описания
// интерфейса; маршалинг через линейную память компонент-модель берёт
// на себя сама (канонический ABI), а не автор плагина.
package example:plugin@0.1.0;

interface transform {
  /// Метаданные плагина -- аналог plugin_abi_version(), но как
  /// структурированная запись, а не голое число.
  record plugin-info {
    name: string,
    abi-version: u32,
  }

  /// Ошибка обработки -- размеченный вариант вместо "магического"
  /// nullptr/0, которым сигнализировалась ошибка в ручном C ABI.
  variant process-error {
    not-initialized,
    empty-input,
    internal(string),
  }

  /// Аналог plugin_init() из Дня 6.
  init: func();

  /// Аналог plugin_abi_version() -- но теперь можно вернуть сразу
  /// имя плагина вместе с версией.
  info: func() -> plugin-info;

  /// Аналог plugin_process(ptr, len) -> packed(ptr,len). Здесь просто
  /// string -> result<string, process-error>: и вход, и выход --
  /// обычные строки, без ручного plugin_alloc/plugin_free.
  process: func(input: string) -> result<string, process-error>;

  /// Аналог plugin_shutdown().
  shutdown: func();
}

world plugin {
  export transform;
}

```

### cargo.toml
```toml
[package]
name = "rust_component_plugin"
version = "0.1.0"
edition = "2021"

[lib]
# cdylib -- тот же смысл, что у upper_rust: библиотечный крейт без
# fn main(), итоговый .wasm компилируется как компонент под
# wasm32-wasip2 (аналог -mexec-model=reactor для C, только здесь это
# делает сам компонентный ABI, а не флаги линковщика).
crate-type = ["cdylib"]

[dependencies]
# Версия крейта specifically совпадает с версией wit-bindgen-cli
# (0.60.0), которой в этой сессии сгенерирован generated/plugin.c для
# Дня 9 -- макрос wit_bindgen::generate! ниже делает ровно то же самое,
# что консольная команда `wit-bindgen rust`, но прямо на этапе cargo
# build, без отдельного шага кодогенерации.
wit-bindgen = "0.60.0"

[profile.release]
opt-level = "z"
panic = "abort"
lto = true

```

### src/lib.rs
```rust
// Тот же интерфейс example:plugin/transform из wit_plugin/plugin.wit,
// что уже реализован на C (impl.c) и C++ (impl.cpp) и грузится
// host_component.cc -- здесь та же самая точка входа хоста, тот же WIT,
// но реализация плагина на Rust. Идея в том, что host_component.cc НЕ
// нужно менять вообще ни на строчку: он видит один и тот же
// example:plugin/transform@0.1.0 независимо от языка гостя -- именно
// это и есть весь смысл Component Model поверх WIT.
//
// В отличие от plugin_abi.h (ручной ABI, plugin_alloc/plugin_free,
// упаковка (ptr<<32)|len) -- здесь весь маршалинг строк, вариантов и
// record'ов берёт на себя канонический ABI, сгенерированный макросом
// wit_bindgen::generate! ниже. Сравните с upper_rust/src/lib.rs -- там
// был явный HEADER_SIZE-трюк для дружбы с C-стороны free(); здесь
// такого кода просто нет, потому что нет ручного plugin_free() как
// части контракта вообще.

// rustup target add wasm32-wasip1
// cargo build --release --target wasm32-wasip1
// wasm-tools component new target/wasm32-wasip1/release/rust_component_plugin.wasm --adapt wasi_snapshot_preview1=wasi_snapshot_preview1.reactor.wasm -o plugin_component_rust.wasm

wit_bindgen::generate!({
    world: "plugin",
    path: "wit",
});

use exports::example::plugin::transform::{Guest, PluginInfo, ProcessError};

// static mut, как и в upper_rust -- состояние плагина живёт в памяти
// самого wasm-инстанса, хосту не нужно ничего сериализовать между
// вызовами init/process/shutdown (host_component.cc это явно проверяет:
// process() после shutdown() снова возвращает Err(not-initialized)).
static mut INITIALIZED: bool = false;

struct Component;

impl Guest for Component {
    fn init() {
        unsafe {
            INITIALIZED = true;
        }
    }

    fn info() -> PluginInfo {
        PluginInfo {
            name: "upper-wit-rust".to_string(),
            abi_version: 1,
        }
    }

    fn process(input: String) -> Result<String, ProcessError> {
        if unsafe { !INITIALIZED } {
            return Err(ProcessError::NotInitialized);
        }
        if input.is_empty() {
            return Err(ProcessError::EmptyInput);
        }
        // В отличие от C-версии (impl.c), которая руками поднимает
        // только байты 'a'..'z' -- то есть только ASCII, -- Rust'овый
        // String хранит настоящий UTF-8, и to_uppercase() честно
        // юникодный (например, "привет" -> "ПРИВЕТ", что для C-версии
        // из этой сессии в принципе не работало бы без ручной таблицы
        // Unicode-регистров).
        Ok(input.to_uppercase())
    }

    fn shutdown() {
        unsafe {
            INITIALIZED = false;
        }
    }
}

export!(Component);
```

---

### День 1 Что такое WASM на самом деле

Цель: понять модель выполнения — модуль, стек-машина, линейная память, песочница — до того как трогать C++.

- [x] Прочитать обзор спецификации на [webassembly.org](https://webassembly.org/) и раздел Concepts на [MDN](https://developer.mozilla.org/en-US/docs/WebAssembly) (2026.08.16)
- [x] Установить `wasmtime` CLI и [WABT](https://github.com/WebAssembly/wabt) (даёт `wat2wasm`/`wasm2wat`/`wasm-objdump`) (2026.08.16)
- [x] Написать вручную крошечный модуль на текстовом формате `.wat` (например, функция сложения двух чисел), скомпилировать в `.wasm` и запустить через `wasmtime run` (2026.08.16)
- [x] Разобраться в разнице: линейная память (один плоский массив байт) vs адресное пространство обычного процесса — почему указатели гостя не совпадают с указателями хоста (2026.08.16)

### День 2 Компиляция C/C++ в WASM

Цель: получить первый рабочий .wasm из своего C++ кода двумя разными тулчейнами.

- [x] Установить [wasi-sdk](https://github.com/WebAssembly/wasi-sdk) (clang с таргетом `wasm32-wasi`), скомпилировать простую C-функцию, запустить через `wasmtime` (2026.08.16)
- [x] Установить [Emscripten](https://emscripten.org/docs/compiling/WebAssembly.html), скомпилировать тот же пример, сравнить: под что заточен каждый тулчейн (WASI/сервер vs браузер), размер бинарника (2026.08.17)
- [x] Прочитать тред ["What is the difference between wasi-sdk and emscripten?"](https://github.com/WebAssembly/wasi-sdk/issues/222) — это прямо отвечает на вопрос «какой инструмент когда» (2026.08.17)
- [x] Почему для плагинной системы (не для браузера) обычно нужен именно `wasi-sdk` / `wasm32-wasi`, а не Emscripten (2026.08.17)

### День 3 Модель памяти и передача данных через границу

Цель: понять, как из хоста передать/получить строку, буфер, структуру — это главный источник багов в плагинных системах на WASM.

- [x] Разобрать паттерн «указатель + длина»: как хост записывает данные в память гостя и как экспортированные `malloc`/`free` дают гостю выделить буфер для хоста (2026.08.17)
- [x] Написать вручную пример: C++ хост копирует строку в память WASM-модуля, вызывает функцию, читает результат обратно (2026.08.17)
- [x] Посмотреть, как это решает готовая библиотека — polistrate-пример [Extism PDK](https://extism.org/docs/concepts/pdk) (не обязательно ставить, достаточно прочитать концепцию памяти) (2026.08.17)

### День 4 Встраивание рантайма в C++ хост

Цель: написать минимальный C++ host-процесс, который сам загружает и исполняет .wasm файл.

- [x] Выбрать рантайм с C API: [Wasmtime C API](https://docs.wasmtime.dev/c-api/) (более активно развивается, из коробки sandboxing/fuel) или [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime) (легче, C-native, хорош для embedded)  (2026.08.17)
- [x] Написать C++ программу: загрузить `.wasm`, создать `Store`/`Instance`, найти экспортированную функцию по имени, вызвать её, получить результат (2026.08.17)
- [x] Прогнать «плохой» модуль (например, с бесконечным циклом) и убедиться, что хост не падает — это и есть главная причина использовать WASM вместо `dlopen` (2026.08.18)

### День 5 Host functions — API хоста для плагина

Цель: научиться давать плагину доступ к «внешнему миру» только через явно разрешённые функции.

- [x] Зарегистрировать в C++ хосте импортируемую функцию (например, `host_log(ptr, len)`) и вызвать её из кода плагина (2026.08.18)
- [x] Понять модель линковки импортов/экспортов: почему это безопаснее, чем то, что плагин через `dlopen` получает вообще всё адресное пространство процесса (2026.08.18)
- [x] Прочитать про реальный пример на проде — [Wasm-плагины в Apache Traffic Server](https://docs.trafficserver.apache.org/en/latest/admin-guide/plugins/wasm.en.html) или Envoy Wasm filters (поиск "envoy wasm filters") (2026.08.18)

### День 6 Проектирование контракта плагина

Цель: спроектировать собственный минимальный ABI плагина — то, что в реальном проекте станет "SDK для авторов плагинов".

- [x] Определить набор функций жизненного цикла плагина: `plugin_init`, `plugin_process`, `plugin_shutdown` (простой C ABI) (2026.08.20)
- [x] Продумать версионирование интерфейса (что происходит, если плагин собран под старую версию API хоста) (2026.08.20)
- [x] Прочитать статью ["Building Native Plugin Systems with WebAssembly Components"](https://tartanllama.xyz/posts/wasm-plugins/) (Sy Brand) — разбирает именно проблему безопасности/интерфейсов/бинарной совместимости при плагинах (2026.08.20)

### День 7 Ресурсные лимиты и отказоустойчивость

Цель: научиться ограничивать плагин по времени/памяти и красиво обрабатывать его сбои.

- [x] Изучить fuel-based и epoch-based прерывание выполнения в Wasmtime (защита от зависшего/злонамеренного плагина) (2026.08.20)
- [x] Настроить лимит на размер линейной памяти модуля (2026.08.20)
- [x] Обработать trap (паника внутри WASM) в C++ хосте так, чтобы это превращалось в обычную ошибку, а не крэш процесса (2026.08.20)

### День 8 Мини-проект: рабочая плагинная система

Цель: увидеть современный, типобезопасный способ описывать интерфейс плагина — это то, к чему сейчас идёт вся индустрия вместо ручного маршалинга.

- [x] C++ хост, который сканирует папку с `.wasm`-файлами и загружает их как плагины (2026.08.21)
- [x] 2–3 простых плагина на C (например, «преобразование текста»: upper-case, реверс строки, подсчёт слов), реализующих общий контракт из Дня 6 (2026.08.21)
- [x] Один намеренно «сломанный» плагин (бесконечный цикл или паника) — хост должен корректно его изолировать и продолжить работу с остальными (2026.08.21)
- [x] Замерить время загрузки модуля и вызова функции — почувствовать порядок величины накладных расходов (2026.08.21)

### День 9 Component Model и WIT — куда движется экосистема

Цель: увидеть современный, типобезопасный способ описывать интерфейс плагина — это то, к чему сейчас идёт вся индустрия вместо ручного маршалинга.

- [x] Прочитать официальный гайд Wasmtime ["An Application with Plugins"](https://docs.wasmtime.dev/wasip2-plugins.html) — эталонный пример плагинной системы на Component Model + WIT (хост на Rust, но архитектура универсальна) (2026.08.21)
- [x] Написать простой `.wit`-файл, описывающий интерфейс плагина (функции + типы), сгенерировать биндинги через [wit-bindgen](https://github.com/bytecodealliance/wit-bindgen) для C (2026.08.21)
- [x] Превратить обычный `.wasm`-модуль в компонент через [wasm-tools](https://github.com/bytecodealliance/wasm-tools) (2026.08.21)
- [x] Учесть нюанс: поддержка Component Model в C API Wasmtime/WAMR менее зрелая, чем в Rust — для реального проекта это стоит проверить на актуальном состоянии перед выбором (2026.08.22)

### День 10 Обзор экосистемы: что не изобретать самому

Цель: понимать, где заканчивается «изучение основ» и начинается «взять готовое решение».

- [x] Плагин на C++ (component model) (2026.08.23)
- [x] Плагин на C++ (2026.08.23)
- [x] Плагин на C++ (async) (2026.08.23)
- [x] Плагин на rust (wasm) (2026.08.25)
- [x] Плагин на rust (wit) (2026.08.25)
- [ ] arch host c++ - proxy rust - plugin
- [ ] Прочитать сравнение рантаймов [Wasmtime vs Wasmer vs WasmEdge (2026)](https://reintech.io/blog/wasmtime-vs-wasmer-vs-wasmedge-wasm-runtime-comparison-2026), чтобы понимать текущий ландшафт
- [ ] Посмотреть на [Extism](https://extism.org/) — готовый фреймворк именно под «WASM как плагины», с SDK для C++ хоста и PDK для авторов плагинов на разных языках
