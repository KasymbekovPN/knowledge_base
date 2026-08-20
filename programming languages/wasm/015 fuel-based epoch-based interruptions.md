---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

**Fuel (топливо) — детерминированный лимит по объёму работы.** Мы это уже разбирали на `bad_plugin.c`: `Config::consume_fuel(true)` + `store.set_fuel(N)`. Топливо тратится на каждую WASM-инструкцию, поэтому лимит абсолютно детерминирован и не зависит от загрузки CPU — один и тот же бюджет топлива всегда даёт одинаковое количество «полезной работы», независимо от того, тормозит ли машина параллельными процессами. Идеально для случаев, где важна воспроизводимость (тесты, биллинг по вычислениям), но плохо подходит, если вам важно именно _реальное время_ — быстрый CPU и медленный CPU потратят один и тот же бюджет топлива за разное время.

**Epoch — лимит по реальному времени, снаружи.** Тут другая модель. Включается `Config::epoch_interruption(true)`, и `Store` получает дедлайн через `set_epoch_deadline(ticks)` — «сколько тиков глобального счётчика эпох ещё можно прожить». Счётчик инкрементируется не рантаймом автоматически, а **хостом вручную**, откуда угодно — я поднял отдельный поток, тикающий каждые 10мс через `engine.increment_epoch()`, пока основной поток блокирован внутри `infiniteLoop.call(...)`. С дедлайном в 50 тиков вызов оборвался ровно за ~507мс — почти идеально совпадает с 50×10мс. Сообщение trap'а другое — `wasm trap: interrupt`, а не «all fuel consumed» — это буквально другой механизм внутри Wasmtime, не переименованный fuel. После прерывания я, как и в fuel-демо, снова продлил дедлайн и позвал `ping()` на том же `Store` — вернул `42`, хост пережил.

Ключевое отличие в характере: fuel отвечает на вопрос «сколько инструкций разрешено выполнить», epoch — «сколько времени разрешено потратить». Для реальной плагинной системы epoch обычно ближе к тому, что реально нужно (пользователь ждёт ответ N миллисекунд, а не N абстрактных единиц), а fuel — там, где важна воспроизводимость поведения независимо от железа.

**Лимит памяти (`Store::limiter`) — и тут поймал интересный нюанс.** Сначала попробовал инстанцировать `bad_plugin.wasm` (который объявляет `initial=2` страницы) с лимитом ровно в 1 страницу — ожидал отказа, но инстанцирование **прошло**. Причина в самой документации C API: «limits are only used to limit the creation/growth of resources in the future, this does not retroactively attempt to apply limits to the store» — то есть лимитер не блокирует то, что модуль объявил как _начальный_ размер памяти, только последующий _рост_ сверх него.

Дальше попробовал вызвать `memory.grow(1)` уже **со стороны хоста**, поверх лимита в 2 страницы — тоже прошло, что меня удивило. Разобрался: лимитер реально проверяется только когда рост инициирует **сам WASM-код** через инструкцию `memory.grow` изнутри гостя — а не когда хост дёргает `Memory::grow()` напрямую через embedder API (хост считается доверенным и не подпадает под собственные же ограничения). Написал отдельный крошечный `.wat`-модуль с функцией, которая сама вызывает `memory.grow` изнутри, — и вот тут лимитер сработал как надо: вызов вернул `-1` (стандартная WASM-конвенция отказа при `memory.grow`), подтверждая, что рост, инициированный именно гостем, действительно блокируется.

Практический вывод для вашей архитектуры: `Store::limiter` защищает от того, что _плагин сам_ попытается неограниченно раздуть свою память — а не от того, что хост случайно вызовет что-то не то на своей стороне (это и логично: хост и так контролирует собственный код).

## Пример

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(interrupts_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(WASMTIME_VERSION "v47.0.3")
set(WASMTIME_ARCHIVE "wasmtime-${WASMTIME_VERSION}-x86_64-windows-c-api.zip")

FetchContent_Declare(
        wasmtime_c_api
        URL "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/${WASMTIME_ARCHIVE}"
)
FetchContent_MakeAvailable(wasmtime_c_api)

# В архиве нет своего CMakeLists.txt -- это просто заголовки + готовая
# библиотека, поэтому объявляем IMPORTED-таргет вручную. Дальше он
# используется в проекте как обычная зависимость через target_link_libraries.
# Windows-архив содержит только shared-вариант (wasmtime.dll + .dll.lib),
# в отличие от Linux/macOS, где есть статическая libwasmtime.a.
if(WIN32)
    add_library(wasmtime SHARED IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            IMPORTED_IMPLIB "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
else()
    add_library(wasmtime STATIC IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/libwasmtime.a"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
endif()

add_executable(host_mem_limit host_mem_limit.cpp)
if(WIN32)
    target_link_libraries(host_mem_limit PRIVATE wasmtime)
    add_custom_command(TARGET host_mem_limit POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_mem_limit>"
    )
else()
    target_link_libraries(host_mem_limit PRIVATE wasmtime pthread dl m)
endif()

add_executable(host_epoch host_epoch.cpp)
if(WIN32)
    target_link_libraries(host_epoch PRIVATE wasmtime)
    add_custom_command(TARGET host_epoch POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_epoch>"
    )
else()
    target_link_libraries(host_epoch PRIVATE wasmtime pthread dl m)
endif()
```

### bad_plugin.c
```c
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

```

### host_epoch.cpp
```cpp
// Тот же bad_plugin.wasm, что и в host_bad_module.cc (fuel), но теперь
// прерывание по ЭПОХАМ -- принципиально другой механизм: не count
// инструкций, а тики реального времени, приходящие из ФОНОВОГО потока.

#include <iostream>
#include <format>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <wasmtime.hh>

#include "tools.h"

int main(int argc, char *argv[]) {
    // 1) epoch_interruption вместо consume_fuel -- другой флаг Config,
    //    оба механизма можно включить и одновременно, но для наглядности
    //    держим их раздельно.
    wasmtime::Config config;
    config.epoch_interruption(true);
    wasmtime::Engine engine{std::move(config)};

    auto wasm_bytes{read::wasm_file("bad_plugin.wasm")};
    wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};

    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    const auto infinite_loop_fn{std::get<wasmtime::Func>(*instance.get(store, "infinite_loop"))};
    const auto ping{std::get<wasmtime::Func>(*instance.get(store, "ping"))};

    // 2) По умолчанию дедлайн -- ТЕКУЩАЯ эпоха движка, то есть выполнение
    //    прервалось бы немедленно. set_epoch_deadline(50) говорит: разреши
    //    ещё 50 "тиков" вперёд, прежде чем прерывать.
    store.context().set_epoch_deadline(50);

    // 3) Тики создаёт ОТДЕЛЬНЫЙ поток, по таймеру -- реальное время, а не
    //    подсчёт исполненных инструкций. Именно в этом ключевое отличие
    //    от fuel: fuel детерминирован по количеству работы, epoch -- по
    //    факту "прошло какое-то время снаружи", независимо от того, что
    //    именно WASM-код успел сделать за это время.
    std::atomic<bool> stop_ticket{false};
    std::thread ticker{[&]() {
        while (!stop_ticket.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            engine.increment_epoch();
        }
    }};

    const auto start{std::chrono::steady_clock::now()};
    std::cout << "Calling infinite_loop with epoch-deadline 50 ticks (1 tick ~10ms)\n";

    const auto result{infinite_loop_fn.call(store, {})};
    const auto elapsed_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count()};

    stop_ticket.store(true);
    ticker.join();

    if (!result) {
        std::cout << std::format("Stopped by epoch ~{} ms4 {}\n",
            elapsed_ms,
            result.err().message());
    } else {
        std::cout << "Unexpected!\n";
    }

    std::cout << "\n--- Host is lived ---\n";
    store.context().set_epoch_deadline(50);
    auto ping_result{ping.call(store, {})};

    std::cout << std::format("ping() => {}\n", ping_result.unwrap()[0].i32());

    return 0;
}

```

### grow.wat
```wat
(module
  (memory (export "memory") 2)
  (func (export "grow_from_guest") (result i32)
    i32.const 1
    memory.grow)
)

;; wat2wasm grow_test.wat -o grow_test.wasm
```

### host_mem_limit.cpp
```cpp
// Проверяем гипотезу: limiter реально проверяется только когда РОСТ
// памяти инициирует сам WASM-код (инструкция memory.grow внутри гостя),
// а не когда хост дёргает Memory::grow() напрямую со своей стороны.

#include <iostream>
#include <format>
#include <vector>

#include <wasmtime.hh>

#include "tools.h"

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto wasm_bytes{read::wasm_file("grow_test.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};

    wasmtime::Store store{engine};
    // Модуль объявляет initial=2 страницы. Лимит ставим ровно в эти же
    // 2 страницы (131072 байта) -- инстанцирование должно пройти
    // (начальный размер не ограничивается ретроактивно), а вот попытка
    // САМОГО ГОСТЯ вырасти ещё на 1 страницу поверх лимита -- должна упасть.
    store.limiter(131072, -1, -1, -1, -1);

    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};
    const auto grow_from_guest{std::get<wasmtime::Func>(*instance.get(store, "grow_from_guest"))};
    if (auto result{grow_from_guest.call(store, {})}) {
        const int32_t prev_pages{result.unwrap()[0].i32()};
        std::cout << std::format("grow_from_guest() returned {}\n", prev_pages);
    } else {
        std::cout << std::format("grow_from_guest() -- trap: {}\n", result.err().message());
    }

    return 0;
}

```

### tools.h
```cpp
#pragma once

#include <fstream>
#include <vector>

namespace read {
    inline std::vector<uint8_t> wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }
}

```
