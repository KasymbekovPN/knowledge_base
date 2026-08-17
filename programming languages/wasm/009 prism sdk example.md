---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

## Example

### vcpkg.json
```json
{
    "name": "wasm-host-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(extism_host_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# vcpkg НЕ содержит порта extism (проверено: vcpkg search extism -- пусто),
# поэтому, как и с wasmtime, подключаем готовую host-библиотеку через
# FetchContent напрямую из GitHub Releases проекта extism/extism.
set(EXTISM_VERSION "v1.30.0")

if(WIN32)
    set(EXTISM_ARCHIVE "libextism-x86_64-pc-windows-msvc-${EXTISM_VERSION}.tar.gz")
    set(EXTISM_LIB_NAME "extism.lib")
elseif(APPLE)
    set(EXTISM_ARCHIVE "libextism-x86_64-apple-darwin-${EXTISM_VERSION}.tar.gz")
    set(EXTISM_LIB_NAME "libextism.a")
else()
    set(EXTISM_ARCHIVE "libextism-x86_64-unknown-linux-gnu-${EXTISM_VERSION}.tar.gz")
    set(EXTISM_LIB_NAME "libextism.a")
endif()

FetchContent_Declare(
        extism_c
        URL "https://github.com/extism/extism/releases/download/${EXTISM_VERSION}/${EXTISM_ARCHIVE}"
)
FetchContent_MakeAvailable(extism_c)

# В архиве только заголовок + готовые .a/.so, своего CMake-проекта нет --
# объявляем IMPORTED-таргет вручную, как и для wasmtime.
add_library(extism STATIC IMPORTED)
set_target_properties(extism PROPERTIES
        IMPORTED_LOCATION "${extism_c_SOURCE_DIR}/${EXTISM_LIB_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${extism_c_SOURCE_DIR}"
)

add_executable(host_extism host_extism.cpp)

if(WIN32)
    # extism.lib -- статическая Rust-библиотека, тянет системные Windows API
    # напрямую (ntdll/ws2_32/userenv/bcrypt), которые линкер сам не подставит.
    target_link_libraries(host_extism PRIVATE extism ntdll ws2_32 userenv bcrypt)
else()
    target_link_libraries(host_extism PRIVATE extism pthread dl m)
endif()

# Плагин (plugin_extism.wasm) собирается ОТДЕЛЬНО через wasi-sdk clang --
# это компиляция гостя под wasm32-wasip1, а не C++-зависимость хоста,
# поэтому в этот CMake-проект её сознательно не включаю:
#
#   $WASI_SDK_PREFIX/bin/clang --target=wasm32-wasip1 -mexec-model=reactor \
#     -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper \
#     -o plugin_extism.wasm plugin_extism.c
#
# dummy.cpp/dummy-таргет для code insight в CLion сюда сознательно не
# добавлен: prism-sdk.h использует GNU-атрибуты import_module/import_name,
# которые понимает только Clang/GCC. MSVC (компилятор этого CXX-проекта)
# не поддерживает __attribute__ вообще, поэтому попытка скомпилировать
# plugin_extism.c этим тулчейном -- не косметическая, а настоящая ошибка
# компиляции, а не просто шум в IDE-анализаторе.

```

### prism-sdk.h
```cpp
#ifndef extism_pdk_h
#define extism_pdk_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t ExtismHandle;

#define EXTISM_ENV_MODULE "extism:host/env"
#define EXTISM_USER_MODULE "extism:host/user"

#define EXTISM_EXPORT_AS(name) __attribute__((export_name(name)))
#define EXTISM_EXPORTED_FUNCTION(name)                                         \
  EXTISM_EXPORT_AS(#name)                                                      \
  name(void)

#define EXTISM_IMPORT(a, b) __attribute__((import_module(a), import_name(b)))
#define EXTISM_IMPORT_ENV(b)                                                   \
  __attribute__((import_module(EXTISM_ENV_MODULE), import_name(b)))
#define EXTISM_IMPORT_USER(b)                                                  \
  __attribute__((import_module(EXTISM_USER_MODULE), import_name(b)))

EXTISM_IMPORT_ENV("length_unsafe")
extern uint64_t extism_length(const ExtismHandle);
EXTISM_IMPORT_ENV("alloc")
extern ExtismHandle extism_alloc(const uint64_t);
EXTISM_IMPORT_ENV("free")
extern void extism_free(ExtismHandle);

EXTISM_IMPORT_ENV("error_set")
extern void extism_error_set(const ExtismHandle);

EXTISM_IMPORT_ENV("config_get")
extern ExtismHandle extism_config_get(const ExtismHandle);

EXTISM_IMPORT_ENV("var_get")
extern ExtismHandle extism_var_get(const ExtismHandle);

EXTISM_IMPORT_ENV("var_set")
extern void extism_var_set(ExtismHandle, const ExtismHandle);

EXTISM_IMPORT_ENV("http_request")
extern ExtismHandle extism_http_request(const ExtismHandle, const ExtismHandle);

EXTISM_IMPORT_ENV("http_status_code")
extern int32_t extism_http_status_code(void);

EXTISM_IMPORT_ENV("http_headers")
extern ExtismHandle extism_http_headers(void);

EXTISM_IMPORT_ENV("log_info")
extern void extism_log_info(const ExtismHandle);
EXTISM_IMPORT_ENV("log_debug")
extern void extism_log_debug(const ExtismHandle);
EXTISM_IMPORT_ENV("log_warn")
extern void extism_log_warn(const ExtismHandle);
EXTISM_IMPORT_ENV("log_error")
extern void extism_log_error(const ExtismHandle);
EXTISM_IMPORT_ENV("log_trace")
extern void extism_log_trace(const ExtismHandle);
EXTISM_IMPORT_ENV("get_log_level")
extern int32_t extism_get_log_level();

EXTISM_IMPORT_ENV("input_offset")
extern ExtismHandle extism_input_offset(void);

EXTISM_IMPORT_ENV("input_length")
extern uint64_t extism_input_length(void);
// extism_length(extism_input_offset()); is also valid

// Load data from Extism memory, verifies load is inbounds
bool extism_load_from_handle(const ExtismHandle src, const uint64_t src_offset,
                             void *dest, const size_t n);

// Load data from input buffer, verifies load is inbounds
static inline bool extism_load_input(const uint64_t src_offset, void *dest,
                                     const size_t n) {
  return extism_load_from_handle(extism_input_offset(), src_offset, dest, n);
}

// Load n-1 bytes and zero terminate
// Verifies load is inbounds
bool extism_load_sz(const ExtismHandle src, uint64_t src_offset, char *dest,
                    const size_t n);

// Load n-1 bytes from input buffer and zero terminate
// Verifies load is inbounds
static inline bool extism_load_input_sz(const uint64_t src_offset, char *dest,
                                        const size_t n) {
  return extism_load_sz(extism_input_offset(), src_offset, dest, n);
}

// Copy data into Extism memory, verifies store is in bounds
bool extism_store_to_handle(ExtismHandle dest, const uint64_t dest_offset,
                            const void *buffer, const size_t n);

// Allocate a buffer in Extism memory and copy into it
ExtismHandle extism_alloc_buf(const void *src, const size_t n);

__attribute__((
    deprecated("Use extism_alloc_buf instead."))) static inline ExtismHandle
extism_alloc_string(const char *s, const size_t n) {
  return extism_alloc_buf(s, n);
}

#ifdef EXTISM_USE_LIBC
// get the length (n) and malloc(n), load n bytes from Extism memory
// into it. If outSize is provided, set it to n
void *extism_load_dup(const ExtismHandle h, size_t *outSize);

// get the input length (n) and malloc(n), load n bytes from Extism memory
// into it. If outSize is provided, set it to n
static inline void *extism_load_input_dup(size_t *outSize) {
  return extism_load_dup(extism_input_offset(), outSize);
}

// get the length, add 1 to it to get n. malloc(n), load n - 1 bytes
// from Extism memory into it. Zero terminate. If outSize is provided, set it
// to n
char *extism_load_sz_dup(const ExtismHandle h, size_t *outSize);

// get the input length, add 1 to it to get n. malloc(n), load n - 1 bytes
// from Extism memory into it. Zero terminate. If outSize is provided, set it
// to n
static inline char *extism_load_input_sz_dup(size_t *outSize) {
  return extism_load_sz_dup(extism_input_offset(), outSize);
}

#endif // EXTISM_USE_LIBC

// Allocate a buffer in Extism memory and copy string data into it
// copied string is NOT null terminated
ExtismHandle extism_alloc_buf_from_sz(const char *sz);

typedef enum {
  ExtismLogTrace,
  ExtismLogDebug,
  ExtismLogInfo,
  ExtismLogWarn,
  ExtismLogError,
} ExtismLog;

// Write to Extism log
void extism_log(const char *s, const size_t len, const ExtismLog level);

// Write zero-terminated string to Extism log
void extism_log_sz(const char *s, const ExtismLog level);

// Set the output from an ExtismHandle, returns false if outside the memory
// block is specified.
bool extism_output_set_from_handle(const ExtismHandle handle,
                                   const uint64_t offset, const uint64_t n);

// Set the output to the entire contents of an ExtismHandle
void extism_output_handle(const ExtismHandle handle);

// Alloc a buf of Extism memory and output it
void extism_output_buf(const void *src, const size_t n);

// set output to extism_alloc_buf_from_sz
void extism_output_buf_from_sz(const char *sz);

// output an error from a buf
void extism_error_set_buf(const char *message, const size_t messageLen);

// output an error from a sz
void extism_error_set_buf_from_sz(const char *message);

// get a config var from a buf key
ExtismHandle extism_config_get_buf(const char *name, const size_t nameLen);

// get a config var from a sz key
ExtismHandle extism_config_get_buf_from_sz(const char *name);

// get a var from a buf key
ExtismHandle extism_var_get_buf(const char *name, const size_t nameLen);

// get a var from a sz key
ExtismHandle extism_var_get_buf_from_sz(const char *name);

// store a var from a buf key
void extism_var_set_buf(const char *name, const size_t nameLen,
                        const ExtismHandle value);

// store a var from a sz key
void extism_var_set_buf_from_sz(const char *name, const ExtismHandle value);

#ifdef __cplusplus
}
#endif
#endif // extism_pdk_h

// avoid greying out the implementation section
#if defined(Q_CREATOR_RUN) || defined(__INTELLISENSE__) ||                     \
    defined(_CDT_PARSER__)
#define EXTISM_IMPLEMENTATION
#endif

#ifndef extism_low_level_imports_h
#if defined(EXTISM_ENABLE_LOW_LEVEL_API) || defined(EXTISM_IMPLEMENTATION)
#define extism_low_level_imports_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EXTISM_ENABLE_LOW_LEVEL_API
typedef uint64_t ExtismPointer;
#else
#define ExtismPointer uint64_t
#endif

EXTISM_IMPORT_ENV("input_load_u8")
extern uint8_t __extism_input_load_u8(const uint64_t);
EXTISM_IMPORT_ENV("input_load_u64")
extern uint64_t __extism_input_load_u64(const uint64_t);
EXTISM_IMPORT_ENV("output_set")
extern void __extism_output_set(const ExtismPointer, const uint64_t);
EXTISM_IMPORT_ENV("store_u8")
extern void __extism_store_u8(ExtismPointer, const uint8_t);
EXTISM_IMPORT_ENV("load_u8")
extern uint8_t __extism_load_u8(const ExtismPointer);
EXTISM_IMPORT_ENV("store_u64")
extern void __extism_store_u64(ExtismPointer, const uint64_t);
EXTISM_IMPORT_ENV("load_u64")
extern uint64_t __extism_load_u64(const ExtismPointer);

#ifdef EXTISM_ENABLE_LOW_LEVEL_API
#define extism_input_load_u8 __extism_input_load_u8
#define extism_input_load_u64 __extism_input_load_u64
#define extism_output_set __extism_output_set
#define extism_store_u8 __extism_store_u8
#define extism_load_u8 __extism_load_u8
#define extism_store_u64 __extism_store_u64
#define extism_load_u64 __extism_load_u64

// Load data from Extism memory, does not verify load is in bounds
#define extism_load __extism_load

// Copy data into Extism memory, does not verify store is inbounds
#define extism_store __extism_store

// Load n-1 bytes and zero terminate
// Does not verify load is inbounds
void extism_load_sz_unsafe(const ExtismPointer src, char *dest, const size_t n);

// Returns 0 when the pointer doesn't refer to the start of
// the data section of a memory block.
EXTISM_IMPORT_ENV("length")
extern uint64_t extism_length_safe(const ExtismPointer);

#else
#undef ExtismPointer
#endif

#ifdef __cplusplus
}
#endif
#endif // defined(EXTISM_ENABLE_LOW_LEVEL_API) || defined(EXTISM_IMPLEMENTATION)
#endif // extism_low_level_imports_h

#ifdef EXTISM_IMPLEMENTATION
#ifndef extism_pdk_c
#define extism_pdk_c

#ifndef EXTISM_ENABLE_LOW_LEVEL_API
#define ExtismPointer uint64_t
#define EXTISM_LL_API static
#else
#define EXTISM_LL_API
#endif

// Load data from Extism memory, does not verify load is in bounds
EXTISM_LL_API void __extism_load(const ExtismPointer offs, void *dest,
                                 const size_t n) {
  const size_t chunk_count = n >> 3;
  uint64_t *i64_buffer = dest;
  for (size_t chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
    i64_buffer[chunk_idx] = __extism_load_u64(offs + (chunk_idx << 3));
  }

  size_t remainder_offset = chunk_count << 3;
  const size_t remainder_end = remainder_offset + (n & 7);
  for (uint8_t *u8_buffer = dest; remainder_offset < remainder_end;
       remainder_offset++) {
    u8_buffer[remainder_offset] = __extism_load_u8(offs + remainder_offset);
  }
}

bool extism_load_from_handle(const ExtismHandle src, const uint64_t src_offset,
                             void *dest, const size_t n) {
  if ((src_offset + n) > extism_length(src)) {
    return false;
  }
  const ExtismPointer offs = src + src_offset;
  __extism_load(offs, dest, n);
  return true;
}

// Load n-1 bytes and zero terminate
// Does not verify load is inbounds
EXTISM_LL_API void extism_load_sz_unsafe(const ExtismPointer src, char *dest,
                                         const size_t n) {
  __extism_load(src, dest, n - 1);
  dest[n - 1] = '\0';
}

// Load n-1 bytes and zero terminate
// Verifies load is inbounds
bool extism_load_sz(const ExtismHandle src, uint64_t src_offset, char *dest,
                    const size_t n) {
  const uint64_t len = extism_length(src);
  if ((src_offset + n - 1) > len) {
    return false;
  }
  extism_load_sz_unsafe(src + src_offset, dest, n);
  return true;
}

// Copy data into Extism memory, does not verify store is inbounds
EXTISM_LL_API void __extism_store(ExtismPointer offs, const void *buffer,
                                  const size_t length) {
  const size_t chunk_count = length >> 3;
  const uint64_t *i64_buffer = buffer;
  for (size_t chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
    __extism_store_u64(offs + (chunk_idx << 3), i64_buffer[chunk_idx]);
  }

  size_t remainder_offset = chunk_count << 3;
  const size_t remainder_end = remainder_offset + (length & 7);
  for (const uint8_t *u8_buffer = buffer; remainder_offset < remainder_end;
       remainder_offset++) {
    __extism_store_u8(offs + remainder_offset, u8_buffer[remainder_offset]);
  }
}

// Copy data into Extism memory, verifies store is in bounds
bool extism_store_to_handle(ExtismHandle dest, const uint64_t dest_offset,
                            const void *buffer, const size_t n) {
  if ((dest_offset + n) > extism_length(dest)) {
    return false;
  }
  ExtismPointer offs = dest + dest_offset;
  __extism_store(offs, buffer, n);
  return true;
}

// Allocate a buffer in Extism memory and copy into it
ExtismHandle extism_alloc_buf(const void *src, const size_t n) {
  ExtismHandle ptr = extism_alloc(n);
  __extism_store(ptr, src, n);
  return ptr;
}

#ifdef EXTISM_USE_LIBC
#include <stdlib.h>
#include <string.h>

#define extism_strlen strlen

// get the length (n) and malloc(n), load n bytes from Extism memory
// into it. If outSize is provided, set it to n
void *extism_load_dup(const ExtismHandle h, size_t *outSize) {
  const uint64_t n = extism_length(h);
  if (n > SIZE_MAX) {
    return NULL;
  }
  void *buf = malloc(n);
  if (!buf) {
    return NULL;
  }
  __extism_load(h, buf, n);
  if (outSize) {
    *outSize = n;
  }
  return buf;
}

// get the length, add 1 to it to get n. malloc(n), load n - 1 bytes
// from Extism memory into it. Zero terminate. If outSize is provided, set it
// to n
char *extism_load_sz_dup(const ExtismHandle h, size_t *outSize) {
  uint64_t n = extism_length(h);
  if (n > (SIZE_MAX - 1)) {
    return NULL;
  }
  n++;
  char *buf = malloc(n);
  if (!buf) {
    return NULL;
  }
  extism_load_sz_unsafe(h, buf, n);
  if (outSize) {
    *outSize = n;
  }
  return buf;
}

#else
static size_t extism_strlen(const char *sz) {
  size_t len;
  for (len = 0; sz[len] != '\0'; len++) {
  }
  return len;
}
#endif

// Allocate a buffer in Extism memory and copy string data into it
// copied string is NOT null terminated
ExtismHandle extism_alloc_buf_from_sz(const char *sz) {
  return extism_alloc_buf(sz, extism_strlen(sz));
}

// Write to Extism log
void extism_log(const char *s, const size_t len, const ExtismLog level) {
  if (level < extism_get_log_level()) {
    return;
  }

  ExtismHandle buf = extism_alloc(len);
  __extism_store(buf, s, len);
  switch (level) {
  case ExtismLogInfo:
    extism_log_info(buf);
    break;
  case ExtismLogDebug:
    extism_log_debug(buf);
    break;
  case ExtismLogWarn:
    extism_log_warn(buf);
    break;
  case ExtismLogError:
    extism_log_error(buf);
    break;
  case ExtismLogTrace:
    extism_log_trace(buf);
    break;
  }
}

// Write zero-terminated string to Extism log
void extism_log_sz(const char *s, const ExtismLog level) {
  const size_t len = extism_strlen(s);
  extism_log(s, len, level);
}

// Set the output from an ExtismHandle, returns false if outside the memory
// block is specified.
bool extism_output_set_from_handle(const ExtismHandle handle,
                                   const uint64_t offset, const uint64_t n) {
  if ((offset + n) > extism_length(handle)) {
    return false;
  }
  __extism_output_set(handle + offset, n);
  return true;
}

// Set the output to the entire contents of an ExtismHandle
void extism_output_handle(const ExtismHandle handle) {
  __extism_output_set(handle, extism_length(handle));
}

// Alloc a buf of Extism memory and output it
void extism_output_buf(const void *src, const size_t n) {
  ExtismHandle handle = extism_alloc_buf(src, n);
  __extism_output_set(handle, n);
}

// set output to extism_alloc_buf_from_sz
void extism_output_buf_from_sz(const char *sz) {
  const size_t n = extism_strlen(sz);
  extism_output_buf(sz, n);
}

// output an error from a buf
void extism_error_set_buf(const char *message, const size_t messageLen) {
  ExtismHandle handle = extism_alloc_buf(message, messageLen);
  extism_error_set(handle);
}

// output an error from a sz
void extism_error_set_buf_from_sz(const char *message) {
  const size_t len = extism_strlen(message);
  extism_error_set_buf(message, len);
}

// get a config var from a buf key
ExtismHandle extism_config_get_buf(const char *name, const size_t nameLen) {
  ExtismHandle key = extism_alloc_buf(name, nameLen);
  ExtismHandle value = extism_config_get(key);
  return value;
}

// get a config var from a sz key
ExtismHandle extism_config_get_buf_from_sz(const char *name) {
  const size_t len = extism_strlen(name);
  return extism_config_get_buf(name, len);
}

// get a var from a buf key
ExtismHandle extism_var_get_buf(const char *name, const size_t nameLen) {
  ExtismHandle key = extism_alloc_buf(name, nameLen);
  ExtismHandle value = extism_var_get(key);
  return value;
}

// get a var from a sz key
ExtismHandle extism_var_get_buf_from_sz(const char *name) {
  const size_t len = extism_strlen(name);
  return extism_var_get_buf(name, len);
}

// store a var from a buf key
void extism_var_set_buf(const char *name, const size_t nameLen,
                        const ExtismHandle value) {
  ExtismHandle key = extism_alloc_buf(name, nameLen);
  extism_var_set(key, value);
}

// store a var from a sz key
void extism_var_set_buf_from_sz(const char *name, const ExtismHandle value) {
  const size_t len = extism_strlen(name);
  extism_var_set_buf(name, len, value);
}

#ifndef EXTISM_ENABLE_LOW_LEVEL_API
#undef ExtismPointer
#endif

#endif // extism_pdk_c
#endif // EXTISM_IMPLEMENTATION

```

### plugin_extism.c
```cpp
// Тот же to_upper, что и в plugin_str.c, но теперь память полностью
// на попечении Extism -- никаких ручных malloc/free "напрямую в хост".
#define EXTISM_IMPLEMENTATION
#define EXTISM_USE_LIBC

#include "prism-sdk.h"

int32_t EXTISM_EXPORTED_FUNCTION(to_upper) {
    size_t len = 0;
    // extism_load_input_dup сам: узнаёт длину входа, malloc'ит буфер нужного
    // размера, копирует туда байты из "Extism-памяти" -- без единого сырого
    // смещения в коде плагина.
    char* buf = extism_load_input_dup(&len);
    if (!buf) return 1;

    for (size_t i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] -= 32;
        }
    }

    // Аллоцирует Extism-хендл, копирует туда buf и сообщает хосту, что это
    // и есть результат вызова -- один вызов вместо
    // malloc + store + output_set_from_handle по отдельности.
    extism_output_buf(buf, len);
    free(buf);

    return 0;
}
```

### host_extism.cpp
```cpp
/*

C++ хост на Extism -- тот же сценарий, что и host_str.cc на голом
Wasmtime C++ API, но обрати внимание, НАСКОЛЬКО короче стал сам вызов:
ни memory.data(), ни ручного malloc/free, ни арифметики со смещениями.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -o plugin_extism.wasm plugin_extism.c

*/

#include <extism.h>

#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <vector>

namespace {
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
        };
    }
}

int main(int argc, char *argv[]) {
    const auto wasm{read_wasm_file("plugin_extism.wasm")};

    // Создаём плагин напрямую из байт .wasm. with_wasi=false -- мы же сами
    // убедились через wasm-objdump, что плагину WASI не требуется.
    char* errmsg{nullptr};
    ExtismPlugin* plugin{extism_plugin_new(
        wasm.data(),
        wasm.size(),
        /*functions=*/nullptr,
        /*n_functions=*/0,
        /*with_wasi=*/false,
        &errmsg)};

    if (!plugin) {
        std::cerr << std::format("Could not load plugin: {}\n",
            (errmsg != nullptr ? errmsg : "unknown error"));
        if (errmsg) {
            extism_plugin_new_error_free(errmsg);
        }

        return 1;
    }

    const std::string input = "hello from the extism host!";
    // Один вызов: имя функции + указатель на входные байты + длина.
    // Extism сам копирует их внутрь плагина, освобождает буфер после вызова --
    // хосту не нужно ничего знать про malloc/free плагина.
    const int32_t rc{extism_plugin_call(
        plugin,
        "to_upper",
        reinterpret_cast<const uint8_t*>(input.data()),
        input.size())};

    if (rc != 0) {
        std::cerr << std::format("Plugin called with error: {}\n",
            extism_plugin_error(plugin));
        extism_plugin_free(plugin);

        return 1;
    }

    const ExtismSize out_len{extism_plugin_output_length(plugin)};
    const uint8_t* out_data{extism_plugin_output_data(plugin)};
    const std::string result{reinterpret_cast<const char*>(out_data), out_len};

    std::cout << std::format("Result after plugin calling: {}\n", result);

    extism_plugin_free(plugin);

    return 0;
}

```


**Плагин** (`plugin_extism.c`) — вместо ручного экспорта `malloc`/`free` из libc используется единственный заголовок Extism PDK (это классическая single-header библиотека вроде stb, скачал прямо из `extism/c-pdk`):

```c
#define EXTISM_IMPLEMENTATION
#define EXTISM_USE_LIBC
#include "extism-pdk.h"

int32_t EXTISM_EXPORTED_FUNCTION(to_upper) {
  size_t len = 0;
  char *buf = extism_load_input_dup(&len);   // сам узнаёт длину входа и копирует его
  for (size_t i = 0; i < len; i++)
    if (buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= 32;
  extism_output_buf(buf, len);               // сам аллоцирует и публикует результат
  free(buf);
  return 0;
}
```

Компилируется тем же wasi-sdk, что и раньше:

```
clang --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles \
  -Wl,--no-entry -Wl,--export=to_upper -o plugin_extism.wasm plugin_extism.c
```

Проверил импорты через `wasm-objdump -x` — там только `extism:host/env.*` (alloc, load_u8, store_u8, input_offset...), WASI вообще не подключался. Это подтверждает то, что мы разбирали в переписке про Extism: он вводит свой собственный протокол поверх голого WASM, не завязанный на WASI.

**Хост** (`host_extism.cc`) — вот где разница с `host_str.cc` действительно бросается в глаза:

```cpp
ExtismPlugin *plugin = extism_plugin_new(wasm.data(), wasm.size(), nullptr, 0,
                                          /*with_wasi=*/false, &errmsg);

extism_plugin_call(plugin, "to_upper",
                    reinterpret_cast<const uint8_t*>(input.data()), input.size());

const uint8_t *outData = extism_plugin_output_data(plugin);
```

Ни `memory.data(store)`, ни явного вызова `malloc`/`free` со стороны хоста, ни арифметики со смещениями — просто «вот имя функции, вот входные байты», и «вот выходные байты». Именно это мы и обсуждали концептуально как главное преимущество Extism — здесь оно видно буквально в количестве строк кода.

**CMake.** Как и с wasmtime — в vcpkg порта `extism` нет (проверил: `vcpkg search extism` пусто), поэтому host-библиотеку (`libextism.a` + `extism.h`) подключил тем же приёмом через `FetchContent`, с релиза `extism/extism`:

```cmake
FetchContent_Declare(extism_c
  URL "https://github.com/extism/extism/releases/download/v1.30.0/libextism-x86_64-unknown-linux-gnu-v1.30.0.tar.gz")
FetchContent_MakeAvailable(extism_c)

add_library(extism STATIC IMPORTED)
set_target_properties(extism PROPERTIES
  IMPORTED_LOCATION "${extism_c_SOURCE_DIR}/libextism.a"
  INTERFACE_INCLUDE_DIRECTORIES "${extism_c_SOURCE_DIR}")
```

`cmake -B build && cmake --build build && ./build/host_extism` — и сразу результат:

```
Результат после вызова плагина: HELLO FROM THE EXTISM HOST!
```

Сборку самого плагина (`.c` → `.wasm` через wasi-sdk) в CMake-проект хоста намеренно не включал — это компиляция гостя под `wasm32-wasip1`, отдельный тулчейн от C++-зависимостей хоста, оставил её отдельной командой в комментарии внутри `CMakeLists.txt`, как я и делал раньше с `plugin_str.wasm`.
