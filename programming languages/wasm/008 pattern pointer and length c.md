---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]


```
curl -L -o wasmtime-c-api.tar.xz \
  https://github.com/bytecodealliance/wasmtime/releases/download/v47.0.3/wasmtime-v47.0.3-x86_64-linux-c-api.tar.xz
tar xJf wasmtime-c-api.tar.xz
```

Внутри — `include/wasmtime.hh` (удобная C++ обёртка поверх голого C API) и `lib/libwasmtime.so` / `libwasmtime.a`.

**Код** (`host_str.cc`) — комментарии в файле объясняют каждый шаг, но вкратце это тот же цикл, что и в Python:

```cpp
Engine engine;
Module module = Module::compile(engine, readWasmFile("plugin_str.wasm")).unwrap();
Store store(engine);
Instance instance = Instance::create(store, module, {}).unwrap();

auto memory = std::get<Memory>(*instance.get(store, "memory"));
auto malloc_fn = std::get<Func>(*instance.get(store, "malloc"));
auto to_upper_fn = std::get<Func>(*instance.get(store, "to_upper"));

int32_t guestPtr = malloc_fn.call(store, {length}).unwrap()[0].i32();
std::memcpy(memory.data(store).data() + guestPtr, text.data(), length);
to_upper_fn.call(store, {guestPtr, length}).unwrap();
```

Ключевая строка — `memory.data(store)`. Она возвращает `Span<uint8_t>`, указывающий на настоящий буфер в памяти _хостового_ процесса, где Wasmtime физически держит linear memory гостя. `data.data() + guestPtr` — это ровно та формула «base_ptr_хоста + смещение», которую мы разбирали концептуально ещё в демо с `mem_demo.wat`, только теперь она написана явно, руками, а не спрятана внутри рантайма.

**Сборка и запуск:**

```
g++ -std=c++17 -I wasmtime-c-api/include host_str.cc \
    -L wasmtime-c-api/lib -lwasmtime -lpthread -ldl -lm -o host_str_cpp
LD_LIBRARY_PATH=wasmtime-c-api/lib ./host_str_cpp
```

Результат:

```
Гость выделил буфер по смещению: 66064
Результат после вызова плагина: HELLO FROM THE C++ HOST!
Буфер освобождён гостем.
```

Обратите внимание — смещение `66064` совпало один в один с тем, что было в Python-версии на том же самом `plugin_str.wasm`. Это хорошая подтверждающая деталь к прошлому разбору: `__heap_base` и оверхед аллокатора — свойство самого модуля и его памяти, а не хоста, который его вызывает, поэтому число не зависит от того, на чём написан хост.

Один нюанс на будущее, которого нет в Python-версии: `LD_LIBRARY_PATH` нужен, потому что слинковались с `.so`. Для реального проекта чаще линкуются статически (`-l:libwasmtime.a` вместо `-lwasmtime`, плюс несколько системных библиотек, которые требует Rust-рантайм внутри) — тогда бинарник хоста ни от чего внешнего не зависит.Это по сути уже пересекается с Днём 4 плана (встраивание рантайма в C++) — можем считать его наполовину закрытым. Дальше по чек-листу — прогнать «плохой» модуль (например, с бесконечным циклом) через этот же хост и убедиться, что он не роняет процесс.

### для vcpkg

```
setx WASI_SDK_PREFIX=C:\путь\к\wasi-sdk
```

- `wasm32-wasi.cmake` — кастомный community-триплет vcpkg, chainload'ящий toolchain-файл из wasi-sdk.

### wasm32-wasi.cmake
```CMake
set(VCPKG_TARGET_ARCHITECTURE wasm32)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

if(NOT DEFINED ENV{WASI_SDK_PREFIX})
   message(FATAL_ERROR "Set WASI_SDK_PREFIX to the wasi-sdk install directory")
endif()

set(VCPKG_ENV_PASSTHROUGH_UNTRACKED WASI_SDK_PREFIX)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{WASI_SDK_PREFIX}/share/cmake/wasi-sdk.cmake")

# wasi-sdk targets are always "static" executables/libraries by nature (no shared .so)
set(VCPKG_CMAKE_SYSTEM_NAME WASI)

```

```bash
# положить в <vcpkg-root>/triplets/community/wasm32-wasi.cmake
export WASI_SDK_PREFIX=/путь/к/wasi-sdk
./vcpkg install fmt --triplet wasm32-wasi --overlay-triplets=triplets/community
```

```bash
$env:WASI_SDK_PREFIX = "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows"
.\vcpkg.exe install fmt --triplet wasm32-wasi --overlay-triplets=triplets/community
```

### Пример

### vcpkg.json
```json
{
    "name": "wasm-host-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}
```


### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(wasm_host_demo CXX)

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

add_executable(host_str main.cpp)

if(WIN32)
    target_link_libraries(host_str PRIVATE wasmtime)
    add_custom_command(TARGET host_str POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
                "$<TARGET_FILE_DIR:host_str>"
    )
else()
    target_link_libraries(host_str PRIVATE wasmtime pthread dl m)
endif()
```

### plugin_str.c
```cpp
/*
Простой "плагин": хост кладёт строку в буфер, выделенный самим гостем,
плагин переводит её в верхний регистр прямо в этой памяти (in-place).

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -Wl,--export=malloc -Wl,--export=free -o plugin_str.wasm plugin_str.c

*/

__attribute__((export_name("to_upper")))
void to_upper(char* buf, int len) {
    for (int i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            buf[i] -= 32;
        }
    }
}

```

### main.cpp
```cpp
/*

C++ хост: копирует строку в память WASM-плагина, вызывает функцию
to_upper прямо там же (in-place), читает результат обратно.
Это тот же сценарий, что и host_str.py, но уже на "боевом" API,
который и пойдёт в реальный C++ хост плагинной системы.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -Wl,--export=malloc -Wl,--export=free -o plugin_str.wasm plugin_str.c

*/

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include <wasmtime.hh>

//<
// using namespace wasmtime;

namespace {
    // Читаем скомпилированный .wasm файл как сырые байты
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
    }
}

int main(int argc, char *argv[]) {
    // 1) Engine создаёт компилятор/рантайм, Store -- изолированный "мир"
    //    для одного набора инстансов (у настоящего хоста обычно свой Store
    //    на каждый вызов плагина или на каждую сессию).
    wasmtime::Engine engine;
    auto wasmBytes{read_wasm_file("plugin_str.wasm")};
    wasmtime::Module module{wasmtime::Module::compile(engine, wasmBytes).unwrap()};

    wasmtime::Store store{engine};
    // Модуль ничего не импортирует (мы сами в этом убедились через
    // wasm-objdump -x -- секции Import нет), поэтому список импортов пуст.
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    // 2) Достаём нужные экспорты плагина по имени.
    auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};
    auto malloc_fn{std::get<wasmtime::Func>(*instance.get(store, "malloc"))};
    auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "free"))};
    auto to_upper_fn{std::get<wasmtime::Func>(*instance.get(store, "to_upper"))};

    const std::string text{"hello from C++ host!"};
    std::string::size_type length{static_cast<std::string::size_type>(text.size())};

    // 3) Просим ГОСТЯ выделить буфер -- хост не выбирает смещение сам.
    int32_t guest_ptr{malloc_fn.call(store, {static_cast<int32_t>(length)}).unwrap()[0].i32()};
    std::cout << std::format("The guest allocated a buffer at its own offset: {}\n", guest_ptr);

    // 4) Копируем байты строки в linear memory гостя по этому смещению.
    //    memory.data(store) -- это Span<uint8_t>, указывающий на реальный
    //    буфер в АДРЕСНОМ ПРОСТРАНСТВЕ ХОСТА, где рантайм хранит память
    //    гостя. guestPtr -- это просто индекс внутри этого span.
    auto data{memory.data(store)};
    std::memcpy(data.data() + guest_ptr, text.data(), length);

    // 5) Вызываем функцию плагина с классической парой (указатель, длина).
    to_upper_fn.call(store, {guest_ptr, static_cast<int32_t>(length)}).unwrap();

    // 6) Читаем результат обратно из той же области памяти гостя.
    //    ВАЖНО: data() нужно получить заново -- если бы между шагами вызывался
    //    memory.grow (или WASM-код внутри сам рос по памяти), старый span
    //    мог инвалидироваться, указывая на уже неактуальный буфер.
    auto result_data{memory.data(store)};
    std::string result{reinterpret_cast<char*>(result_data.data() + guest_ptr), length};
    std::cout << std::format("Result after plugin calling: {}\n", result);

    // 7) Освобождаем буфер в госте -- как и в Python-версии, free принимает
    //    только указатель (обычный C ABI free(void*)).
    free_fn.call(store, {guest_ptr}).unwrap();
    std::cout << "Buffer free by guest (free-method called form host) !!!\n";

    return 0;
}

```

