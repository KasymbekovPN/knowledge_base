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
