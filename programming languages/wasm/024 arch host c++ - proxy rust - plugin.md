---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

Всё собрано и прогнано целиком, с нуля, живьём (`cmake -S . -B build && cmake --build build` пересобрал и Rust-библиотеку, и C++-хост, а `plugin_src/build_plugin.sh` заново перегнал `impl.c` в `plugin_component.wasm` через `wit-bindgen c` → wasi-sdk clang → `wasm-tools component new`). Финальный прогон:

```
proxy ABI version: 1
process() до init(): ошибка (ожидаемо) -- not-initialized
init() выполнен
info(): name="upper-wit-demo", abi_version=1
process("hello from cpp host") -> Ok("HELLO FROM CPP HOST")
process("") (ожидаемо) -- empty-input
shutdown(): код 0
process() после shutdown() (ожидаемо) -- not-initialized
```

По пунктам:

**1) WIT** — переиспользовал `example:plugin/transform` из Дня 9 (`init/info/process/shutdown`, `process: string -> result<string, process-error>`) — он уже ровно "transform", как вы и просили.

**2) Плагин на C (upper)** — `impl.c`, тот же, что в Дне 9: поднимает `'a'..'z'` в верхний регистр (ASCII), различает состояния `not-initialized`/`empty-input`. `wasm-tools component wit` на собранном `plugin_component.wasm` показывает: **ноль импортов** — это плагину пригодилось напрямую в пункте 3.

**3) Rust-библиотека** (`transform_proxy`, `.so`/`.dll`) — грузит `.wasm` по имени файла (`proxy_load(engine, path, ...)`) и даёт ручки управления: `proxy_init`, `proxy_shutdown`, `proxy_info`, и главную — `proxy_process(component, input_ptr, input_len, &out_ptr, &out_len, ...)`, строка на входе → строка на выходе. Раз у плагина нет импортов вообще, линкер внутри — пустой (`Linker<()>`, без WASI) — принцип наименьших полномочий вместо "на всякий случай подключить всё". Владение выходной строкой — через `proxy_free_string(ptr, len)`, не `free()`: в отличие от `plugin_free(ptr)` из ручного ABI Дня 6 (там нужен был скрытый заголовок с размером), здесь размер передаётся явно вторым параметром, потому что обе половины контракта проектируем сами.

**4) Хост на C++** — `host_transform_test.cpp`, не включает ни одного заголовка wasmtime, только `transform_proxy.h`. Прогоняет полный жизненный цикл: `process()` до `init()` (ожидаемая ошибка), `init()`, `info()`, `process()` с реальной строкой, `process("")`, `shutdown()`, `process()` после `shutdown()` — то же самое покрытие, что было в `host_component.cc`, но теперь через C ABI Rust-библиотеки, а не напрямую через `wasmtime::component`.

**5) CMakeLists.txt** — сам вызывает `cargo build --release` как часть обычной сборки, копирует `plugin_component.wasm` рядом с исполняемым файлом, на Linux/macOS выставляет `rpath`, на Windows — копирует `.dll` рядом с `.exe` (та же схема, что в прошлый раз для `wasm_proxy`).

## Пример

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

### impl.c
```c
// Реализация плагина поверх сгенерированных wit-bindgen биндингов.
// Обрати внимание: никакого ручного malloc/free буфера под результат,
// никакой упаковки (ptr<<32)|len -- это всё уже сделано генератором
// (plugin_string_dup/plugin_string_set) и каноническим ABI компонент-модели.

#include <string.h>
#include <stdlib.h>
#include "plugin_src/generated/plugin.h"

static int g_initialized = 0;


void exports_example_plugin_transform_init(void) {
    g_initialized = 1;
}

void exports_example_plugin_transform_info(exports_example_plugin_transform_plugin_info_t *ret) {
    plugin_string_set(&ret->name, "upper-wit-demo");
    ret->abi_version = 1;
}

bool exports_example_plugin_transform_process(plugin_string_t *input,
                                              plugin_string_t *ret,
                                              exports_example_plugin_transform_process_error_t *err) {
    // Внимание: сгенерированный шим делает `ret.is_err = !process(...)`
    // (см. generated/plugin.c) -- то есть возврат true здесь означает
    // Ok (заполнен ret), а false -- Err (заполнен err). Это НЕ то же
    // самое, что "true = есть ошибка", как можно интуитивно подумать.
    if (!g_initialized) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_NOT_INITIALIZED;
        return false;
    }
    if (input->len == 0) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_EMPTY_INPUT;
        return false;
    }

    // Та же трансформация, что в upper.c из Дня 8 -- но вход/выход уже
    // не (ptr,len) пара, а plugin_string_t, и хосту не нужно самому
    // звать plugin_alloc/plugin_free для этого буфера.
    char* buf = (char*)malloc(input->len);
    for (size_t i = 0; i < input->len; i++) {
        char c = ((char*)input->ptr)[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }

    ret->ptr = (uint8_t*)buf;
    ret->len = input->len;

    return true;
}

void exports_example_plugin_transform_shutdown(void) {
    g_initialized = 0;
}

```

## Cargo.toml
```toml
[package]
name = "transform_proxy"
version = "0.1.0"
edition = "2021"

[lib]
# cdylib -- та же идея, что в wasm_proxy: наружу торчит только C ABI
# (см. include/transform_proxy.h), C++-хост про wasmtime не знает вообще.
crate-type = ["cdylib"]
name = "transform_proxy"

[dependencies]
wasmtime = { version = "48.0.0", features = ["component-model"] }
# Обрати внимание: в отличие от wasm_proxy (netcheck, wasi:sockets),
# здесь НЕТ зависимости wasmtime-wasi. plugin_component.wasm (см.
# `wasm-tools component wit`) не импортирует ничего -- ни WASI, ни
# чего-либо ещё, значит и подключать WASI в линкер не нужно. Меньше
# зависимостей, меньше поверхность атаки на плагин -- принцип наименьших
# полномочий, а не "на всякий случай добавить всё".
anyhow = "1.0.104"

[profile.release]
# обязательно, иначе catch_unwind на границе FFI бесполезен
panic = "unwind"
```

### src/lib.rs
```rust
// Прокси-библиотека под интерфейс example:plugin/transform (тот же
// WIT, что в wit_plugin/plugin.wit, тот же C-плагин impl.c из Дня 9).
// В отличие от wasm_proxy (там был один жёстко зашитый вызов
// tcp-ping), здесь "ручки" покрывают весь жизненный цикл плагина --
// init/process/shutdown/info -- но всё ещё без общего динамического
// маршалинга произвольных WIT-функций: имена функций и их сигнатуры
// (в частности process: string -> result<string, process-error>)
// зашиты в код так же жёстко, как имя интерфейса.
//
// C++-хост видит только include/transform_proxy.h -- ни wasmtime, ни
// Val, ни ExportIndex он не знает.

use std::ffi::{c_char, CStr};
use std::os::raw::c_int;
use std::panic::{catch_unwind, AssertUnwindSafe};
use std::ptr;

use wasmtime::component::{Component, Func, Linker, Val};
use wasmtime::{Config, Engine, Store};

pub struct ProxyEngine {
    engine: Engine,
}

/// Кэшируем индексы всех четырёх функций интерфейса в момент загрузки,
/// чтобы каждый proxy_process()/proxy_init() не делал get_export_index
/// заново -- ровно то же соображение, что "cached export indices", уже
/// использованное в host_component.cc.
pub struct ProxyComponent {
    store: Store<()>,
    init_fn: Func,
    info_fn: Func,
    process_fn: Func,
    shutdown_fn: Func,
}

#[repr(C)]
pub enum ProxyStatus {
    Ok = 0,
    NullArg = -1,
    Load = -2,
    ExportNotFound = -3,
    Trap = -4,
    // гость вернул result::err(process-error)
    Application = -5,
    InvalidUtf8 = -6,
    Panic = -99,
}

fn write_cstr(buf: *mut c_char, buf_len: usize, msg: &str) {
    if buf.is_null() || buf_len == 0 {
        return;
    }
    let bytes = msg.as_bytes();
    let n = bytes.len().min(buf_len - 1);

    unsafe {
        ptr::copy_nonoverlapping(bytes.as_ptr() as *const c_char, buf, n);
        *buf.add(n) = 0;
    }
}

#[no_mangle]
pub extern "C" fn proxy_abi_version() -> i32 {
    1
}

#[no_mangle]
pub extern "C" fn proxy_engine_new() -> *mut ProxyEngine {
    match catch_unwind(|| {
        let mut config = Config::new();
        config.wasm_component_model(true);
        Engine::new(&config)
    }) {
        Ok(Ok(engine)) => Box::into_raw(Box::new(ProxyEngine { engine })),
        _ => ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn proxy_engine_free(engine: *mut ProxyEngine) {
    if engine.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(engine));
    }
}

/// Загружает файл по имени (путь передаёт C++), инстанцирует
/// компонент и сразу же кэширует индексы всех функций
/// example:plugin/transform. init() здесь НЕ вызывается автоматически:
/// вызывающая сторона сама решает, когда переводить плагин в рабочее
/// состояние (proxy_init()) -- то есть "ручка" управления жизненным
/// циклом торчит наружу, а не спрятана внутри загрузки.
#[no_mangle]
pub extern "C" fn proxy_load(
    engine: *mut ProxyEngine,
    path: *const c_char,
    err_buf: *mut c_char,
    err_buf_len: usize,
) -> *mut ProxyComponent {
    if engine.is_null() || path.is_null() {
        write_cstr(err_buf, err_buf_len, "null engine/path");
        return ptr::null_mut();
    }

    let outcome: Result<ProxyComponent, String> = (|| {
        let engine_ref = unsafe { &(*engine).engine };
        let path_str = unsafe { CStr::from_ptr(path) }
            .to_str()
            .map_err(|e| format!("path is not UTF-8: {e}"))?;

        let component = Component::from_file(engine_ref, path_str)
            .map_err(|e| format!("component loading error: {e}"))?;

        // Пустой линкер -- у plugin_component.wasm нет ни одного
        // импорта (проверено `wasm-tools component wit`), значит и
        // подключать WASI незачем: инстанцирование либо пройдёт как
        // есть, либо честно упадёт, если у плагина найдётся
        // неожиданный импорт (в этом случае -- расширять линкер).
        let linker: Linker<()> = Linker::new(engine_ref);
        let mut store = Store::new(engine_ref, ());

        let instance = linker
            .instantiate(&mut store, &component)
            .map_err(|e| format!("instantiation error: {e}"))?;

        let (_, transform_idx) = instance
            .get_export(&mut store, None, "example:plugin/transform@0.1.0")
            .ok_or_else(|| "interface 'example:plugin/transform@0.1.0' not found".to_string())?;

        let get_fn = |store: &mut Store<()>, name: &str| -> Result<Func, String> {
            let (_, idx) = instance
                .get_export(&mut *store, Some(&transform_idx), name)
                .ok_or_else(|| format!("function '{name}' not found"))?;
            instance
                .get_func(&mut *store, &idx)
                .ok_or_else(|| format!("export '{name}' exists but it is not function"))
        };

        let init_fn = get_fn(&mut store, "init")?;
        let info_fn = get_fn(&mut store, "info")?;
        let process_fn = get_fn(&mut store, "process")?;
        let shutdown_fn = get_fn(&mut store, "shutdown")?;

        Ok(ProxyComponent {
            store,
            init_fn,
            info_fn,
            process_fn,
            shutdown_fn,
        })
    })();

    match catch_unwind(AssertUnwindSafe(|| outcome)) {
        Ok(Ok(pc)) => Box::into_raw(Box::new(pc)),
        Ok(Err(msg)) => {
            write_cstr(err_buf, err_buf_len, &msg);
            ptr::null_mut()
        },
        Err(_) => {
            write_cstr(err_buf, err_buf_len, "panic in proxy_load");
            ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn proxy_free(component: *mut ProxyComponent) {
    if component.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(component));
    }
}

#[no_mangle]
pub extern "C" fn proxy_init(
    component: *mut ProxyComponent,
    err_buf: *mut c_char,
    err_buf_len: usize,
) -> c_int {
    if component.is_null() {
        write_cstr(err_buf, err_buf_len, "null component");
        return ProxyStatus::NullArg as c_int;
    }
    let result = catch_unwind(AssertUnwindSafe(|| {
        let pc = unsafe { &mut *component };
        pc.init_fn.call(&mut pc.store, &[], &mut [])
    }));
    match result {
        Ok(Ok(())) => ProxyStatus::Ok as c_int,
        Ok(Err(trap)) => {
            write_cstr(err_buf, err_buf_len, &format!("TRAP: {trap}"));
            ProxyStatus::Trap as c_int
        },
        Err(_) => {
            write_cstr(err_buf, err_buf_len, "panic in proxy_init");
            ProxyStatus::Panic as c_int
        }
    }
}

#[no_mangle]
pub extern "C" fn proxy_shutdown(
    component: *mut ProxyComponent,
    err_buf: *mut c_char,
    err_buf_len: usize,
) -> c_int {
    if component.is_null() {
        write_cstr(err_buf, err_buf_len, "null component");
        return ProxyStatus::NullArg as c_int;
    }
    let result = catch_unwind(AssertUnwindSafe(|| {
        let pc = unsafe { &mut *component };
        pc.shutdown_fn.call(&mut pc.store, &[], &mut [])
    }));

    match result {
        Ok(Ok(())) => ProxyStatus::Ok as c_int,
        Ok(Err(trap)) => {
            write_cstr(err_buf, err_buf_len, &format!("TRAP: {trap}"));
            ProxyStatus::Trap as c_int
        },
        Err(_) => {
            write_cstr(err_buf, err_buf_len, "panic in proxy_shutdown");
            ProxyStatus::Panic as c_int
        }
    }
}

/// Метаданные плагина: info() -> record { name: string, abi-version: u32 }.
#[no_mangle]
pub extern "C" fn proxy_info(
    component: *mut ProxyComponent,
    name_buf: *mut c_char,
    name_buf_len: usize,
    abi_version_out: *mut u32,
    err_buf: *mut c_char,
    err_buf_len: usize,
) -> c_int {
    if component.is_null() {
        write_cstr(err_buf, err_buf_len, "null component");
        return ProxyStatus::NullArg as c_int;
    }
    let result = catch_unwind(AssertUnwindSafe(|| {
        let pc = unsafe { &mut *component };
        let mut results = [Val::Bool(false)];
        pc.info_fn.call(&mut pc.store, &[], &mut results)?;
        anyhow::Result::<Val>::Ok(results.into_iter().next().unwrap())
    }));

    match result {
        Ok(Ok(Val::Record(fields))) => {
            let mut name = String::new();
            let mut abi_version: u32 = 0;
            for (field_name, value) in fields {
                match (field_name.as_str(), value) {
                    ("name", Val::String(s)) => name = s,
                    ("abi-version", Val::U32(v)) => abi_version = v,
                    _ => {}
                }
            }
            write_cstr(name_buf, name_buf_len, &name);
            if !abi_version_out.is_null() {
                unsafe { *abi_version_out = abi_version }
            }
            ProxyStatus::Ok as c_int
        }
        Ok(Ok(_)) => {
            write_cstr(err_buf, err_buf_len, "unexpected form of result info()");
            ProxyStatus::ExportNotFound as c_int
        }
        Ok(Err(trap)) => {
            write_cstr(err_buf, err_buf_len, &format!("TRAP: {trap}"));
            ProxyStatus::Trap as c_int
        }
        Err(_) => {
            write_cstr(err_buf, err_buf_len, "panic in proxy_info");
            ProxyStatus::Panic as c_int
        }
    }
}

/// Собственно "ручка" из пункта 3: строка на входе, строка на выходе.
/// Вход берём как (ptr, len), а не как NUL-terminated C-строку -- тот
/// же принцип, что и в plugin_abi.h из Дня 6: строка не обязана быть
/// NUL-terminated и может содержать произвольные UTF-8 байты.
///
/// Выходная строка -- через (out_ptr, out_len): память выделяет Rust
/// (Vec<u8> -> into_raw_parts), владение переходит к вызывающей
/// стороне, освобождать обязательно через proxy_free_string(), а не
/// через free()/delete[] на C++-стороне -- аллокаторы разные. В
/// отличие от plugin_free(ptr) из ручного ABI (Дня 6), здесь не нужен
/// трюк со скрытым заголовком: раз мы сами проектируем обе половины
/// контракта, освобождающая функция просто получает len явным
/// параметром.
#[no_mangle]
pub extern "C" fn proxy_process(
    component: *mut ProxyComponent,
    input_ptr: *const u8,
    input_len: usize,
    out_ptr: *mut *mut u8,
    out_len: *mut usize,
    err_buf: *mut c_char,
    err_buf_len: usize,
) -> c_int {
    if component.is_null() || input_ptr.is_null() || out_ptr.is_null() || out_len.is_null() {
        write_cstr(err_buf, err_buf_len, "null argument");
        return ProxyStatus::NullArg as c_int;
    }

    let input_bytes = unsafe { std::slice::from_raw_parts(input_ptr, input_len) };
    let input_str = match std::str::from_utf8(input_bytes) {
        Ok(s) => s.to_owned(),
        Err(e) => {
            write_cstr(err_buf, err_buf_len, &format!("input is not UTF-8: {e}"));
            return ProxyStatus::InvalidUtf8 as c_int;
        }
    };

    let result = catch_unwind(AssertUnwindSafe(|| {
        let pc = unsafe { &mut *component };
        let args = [Val::String(input_str)];
        let mut results = [Val::Bool(false)];
        pc.process_fn.call(&mut pc.store, &args, &mut results)?;
        anyhow::Result::<Val>::Ok(results.into_iter().next().unwrap())
    }));

    match result {
        Ok(Ok(Val::Result(Ok(Some(payload))))) => {
            if let Val::String(s) = *payload {
                let mut bytes = s.into_bytes().into_boxed_slice();
                let len = bytes.len();
                let ptr = bytes.as_mut_ptr();
                std::mem::forget(bytes);
                unsafe {
                    *out_ptr = ptr;
                    *out_len = len;
                }
                ProxyStatus::Ok as c_int
            } else {
                write_cstr(err_buf, err_buf_len, "expected type of Ok-value");
                ProxyStatus::ExportNotFound as c_int
            }
        }
        Ok(Ok(Val::Result(Err(payload)))) => {
            let msg = match payload {
                Some(inner) => match *inner {
                    Val::Variant(tag, Some(detail)) => match *detail {
                        Val::String(s) => format!("{tag}: {s}"),
                        _ => tag,
                    },
                    Val::Variant(tag, None) => tag,
                    other => format!("{other:?}"),
                },
                None => "process-error without details".to_string(),
            };
            write_cstr(err_buf, err_buf_len, &msg);
            ProxyStatus::Application as c_int
        }
        Ok(Ok(_)) => {
            write_cstr(err_buf, err_buf_len, "unexpected form of result of process()");
            ProxyStatus::ExportNotFound as c_int
        }
        Ok(Err(trap)) => {
            write_cstr(err_buf, err_buf_len, &format!("TRAP: {trap}"));
            ProxyStatus::Trap as c_int
        }
        Err(_) => {
            write_cstr(err_buf, err_buf_len, "panic in proxy_process");
            ProxyStatus::Panic as c_int
        }
    }
}

#[no_mangle]
pub extern "C" fn proxy_free_string(ptr: *mut u8, len: usize) {
    if ptr.is_null() {
        return;
    }
    unsafe {
        drop(Vec::from_raw_parts(ptr, len, len));
    }
}

```

### include/transform_proxy.h
```c
// C ABI для libtransform_proxy.so|dll -- C++-хост знает только этот
// заголовок, ни одного заголовка wasmtime он не подключает.

#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ProxyEngine ProxyEngine;
typedef struct ProxyComponent ProxyComponent;

// 0 = ok, -1 = null arg, -2 = load, -3 = export not found, -4 = trap,
// -5 = application error (process-error из WIT), -6 = вход не UTF-8,
// -99 = внутри .so поймана паника Rust (баг в проксе, не в госте).
int32_t proxy_abi_version(void);

ProxyEngine* proxy_engine_new(void);
void proxy_engine_free(ProxyEngine* engine);

// Загружает .wasm по пути и инстанцирует его. init() НЕ вызывается
// автоматически -- это отдельная ручка ниже.
ProxyComponent* proxy_load(ProxyEngine* engine, const char* path,
                           char* err_buf, size_t err_buf_len);
void proxy_free(ProxyComponent* component);

int32_t proxy_init(ProxyComponent* component, char* err_buf, size_t err_buf_len);
int32_t proxy_shutdown(ProxyComponent* component, char* err_buf, size_t err_buf_len);

int32_t proxy_info(ProxyComponent* component, char* name_buf, size_t name_buf_len,
                   uint32_t* abi_version_out, char* err_buf, size_t err_buf_len);

// Главная "ручка": строка на входе (ptr,len -- не обязана быть
// NUL-terminated), строка на выходе. out_ptr/out_len -- выделены
// Rust-стороной, освобождать ТОЛЬКО через proxy_free_string(), не
// free()/delete[].
int32_t proxy_process(ProxyComponent* component, const uint8_t* input_ptr,
                      size_t input_len, uint8_t** output_ptr, size_t* output_len,
                      char* err_buf, size_t err_buf_len);
void proxy_free_string(uint8_t* ptr, size_t len);

#ifdef __cplusplus
}
#endif

```

### host_transform_test.cpp
```cpp
// Живая проверка [C++ host] <-> [transform_proxy.so, Rust] <-> [C-плагин
// example:plugin/transform, Component Model]. Этот .cpp не включает ни
// одного заголовка wasmtime -- только transform_proxy.h.


#include "include/transform_proxy.h"

#include <iostream>
#include <format>
#include <string>

namespace {
    // Небольшая C++-обёртка вокруг сырых (ptr,len)-ручек -- то, что в
    // реальном проекте вынесли бы в отдельный класс Plugin, здесь для
    // наглядности прямо в main().
    bool call_process(ProxyComponent* component,
                      const std::string& input,
                      std::string& out,
                      std::string& err) {
        uint8_t* out_ptr{nullptr};
        size_t out_len{0};
        char err_buf[256] = {0};

        const int32_t status{proxy_process(
            component,
            reinterpret_cast<const uint8_t*>(input.data()),
            input.size(),
            &out_ptr,
            &out_len,
            err_buf,
            sizeof(err_buf))};

        if (status == 0) {
            out.assign(reinterpret_cast<char*>(out_ptr), out_len);
            // обязательно -- не free()/delete[]
            proxy_free_string(out_ptr, out_len);
            return true;
        }

        err = err_buf;
        return false;
    }
}

int main(int argc, char *argv[]) {
    std::cout << std::format("proxy ABI version: {}\n", proxy_abi_version());

    ProxyEngine* engine{proxy_engine_new()};
    if (!engine) {
        std::cerr << std::format("proxy_engine_new() failed\n");
        return 1;
    }

    char err[256] = {0};
    ProxyComponent* comp{
        proxy_load(engine, "plugin_component.wasm", err, sizeof(err))
    };
    if (!comp) {
        std::cerr << std::format("proxy_load() failed: {}\n", err);
        proxy_engine_free(engine);
        return 1;
    }

    // process() до init() -- плагин обязан вернуть Err(not-initialized),
    // ровно как в host_component.cc из Дня 9.
    std::string out, call_err;
    if (call_process(comp, "too early", out, call_err)) {
        std::cerr << std::format("unexpected: process() until init() OK('{}')\n", out);
    } else {
        std::cerr << std::format("process() until init(): error (expected) -- {}\n", call_err);
    }

    int32_t status{proxy_init(comp, err, sizeof(err))};
    if (status != 0) {
        std::cerr << std::format("proxy_init() failed: ({}): {}\n", status, err);
        proxy_free(comp);
        proxy_engine_free(engine);
        return 1;
    }
    std::cout << "init() executed\n";

    char name_buf[128] = {0};
    uint32_t abi_version = 0;
    status = proxy_info(comp, name_buf, sizeof(name_buf), &abi_version, err, sizeof(err));
    if (status == 0) {
        std::cerr << std::format("info(): name='{}', abi_version={}\n", name_buf, abi_version);
    }

    if (const std::string line0{"hello from cpp host"};
        call_process(comp, line0, out, call_err)) {
        std::cout << std::format("process('{}') -> Ok('{}')\n", line0, out);
    } else {
        std::cerr << std::format("process() failed: {}\n", call_err);
    }

    if (const std::string line1;
        call_process(comp, line1, out, call_err)) {
        std::cerr << std::format("unexpected: process('{}') -> Ok('{}')\n", line1, out);
    } else {
        std::cout << std::format("process('{}') -> '{}'\n", line1, call_err);
    }

    status = proxy_shutdown(comp, err, sizeof(err));
    std::cout << std::format("shutdown(): code {}\n", status);

    if (const std::string line2{"after shutdown"};
        call_process(comp, line2, out, call_err)) {
        std::cerr << std::format("unexpected: process() after shutdown() -> Ok('{}')", out);
    } else {
        std::cout << std::format("process() after shutdown() -> {}\n", call_err);
    }

    proxy_free(comp);
    proxy_engine_free(engine);

    return 0;
}

```

### vcpkg.json
```json
{
    "name": "demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CmakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_transform_test CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TRANSFORM_PROXY_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TRANSFORM_PROXY_TARGET_DIR ${TRANSFORM_PROXY_DIR}/target/release)

if(WIN32)
    set(TRANSFORM_PROXY_LIB ${TRANSFORM_PROXY_TARGET_DIR}/transform_proxy.dll)
    set(TRANSFORM_PROXY_IMPLIB ${TRANSFORM_PROXY_TARGET_DIR}/transform_proxy.dll.lib)
elseif(APPLE)
    set(TRANSFORM_PROXY_LIB ${TRANSFORM_PROXY_TARGET_DIR}/libtransform_proxy.dylib)
else()
    set(TRANSFORM_PROXY_LIB ${TRANSFORM_PROXY_TARGET_DIR}/libtransform_proxy.so)
endif()

# cargo build как часть обычной сборки CMake -- см. подробный комментарий
# в wasm_proxy/CMakeLists.txt из прошлого примера, идея та же самая.
add_custom_command(
        OUTPUT ${TRANSFORM_PROXY_LIB}
        COMMAND cargo build --release
        WORKING_DIRECTORY ${TRANSFORM_PROXY_DIR}
        DEPENDS ${TRANSFORM_PROXY_DIR}/src/lib.rs ${TRANSFORM_PROXY_DIR}/Cargo.toml
        COMMENT "cargo build --release (transform_proxy)"
        VERBATIM
)
add_custom_target(transform_proxy_build DEPENDS ${TRANSFORM_PROXY_LIB})

add_library(transform_proxy SHARED IMPORTED)
set_target_properties(transform_proxy PROPERTIES IMPORTED_LOCATION ${TRANSFORM_PROXY_LIB})

if(WIN32)
    set_target_properties(transform_proxy PROPERTIES IMPORTED_IMPLIB ${TRANSFORM_PROXY_IMPLIB})
endif()
add_dependencies(transform_proxy transform_proxy_build)

add_executable(host_transform_test host_transform_test.cpp)
target_include_directories(host_transform_test PRIVATE ${TRANSFORM_PROXY_DIR}/include)
target_link_libraries(host_transform_test PRIVATE transform_proxy)

if(NOT WIN32)
    set_target_properties(host_transform_test PROPERTIES
        BUILD_RPATH ${TRANSFORM_PROXY_TARGET_DIR})
endif()

# plugin_component.wasm должен лежать рядом с исполняемым файлом --
# proxy_load() открывает его по относительному пути.
add_custom_command(TARGET host_transform_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TRANSFORM_PROXY_DIR}/plugin_component.wasm
        $<TARGET_FILE_DIR:host_transform_test>
)

if(WIN32)
    add_custom_command(TARGET host_transform_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${TRANSFORM_PROXY_LIB} $<TARGET_FILE_DIR:host_transform_test>
    )
endif()

```

### build_plugin.sh
```sh
#!/usr/bin/env bash
# Пересобирает plugin_component.wasm из wit/plugin.wit + impl.c с нуля.
# Проверено вживую в этой сессии -- результат побайтово совпадает (с
# точностью до незначащих отличий) с тем, что уже лежит в
# ../plugin_component.wasm.
#
# Нужны: wasi-sdk (clang с таргетами wasm32-wasip1/wasip2),
# wit-bindgen-cli, wasm-tools -- все три уже стоят в этой сессии.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CLANG="${WASI_SDK_CLANG:-/home/claude/wasi-sdk/bin/clang}"
# Адаптер лежит прямо в пакете (plugin_src/adapters/) -- раньше здесь
# был абсолютный путь внутрь этой песочницы (/home/claude/wit_plugin/...),
# который на машине пользователя не существует. WASI_SDK_CLANG всё ещё
# указывает в песочницу по умолчанию -- сам wasi-sdk в пакет не входит
# (слишком большой), его нужно поставить отдельно и передать путь через
# env/-параметр.
ADAPTER="${WASI_ADAPTER:-$ROOT/plugin_src/adapters/wasi_snapshot_preview1.reactor.wasm}"

cd "$ROOT"

# 1) WIT -> биндинги на C (generated/plugin.c, generated/plugin.h,
#    generated/plugin_component_type.o -- последний хранит секцию с
#    описанием типа компонента, без него wasm-tools не поймёт, что
#    экспортирует core-модуль).
wit-bindgen c wit/plugin.wit --out-dir plugin_src/generated

# 2) impl.c + сгенерированные биндинги -> core wasm-модуль в
#    reactor-режиме (без _start/main, как и все плагины в этой сессии).
"$CLANG" --target=wasm32-wasip1 -mexec-model=reactor -nostartfiles -Wl,--no-entry \
  -I plugin_src/generated -O2 \
  plugin_src/impl.c plugin_src/generated/plugin.c plugin_src/generated/plugin_component_type.o \
  -o plugin_src/plugin_core.wasm

# 3) core-модуль -> настоящий компонент. Адаптер нужен, потому что
#    wasi-sdk даже для чистого C реактора тянет пути через
#    wasi_snapshot_preview1 (abort/exit); он конвертирует их в
#    компонентные wasi:cli/wasi:io импорты. Сам plugin_component.wasm
#    их при этом не импортирует вообще (см. `wasm-tools component wit`) --
#    адаптер тут просто техническая необходимость toolchain'а wasi-sdk,
#    а не то, что реально требуется в рантайме.
wasm-tools component new plugin_src/plugin_core.wasm \
  --adapt wasi_snapshot_preview1="$ADAPTER" \
  -o plugin_component.wasm

echo "Готово: $ROOT/plugin_component.wasm"
wasm-tools validate plugin_component.wasm --features component-model
```

### build_plugin.ps1
```powershell
# Windows-аналог build_plugin.sh -- та же самая последовательность
# команд (WIT -> C-биндинги -> core wasm-модуль -> компонент), только
# под PowerShell. ВНИМАНИЕ: в отличие от build_plugin.sh, этот скрипт
# НЕ прогнан вживую -- в этой (Linux-)песочнице нет Windows, чтобы его
# реально исполнить. Команды и флаги -- те же самые, что уже проверены
# в build_plugin.sh (wasi-sdk clang, wit-bindgen, wasm-tools кросс-
# платформенны и на Windows принимают идентичные аргументы), но сам
# .ps1 стоит прогнать и свериться с выводом при первом запуске.
#
# Нужны (все три -- обычные Windows-бинарники, .exe):
#   - wasi-sdk для Windows: https://github.com/WebAssembly/wasi-sdk/releases
#     (архив содержит bin\clang.exe с теми же таргетами wasm32-wasip1/wasip2)
#   - wit-bindgen-cli:  cargo install wit-bindgen-cli
#   - wasm-tools:       cargo install wasm-tools
#   (два последних ставятся через cargo одинаково что на Linux, что на
#   Windows -- crates.io кросс-платформенный)
#

param(
    #[string]$WasiSdkClang = $(if ($env:WASI_SDK_CLANG) { $env:WASI_SDK_CLANG } else { "C:\wasi-sdk\bin\clang.exe" }),
    [string]$WasiSdkClang = $(if ($env:WASI_SDK_CLANG) { $env:WASI_SDK_CLANG } else { "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" }),
    # Адаптер лежит прямо в пакете, рядом с этим скриптом
    # (plugin_src\adapters\) -- не нужно ничего скачивать отдельно.
    [string]$WasiAdapter  = $(if ($env:WASI_ADAPTER)  { $env:WASI_ADAPTER }  else { Join-Path $PSScriptRoot "adapters\wasi_snapshot_preview1.reactor.wasm" })
)

$ErrorActionPreference = "Stop"

$Root = $PSScriptRoot
Set-Location $Root

if (-not (Test-Path $WasiSdkClang)) {
    throw "clang.exe not found: $WasiSdkClang (pass -WasiSdkClang <path> or set env:WASI_SDK_CLANG)"
}
if (-not (Test-Path $WasiAdapter)) {
    throw "wasi_snapshot_preview1.reactor.wasm not found: $WasiAdapter (pass -WasiAdapter <path> or env:WASI_ADAPTER)"
}

New-Item -ItemType Directory -Force -Path "plugin_src\generated" | Out-Null

# 1) WIT -> биндинги на C.
& wit-bindgen c "wit\plugin.wit" --out-dir "plugin_src\generated"
if ($LASTEXITCODE -ne 0) { throw "wit-bindgen returned code $LASTEXITCODE" }

# 2) impl.c + сгенерированные биндинги -> core wasm-модуль (reactor,
#    без _start/main -- те же флаги, что в build_plugin.sh).
& $WasiSdkClang `
    --target=wasm32-wasip1 -mexec-model=reactor -nostartfiles "-Wl,--no-entry" `
    -I "plugin_src\generated" -O2 `
    "impl.c" "plugin_src\generated\plugin.c" "plugin_src\generated\plugin_component_type.o" `
    -o "plugin_src\plugin_core.wasm"
if ($LASTEXITCODE -ne 0) { throw "clang returned code $LASTEXITCODE" }

# 3) core-модуль -> компонент (адаптер нужен по той же причине, что и
#    в build_plugin.sh -- см. комментарий там; сам плагин WASI-импортов
#    не использует, это техническая деталь toolchain'а wasi-sdk).
& wasm-tools component new "plugin_src\plugin_core.wasm" `
    --adapt "wasi_snapshot_preview1=$WasiAdapter" `
    -o "plugin_component.wasm"
if ($LASTEXITCODE -ne 0) { throw "wasm-tools component new returned code $LASTEXITCODE" }

Write-Host "Done: $Root\plugin_component.wasm"
& wasm-tools validate "plugin_component.wasm" --features component-model
if ($LASTEXITCODE -ne 0) { throw "validation failed (code $LASTEXITCODE)" }
```
