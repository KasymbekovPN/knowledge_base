---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Команды для потоков: info threads, thread apply all bt, переключение

## Базовые команды

| Действие                              | GDB                      | LLDB                            |
| ------------------------------------- | ------------------------ | ------------------------------- |
| Список потоков                        | `info threads`           | `thread list`                   |
| Переключиться на поток N              | `thread N`               | `thread select N`               |
| Backtrace текущего потока             | `bt`                     | `bt`                            |
| Backtrace **всех** потоков            | `thread apply all bt`    | `thread backtrace all`          |
| Выполнить команду в конкретном потоке | `thread N` затем команда | `thread select N` затем команда |

## Пример

### main.cpp
```cpp
#include <atomic>  
#include <chrono>  
#include <iostream>  
#include <format>  
#include <mutex>  
#include <thread>  
#include <vector>  
  
std::mutex log_mutex;  
std::atomic<int> shared_counter{0};  
  
void worker(const int id, const int iterations) {  
    for (int i{}; i < iterations; ++i) {  
        int local_value{id * 100 + i}; // <-- breakpoint здесь, разный для каждого потока  
        shared_counter.fetch_add(1);  
  
        {            std::lock_guard<std::mutex> lock(log_mutex);  
            std::cout << std::format(  
                "worker {}: iteration= {}, local_value = {}",  
                id,  
                i,  
                local_value);  
        }  
        std::this_thread::sleep_for(std::chrono::milliseconds(200));  
    }}  
  
int main() {  
    const int NUM_WORKERS{4};  
    std::vector<std::thread> threads;  
  
    for (int i{}; i < NUM_WORKERS; ++i) {  
        threads.emplace_back(worker, i, 10);  
    }  
    for (auto& t : threads) {  
        t.join();  
    }  
    std::cout << std::format("total countre: {}", shared_counter.load());  
    return 0;  
}  
  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 14  
thread list  
bt  
thread backtrace all  
thread select 7  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:14  
info thread  
bt  
thread apply all bt  
thread 7  
  
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

## Сводная таблица

|Действие|GDB|LLDB|
|---|---|---|
|Список потоков|`info threads` (звёздочка `*` = текущий)|`thread list`|
|Backtrace всех потоков|`thread apply all bt`|`thread backtrace all`|
|Ограничить глубину backtrace|`thread apply all bt 3`|`thread backtrace all` (без встроенного лимита — обрезай вручную)|
|Переключиться на поток|`thread N`|`thread select N`|
|Выполнить одну команду в конкретном потоке без переключения|`thread N print var`|нет короткого аналога — нужен `thread select`|

## Главный практический вывод

Когда исследуешь состояние **сразу нескольких** потоков в момент гонки — **не делай лишних `continue`** между остановкой и анализом. Правильный порядок: `run` → сразу `info threads`/`thread apply all bt` (или `thread list`/`thread backtrace all` в LLDB) → и только потом, если нужно, `continue`. Каждый лишний `continue` в all-stop mode двигает **все** потоки вперёд, а не только тот, что ты собирался изучить.
