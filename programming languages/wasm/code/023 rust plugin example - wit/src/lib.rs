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