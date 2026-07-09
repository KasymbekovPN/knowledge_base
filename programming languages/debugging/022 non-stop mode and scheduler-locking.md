---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Non-stop mode и scheduler-locking в GDB

## Проблема, которую решают обе фичи

Мы только что увидели: в **all-stop mode** (дефолт) любой `continue` двигает **все** потоки разом, а любая остановка (breakpoint) замораживает **все** потоки. Это мешает изучать конкретный поток изолированно. Две фичи решают это по-разному:

- **`scheduler-locking`** — во время **пошаговой отладки** (`step`/`next`) держит **остальные потоки замороженными**, пока ты шагаешь по одному конкретному
- **`non-stop mode`** — полная смена модели: потоки останавливаются/продолжаются **независимо друг от друга**, `continue` для одного потока не трогает остальные вообще

## 1. Scheduler-locking

### Синтаксис

```
(gdb) set scheduler-locking off       # дефолт — все потоки двигаются вместе
(gdb) set scheduler-locking on        # ТОЛЬКО текущий поток двигается на step/next/continue
(gdb) set scheduler-locking step      # заблокировать остальные ТОЛЬКО во время step/next, continue — как обычно
(gdb) show scheduler-locking            # посмотреть текущий режим
```


## `scheduler-locking step` — компромисс

Блокирует остальные потоки **только во время самого `step`/`next`**, но `continue` продолжает работать в обычном all-stop режиме.

## 2. Non-stop mode — полная независимость потоков

### Как включить

```
(gdb) set non-stop on          # ДО подключения к target/запуска процесса!
(gdb) set target-async on       # обычно требуется вместе с non-stop
(gdb) run
```

**Критичный нюанс:** `non-stop` нужно установить **до** старта процесса — в отличие от `scheduler-locking`, которую можно менять на лету.

```
* 1    Thread 0x7ffff7e92740 (LWP 616) "threads_demo" (running)
  2    Thread 0x7ffff77ff6c0 (LWP 619) "threads_demo" worker (id=0, iterations=10) at threads_demo.cpp:13
```

**Thread 1 (`(running)`)** продолжает выполняться **прямо во время** того, как **Thread 2** стоит на breakpoint — в all-stop mode это было бы невозможно, main-поток был бы заморожен вместе со всеми. Именно ради этого существует non-stop mode: можно ставить breakpoint внутри одного worker-потока и спокойно наблюдать/продолжать его, пока остальная программа (включая другие потоки) живёт своей жизнью.Процессов не осталось, всё чисто.

## Управление потоками в non-stop mode

```
(gdb) thread 2
(gdb) continue          # продолжить ТОЛЬКО поток 2, остальные не трогать
(gdb) interrupt          # остановить ТЕКУЩИЙ поток, не трогая остальные
(gdb) interrupt -a       # остановить ВСЕ потоки разом
```

В all-stop mode `continue` без указания потока двигает всех; в non-stop mode `continue` двигает **только выбранный** поток — это фундаментальная смена модели, а не просто ещё одна настройка.
## Сводная таблица: когда что использовать

|Задача|Инструмент|
|---|---|
|Пошагово пройти один поток, не боясь, что другие "убегут"|`scheduler-locking on`|
|То же, но чтобы обычный `continue` работал как раньше|`scheduler-locking step`|
|Полная независимость потоков — один можно морозить/шевелить не трогая другие|`non-stop mode`|
|Обычная отладка одного потока в однопоточной программе|ничего не нужно (дефолт `off` подходит)|

## Важные практические ограничения

1. **`non-stop` требует поддержки со стороны target** — на Linux native debugging обычно работает, но через `gdbserver`/remote debugging может не поддерживаться в зависимости от версии
2. **`non-stop` нужно включать ДО `run`** — если процесс уже запущен, `set non-stop on` не подействует задним числом
3. **`scheduler-locking on`** может неожиданно **застопорить** прогресс, если заблокированный поток держит мьютекс, который нужен твоему "активному" потоку — классический deadlock-подобный эффект именно из-за отладчика, а не из-за реального бага в коде (актуально помнить при отладке твоего Boost.Asio кода с `cout_mutex`)

## Про LLDB — короткая ремарка

LLDB **не имеет** прямых аналогов `scheduler-locking`/`non-stop mode` в таком явном виде — ближайшее по духу: LLDB по умолчанию тоже all-stop, но у него нет settings-переключателя формата GDB. Для похожего эффекта в LLDB используют `thread continue <N>` для точечного возобновления, но полноценной "non-stop" инфраструктуры, как в GDB, там нет — ещё один случай, где GDB эргономичнее для сложных многопоточных сценариев.

### Пример

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
  
### Scheduler-locking  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:14  
run  
set scheduler-locking on  
next  
next  
info threads  
  
### Non-stop mode  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
set non-stop on  
set target-async on  
break main.cpp:14  
run  
info threads  
thread 2  
continue  
  
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
