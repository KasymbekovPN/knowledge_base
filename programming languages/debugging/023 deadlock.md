---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

**Deadlock** — потоки **стоят замертво**, взаимно ждут друг друга. Диагностируется через **backtrace всех потоков** — увидишь, кто на каком мьютексе завис.

|Симптом|Что использовать|Почему|
|---|---|---|
|Программа **зависла**, не завершается|`thread apply all bt` (GDB) / `thread backtrace all` (LLDB) на attached-процессе|Потоки стоят на месте — статичный снимок стека сразу показывает circular wait|
|Программа **иногда** даёт неверный результат, не виснет|**ThreadSanitizer** (`-fsanitize=thread`), не GDB/LLDB напрямую|Гонка может не проявиться в конкретном запуске (особенно на малом числе ядер) — нужен инструмент, ловящий её на уровне happens-before анализа, а не по факту сбоя|
|Нужно найти **конкретное** место гонки после того, как TSan её нашёл|GDB/LLDB breakpoint на указанной TSan строке + `watch` переменной|Комбинация: TSan говорит "где", отладчик — "как воспроизвести руками"|

### Пример

### main.cpp
```cpp
#include <chrono>  
#include <iostream>  
#include <mutex>  
#include <thread>  
  
std::mutex mutex_a;  
std::mutex mutex_b;  
  
void task_one() {  
    std::cout << "task_one: locking mutex_a\n";  
    std::lock_guard<std::mutex> lock_a{mutex_a};  
  
    std::this_thread::sleep_for(std::chrono::milliseconds(200));  
  
    std::cout << "task_one: locking mutex_b\n";  
    std::lock_guard<std::mutex> lock_b{mutex_b};  
  
    std::cout << "task_one: got both locks\n";  
  
}  
  
void task_two() {  
    std::cout << "task_two: locking mutex_b\n";  
    std::lock_guard<std::mutex> lock_b{mutex_b};  
  
    std::this_thread::sleep_for(std::chrono::milliseconds(200));  
  
    std::cout << "task_two: locking mutex_a\n";  
    std::lock_guard<std::mutex> lock_a{mutex_a};  
  
    std::cout << "task_two: got both locks\n";  
  
}  
  
int main() {  
    {        std::jthread t1{task_one};  
        std::jthread t2{task_two};  
    }  
    std::cout << "Unreachable\n";  
    return 0;  
}  
  
/*  
  
###  
Get-Process app | Select-Object Id  
lldb -p 24596  
thread backtrace all  
  
###  
Get-Process app | Select-Object Id  
gdb -p 24596  
thread apply all bt  
  
*/
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(proj LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {
			"name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {
			"name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }
	],
	"buildPresets": [  
        {
			"name": "debug",  
            "configurePreset": "debug"  
        }  
    ]
}
```

### vcpkg.json
```json
{  
    "name": "demo",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

## Разбор diagnosis

```
Thread 3: task_one() at deadlock_demo.cpp:16 — ждёт futex на mutex_b
Thread 2: task_two() at deadlock_demo.cpp:28 — ждёт futex на mutex_a
Thread 1: main() at deadlock_demo.cpp:37 — ждёт join() обоих потоков
```

Это ровно то, что нужно искать в реальном инциденте: **два потока, каждый держит один мьютекс и ждёт другой** — классическая инверсия порядка блокировки (`task_one`: A→B, `task_two`: B→A). `thread apply all bt` показал это моментально — без единой строчки логов, просто по факту "кто на каком `futex_wait` завис".

## Алгоритм диагностики deadlock по backtrace

1. **`thread apply all bt`** (или `thread backtrace all` в LLDB) — сразу
2. Ищи потоки, застрявшие в `futex_wait` / `pthread_mutex_lock` / `__lll_lock_wait` — это "живые" кандидаты на deadlock (в отличие от, скажем, `clock_nanosleep`, где поток просто спит намеренно)
3. Для каждого такого потока смотри **твою** строку кода (`task_one () at deadlock_demo.cpp:16`) — какой мьютекс он пытается взять
4. Сопоставь: если Thread A ждёт мьютекс, который **уже держит** Thread B, а Thread B в это же время ждёт мьютекс, который держит Thread A — вот и подтверждённый deadlock через circular waitЧисто, процесс уже был убит.

## Как это же выглядит в LLDB
Идентичная картина, что и в GDB: `thread #2 → task_one() at deadlock_demo.cpp:16`, `thread #3 → task_two() at deadlock_demo.cpp:28`, оба зависли в `__lll_lock_wait`/`futex_wait` на **разных** мьютексах — тот же deadlock, тот же diagnostic paths, просто больше деталей во фреймах (`clone3.S:78`, точные номера строк с колонками `16:47`, `28:47`).Чисто.
