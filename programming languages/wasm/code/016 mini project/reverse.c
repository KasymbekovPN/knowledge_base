/*

Плагин 2: реверс строки. Тот же контракт, другая трансформация.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/reverse.wasm reverse.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    char* out = (char*)plugin_alloc(in_len);
    if (out == NULL) return 0;

    for (int32_t i = 0; i < in_len; i++) {
        out[i] = in_ptr[in_len - 1 - i];
    }

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)in_len;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }
