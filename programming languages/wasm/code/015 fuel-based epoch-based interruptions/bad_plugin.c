/*

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=infinite_loop,--export=ping -o bad_plugin.wasm bad_plugin.c

*/

#include <stdint.h>

// "Плохой" плагин: бесконечный цикл. volatile не даёт компилятору
// оптимизировать пустой цикл в ничто -- это гарантированно реальное
// бесконечное выполнение внутри WASM, а не мираж после -O2.
__attribute__((export_name("infinite_loop")))
void infinite_loop() {
    volatile int counter = 0;
    while (1) counter++;
}

// "Хороший" сосед в том же модуле -- вызовем его ПОСЛЕ того, как
// оборвём infinite_loop, чтобы доказать: сам инстанс/раннее выполнение
// не превратили процесс хоста в труп.
__attribute__((export_name("ping")))
int32_t ping() {
    return 42;
}
