/*

Плагин 4 -- намеренно "сломанный". Снаружи выглядит как честный
участник контракта (те же экспорты, версия ABI совпадает), но
plugin_process зависает в бесконечном цикле. Хост не может знать
заранее, какой из плагинов в папке окажется таким -- поэтому защита
(fuel) должна применяться КО ВСЕМ плагинам одинаково, не только к
этому.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/broken.wasm broken.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) { return 0; }

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    (void)in_ptr;
    (void)in_len;
    volatile int x = 0;
    while (1) x++;

    return 0;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { }
