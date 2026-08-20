---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

### vcpkg.json
```json
{
    "name": "host-import-func",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_lifecycle CXX)

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

add_executable(host_lifecycle host_lifecycle.cpp)

if(WIN32)
    target_link_libraries(host_lifecycle PRIVATE wasmtime)
    add_custom_command(TARGET host_lifecycle POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_lifecycle>"
    )
else()
    target_link_libraries(host_lifecycle PRIVATE wasmtime pthread dl m)
endif()

```

### plugin_abi.h
```c
// Контракт плагинной системы -- версия 1.
//
// Каждый плагин обязан экспортировать РОВНО эти функции с РОВНО этими
// сигнатурами. Хост полагается на этот контракт при загрузке любого
// плагина, независимо от того, что именно плагин делает внутри.
//
//   int32_t plugin_abi_version(void)
//     Возвращает версию контракта, под которую собран плагин (сейчас: 1).
//     Хост обязан проверить это ДО вызова чего-либо ещё и отказаться
//     работать с несовместимой версией -- иначе бинарный формат
//     дальнейших вызовов не гарантирован.
//
//   int32_t plugin_init(void)
//     Вызывается РОВНО ОДИН РАЗ после инстанцирования, до первого
//     plugin_process. Возвращает 0 при успехе, ненулевой код ошибки
//     при провале инициализации (хост не должен звать process дальше).
//
//   void *plugin_alloc(int32_t size)
//     Выделяет буфер в памяти плагина -- используется хостом, чтобы
//     скопировать туда входные данные перед вызовом plugin_process
//     (тот самый паттерн "указатель + длина" из Дня 3).
//
//   void plugin_free(void *ptr)
//     Освобождает буфер, ранее выделенный через plugin_alloc. Хост
//     обязан вызвать это для ЛЮБОГО указателя, полученного от плагина
//     (и на входной, и на выходной буфер) -- иначе куча плагина растёт
//     без ограничений при повторных вызовах.
//
//   uint64_t plugin_process(const char *in_ptr, int32_t in_len)
//     Основной вызов. Может вызываться многократно после одного
//     plugin_init. Упаковывает результат в одно 64-битное значение,
//     т.к. у экспортируемых wasm-функций из C нет естественного способа
//     вернуть пару (ptr, len) отдельными значениями:
//       старшие 32 бита -- указатель на буфер результата (0, если ошибка)
//       младшие 32 бита -- длина результата в байтах
//     Буфер результата выделен через plugin_alloc -- освобождать его
//     обязанность хоста, через plugin_free, ПОСЛЕ того как данные прочитаны.
//
//   void plugin_shutdown(void)
//     Вызывается РОВНО ОДИН РАЗ, когда хост закончил работу с плагином
//     (например, перед тем как уничтожить Instance/Store). Плагин должен
//     освободить любое собственное глобальное состояние здесь.
//
// Порядок вызовов со стороны хоста всегда:
//   plugin_abi_version -> plugin_init -> [ plugin_process ]* -> plugin_shutdown

```

### plugin_lifecycle.c
```c
/*

Реализация контракта из plugin_abi.h. Специально держим небольшое
внутреннее состояние (счётчик вызовов, флаг инициализации), чтобы было
видно, ЗАЧЕМ вообще нужен init/shutdown отдельно от process -- в plugin_str.c
раньше состояния не было вообще, каждый вызов был независим.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_call_count,--export=plugin_shutdown -o plugin_lifecycle.wasm plugin_lifecycle.c

*/

#include <stdint.h>
#include <stdlib.h>

#define PLUGIN_API_VERSION 1

static int g_initialized = 0;
static int32_t g_call_count = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version() {
    return PLUGIN_API_VERSION;
}

__attribute__((export_name("plugin_init")))
int32_t plugin_init() {
    g_initialized = 1;
    g_call_count = 0;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) {
    return malloc((size_t)size);
}

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) {
    free(ptr);
}

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    // Контракт требует init до первого process -- отказ, если его не было.
    if (!g_initialized) {
        // ptr=0, len=0 -- сигнал ошибки хосту
        return 0;
    }
    g_call_count++;

    char* out = (char*)plugin_alloc(in_len);
    if (out == NULL) return 0;

    for (int32_t i = 0; i < in_len; i++) {
        char c = in_ptr[i];
        if (c >= 'a' && c <= 'z') {
            c = (char)(c-32);
        }
        out[i] = c;
    }

    // Упаковка (ptr, len) в один i64: старшие 32 бита -- указатель,
    // младшие 32 -- длина. См. пояснение в plugin_abi.h.
    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)in_len;
}

__attribute__((export_name("plugin_call_count")))
int32_t plugin_call_count() {
    return g_call_count;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown() {
    g_initialized = 0;
    g_call_count = 0;
}

```

### host_lifecycle.cpp
```cpp
// C++ хост, который проводит плагин через полный жизненный цикл
// контракта из plugin_abi.h:
//   plugin_abi_version -> plugin_init -> plugin_process (x2) -> plugin_shutdown
//
// Два вызова plugin_process подряд специально сделаны, чтобы показать,
// зачем вообще нужно состояние (plugin_call_count) -- это то, что
// невозможно было продемонстрировать на предыдущих stateless-примерах
// вроде plugin_str.c.

#include <iostream>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "wasmtime.hh"

namespace {
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file(name, std::ios::binary);
        return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
    }

    // Вызывает plugin_process для одной строки, читает и печатает результат.
    void call_process(wasmtime::Store& store,
                      wasmtime::Instance& instance,
                      const std::string& input) {
        const auto alloc_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_alloc"))};
        const auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_free"))};
        const auto process_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_process"))};
        const auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};

        const int32_t in_len{static_cast<int32_t>(input.size())};
        const int32_t in_ptr{alloc_fn.call(store, {in_len}).unwrap()[0].i32()};
        std::memcpy(memory.data(store).data() + in_ptr, input.data(), in_len);

        const int64_t packed{process_fn.call(store, {in_ptr, in_len}).unwrap()[0].i64()};
        // входной буфер больше не нужен
        free_fn.call(store, {in_ptr}).unwrap();

        // Распаковка контракта: старшие 32 бита -- указатель, младшие -- длина.
        const int32_t out_ptr{static_cast<int32_t>(static_cast<int64_t>(packed) >> 32)};
        std::string::size_type out_len{static_cast<std::string::size_type>(
            static_cast<int32_t>(static_cast<int64_t>(packed) & 0xFFFFFFFFu))};

        if (out_ptr == 0 && out_len == 0) {
            std::cout << "  plugin_process returns error (0, 0)\n";
            return;
        }

        const auto data{memory.data(store)};
        std::string result{reinterpret_cast<char*>(data.data() + out_ptr), out_len};
        std::cout << std::format("  {} -> {}\n", input, result);

        // выходной буфер тоже за хостом
        free_fn.call(store, {out_ptr}).unwrap();
    }
}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto wasm_bytes{read_wasm_file("plugin_lifecycle.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};
    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    // 1) Проверяем версию контракта ДО чего-либо ещё.
    const auto abi_version_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_abi_version"))};
    const int32_t version{abi_version_fn.call(store, {}).unwrap()[0].i32()};
    std::cout << std::format("ABI version: {}\n", version);
    if (version != 1) {
        std::cerr << "Mismatching ABI version\n";
        return 1;
    }

    // 2) init -- ровно один раз.
    const auto init_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_init"))};
    const int32_t init_status{init_fn.call(store, {}).unwrap()[0].i32()};
    std::cout << std::format("plugin_init -> {} (0 = success)\n\n", init_status);

    // 3) process -- сколько угодно раз между init и shutdown.
    std::cout << "Calling #1\n";
    call_process(store, instance, "first call");
    std::cout << "Calling #2\n";
    call_process(store, instance, "second call, same instance");

    const auto call_count_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_call_count"))};
    std::cout << std::format("plugin_call_count -> {}\n\n",
        call_count_fn.call(store, {}).unwrap()[0].i32());

    // 4) shutdown -- ровно один раз, в конце.
    const auto shutdown{std::get<wasmtime::Func>(*instance.get(store, "plugin_shutdown"))};
    shutdown.call(store, {}).unwrap();
    std::cout << "plugin_shutdown called\n";

    return 0;
}

```

**Контракт (`plugin_abi.h`)** — набор функций, который обязан реализовать _любой_ плагин, независимо от того, что он делает внутри:

```
plugin_abi_version() -> int32_t        // проверка совместимости, ПЕРВЫЙ вызов
plugin_init()        -> int32_t        // ровно один раз, до всего остального
plugin_alloc(size)   -> ptr            // выделить буфер для входных данных
plugin_process(ptr, len) -> uint64_t   // сколько угодно раз между init/shutdown
plugin_free(ptr)     -> void           // освободить вход ИЛИ выход
plugin_shutdown()    -> void           // ровно один раз, в конце
```

Порядок вызовов у хоста жёстко зафиксирован: `abi_version → init → [process]* → shutdown`. Прогон в логе это подтверждает буквально по шагам — версия проверена первой, `init` вызван один раз, `process` дважды подряд на одном и том же инстансе, `shutdown` в самом конце.

**Зачем вообще разделять `init`/`process`/`shutdown`, а не иметь одну функцию, как в `plugin_str.c`.** До этого все наши плагины (`to_upper` и так далее) были _stateless_ — каждый вызов ничего не помнил о предыдущих. Реальные плагины почти всегда чему-то учатся между вызовами: держат конфигурацию, кэш, счётчики, соединения. Специально добавил `g_call_count` — глобальное состояние модуля — и `plugin_call_count()` для проверки: `plugin_call_count() -> 2` после двух вызовов `process` доказывает, что состояние действительно живёт между вызовами внутри одного `Instance`, а `plugin_init`/`plugin_shutdown` — это единственные точки, где это состояние законно инициализируется и очищается.

**Зачем проверять `plugin_abi_version` первым делом.** Это версионирование интерфейса, которое мы обсуждали как задачу Дня 6. Если завтра контракт изменится (например, `plugin_process` станет принимать третий параметр), у старых плагинов, собранных под версию 1, номер версии не поменяется сам — хост может сразу отказаться их загружать вместо того, чтобы упасть посреди вызова с непонятной ошибкой из-за несовпадения ABI.

**Зачем упаковывать `(ptr, len)` в один `uint64_t`, а не возвращать два значения.** У экспортируемых из C функций WASM нет естественного способа вернуть пару значений одной функцией (в отличие от `.wat`, где multi-value технически возможен на уровне спецификации, но обычные C-компиляторы так не генерируют код). Решение — bit packing: `(ptr << 32) | len`. Хост это распаковывает сдвигом и маской:

```cpp
int32_t outPtr = static_cast<int32_t>(static_cast<uint64_t>(packed) >> 32);
int32_t outLen = static_cast<int32_t>(static_cast<uint64_t>(packed) & 0xFFFFFFFFu);
```

Ноль в обеих половинах (`0,0`) намеренно зарезервирован как сигнал ошибки — `plugin_process` возвращает его, если позвали до `init` или если `plugin_alloc` не смог выделить память.

**Почему `plugin_alloc`/`plugin_free`, а не голый `malloc`/`free`, как в `plugin_str.c`.** Это чисто вопрос дизайна контракта — оборачивая аллокатор в свои имена, вы оставляете за собой право сменить его реализацию внутри плагина (не обязательно всегда будет libc `malloc`) без изменения самого ABI, который видит хост.

