/*
Простой "плагин": хост кладёт строку в буфер, выделенный самим гостем,
плагин переводит её в верхний регистр прямо в этой памяти (in-place).

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -Wl,--export=malloc -Wl,--export=free -o plugin_str.wasm plugin_str.c

*/

__attribute__((export_name("to_upper")))
void to_upper(char* buf, int len) {
    for (int i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] -= 32;
        }
    }
}
