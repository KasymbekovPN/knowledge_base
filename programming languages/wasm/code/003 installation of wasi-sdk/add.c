/*

Простая экспортируемая функция — тот же смысл, что и add.wat из Дня 1,
но теперь это настоящий C, скомпилированный в WASM.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=add -o add_c.wasm add.c
wasmtime run --invoke add add_c.wasm 2 3

*/

__attribute__((export_name("add")))
int add(const int a, const int b) {
    return a + b;
}
