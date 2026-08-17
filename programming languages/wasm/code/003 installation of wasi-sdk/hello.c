/*

Обычная WASI-программа с main() — работает как маленький консольный процесс

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --target=wasm32-wasip1 -O2 -o hello.wasm hello.c
wasmtime run hello.wasm

*/

#include <stdio.h>

int main(void) {
    printf("Hello from C, compiled to WASM via wasi-sdk!\n");

    return 0;
}
