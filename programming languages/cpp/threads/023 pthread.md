---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## -pthread

Флаг компилятора (GCC/Clang) для включения поддержки POSIX threads.

Делает две вещи одновременно:

```bash
clang++ -pthread main.cpp -o main
```

**1. Флаг компиляции** — определяет макрос `_REENTRANT`, переключает стандартную библиотеку в потокобезопасный режим.

**2. Флаг линковки** — подключает библиотеку `libpthread` (эквивалент `-lpthread`).

### Когда нужен

На Linux при использовании `<thread>`, `<mutex>`, `<future>` и других threading-примитивов:

```bash
clang++ -std=c++23 -pthread main.cpp -o main
```

Без него линковщик может выдать:

```
undefined reference to `pthread_create`
```

### На Windows

На Windows с MSVC или MinGW флаг не нужен — потоки реализованы через Win32 API, поддержка встроена автоматически. Именно поэтому в вашем проекте (`clang++` на Windows с MSVC runtime) компиляция работает без `-pthread`.
