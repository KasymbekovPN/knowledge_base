---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]


**Установка wasi-sdk**

Актуальный релиз — `wasi-sdk-33` (апрель 2026). Под Linux x86_64:

```
curl -L -o wasi-sdk.tar.gz \
  https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-33/wasi-sdk-33.0-x86_64-linux.tar.gz
mkdir wasi-sdk
tar xzf wasi-sdk.tar.gz -C wasi-sdk --strip-components=1
```

Под macOS/Windows на той же странице [релизов](https://github.com/WebAssembly/wasi-sdk/releases) есть архивы под соответствующую платформу — процесс тот же, просто другой tarball/zip. Никакой установки в систему не требуется — это самодостаточный тулчейн, `clang` внутри `wasi-sdk/bin/` уже настроен на target `wasm32-wasip1` через встроенный `clang.cfg`.

Проверка:

```
./wasi-sdk/bin/clang --version
```

показывает `clang version 22.1.0-wasi-sdk ... Target: wasm32-unknown-wasip1`.

**Компиляция — два разных стиля, и разница важна для плагинов**

_Стиль 1 — «библиотека» (reactor), без `main()`, только экспортируемые функции — именно так будут выглядеть настоящие плагины:_

### add.c
```c
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

```

`-mexec-model=reactor` говорит компилятору: это не «программа с точкой входа», а модуль, который просто предоставляет функции по требованию — ровно модель плагина. Вызов через `wasmtime run --invoke add add_c.wasm 10 32` дал `42` — тот же результат, что и рукописный `add.wat` из Дня 1, только теперь это настоящий C, скомпилированный тулчейном. Заметный нюанс: файл вышел 19 КБ против 41 байта у ручного `.wat` — компилятор подтягивает рантайм-обвязку и часть libc, даже для тривиальной функции.

_Стиль 2 — обычная WASI-программа с `main()`, работает как маленький консольный процесс:_

### hello.c

```c
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

```

Тут `wasmtime` сам находит точку входа (`_start`) и запускает модуль как процесс — WASI даёт `printf` доступ к stdout через свой стандартизованный слой syscalls. Вывод: `Hello from C, compiled to WASM via wasi-sdk!`.

Для будущей плагинной системы актуален именно первый стиль (reactor) — плагин не «запускается» как программа, а живёт и ждёт вызовов от хоста, как обычная динамическая библиотека, только в песочнице.
