---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

**C++ без Component Model** — это ровно то, что мы уже видели в дне 9 (`impl.cpp`), только компилируется как обычный core-модуль по контракту `plugin_abi.h` вместо WIT: `TransformPlugin` — обычный класс, `std::string`/`std::transform` вместо ручного цикла, снаружи торчат только `extern "C"` пять экспортов.
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
project(host CXX)

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

add_executable(host host.cpp)
if(WIN32)
    target_link_libraries(host PRIVATE wasmtime)
    add_custom_command(TARGET host POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host>"
    )
else()
    target_link_libraries(host PRIVATE wasmtime pthread dl m)
endif()

#add_library(ide OBJECT upper.cpp)

```

### upper.cpp
```cpp
/*

Тот же контракт plugin_abi.h (Дни 6/8), что у upper.c/reverse.c/
wordcount.c, но реализация -- на C++: класс с состоянием, std::string
и std::transform вместо ручного цикла по char*. НИКАКОГО Component
Model -- обычный core-модуль, ровно как чисто-сишные плагины из Дня 8.
Цель примера -- показать, что хосту (host_miniproject.cc) совершенно
всё равно, на каком языке был написан плагин: граница -- это ABI
(пять экспортов + соглашение о ptr/len), а не исходный код.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang++.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -fno-exceptions -fno-rtti -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -x c++ upper.cpp -o plugins/upper.wasm


*/

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#define PLUGIN_ABI_VERSION 1

namespace {

    class TransformPlugin {
    public:
        void init() { initialized_ = true; }

        [[nodiscard]] bool initialized() const { return initialized_; }

        [[nodiscard]] std::string toUpper(const std::string& input) const {
            std::string out{input};
            std::transform(out.begin(), out.end(), out.begin(), ::toupper);
            return out;
        }

        void shutdown() { initialized_ = false; }

    private:
        bool initialized_{false};
    };

    TransformPlugin g_plugin;

}

extern "C" {

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_plugin.init();
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_plugin.initialized()) return 0;

    const std::string text{in_ptr, (size_t)in_len};
    const std::string upper{g_plugin.toUpper(text)};

    char* out{static_cast<char*>(plugin_alloc(
        static_cast<int32_t>(text.size())))};
    if (out == NULL) return 0;
    std::memcpy(out, upper.data(), upper.size());

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)upper.size();
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_plugin.shutdown(); }

}

```

### host.cpp
```cpp
// День 8 -- мини-проект: хост сканирует папку plugins/, загружает каждый
// .wasm как плагин по контракту из plugin_abi.h, единообразно защищает
// КАЖДЫЙ вызов plugin_process топливным лимитом (fuel), изолирует
// сломанный плагин (не падает и не останавливает обработку остальных),
// и замеряет время компиляции модуля и время вызова функции.

#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <format>
#include <vector>

#include <wasmtime.hh>

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
        std::string output;
        double compile_ms{0.0};
        double call_ms{0.0};
    };

    std::vector<uint8_t> read_file(const std::filesystem::path& path) {
        std::ifstream file{path,std::ios::binary};
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

        // Плагины могут быть собраны из C++ (libc++/libc абортят через
        // настоящий stdio, см. 021 cpp plugin example.md), поэтому хост
        // выдаёт им минимальное WASI-окружение (stdin/stdout/stderr,
        // унаследованные от процесса) вместо пустого списка импортов.
        wasmtime::Linker linker{engine};
        linker.define_wasi().unwrap();

        wasmtime::WasiConfig wasi_config;
        wasi_config.inherit_stdin();
        wasi_config.inherit_stdout();
        wasi_config.inherit_stderr();
        store.context().set_wasi(std::move(wasi_config)).unwrap();

        auto instance_result{linker.instantiate(store.context(), module)};
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
