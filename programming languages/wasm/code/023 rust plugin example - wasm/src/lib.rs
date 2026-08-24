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
