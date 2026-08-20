---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

### vcpkg.json
```json
{
    "name": "bad-host-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(bad_host_demo CXX)

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

add_executable(host_bad_module host_bad_module.cpp)

if(WIN32)
    target_link_libraries(host_bad_module PRIVATE wasmtime)
    add_custom_command(TARGET host_bad_module POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_bad_module>"
    )
else()
    target_link_libraries(host_bad_module PRIVATE wasmtime pthread dl m)
endif()

```

### bad_plugin.c
```cpp
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

### host_bad_module.cpp
```cpp
// Хост, который намеренно скармливает себе "плохой" плагин с
// бесконечным циклом -- и не падает благодаря топливному лимиту (fuel)
// Wasmtime. Это последний пункт мини-проекта: доказать, что WASM даёт то,
// чего dlopen() дать не может -- крашнутый/зависший плагин не роняет и
// не подвешивает сам хост-процесс.

#include <iostream>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "wasmtime.hh"

namespace {
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
        };
    }
}


int main(int argc, char *argv[]) {
    // 1) Включаем расход топлива на уровне Config -- без этого set_fuel
    //    ниже просто вернёт ошибку "fuel is not configured".
    wasmtime::Config config;
    config.consume_fuel(true);
    wasmtime::Engine engine{std::move(config)};

    auto wasmBytes{read_wasm_file("bad_plugin.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasmBytes).unwrap()};

    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    const auto infinite_loop_fn{std::get<wasmtime::Func>(*instance.get(store, "infinite_loop"))};
    const auto ping_fn{std::get<wasmtime::Func>(*instance.get(store, "ping"))};

    // 2) Даём этому конкретному вызову ограниченный бюджет "топлива" --
    //    условных единиц выполнения WASM-инструкций. Без явного set_fuel
    //    выполнение вообще не началось бы (0 топлива по умолчанию).
    constexpr uint64_t FUEL_BUDGET{10'000'000};
    store.context().set_fuel(FUEL_BUDGET).unwrap();
    std::cout << std::format("Call infinite_loop with budget: {}\n", FUEL_BUDGET);

    if (const auto result{infinite_loop_fn.call(store, {})}) {
        // Сюда мы попасть не должны -- бесконечный цикл не может завершиться
        // сам по себе.
        std::cout << "Unexpected: calling is finished successfully!\n";
    } else {
        // А вот сюда -- обязаны. Wasmtime оборвал выполнение trap'ом, как
        // только топливо кончилось, и вернул управление хосту как обычную
        // ошибку, а не убил процесс.
        std::cout << std::format("Plugin is stopped by runtime: '{}'\n",
            result.err().message());
    }

    std::cout << "Host is alve\n";

    // 3) Тот же Store, тот же Instance -- просто пополняем топливо и
    //    вызываем СОСЕДНЮЮ функцию из ТОГО ЖЕ модуля. Если бы плагин
    //    реально уронил процесс, до этой строчки мы бы не дошли вообще.
    store.context().set_fuel(FUEL_BUDGET).unwrap();
    auto ping_result{ping_fn.call(store, {})};
    std::cout << std::format("Ping result: {}\n",
        ping_result.unwrap()[0].i32());

    return 0;
}

```

```
Вызываю infinite_loop() с бюджетом 10000000 единиц топлива...
Плагин остановлен рантаймом: error while executing at wasm backtrace:
    0:     0x51 - <unknown>!<wasm function 0>

Caused by:
    wasm trap: all fuel consumed by WebAssembly

--- Хост всё ещё жив после этого. Доказываю: ---
Повторный вызов ping() в том же Store: 42 (ожидали 42)
```

**Что произошло по шагам.** Плагин (`bad_plugin.c`) экспортирует `infinite_loop()` — буквально `while(1) { counter++; }` с `volatile`, чтобы компилятор не выкинул «бессмысленный» цикл при оптимизации. Хост включает расход топлива на уровне `Config`:

```cpp
Config config;
config.consume_fuel(true);
Engine engine(std::move(config));
```

и перед вызовом выдаёт конкретному `Store` ограниченный бюджет:

```cpp
store.context().set_fuel(10'000'000).unwrap();
auto result = infiniteLoop.call(store, {});
```

Топливо — это не время, а условные единицы выполнения WASM-инструкций, поэтому лимит детерминирован и не зависит от загрузки процессора. Как только оно кончилось, Wasmtime **сам** прерывает выполнение и возвращает `Err` с trap'ом `"all fuel consumed by WebAssembly"` — это обычное значение, которое `.call()` возвращает хосту, как любая другая ошибка. Не сигнал ОС, не исключение, не crash — просто false из функции.

**Самое главное — строка после этого.** Тот же `Store`, тот же `Instance`, тот же WASM-модуль — хост просто доливает топлива и зовёт соседнюю функцию `ping()` из **того же самого** плагина, и получает `42`. Если бы «плохой» плагин реально уронил или подвесил процесс, до этой строчки код бы никогда не добрался — ни с `timeout`, ни без него.

**Вот и ответ на вопрос, почему это невозможно с `dlopen`.** Если бы `infinite_loop()` была обычной функцией в `.so`, загруженной через `dlopen`+`dlsym`, вызов `func()` — это просто прыжок по указателю в общем адресном пространстве процесса. Ни у ОС, ни у вызывающего кода нет встроенного способа сказать «выполни не больше N инструкций и верни мне управление» — единственные грубые обходные пути: запускать плагин в отдельном потоке/процессе и убивать его снаружи по таймеру (ненадёжно и медленно — `pthread_cancel` на потоке, зависшем в CPU-цикле без safe cancellation point, может не сработать вообще), либо смириться с тем, что зависший плагин подвесил весь процесс. А если бы плагин не зациклился, а просто разыменовал плохой указатель — в `dlopen`-модели это сразу segfault всего процесса, потому что у него полный доступ к памяти хоста. С WASM у плагина физически нет способа тронуть память хоста (это мы уже разбирали в Дне 3), а зависание душится топливом на уровне рантайма, а не постфактум снаружи.Это закрывает последний открытый пункт мини-проекта (День 8) — сборка полностью работоспособна: изоляция плагинов от памяти хоста (День 3), встраивание рантайма (День 4), и вот теперь доказанная отказоустойчивость к зависшему/сломанному плагину. Дальше по плану остаются только опциональные темы — Component Model/WIT и обзор экосистемы (Extism мы уже неплохо покрыли по пути).
