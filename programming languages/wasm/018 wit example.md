---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

### vcpkg.json
```json
{
    "name": "wit-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_component_demo CXX)

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

add_executable(host_component host_component.cpp)
if(WIN32)
    target_link_libraries(host_component PRIVATE wasmtime)
    add_custom_command(TARGET host_component POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_component>"
    )
else()
    target_link_libraries(host_component PRIVATE wasmtime pthread dl m)
endif()

```

### impl.c
```c
/*

Реализация плагина поверх сгенерированных wit-bindgen биндингов.
Обрати внимание: никакого ручного malloc/free буфера под результат,
никакой упаковки (ptr<<32)|len -- это всё уже сделано генератором
(plugin_string_dup/plugin_string_set) и каноническим ABI компонент-модели.

*/

#include <string.h>
#include <stdlib.h>

#include "generated/plugin.h"

static int g_initialized = 0;

void exports_example_plugin_transform_init(void) {
    g_initialized = 1;
}

void exports_example_plugin_transform_info(exports_example_plugin_transform_plugin_info_t *ret) {
    plugin_string_set(&ret->name, "upper-wit-demo");
    ret->abi_version = 1;
}

bool exports_example_plugin_transform_process(plugin_string_t *input,
                                              plugin_string_t *ret,
                                              exports_example_plugin_transform_process_error_t *err) {
    // Внимание: сгенерированный шим делает `ret.is_err = !process(...)`
    // (см. generated/plugin.c) -- то есть возврат true здесь означает
    // Ok (заполнен ret), а false -- Err (заполнен err). Это НЕ то же
    // самое, что "true = есть ошибка", как можно интуитивно подумать.
    if (!g_initialized) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_NOT_INITIALIZED;
        return false;
    }

    if (input->len == 1) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_EMPTY_INPUT;
        return false;
    }

    // Та же трансформация, что в upper.c из Дня 8 -- но вход/выход уже
    // не (ptr,len) пара, а plugin_string_t, и хосту не нужно самому
    // звать plugin_alloc/plugin_free для этого буфера.
    char* buf = (char*)malloc(input->len);
    for (size_t i = 0; i < input->len; i++) {
        char c = ((char*)input->ptr)[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        buf[i] = c;
    }

    ret->ptr = (uint8_t*) buf;
    ret->len = input->len;
    return true;
}

void exports_example_plugin_transform_shutdown(void) {
    g_initialized = 0;
}

```

### plugin.wit
```wit
// Простой WIT-интерфейс плагина. В отличие от plugin_abi.h (День 6),
// где pointer+length и упаковка (ptr<<32)|len были ручной работой хоста
// и плагина, здесь строки, ошибки и записи -- это ЧАСТЬ языка описания
// интерфейса; маршалинг через линейную память компонент-модель берёт
// на себя сама (канонический ABI), а не автор плагина.

// wit-bindgen c ./plugin.wit --out-dir generated

package example:plugin@0.1.0;

interface transform {
  /// Метаданные плагина -- аналог plugin_abi_version(), но как
  /// структурированная запись, а не голое число.
  record plugin-info {
    name: string,
    abi-version: u32,
  }

  /// Ошибка обработки -- размеченный вариант вместо "магического"
  /// nullptr/0, которым сигнализировалась ошибка в ручном C ABI.
  variant process-error {
    not-initialized,
    empty-input,
    internal(string),
  }

  /// Аналог plugin_init() из Дня 6.
  init: func();

  /// Аналог plugin_abi_version() -- но теперь можно вернуть сразу
  /// имя плагина вместе с версией.
  info: func() -> plugin-info;

  /// Аналог plugin_process(ptr, len) -> packed(ptr,len). Здесь просто
  /// string -> result<string, process-error>: и вход, и выход --
  /// обычные строки, без ручного plugin_alloc/plugin_free.
  process: func(input: string) -> result<string, process-error>;

  /// Аналог plugin_shutdown().
  shutdown: func();
}

world plugin {
  export transform;
}

```

### host_component.cpp
```cpp
/*

Хост поверх РЕАЛЬНОГО Component Model API Wasmtime (wasmtime/component/*.hh),
а не Core-модуля через Module/Instance, как во всех предыдущих днях.
Вызывает init -> info -> process -> process(без init была бы ошибка,
поэтому здесь порядок правильный) -> shutdown на одном и том же
инстансе -- то, что через `wasmtime run --invoke` сделать нельзя,
т.к. каждый его вызов -- это новый инстанс.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -I. -o plugin_core.wasm generated/plugin.c impl.c generated/plugin_component_type.o

wasm-tools component new plugin_core.wasm -o plugin_component.wasm

*/

#include <fstream>
#include <iostream>
#include <format>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>

namespace {

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file(name, std::ios::binary);
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    wasmtime::component::ExportIndex must_get_index(const wasmtime::component::Instance& instance,
                                                    const wasmtime::Store::Context cx,
                                                    const wasmtime::component::ExportIndex* parent,
                                                    const char* name) {
        const auto idx{instance.get_export_index(cx, parent, name)};
        if (!idx) {
            std::cerr << std::format("Export not found: {}\n", name);
            std::exit(1);
        }

        return *idx;
    }

    wasmtime::component::Func must_get_func(const wasmtime::component::Instance& instance,
                                            const wasmtime::Store::Context cx,
                                            const wasmtime::component::ExportIndex& idx) {
        const auto f{instance.get_func(cx, idx)};
        if (!f) {
            std::cerr << "Export found, but it is not function\n";
            std::exit(1);
        }

        return *f;
    }

}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto bytes{read_file("plugin_component.wasm")};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Compilation error of component: {}\n",
            component_result.err().message());
        return 1;

    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::component::Linker linker{engine};
    // У нашего компонента нет импортов вообще (не подключали WASI/wasip2),
    // но define_unknown_imports_as_traps на всякий случай не помешает.
    (void)linker.define_unknown_imports_as_traps(component);

    auto instance_result{linker.instantiate(store.context(), component)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n",
            instance_result.err().message());
        return 1;
    }

    wasmtime::component::Instance instance{instance_result.unwrap()};

    const wasmtime::Store::Context ctx = store.context();

    // Сначала находим индекс интерфейса-экспорта "example:plugin/transform@0.1.0",
    // затем внутри него -- индексы отдельных функций.
    const wasmtime::component::ExportIndex transform_idx{must_get_index(
        instance,
        ctx,
        nullptr,
        "example:plugin/transform@0.1.0")};

    wasmtime::component::Func init_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "init"))};
    wasmtime::component::Func info_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "info"))};
    wasmtime::component::Func process_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "process"))};
    wasmtime::component::Func shutdown_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "shutdown"))};

    // 1) init() -- без аргументов, без результата.
    std::vector<wasmtime::component::Val> no_args, no_returns;
    if (auto init_res{init_fn.call(ctx, no_args, no_returns)}; !init_res) {
        std::cerr << std::format("init() failed: {}\n", init_res.err().message());
        return 1;
    }
    std::cout << "init() done\n";

    // 2) info() -> record { name: string, abi-version: u32 }
    // плейсхолдер, перезапишется вызовом
    std::vector<wasmtime::component::Val> info_results{wasmtime::component::Val{false}};
    std::vector<wasmtime::component::Val> info_args;
    if (auto info_res{info_fn.call(ctx, info_args, info_results)}; !info_res) {
        std::cerr << std::format("info() failed: {}\n", info_res.err().message());
        return 1;
    }

    const wasmtime::component::Record& record{info_results[0].get_record()};
    std::cout << "info(): {";
    std::string delimiter;
    for (const auto& field: record) {
        std::cout << std::format("{}{}=", delimiter, field.name());
        delimiter = ", ";
        if (const auto& value{field.value()};
            value.is_string()) {
            std::cout << std::format("\"{}\"", value.get_string());
        } else if (value.is_u32()) {
            std::cout << value.get_u32();
        }
    }
    std::cout << "}\n";

    // 3) process(input: string) -> result<string, process-error>
    auto call_process{[&](const std::string& text) {
        std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(text)};
        std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};
        if (auto result{process_fn.call(ctx, args, results)};
            !result) {
            std::cerr << std::format("  TRAP: {}\n", result.err().message());
            return;
        }

        if (const wasmtime::component::WitResult& wr{results[0].get_result()};
            wr.is_ok()) {
            std::cout << std::format("  Ok(\"{}\")", wr.payload()->get_string());
        } else {
            const wasmtime::component::Variant& err_variant{wr.payload()->get_variant()};
            std::cout << std::format("  Err({})\n", err_variant.discriminant());
        }
    }};
    const std::string line0{"hello wit component"};
    std::cout << std::format("process(\"{}\")", line0);
    call_process(line0);


    // 4) shutdown()
    std::vector<wasmtime::component::Val> shutdown_args, shutdown_results;
    if (auto shutdown_res{shutdown_fn.call(ctx, shutdown_args, shutdown_results)}; !shutdown_res) {
        std::cerr << std::format("shutdown() failed: {}\n", shutdown_res.err().message());
        return 1;
    }
    std::cout << "shutdown() done\n";

    // 5) Вызов после shutdown -- у нас plugin_shutdown() просто сбрасывает
    // g_initialized, так что process() после него снова должен вернуть
    // Err(not-initialized) -- проверим, что состояние действительно общее
    // на весь инстанс (тот самый плюс к ручному ABI: здесь это staticная
    // переменная в WASM-памяти инстанса, а не что-то, что хост обязан
    // сериализовать сам).
    const std::string line1{"after shutdown"};
    std::cout << std::format("process(\"{}\")", line1);
    call_process(line1);

    return 0;
}

```

---

### День 1 Что такое WASM на самом деле

Цель: понять модель выполнения — модуль, стек-машина, линейная память, песочница — до того как трогать C++.

- [x] Прочитать обзор спецификации на [webassembly.org](https://webassembly.org/) и раздел Concepts на [MDN](https://developer.mozilla.org/en-US/docs/WebAssembly) (2026.08.16)
- [x] Установить `wasmtime` CLI и [WABT](https://github.com/WebAssembly/wabt) (даёт `wat2wasm`/`wasm2wat`/`wasm-objdump`) (2026.08.16)
- [x] Написать вручную крошечный модуль на текстовом формате `.wat` (например, функция сложения двух чисел), скомпилировать в `.wasm` и запустить через `wasmtime run` (2026.08.16)
- [x] Разобраться в разнице: линейная память (один плоский массив байт) vs адресное пространство обычного процесса — почему указатели гостя не совпадают с указателями хоста (2026.08.16)

### День 2 Компиляция C/C++ в WASM

Цель: получить первый рабочий .wasm из своего C++ кода двумя разными тулчейнами.

- [x] Установить [wasi-sdk](https://github.com/WebAssembly/wasi-sdk) (clang с таргетом `wasm32-wasi`), скомпилировать простую C-функцию, запустить через `wasmtime` (2026.08.16)
- [x] Установить [Emscripten](https://emscripten.org/docs/compiling/WebAssembly.html), скомпилировать тот же пример, сравнить: под что заточен каждый тулчейн (WASI/сервер vs браузер), размер бинарника (2026.08.17)
- [x] Прочитать тред ["What is the difference between wasi-sdk and emscripten?"](https://github.com/WebAssembly/wasi-sdk/issues/222) — это прямо отвечает на вопрос «какой инструмент когда» (2026.08.17)
- [x] Почему для плагинной системы (не для браузера) обычно нужен именно `wasi-sdk` / `wasm32-wasi`, а не Emscripten (2026.08.17)

### День 3 Модель памяти и передача данных через границу

Цель: понять, как из хоста передать/получить строку, буфер, структуру — это главный источник багов в плагинных системах на WASM.

- [x] Разобрать паттерн «указатель + длина»: как хост записывает данные в память гостя и как экспортированные `malloc`/`free` дают гостю выделить буфер для хоста (2026.08.17)
- [x] Написать вручную пример: C++ хост копирует строку в память WASM-модуля, вызывает функцию, читает результат обратно (2026.08.17)
- [x] Посмотреть, как это решает готовая библиотека — polistrate-пример [Extism PDK](https://extism.org/docs/concepts/pdk) (не обязательно ставить, достаточно прочитать концепцию памяти) (2026.08.17)

### День 4 Встраивание рантайма в C++ хост

Цель: написать минимальный C++ host-процесс, который сам загружает и исполняет .wasm файл.

- [x] Выбрать рантайм с C API: [Wasmtime C API](https://docs.wasmtime.dev/c-api/) (более активно развивается, из коробки sandboxing/fuel) или [WAMR](https://github.com/bytecodealliance/wasm-micro-runtime) (легче, C-native, хорош для embedded)  (2026.08.17)
- [x] Написать C++ программу: загрузить `.wasm`, создать `Store`/`Instance`, найти экспортированную функцию по имени, вызвать её, получить результат (2026.08.17)
- [x] Прогнать «плохой» модуль (например, с бесконечным циклом) и убедиться, что хост не падает — это и есть главная причина использовать WASM вместо `dlopen` (2026.08.18)

### День 5 Host functions — API хоста для плагина

Цель: научиться давать плагину доступ к «внешнему миру» только через явно разрешённые функции.

- [x] Зарегистрировать в C++ хосте импортируемую функцию (например, `host_log(ptr, len)`) и вызвать её из кода плагина (2026.08.18)
- [x] Понять модель линковки импортов/экспортов: почему это безопаснее, чем то, что плагин через `dlopen` получает вообще всё адресное пространство процесса (2026.08.18)
- [x] Прочитать про реальный пример на проде — [Wasm-плагины в Apache Traffic Server](https://docs.trafficserver.apache.org/en/latest/admin-guide/plugins/wasm.en.html) или Envoy Wasm filters (поиск "envoy wasm filters") (2026.08.18)

### День 6 Проектирование контракта плагина

Цель: спроектировать собственный минимальный ABI плагина — то, что в реальном проекте станет "SDK для авторов плагинов".

- [x] Определить набор функций жизненного цикла плагина: `plugin_init`, `plugin_process`, `plugin_shutdown` (простой C ABI) (2026.08.20)
- [x] Продумать версионирование интерфейса (что происходит, если плагин собран под старую версию API хоста) (2026.08.20)
- [x] Прочитать статью ["Building Native Plugin Systems with WebAssembly Components"](https://tartanllama.xyz/posts/wasm-plugins/) (Sy Brand) — разбирает именно проблему безопасности/интерфейсов/бинарной совместимости при плагинах (2026.08.20)

### День 7 Ресурсные лимиты и отказоустойчивость

Цель: научиться ограничивать плагин по времени/памяти и красиво обрабатывать его сбои.

- [x] Изучить fuel-based и epoch-based прерывание выполнения в Wasmtime (защита от зависшего/злонамеренного плагина) (2026.08.20)
- [x] Настроить лимит на размер линейной памяти модуля (2026.08.20)
- [x] Обработать trap (паника внутри WASM) в C++ хосте так, чтобы это превращалось в обычную ошибку, а не крэш процесса (2026.08.20)

### День 8 Мини-проект: рабочая плагинная система

Цель: увидеть современный, типобезопасный способ описывать интерфейс плагина — это то, к чему сейчас идёт вся индустрия вместо ручного маршалинга.

- [x] C++ хост, который сканирует папку с `.wasm`-файлами и загружает их как плагины (2026.08.21)
- [x] 2–3 простых плагина на C (например, «преобразование текста»: upper-case, реверс строки, подсчёт слов), реализующих общий контракт из Дня 6 (2026.08.21)
- [x] Один намеренно «сломанный» плагин (бесконечный цикл или паника) — хост должен корректно его изолировать и продолжить работу с остальными (2026.08.21)
- [x] Замерить время загрузки модуля и вызова функции — почувствовать порядок величины накладных расходов (2026.08.21)

### День 9 Component Model и WIT — куда движется экосистема

Цель: увидеть современный, типобезопасный способ описывать интерфейс плагина — это то, к чему сейчас идёт вся индустрия вместо ручного маршалинга.

- [x] Прочитать официальный гайд Wasmtime ["An Application with Plugins"](https://docs.wasmtime.dev/wasip2-plugins.html) — эталонный пример плагинной системы на Component Model + WIT (хост на Rust, но архитектура универсальна) (2026.08.21)
- [x] Написать простой `.wit`-файл, описывающий интерфейс плагина (функции + типы), сгенерировать биндинги через [wit-bindgen](https://github.com/bytecodealliance/wit-bindgen) для C (2026.08.21)
- [x] Превратить обычный `.wasm`-модуль в компонент через [wasm-tools](https://github.com/bytecodealliance/wasm-tools) (2026.08.21)
- [ ] Учесть нюанс: поддержка Component Model в C API Wasmtime/WAMR менее зрелая, чем в Rust — для реального проекта это стоит проверить на актуальном состоянии перед выбором

### День 10 Обзор экосистемы: что не изобретать самому

Цель: понимать, где заканчивается «изучение основ» и начинается «взять готовое решение».

- [ ] Посмотреть на [Extism](https://extism.org/) — готовый фреймворк именно под «WASM как плагины», с SDK для C++ хоста и PDK для авторов плагинов на разных языках
- [ ] Прочитать сравнение рантаймов [Wasmtime vs Wasmer vs WasmEdge (2026)](https://reintech.io/blog/wasmtime-vs-wasmer-vs-wasmedge-wasm-runtime-comparison-2026), чтобы понимать текущий ландшафт
- [ ] Сформулировать для себя: для будущего реального проекта — писать маршалинг руками, взять Component Model/WIT, или взять Extism целиком
