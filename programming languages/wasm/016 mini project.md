---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

Хост (`host_miniproject.cc`) сканирует `plugins/` через `std::filesystem`, находит все `.wasm` и для каждого проходит один и тот же путь: компилирует модуль (замер времени), инстанцирует, проверяет `plugin_abi_version`, зовёт `plugin_init`, копирует тестовую строку в память гостя через `plugin_alloc`, вызывает `plugin_process` (второй замер времени) под единым топливным лимитом (5 000 000 fuel — выставляется одинаково для всех плагинов ещё до того, как хост знает, какой из них окажется «сломанным»), читает и освобождает буферы, зовёт `plugin_shutdown`.

Реальный прогон:

`upper.wasm` → `"HELLO WASM PLUGIN WORLD, THIS IS A TEST"`, `reverse.wasm` → строка задом наперёд, `wordcount.wasm` → `"8"` — все три отработали корректно. `broken.wasm` на вызове `plugin_process` поймал trap `wasm trap: all fuel consumed by WebAssembly` — топливо кончилось на бесконечном цикле, `Func::call` вернул не exception, а обычный `Result` с ошибкой, хост это увидел через `if (!processRes)`, напечатал понятную ошибку и как ни в чём не бывало продолжил со следующим плагином. Итог: 3 из 4 успешно, хост ни разу не упал.

По цифрам — компиляция модуля заняла 9–19 мс на каждый плагин (это JIT-компиляция Cranelift, разовая стоимость на модуль), а сам вызов `plugin_process` на рабочих плагинах — 0.001–0.004 мс, то есть единицы микросекунд. Разница на три порядка величины: компиляция — миллисекунды, вызов — микросекунды. Именно поэтому в реальных плагинных системах модуль компилируют один раз при старте (или ещё раньше — AOT), а не на каждый вызов. У сломанного плагина сам «вызов» занял 1.5–1.7 мс — это не полезная работа, а то время, за которое цикл `while(1)` успел проглотить все 5 миллионов единиц топлива, прежде чем Wasmtime его оборвал.

## Пример

### vcpkg.json
```json
{
    "name": "interrupts",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CmakePresets.json
```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "base",
            "hidden": true,
            "generator": "Ninja Multi-Config",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
            "cacheVariables": {
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md",
                "VCPKG_APPLOCAL_DEPS": "OFF",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
                "CMAKE_CXX_COMPILER": "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe",
                "VCPKG_INSTALL_OPTIONS": "--x-buildtrees-root=C:/vb"
            },
            "environment": {
                "INCLUDE": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\include;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\include;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\ucrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\um;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\shared;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\winrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\cppwinrt",
                "LIB": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\lib\\x64;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\lib\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\ucrt\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\um\\x64",
                "PATH": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\bin\\Hostx64\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.26100.0\\x64;$penv{PATH}"
            }
        },
        {
            "name": "debug",
            "inherits": "base"
        },
        {
            "name": "release",
            "inherits": "base"
        }
    ],
    "buildPresets": [
        {
            "name": "build-base",
            "hidden": true,
            "environment": {
                "INCLUDE": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\include;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\include;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\ucrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\um;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\shared;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\winrt;C:\\Program Files (x86)\\Windows Kits\\10\\include\\10.0.26100.0\\cppwinrt",
                "LIB": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\ATLMFC\\lib\\x64;C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\lib\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\ucrt\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\lib\\10.0.26100.0\\um\\x64",
                "PATH": "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\MSVC\\14.51.36231\\bin\\Hostx64\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.26100.0\\x64;$penv{PATH}"
            }
        },
        {
            "name": "debug",
            "inherits": "build-base",
            "configurePreset": "debug",
            "configuration": "Debug"
        },
        {
            "name": "release",
            "inherits": "build-base",
            "configurePreset": "release",
            "configuration": "Release"
        }
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_mini_project_demo CXX)

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

add_executable(host_mini_project host_mini_project.cpp)
if(WIN32)
    target_link_libraries(host_mini_project PRIVATE wasmtime)
    add_custom_command(TARGET host_mini_project POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_mini_project>"
    )
else()
    target_link_libraries(host_mini_project PRIVATE wasmtime pthread dl m)
endif()

```

### broken.c
```c
/*

Плагин 4 -- намеренно "сломанный". Снаружи выглядит как честный
участник контракта (те же экспорты, версия ABI совпадает), но
plugin_process зависает в бесконечном цикле. Хост не может знать
заранее, какой из плагинов в папке окажется таким -- поэтому защита
(fuel) должна применяться КО ВСЕМ плагинам одинаково, не только к
этому.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/broken.wasm broken.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) { return 0; }

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    (void)in_ptr;
    (void)in_len;
    volatile int x = 0;
    while (1) x++;

    return 0;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { }

```

### reverse.c
```c
/*

Плагин 2: реверс строки. Тот же контракт, другая трансформация.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/reverse.wasm reverse.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    char* out = (char*)plugin_alloc(in_len);
    if (out == NULL) return 0;

    for (int32_t i = 0; i < in_len; i++) {
        out[i] = in_ptr[in_len - 1 - i];
    }

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)in_len;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }

```

### upper.c
```c
/*

Плагин 1: перевод строки в верхний регистр. Реализует контракт из
plugin_abi.h (День 6): plugin_abi_version/init/alloc/free/process/shutdown.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/upper.wasm upper.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    char* out = (char*)plugin_alloc(in_len);
    if (out == NULL) return 0;

    for (int32_t i = 0; i < in_len; i++) {
        char c = in_ptr[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        out[i] = c;
    }

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)in_len;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }

```

### wordcount.c
```c
/*

Плагин 3: подсчёт слов, результат -- десятичная строка ("3", "12"...).
Тот же контракт, но выход не совпадает по длине со входом -- проверка,
что паттерн "указатель + длина" в контракте не завязан на in_len==out_len.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugins/wordcount.wasm wordcount.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_ABI_VERSION 1

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

static int32_t count_words(const char* s, int32_t len) {
    int32_t count = 0;
    int in_word = 0;
    for (int32_t i = 0; i < len; i++) {
        int is_space = (s[i] == ' ' || s[i] == '\t' || s[i] == '\n');
        if (!is_space && !in_word) {
            count++;
            in_word = 1;
        } else {
            in_word = 0;
        }
    }

    return count;
}

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    int32_t n = count_words(in_ptr, in_len);

    // Ручная конвертация числа в ASCII-десятичную строку -- без snprintf,
    // чтобы не тянуть лишнего в такой мелкий плагин.
    char digits[12];
    int32_t digitCount = 0;
    if (n == 0) {
        digits[digitCount++] = '0';
    } else {
        int32_t tmp = n;
        while (tmp > 0) {
            digits[digitCount++] = (char)('0' + (tmp % 10));
            tmp /= 10;
        }
    }

    char *out = (char*)plugin_alloc(digitCount);
    if (out == NULL) return 0;
    for (int32_t i = 0; i < digitCount; i++) {
        // реверс, т.к. цифры набирались с конца
        out[i] = digits[digitCount - 1 - i];
    }

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)digitCount;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }

```

### host_ mini_project.cpp
```cpp
/*

День 8 -- мини-проект: хост сканирует папку plugins/, загружает каждый
.wasm как плагин по контракту из plugin_abi.h, единообразно защищает
КАЖДЫЙ вызов plugin_process топливным лимитом (fuel), изолирует
сломанный плагин (не падает и не останавливает обработку остальных),
и замеряет время компиляции модуля и время вызова функции.

*/

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <ranges>

#include  <wasmtime.hh>

namespace {

    // Единый лимит топлива на весь жизненный цикл плагина (abi_version -> init
    // -> process -> shutdown). Специально не завышен: реальным плагинам из
    // этого проекта его с большим запасом хватает, а "сломанный" плагин с
    // while(1) исчерпает его практически мгновенно -- защита одинакова для
    // всех, никакого спецкейса под broken.wasm в коде хоста нет.
    constexpr uint64_t FUEL_BUDGET{5'000'000};
    const std::string TEST_INPUT{"Hello WASM plugin world, this is a test"};
    const std::filesystem::path PLUGIN_DIR{"plugins"};
    const std::string EXT{".wasm"};

    struct PluginResult {
        std::string name;
        bool ok{false};
        std::string status;
        double compile_ms{0.0};
        double call_ms{0.0};
        std::string output;
    };

    std::vector<uint8_t> read_file(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

}

int main(int argc, char *argv[]) {
    std::vector<std::filesystem::path> wasm_files;
    for (auto& entry: std::filesystem::directory_iterator{PLUGIN_DIR}) {
        if (entry.path().extension() != EXT) continue;
        wasm_files.push_back(entry.path());
    }
    std::ranges::sort(wasm_files);

    std::cout << std::format("Found {} {} files in {}\n",
        wasm_files.size(),
        EXT,
        PLUGIN_DIR.string());
    for (auto& p: wasm_files) std::cout << std::format("  {}\n", p.filename().string());
    std::cout << '\n';

    wasmtime::Config config;
    config.consume_fuel(true);
    wasmtime::Engine engine{std::move(config)};

    std::vector<PluginResult> results;
    for (auto& path: wasm_files) {
        PluginResult r;
        r.name = path.filename().string();
        std::cout << std::format("==== {} ====\n", r.name);

        auto bytes{read_file(path)};

        auto t0{std::chrono::steady_clock::now()};
        auto module_result{wasmtime::Module::compile(engine, bytes)};
        auto t1{std::chrono::steady_clock::now()};
        r.compile_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (!module_result) {
            r.status = std::format("COMPILATION ERROR: {}", module_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        wasmtime::Module module{module_result.unwrap()};
        std::cout << std::format("  Module is compiled in {:.3f} ms\n", r.compile_ms);

        wasmtime::Store store{engine};
        // Топливный лимит выставляется ОДИНАКОВО для каждого плагина, ДО
        // того как хост знает, "хороший" он или "сломанный" -- это и есть
        // защита по умолчанию, а не отдельная ветка для broken.wasm.
        store.context().set_fuel(FUEL_BUDGET).unwrap();
        auto instance_result{wasmtime::Instance::create(store, module, {})};
        if (!instance_result) {
            r.status = std::format("INSTANTIATION ERROR: {}", instance_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        wasmtime::Instance instance{instance_result.unwrap()};

        auto get_export{[&](const char* name) -> std::optional<wasmtime::Func> {
            const auto e{instance.get(store, name)};
            if (!e || !std::holds_alternative<wasmtime::Func>(*e)) return std::nullopt;

            return std::get<wasmtime::Func>(*e);
        }};

        const auto plugin_abi_version_fn{get_export("plugin_abi_version")};
        const auto plugin_init_fn{get_export("plugin_init")};
        const auto plugin_alloc_fn{get_export("plugin_alloc")};
        const auto plugin_free_fn{get_export("plugin_free")};
        const auto plugin_process_fn{get_export("plugin_process")};
        const auto plugin_shutdown_fn{get_export("plugin_shutdown")};
        auto memory_export{instance.get(store, "memory")};

        if (!plugin_abi_version_fn || !plugin_init_fn || !plugin_alloc_fn || !plugin_free_fn || !plugin_process_fn ||
            !plugin_shutdown_fn || !memory_export || !std::holds_alternative<wasmtime::Memory>(*memory_export)) {

            r.status = "ERROR: plugin does not implement contract completely";
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        wasmtime::Memory memory{std::get<wasmtime::Memory>(*memory_export)};
        // 1) abi_version
        auto abi_result{plugin_abi_version_fn->call(store, {})};
        if (!abi_result) {
            r.status = std::format("TRAP on plugin_abi_version: {}", abi_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        if (int32_t abi_version{abi_result.unwrap()[0].i32()}; abi_version != 1) {
            r.status = std::format("ERROR: ABI version {}", abi_version);
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 2) init
        auto init_result{plugin_init_fn->call(store, {})};
        if (!init_result) {
            r.status = std::format("TRAP on plugin_init: {}", init_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        if (auto init_result_code{init_result.unwrap()[0].i32()}; init_result_code != 0) {
            r.status = std::format("plugin_init returned error code {}", init_result_code);
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 3) записываем тестовый вход в память гостя
        auto in_alloc_res{plugin_alloc_fn->call(store, {
            static_cast<int32_t>(TEST_INPUT.size())})};
        if (!in_alloc_res) {
            r.status = std::format("TRAP on plugin_alloc(input): {}", in_alloc_res.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        const int32_t in_ptr{in_alloc_res.unwrap()[0].i32()};
        std::memcpy(memory.data(store).data() + in_ptr, TEST_INPUT.data(), TEST_INPUT.size());

        // 4) вызов plugin_process -- под тем же топливным лимитом, что и
        // весь остальной жизненный цикл. Именно здесь "сломанный" плагин
        // споткнётся об исчерпание топлива.
        auto t2{std::chrono::steady_clock::now()};
        auto process_result{plugin_process_fn->call(store, {
            in_ptr,
            static_cast<int32_t>(TEST_INPUT.size())})};
        auto t3{std::chrono::steady_clock::now()};
        r.call_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        if (!process_result) {
            // Именно эта ветка ловит "сломанный" плагин: хост не падает,
            // печатает понятную ошибку и переходит к следующему плагину.
            std::string full_msg{process_result.err().message()};
            // Для сводки вытаскиваем содержательную строку "wasm trap: ..."
            // (сама причина), а не первую строку backtrace'а; полный текст
            // остаётся виден в подробном логе выше.
            std::string short_msg{full_msg};
            if (const auto trap_pos{full_msg.find("wasm trap:")}; trap_pos != std::string::npos) {
                std::string rest{full_msg.substr(trap_pos)};
                short_msg = rest.substr(0, rest.find('\n'));
            } else {
                short_msg = full_msg.substr(0, full_msg.find('\n'));
            }

            r.status = std::format("TRAP on plugin_process(isolated): {}", short_msg);
            std::cout << std::format("  plugin_process() broken in {:.3f} ms\n", r.call_ms);
            std::cout << std::format("  {}\n\n", full_msg);
            results.push_back(r);
            continue;
        }

        uint64_t packed{static_cast<uint64_t>(process_result.unwrap()[0].i64())};
        int32_t out_ptr{static_cast<int32_t>(packed >> 32)};
        int32_t out_len{static_cast<int32_t>(packed & 0xFFFFFFFFu)};

        if (out_ptr == 0) {
            r.status = "plugin_process notify about error (ptr == 0)";
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        std::string output{
            reinterpret_cast<char *>(memory.data(store).data() + out_ptr),
            static_cast<std::string::size_type>(out_len)};

        // 5) освобождаем оба буфера
        (void)plugin_process_fn->call(store, {in_ptr});
        (void)plugin_process_fn->call(store, {out_ptr});

        // 6) shutdown
        (void)plugin_process_fn->call(store, {});

        r.ok = true;
        r.status = "OK";
        r.output = output;
        std::cout << std::format("  Calling of plugin_process() took {:.3f} ms\n", r.call_ms);
        std::cout << std::format("  INPUT: {}\n", TEST_INPUT);
        std::cout << std::format("  OUTPUT: {}\n\n", output);

        results.push_back(r);
    }

    // Итоговая сводка
    // Примечание: setw считает БАЙТЫ, а не отображаемые символы, так что
    // с кириллицей (2 байта/символ в UTF-8) ручное выравнивание строкой
    // надёжнее, чем std::setw на самой строке заголовка.
    //
    std::cout << "===== DONE =====\n";
    std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n", "Plugin", "Status", "Compilation(ms)", "Call(ms)");
    for (const auto& r : results) {
        std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n",
            r.name,
            (r.ok ? "OK" : "FAIL"),
            r.compile_ms,
            r.call_ms);
    }

    std::cout << "\nError details\n";
    int ok_count{};
    for (const auto& r : results) {
        if (r.ok) ok_count++;
        else std::cout << std::format("  {}: {}\n", r.name, r.status);
    }

    std::cout << std::format("\nHandled {} out of {}\n", ok_count, results.size());

    return 0;
}

```

```
Found 4 .wasm files in plugins
  broken.wasm
  reverse.wasm
  upper.wasm
  wordcount.wasm

==== broken.wasm ====
  Module is compiled in 9.383 ms
  plugin_process() broken in 0.344 ms
  error while executing at wasm backtrace:
    0:     0xfd - broken.wasm!plugin_process
    1:   0x2192 - broken.wasm!plugin_process.command_export
note: using the `WASMTIME_BACKTRACE_DETAILS=1` environment variable may show more debugging information

Caused by:
    wasm trap: all fuel consumed by WebAssembly


==== reverse.wasm ====
  Module is compiled in 7.135 ms
  Calling of plugin_process() took 0.003 ms
  INPUT: Hello WASM plugin world, this is a test
  OUTPUT: tset a si siht ,dlrow nigulp MSAW olleH

==== upper.wasm ====
  Module is compiled in 6.538 ms
  Calling of plugin_process() took 0.003 ms
  INPUT: Hello WASM plugin world, this is a test
  OUTPUT: HELLO WASM PLUGIN WORLD, THIS IS A TEST

==== wordcount.wasm ====
  Module is compiled in 6.243 ms
  Calling of plugin_process() took 0.007 ms
  INPUT: Hello WASM plugin world, this is a test
  OUTPUT: 17

===== DONE =====
|Plugin              |Status  |Compilation(ms)|Call(ms)|
|broken.wasm         |FAIL    |         9.3833|  0.3441|
|reverse.wasm        |OK      |         7.1354|   0.003|
|upper.wasm          |OK      |         6.5384|  0.0031|
|wordcount.wasm      |OK      |         6.2433|  0.0075|

Error details
  broken.wasm: TRAP on plugin_process(isolated): wasm trap: all fuel consumed by WebAssembly

Handled 3 out of 4
```
