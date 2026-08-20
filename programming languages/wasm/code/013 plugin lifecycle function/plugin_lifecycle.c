/*

Реализация контракта из plugin_abi.h. Специально держим небольшое
внутреннее состояние (счётчик вызовов, флаг инициализации), чтобы было
видно, ЗАЧЕМ вообще нужен init/shutdown отдельно от process -- в plugin_str.c
раньше состояния не было вообще, каждый вызов был независим.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_call_count,--export=plugin_shutdown -o plugin_lifecycle.wasm plugin_lifecycle.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_API_VERSION 1

static int g_initialized = 0;
static int32_t g_call_count = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version() {
    return PLUGIN_API_VERSION;
}

__attribute__((export_name("plugin_init")))
int32_t plugin_init() {
    g_initialized = 1;
    g_call_count = 0;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) {
    return malloc((size_t)size);
}

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) {
    free(ptr);
}

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    // Контракт требует init до первого process -- отказ, если его не было.
    if (!g_initialized) {
        // ptr=0, len=0 -- сигнал ошибки хосту
        return 0;
    }
    g_call_count++;

    char* out = (char*)plugin_alloc(in_len);
    if (out == NULL) return 0;

    for (int32_t i = 0; i < in_len; i++) {
        char c = in_ptr[i];
        if (c >= 'a' && c <= 'z') {
            c = (char)(c-32);
        }
        out[i] = c;
    }

    // Упаковка (ptr, len) в один i64: старшие 32 бита -- указатель,
    // младшие 32 -- длина. См. пояснение в plugin_abi.h.
    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)in_len;
}

__attribute__((export_name("plugin_call_count")))
int32_t plugin_call_count() {
    return g_call_count;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown() {
    g_initialized = 0;
    g_call_count = 0;
}
