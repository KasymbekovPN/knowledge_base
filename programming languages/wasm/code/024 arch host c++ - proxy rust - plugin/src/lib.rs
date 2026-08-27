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
