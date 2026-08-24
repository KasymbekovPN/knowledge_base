---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

Оба дали идентичный результат (`init → info → process → shutdown → process`), C++ отдал `"upper-wit-demo-cpp"` в `info()` — подтверждение, что реально исполнилась C++-сборка, а не старая C. Разница в размере наглядно показывает цену C++ на этом пути: core-модуль C — 58 КБ, C++ — 529 КБ (почти в 9 раз больше, весь этот вес — libc++ и WASI-переходники, которых у чистого C нет вообще).

## Пример

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

add_library(ide OBJECT impl.cpp)

```

### plugin.wit
```
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

### impl.cpp
```cpp
// Тот же контракт example:plugin/transform@0.1.0, что в impl.c, но
// реализация уже на C++: состояние плагина -- класс, а не static int,
// трансформация -- std::string + std::transform, а не ручной цикл по
// char*. Единственное, что должно остаться "по-сишному" -- сигнатуры
// экспортируемых функций: они обязаны совпадать с тем, что сгенерировал
// wit-bindgen в plugin.h, и должны быть extern "C" (иначе линковщик не
// найдёт символ example:plugin/transform@0.1.0#process -- C++ исказит
// имя мэнглингом, как мы уже ловили на Extism-плагине).

#include <algorithm>
#include <cctype>
#include <string>

#include "generated/plugin.h"

namespace {
    // Состояние плагина как обычный C++ класс -- никакой разницы с "хостовым"
    // C++, кроме того, что живёт это всё внутри памяти WASM-инстанса.
    class TransformPlugin {
    public:
        void init() { initialized_ = true; }

        [[nodiscard]] bool initialized() const { return initialized_; }

        [[nodiscard]] std::string toUpper(const std::string& input) const {
            std::string out{input};
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
              return static_cast<char>(std::toupper(c));
            });
            return out;
        }

        void shutdown() { initialized_ = false; }

    private:
        bool initialized_{false};
    };

    // Единственный экземпляр состояния на весь инстанс WASM-модуля -- ровно
    // та же модель, что и static int g_initialized в C-версии, просто
    // обёрнутая в класс.
    TransformPlugin g_plugin;
}

extern "C" {

void exports_example_plugin_transform_init(void) { g_plugin.init(); }

void exports_example_plugin_transform_info(exports_example_plugin_transform_plugin_info_t *ret) {
    // plugin_string_dup копирует C-строку через тот же аллокатор
    // (cabi_realloc), которым пользуется сгенерированный код -- так что
    // post_return потом сможет её корректно освободить через free().
    plugin_string_dup(&ret->name, "upper-wit-demo-cpp");
    ret->abi_version = 1;
}

bool exports_example_plugin_transform_process(plugin_string_t *input,
                                              plugin_string_t *ret,
                                              exports_example_plugin_transform_process_error_t *err) {
    // Как и в C-версии: true здесь означает Ok (см. сгенерированный шим
    // __wasm_export_..._process в plugin.c -- `ret.is_err = !process(...)`).
    if (!g_plugin.initialized()) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_NOT_INITIALIZED;
        return false;
    }
    if (input->len == 0) {
        err->tag = EXPORTS_EXAMPLE_PLUGIN_TRANSFORM_PROCESS_ERROR_EMPTY_INPUT;
        return false;
    }

    const std::string text{reinterpret_cast<const char *>(input->ptr), input->len};
    const std::string upper{g_plugin.toUpper(text)};

    // plugin_string_dup_n сам выделяет буфер нужного размера через
    // cabi_realloc и копирует туда данные -- аналог plugin_alloc+memcpy
    // из ручного ABI, но уже сгенерированный, а не написанный руками.
    plugin_string_dup_n(ret, upper.data(), upper.size());

    return true;
}

void exports_example_plugin_transform_shutdown(void) {
    g_plugin.shutdown();
}

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

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang++.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -fno-exceptions -nostartfiles -Wl,--no-entry -I. -o plugin_core.wasm -x c generated/plugin.c -x c++ impl.cpp -x none generated/plugin_component_type.o

wasm-tools component new plugin_core.wasm -o plugin_component.wasm --adapt wasi_snapshot_preview1=generated/wasi_snapshot_preview1.reactor.wasm

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
        std::ifstream file{name, std::ios::binary};
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
            std::cerr << std::format("export not found: {}\n", name);
            std::exit(1);
        }

        return *idx;
    }

    wasmtime::component::Func must_get_func(const wasmtime::component::Instance& instance,
                                            const wasmtime::Store::Context cx,
                                            const wasmtime::component::ExportIndex& idx) {
        const auto f{instance.get_func(cx, idx)};
        if (!f) {
            std::cerr << "export found, but it is not func\n";
            std::exit(1);
        }

        return *f;
    }

}

int main(const int argc, char *argv[]) {
    wasmtime::Engine engine;
    const char* path{argc > 1 ? argv[1] : "plugin_component.wasm"};
    std::cout << std::format("Loading {}\n", path);
    auto bytes{read_file(path)};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Compilation failed: {}\n", component_result.err().message());
        return 1;
    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::component::Linker linker{engine};

    // C++-версия плагина (impl.cpp, собранная через wasi-sdk clang++)
    // тянет за собой libc++ -- а через её abort/terminate пути в модуль
    // попадают низкоуровневые импорты wasi_snapshot_preview1 (fd_write и
    // т.п.), которые после конвертации через wasi-preview1-адаптер в
    // wasm-tools превращаются в НАСТОЯЩИЕ импорты компонент-модели --
    // wasi:cli/stdout, wasi:io/streams и т.д. (см. `wasm-tools component
    // wit`). Чтобы инстанцирование не упало на нехватке этих импортов,
    // подключаем стандартный набор WASIp2 в линкер -- ровно то же самое
    // решение, что with_wasi=true для Extism-плагина на C++ в этой же
    // сессии, только на уровне компонент-модели.
    (void)linker.add_wasip2();
    // На случай если модуль всё же попросит что-то ещё не покрытое --
    // не падаем при инстанцировании, падаем только если реально позовут.
    (void)linker.define_unknown_imports_as_traps(component);

    auto instance_result{linker.instantiate(store.context(), component)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n", instance_result.err().message());
        return 1;
    }
    wasmtime::component::Instance instance{instance_result.unwrap()};

    const auto ctx{store.context()};

    // Сначала находим индекс интерфейса-экспорта "example:plugin/transform@0.1.0",
    // затем внутри него -- индексы отдельных функций.
    wasmtime::component::ExportIndex transform_idx{must_get_index(
        instance,
        ctx,
        nullptr,
        "example:plugin/transform@0.1.0")};

    wasmtime::component::Func init_fn{must_get_func(instance, ctx, must_get_index(
        instance,
        ctx,
        &transform_idx,
        "init"))};
    wasmtime::component::Func info_fn{must_get_func(instance, ctx, must_get_index(
            instance,
            ctx,
            &transform_idx,
            "info"))};
    wasmtime::component::Func process_fn{must_get_func(instance, ctx, must_get_index(
            instance,
            ctx,
            &transform_idx,
            "process"))};
    wasmtime::component::Func shutdown_fn{must_get_func(instance, ctx, must_get_index(
            instance,
            ctx,
            &transform_idx,
            "shutdown"))};

    // 1) init() -- без аргументов, без результата.
    std::vector<wasmtime::component::Val> no_args, no_results;
    if (auto init_res{init_fn.call(ctx, no_args, no_results)}; !init_res) {
        std::cout << std::format("init() failed: {}\n", init_res.err().message());
        return 1;
    }
    std::cout << "init() executed\n";

    // 2) info() -> record { name: string, abi-version: u32 }
    // плейсхолдер, перезапишется вызовом
    std::vector<wasmtime::component::Val> info_results{wasmtime::component::Val{false}};
    std::vector<wasmtime::component::Val> info_args;
    if (auto info_res{info_fn.call(ctx, info_args, info_results)}; !info_res) {
        std::cerr << std::format("info() failed: {}\n", info_res.err().message());
        return 1;
    }

    const wasmtime::component::Record& record{info_results[0].get_record()};
    std::cout << "info() {";
    std::string delimiter;
    for (const wasmtime::component::RecordField& field: record) {
        std::cout << std::format("{}{}=", delimiter, field.name());
        delimiter = ", ";
        if (const auto& value{field.value()}; value.is_string()) {
            std::cout << std::format("'{}'", value.get_string());
        } else if (value.is_u32()) {
            std::cout << value.get_u32();
        }
    }
    std::cout << "}\n";

    // 3) process(input: string) -> result<string, process-error>
    auto call_process{[&](const std::string& text) {
        std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(text)};
        std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};

        if (const auto result{process_fn.call(ctx, args, results)}; !result) {
            std::cerr << std::format("  TRAP: {}\n", result.err().message());
        }

        if (const wasmtime::component::WitResult& wr{results[0].get_result()}; wr.is_ok()) {
            std::cout << std::format("  Ok('{}')\n", wr.payload()->get_string());
        } else {
            std::cout << std::format("  Err({})\n", wr.payload()->get_variant().discriminant());
        }
    }};

    std::cout << "process(\"hello wit component\"):\n";
    call_process("hello wit component");

    // 4) shutdown()
    std::vector<wasmtime::component::Val> shutdown_args, shutdown_results;
    if (const auto shutdown_result{shutdown_fn.call(ctx, shutdown_args, shutdown_results)}; !shutdown_result) {
        std::cerr << std::format("shutdown() failed: {}\n", shutdown_result.err().message());
        return 1;
    }
    std::cout << "shutdown() executed\n";

    // 5) Вызов после shutdown -- у нас plugin_shutdown() просто сбрасывает
    // g_initialized, так что process() после него снова должен вернуть
    // Err(not-initialized) -- проверим, что состояние действительно общее
    // на весь инстанс (тот самый плюс к ручному ABI: здесь это staticная
    // переменная в WASM-памяти инстанса, а не что-то, что хост обязан
    // сериализовать сам).
    std::cout << "process(\"after shutdown\"):\n";
    call_process("after shutdown");

    return 0;
}

```
