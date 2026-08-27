---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

**Плагин** — тот же контракт `plugin_abi.h`, что и у C/C++/Python-версий: `plugin_abi_version/init/alloc/free/process/shutdown`, `(ptr,len)` на входе, упакованный `u64` на выходе. `crate-type = ["cdylib"]` вместо `bin` — у библиотечного крейта нет `fn main()`, поэтому получается reactor-модуль без `_start`, тот же смысл, что `-nostartfiles -Wl,--no-entry` в сишных примерах.

Единственное место, где контракт реально трётся об идиомы Rust — `plugin_free(ptr)`. В C `free()` сама помнит размер блока (аллокатор хранит его в служебном заголовке). У Rust `std::alloc::dealloc` требует **тот же `Layout`** (включая размер), с которым звали `alloc()` — без этого UB. А контракт `plugin_free(ptr)` размер не передаёт вообще. Решил так же, как это делает malloc внутри себя: `plugin_alloc` прячет размер в 4 байта (`size_of::<usize>()` на wasm32) перед возвращаемым указателем, `plugin_free` их оттуда читает. Контракт снаружи остаётся однопараметрическим, как у всех остальных языков в этой сессии — просто изнутри пришлось воспроизвести то, что в C дают бесплатно.

Собрать и прогнать у себя:

```bash
rustup target add wasm32-wasip1
cd upper_rust && cargo build --release --target wasm32-wasip1
cp target/wasm32-wasip1/release/upper_rust.wasm miniproject/plugins/
```

и запустить `host_miniproject` (тот, что уже умеет `Linker::define_wasi()` — на всякий случай, я не могу проверить, тянет ли рантайм Rust какие-то WASI-импорты сам по себе, как это было с libc++ у C++-плагина, поэтому лучше подстраховаться тем же хостом, а не «голым» `Instance::create`).

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
project(host CXX)

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

add_executable(host host.cpp)
if(WIN32)
    target_link_libraries(host PRIVATE wasmtime)
    add_custom_command(TARGET host POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host>"
    )
else()
    target_link_libraries(host PRIVATE wasmtime pthread dl m)
endif()


#add_library(ide OBJECT upper.cpp)

```

### host.cpp
```cpp
// День 8 -- мини-проект: хост сканирует папку plugins/, загружает каждый
// .wasm как плагин по контракту из plugin_abi.h, единообразно защищает
// КАЖДЫЙ вызов plugin_process топливным лимитом (fuel), изолирует
// сломанный плагин (не падает и не останавливает обработку остальных),
// и замеряет время компиляции модуля и время вызова функции.

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <ranges>

#include <wasmtime.hh>

// Единый лимит топлива на весь жизненный цикл плагина (abi_version -> init
// -> process -> shutdown). Специально не завышен: реальным плагинам из
// этого проекта его с большим запасом хватает, а "сломанный" плагин с
// while(1) исчерпает его практически мгновенно -- защита одинакова для
// всех, никакого спецкейса под broken.wasm в коде хоста нет.

namespace {
    constexpr uint64_t FUEL_BUDGET{5'000'000};
    const std::string TEST_INPUT{"Hello WASM plugin world, this is a test"};
    const std::filesystem::path PLUGIN_DIR{"plugins"};
    const std::string EXT{".wasm"};

    struct PluginResult {
        bool ok{false};
        std::string name;
        std::string status;
        std::string output;
        double compile_ms{0.0};
        double call_ms{0.0};
    };

    std::vector<uint8_t> read_file(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    std::optional<wasmtime::Func> get_export(wasmtime::Store& store,
                                             wasmtime::Instance& instance,
                                             const char* name) {
        const auto e{instance.get(store, name)};
        if (!e || !std::holds_alternative<wasmtime::Func>(*e)) return std::nullopt;
        return std::get<wasmtime::Func>(*e);
    }

}

int main(int argc, char *argv[]) {
    std::vector<std::filesystem::path> wasm_files;
    for (const auto& entry: std::filesystem::directory_iterator(PLUGIN_DIR)) {
        if (entry.path().extension() != EXT) continue;
        wasm_files.push_back(entry.path());
    }
    std::ranges::sort(wasm_files);

    std::cout << std::format("Found {} .wasm files in {}:\n",
        wasm_files.size(),
        PLUGIN_DIR.string());
    for (const auto& p: wasm_files) std::cout << std::format("  {}\n", p.filename().string());
    std::cout << '\n';

    wasmtime::Config config;
    config.consume_fuel(true);
    wasmtime::Engine engine{std::move(config)};

    std::vector<PluginResult> results;
    for (const auto& path: wasm_files) {
        PluginResult r;
        r.name = path.filename().string();
        std::cout << std::format("==== {} ====\n", r.name);

        auto bytes{read_file(path)};

        auto t0{std::chrono::steady_clock::now()};
        auto module_result{wasmtime::Module::compile(engine, bytes)};
        auto t1{std::chrono::steady_clock::now()};
        r.compile_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (!module_result) {
            r.status = std::format("COMPILATION ERROR: {}\n", module_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        wasmtime::Module module{module_result.unwrap()};
        std::cout << std::format("  Module compiled in {:.3f} ms\n", r.compile_ms);

        wasmtime::Store store{engine};
        // Топливный лимит выставляется ОДИНАКОВО для каждого плагина, ДО
        // того как хост знает, "хороший" он или "сломанный" -- это и есть
        // защита по умолчанию, а не отдельная ветка для broken.wasm.
        store.context().set_fuel(FUEL_BUDGET).unwrap();

        // upper_cpp.wasm (C++, libc++) тянет за собой WASI-импорты
        // (fd_write/fd_seek/fd_close) через свои abort/terminate-пути, даже
        // с -fno-exceptions -fno-rtti -- ровно та же история, что с Extism
        // и с impl.cpp в Дне 9. Чисто-сишные плагины этого не делают, но
        // хост не должен полагаться на язык плагина -- поэтому WASI теперь
        // подключается всегда, через Linker вместо голого Instance::create.
        wasmtime::WasiConfig wasi;
        wasi.inherit_stdout();
        wasi.inherit_stderr();
        store.context().set_wasi(std::move(wasi)).unwrap();

        wasmtime::Linker linker{engine};
        linker.define_wasi().unwrap();
        auto instance_result{linker.instantiate(store, module)};
        if (!instance_result) {
            r.status = std::format("INSTANTIATION ERROR: {}\n", instance_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        wasmtime::Instance instance{instance_result.unwrap()};

        auto abi_version_fn{get_export(store, instance, "plugin_abi_version")};
        auto init_fn{get_export(store, instance, "plugin_init")};
        auto alloc_fn{get_export(store, instance, "plugin_alloc")};
        auto free_fn{get_export(store, instance, "plugin_free")};
        auto process_fn{get_export(store, instance, "plugin_process")};
        auto shutdown_fn{get_export(store, instance, "plugin_shutdown")};
        auto memory_export{instance.get(store, "memory")};

        if (!abi_version_fn ||
            !init_fn ||
            !alloc_fn ||
            !free_fn ||
            !process_fn ||
            !shutdown_fn ||
            !memory_export || !std::holds_alternative<wasmtime::Memory>(*memory_export)) {

            r.status = std::format("ERROR: plugin does not implement contract completely");
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        wasmtime::Memory memory{std::get<wasmtime::Memory>(*memory_export)};

        // 1) abi_version
        auto abi_res{abi_version_fn->call(store, {})};
        if (!abi_res) {
            r.status = std::format("TRAP on plugin_abi_version: {}\n", abi_res.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        if (const int32_t abi_version{abi_res.unwrap()[0].i32()}; abi_version != 1) {
            r.status = std::format("ABI VERSION MISMATCHING: {}", abi_version);
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 2) init
        auto init_result{init_fn->call(store, {})};
        if (!init_result) {
            r.status = std::format("TRAP on plugin_init: {}\n", init_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        if (init_result.unwrap()[0].i32() != 0) {
            r.status = "plugin_init returned error code";
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 3) записываем тестовый вход в память гостя
        auto in_alloc_result{alloc_fn->call(store, {static_cast<int32_t>(TEST_INPUT.size())})};
        if (!in_alloc_result) {
            r.status = std::format("TRAP on plugin_alloc: {}\n", in_alloc_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        const int32_t in_ptr{in_alloc_result.unwrap()[0].i32()};
        std::memcpy(memory.data(store).data() + in_ptr, TEST_INPUT.data(), TEST_INPUT.size());

        // 4) вызов plugin_process -- под тем же топливным лимитом, что и
        // весь остальной жизненный цикл. Именно здесь "сломанный" плагин
        // споткнётся об исчерпание топлива.
        const auto t2{std::chrono::steady_clock::now()};
        auto process_result{process_fn->call(store, {
            in_ptr,
            static_cast<int32_t>(TEST_INPUT.size()),})};
        const auto t3{std::chrono::steady_clock::now()};
        r.call_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        if (!process_result) {
            // Именно эта ветка ловит "сломанный" плагин: хост не падает,
            // печатает понятную ошибку и переходит к следующему плагину.
            const std::string full_message{process_result.err().message()};
            // Для сводки вытаскиваем содержательную строку "wasm trap: ..."
            // (сама причина), а не первую строку backtrace'а; полный текст
            // остаётся виден в подробном логе выше.
            std::string short_message{full_message};
            const auto trap_pos{full_message.find("wasm trap:")};
            if (trap_pos != std::string::npos) {
                std::string rest{full_message.substr(trap_pos)};
                short_message = rest.substr(0, rest.find('\n'));
            } else {
                short_message = full_message.substr(0, full_message.find('\n'));
            }
            r.status = std::format("TRAP on plugin_process: {}\n", short_message);
            std::cout << std::format("  plugin_process failed in {:.3f} ms\n", r.call_ms);
            std::cout << std::format(" {}\n\n", full_message);
            results.push_back(r);
            continue;
        }

        const uint64_t packed{static_cast<uint64_t>(process_result.unwrap()[0].i64())};
        const int32_t out_ptr{static_cast<int32_t>(static_cast<uint32_t>(packed >> 32))};
        const int32_t out_len{static_cast<int32_t>(static_cast<uint32_t>(packed & 0xFFFFFFFFu))};
        if (out_ptr == 0) {
            r.status = std::format("plugin_process notify about error (ptr == 0)");
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        std::string output{
            reinterpret_cast<char*>(memory.data(store).data() + out_ptr),
            static_cast<std::string::size_type>(out_len)
        };

        // 5) освобождаем оба буфера
        (void)free_fn->call(store, {in_ptr});
        (void)free_fn->call(store, {out_ptr});

        // 6) shutdown
        (void)shutdown_fn->call(store, {});

        r.ok = true;
        r.status = "OK";
        r.output = output;
        std::cout << std::format("  Calling plugin_process() took {:.3f} ms\n", r.call_ms);
        std::cout << std::format("  Input: '{}'\n", TEST_INPUT);
        std::cout << std::format("  Output: '{}'\n", output);

        results.push_back(r);
    }

    // Итоговая сводка
    std::cout << "===== CONCLUSION =====\n";
    std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n", "Plugin", "Status", "Compilation(ms)", "Call(ms)");
    for (const auto& r : results) {
        std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n",
            r.name,
            (r.ok ? "OK" : "FAIL"),
            r.compile_ms,
            r.call_ms);
    }

    std::cout << "\nError details:\n";
    int ok_count{};
    for (const auto& r : results) {
        if (r.ok) ok_count++;
        else std::cout << std::format("  {}: {}\n", r.name, r.status);
    }
    std::cout << std::format("\nProcessed {} out of {}\n", ok_count, results.size());

    return 0;
}

```


### cargo.toml
```toml
[package]
name = "upper_rust"
version = "0.1.0"
edition = "2021"

[lib]
# cdylib, а не bin -- у библиотечного крейта нет fn main(), поэтому
# итоговый .wasm получается reactor-модулем без _start (тот же смысл,
# что -nostartfiles -Wl,--no-entry в сишных примерах этой сессии), а не
# command-модулем.
crate-type = ["cdylib"]

[profile.release]
# оптимизация по размеру -- типично для wasm-плагинов
opt-level = "z"
# без unwinding -- меньше кода, нет зависимостей на eh_personality
panic = "abort"
lto = true
```
### src/lib.rs
```rust
/*
Плагин на Rust под тот же ручной ABI (без Component Model) --
plugin_abi.h из Дня 6/8: plugin_abi_version/init/alloc/free/process/
shutdown, вход и выход -- (ptr, len), результат process() упакован в
один u64, ровно как у C/C++-версий из этой сессии.

Единственное место, где контракт C ABI трётся об идиомы Rust --
plugin_free(ptr). В C free() сама помнит размер выделенного блока
(аллокатор хранит его в служебном заголовке перед данными). У Rust
std::alloc::dealloc никакой памяти не хватает -- ей ОБЯЗАТЕЛЬНО нужен
тот же Layout (включая размер), с которым звали alloc(), иначе UB.
А контракт plugin_free(ptr) размер не передаёт вообще. Решение здесь
то же самое, что делает malloc/free внутри себя: перед пользовательскими
данными прячется маленький заголовок с размером -- см. HEADER_SIZE.

rustup target add wasm32-wasip1
cargo build --release --target wasm32-wasip1
cp ./target/wasm32-wasip1/release/upper_rust.wasm ./plugins/

*/

use std::alloc::{alloc, dealloc, Layout};
use std::slice;

const PLUGIN_ABI_VERSION: i32 = 1;

// На wasm32 usize -- 4 байта, так что заголовок съедает всего 4 байта
// перед данными, а не 8, как было бы на x86_64.
const HEADER_SIZE: usize = std::mem::size_of::<usize>();

static mut INITIALIZED: bool = false;

#[no_mangle]
pub extern "C" fn plugin_abi_version() -> i32 {
    PLUGIN_ABI_VERSION
}

#[no_mangle]
pub extern "C" fn plugin_init() -> i32 {
    unsafe {
        INITIALIZED = true;
    }
    0
}

#[no_mangle]
pub extern "C" fn plugin_alloc(size: i32) -> *mut u8 {
    if size < 0 {
        return std::ptr::null_mut();
    }
    let payload = size as usize;
    let total = payload + HEADER_SIZE;

    let layout = match Layout::from_size_align(total, HEADER_SIZE) {
        Ok(l) => l,
        Err(_) => return std::ptr::null_mut(),
    };

    unsafe {
        let base = alloc(layout);
        if base.is_null() {
            return std::ptr::null_mut();
        }
        // Прячем размер payload'а перед данными -- ровно та бухгалтерия,
        // которую в обычном malloc() делает сам аллокатор незаметно для нас.
        *(base as *mut usize) = payload;
        base.add(HEADER_SIZE)
    }
}

#[no_mangle]
pub extern "C" fn plugin_free(ptr: *mut u8) {
    if ptr.is_null() {
        return;
    }

    unsafe {
        let base = ptr.sub(HEADER_SIZE);
        let payload = *(base as *const usize);
        let total = payload + HEADER_SIZE;
        if let Ok(layout) = Layout::from_size_align(total, HEADER_SIZE) {
            dealloc(base, layout);
        }
    }
}

#[no_mangle]
pub extern "C" fn plugin_process(in_ptr: *const u8, in_len: i32) -> u64 {
    if in_len < 0 || (unsafe { !INITIALIZED }) {
        return 0;
    }
    if in_ptr.is_null() {
        return 0;
    }

    // Безопасный Rust начинается сразу после границы с "сырым" ABI:
    // из (ptr, len) собираем обычный &[u8] и дальше работаем с ним как
    // с нормальными срезами/итераторами, а не руками бегаем по указателю.
    let input: &[u8] = unsafe { slice::from_raw_parts(in_ptr, in_len as usize) };
    let upper: Vec<u8> = input.iter().map(u8::to_ascii_uppercase).collect();
    let len = upper.len();

    let out_ptr = plugin_alloc(len as i32);
    if out_ptr.is_null() {
        return 0;
    }

    unsafe {
        std::ptr::copy_nonoverlapping(upper.as_ptr(), out_ptr, len);
    }

    // Та же упаковка (ptr<<32)|len, что и в C: явно проходим через u32,
    // чтобы обрезать указатель до 32 бит -- на wasm32 он и так влезает
    // целиком, но так контракт совпадает с C-версией буква в букву.
    ((out_ptr as u32 as u64) << 32) | (len as u32 as u64)
}

#[no_mangle]
pub extern "C" fn plugin_shutdown() {
    unsafe {
        INITIALIZED = false;
    }
}

```
