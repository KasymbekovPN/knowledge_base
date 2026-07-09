---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

**Data race** — потоки **работают**, но результат непредсказуем из-за гонки за данные без синхронизации. `bt` тут почти бесполезен (программа не виснет) — нужен либо ThreadSanitizer, либо очень удачный breakpoint/watchpoint в момент гонки.

## Правильный инструмент — ThreadSanitizer, а не GDB/LLDB
**Вот ключевое доказательство тезиса**: результат теста (`expected == actual`) был "OK" все 5 запусков (гонка не проявилась в наблюдаемом поведении — на 1 ядре она физически почти не может дать неверный результат), но **ThreadSanitizer поймал её мгновенно с первого же запуска**, потому что TSan не ждёт, пока гонка "сломает" результат — он отслеживает **happens-before отношения** между операциями памяти на уровне инструментации, независимо от того, привело ли это к видимому багу.

Точная диагностика: `Read... by thread T1` и `Previous write... by thread T2` на **одной и той же строке** (`race_demo.cpp:9`, `race_counter++`) — прямое указание, где race и кто конкретно (T1 vs T2) в ней участвует.## Итоговая методология

|Симптом|Что использовать|Почему|
|---|---|---|
|Программа **зависла**, не завершается|`thread apply all bt` (GDB) / `thread backtrace all` (LLDB) на attached-процессе|Потоки стоят на месте — статичный снимок стека сразу показывает circular wait|
|Программа **иногда** даёт неверный результат, не виснет|**ThreadSanitizer** (`-fsanitize=thread`), не GDB/LLDB напрямую|Гонка может не проявиться в конкретном запуске (особенно на малом числе ядер) — нужен инструмент, ловящий её на уровне happens-before анализа, а не по факту сбоя|
|Нужно найти **конкретное** место гонки после того, как TSan её нашёл|GDB/LLDB breakpoint на указанной TSan строке + `watch` переменной|Комбинация: TSan говорит "где", отладчик — "как воспроизвести руками"|

### Пример

### main.cpp
```cpp
#include <iostream>
#include <thread>
#include <vector>

int race_counter = 0;   // НЕТ atomic, НЕТ мьютекса - намеренно, для демонстрации гонки

void increment_racy(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        race_counter++;   // <-- read-modify-write, НЕ атомарно
    }
}

int main() {
    const int num_threads = 8;
    const int iterations = 100000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_racy, iterations);
    }
    for (auto& t : threads) {
        t.join();
    }

    int expected = num_threads * iterations;
    std::cout << "expected = " << expected << "\n";
    std::cout << "actual   = " << race_counter << "\n";
    std::cout << (expected == race_counter ? "OK (не воспроизвелась в этот раз)" : "DATA RACE ПОЙМАНА - результат неверный") << "\n";
    return 0;
}
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
