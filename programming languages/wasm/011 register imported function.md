---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_import_func CXX)

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

add_executable(host_import_func host_import_func.cpp)

if(WIN32)
    target_link_libraries(host_import_func PRIVATE wasmtime)
    add_custom_command(TARGET host_import_func POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_import_func>"
    )
else()
    target_link_libraries(host_import_func PRIVATE wasmtime pthread dl m)
endif()

```
### plugin_hostlog.c
```c
// Плагин импортирует функцию хоста host_log(ptr, len) -- зеркально
// противоположность export_name: import_module задаёт неймспейс
// ("env"), import_name -- имя, под которым хост должен зарегистрировать
// свою реализацию.
__attribute__((import_module("env"), import_name("host_log")))
extern void host_log(const char* ptr, int len);

__attribute__((export_name("run")))
void run(void) {
    const char* msg = "Hello from the plugin, logged via host!";
    int len = 0;
    while (msg[len]) len++;

    // Плагин просто передаёт указатель+длину НА СВОЮ ЖЕ память -- как
    // и раньше, это просто смещение внутри его linear memory.
    host_log(msg, len);
}

```

### host_import_func.cpp
```cpp
/*

C++ хост регистрирует host_log(ptr, len) как импортируемую функцию
и передаёт её при инстанцировании -- плагин вызывает её изнутри,
хост читает переданную строку из памяти ГОСТЯ (а не хоста), используя
тот же паттерн "указатель + длина", что и в host_str.cc.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=run -o plugin_hostlog.wasm plugin_hostlog.c

*/

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
}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto wasmBytes{read_wasm_file("plugin_hostlog.wasm")};
    wasmtime::Module module{wasmtime::Module::compile(engine, wasmBytes).unwrap()};

    // with definition
    {
        std::cout << "=== 0) WITH DEFINITION ===\n";

        wasmtime::Store store{engine};

        // Func::wrap создаёт host-функцию из обычной C++ лямбды. Caller --
        // специальный первый параметр (опциональный), через который можно
        // получить доступ к экспортам ВЫЗЫВАЮЩЕГО инстанса -- в частности,
        // к его памяти, чтобы прочитать строку по (ptr, len).
        wasmtime::Func hostlog = wasmtime::Func::wrap(
            store,
            [](wasmtime::Caller caller, const int32_t ptr, const int32_t len) {
                const auto memExtern{caller.get_export("memory")};
                const auto memory{std::get<wasmtime::Memory>(*memExtern)};
                auto data{memory.data(caller.context())};

                std::string msg{reinterpret_cast<char *>(data.data() + ptr), static_cast<std::string::size_type>(len)};
                std::cout << std::format("[host_log from plugin]: {}\n", msg);
            });

        // Импорты передаются ПОЗИЦИОННО, в том порядке, в котором модуль их
        // объявляет (см. wasm-objdump -x -- Import[0] = env.host_log).
        // Раньше здесь был {} -- пустой список, т.к. plugin_str.wasm/bad_plugin.wasm
        // не импортировали ничего.
        wasmtime::Instance instance{wasmtime::Instance::create(
            store, module, {hostlog}).unwrap()};

        const auto run{std::get<wasmtime::Func>(*instance.get(store, "run"))};
        run.call(store, {}).unwrap();
        std::cout << "Plugin calling completed\n";
    }

    // --- Сценарий 1: голый Instance::create без реализации host_log ---
    {
        std::cout << "=== 1) Nothing ===\n";
        wasmtime::Store store{engine};
        if (const auto result = wasmtime::Instance::create(store, module, {}); !result) {
            std::cout << std::format("Error during instantiation: {}\n", result.err().message());
        } else {
            std::cout << "Unexpected\n";
        }
    }

    // --- Сценарий 2: define_unknown_imports_as_traps ---
    {
        std::cout << "=== 2) define_unknown_imports_as_traps ===\n";
        wasmtime::Store store{engine};
        wasmtime::Linker linker{engine};
        linker.define_unknown_imports_as_traps(module).unwrap();

        if (auto instance_result{linker.instantiate(store, module)}; !instance_result) {
            std::cout << std::format("fail instantiation: {}\n",
                instance_result.err().message());
        } else {
            std::cout << "Instantiation success (!)\n";
            wasmtime::Instance instance{instance_result.unwrap()};
            const auto run{std::get<wasmtime::Func>(*instance.get(store, "run"))};
            if (const auto call_result{run.call(store, {})}; !call_result) {
                std::cout << std::format("run() -- trap: {}\n", call_result.err().message());
            } else {
                std::cout << "Unexpected\n";
            }
        }
    }

    // --- Сценарий 3: define_unknown_imports_as_default_values ---
    {
        std::cout << "\n=== 3) define_unknown_imports_as_default_values ===\n";
        wasmtime::Store store{engine};
        wasmtime::Linker linker{engine};
        linker.define_unknown_imports_as_default_values(store, module).unwrap();

        wasmtime::Instance instance{linker.instantiate(store, module).unwrap()};
        std::cout << "Instantiation succeeded\n";

        const auto run{std::get<wasmtime::Func>(*instance.get(store, "run"))};
        if (const auto call_result{run.call(store, {})}) {
            std::cout << "run succeeded\n";
        } else {
            std::cout << std::format("Unexpected: trap -- {}\n", call_result.err().message());
        }
    }

    return 0;
}

```


**Сторона плагина** (`plugin_hostlog.c`) — зеркальная противоположность `export_name`, который мы уже использовали десяток раз:

```c
__attribute__((import_module("env"), import_name("host_log")))
extern void host_log(const char *ptr, int len);

__attribute__((export_name("run")))
void run(void) {
  const char *msg = "Hello from the plugin, logged via host!";
  int len = 0;
  while (msg[len]) len++;
  host_log(msg, len);
}
```

`import_module`/`import_name` говорят компилятору: эта функция не определена здесь, её нужно взять из внешнего мира, под неймспейсом `env` и именем `host_log`. Проверил через `wasm-objdump -x` — модуль честно объявил `Import[0]: env.host_log`. Обратите внимание — `msg`/`len`, которые плагин передаёт в `host_log`, это указатель на _его собственную_ linear memory (строковый литерал лежит в data-секции самого модуля) — то есть та же пара «указатель + длина», что мы разбирали весь план, просто теперь она едет в обратную сторону: не хост даёт данные гостю, а гость сообщает хосту, где у него лежат данные для чтения.

**Сторона хоста** (`host_import_func.cc`) — регистрация через `Func::wrap`:

```cpp
Func hostLog = Func::wrap(store, [](Caller caller, int32_t ptr, int32_t len) {
  auto memory = std::get<Memory>(*caller.get_export("memory"));
  auto data = memory.data(caller.context());
  std::string msg(reinterpret_cast<char*>(data.data() + ptr), len);
  std::cout << "[host_log из плагина]: " << msg << "\n";
});
```

Ключевой момент — `Caller`. Это специальный первый параметр (необязательный — можно писать лямбду и без него, если функции не нужен доступ к вызывающему инстансу), через который хост-функция достаёт `caller.get_export("memory")` — то есть получает доступ ровно к той же памяти гостя, что мы весь план читали/писали снаружи через `Instance`. Разница лишь в том, что здесь мы внутри callback'а, вызванного _из_ исполняющегося WASM-кода, а не до/после его вызова.

Дальше при создании инстанса функция передаётся в списке импортов — **позиционно**, в том порядке, в котором модуль их объявляет:

```cpp
Instance instance = Instance::create(store, module, {hostLog}).unwrap();
```

Раньше здесь везде стоял пустой `{}`, потому что ни `plugin_str.wasm`, ни `bad_plugin.wasm` ничего не импортировали. Заметь — `wasmtime.hh` на этом уровне API (`Instance::create`) не сверяет имя `env.host_log`, только тип и количество — сверка по имени происходит только если использовать `Linker` вместо прямого `Instance::create` (в комментариях к API это явно упомянуто: «recommended to use `Linker` instead for name-based instantiation» — для плагинной системы с несколькими импортами это, вероятно, надёжнее, чтобы не перепутать порядок).Дальше по Дню 5 остаётся сравнение с моделью через `dlopen` (у плагина через `dlopen` был бы доступ вообще ко всему процессу, а не только к явно зарегистрированным функциям) — это мы уже фактически обсуждали в контексте с `infinite_loop`, так что День 5 можно считать закрытым. 

**Если реализации снаружи действительно нет — модуль вообще не запустится.** Через голый `Instance::create` с пустым списком импортов инстанцирование падает сразу, ещё до всякого вызова:

```
Ошибка при инстанцировании: expected 1 imports, found 0
```

Это происходит на этапе _линковки/инстанцирования_, а не на этапе вызова — то есть плагин даже не начинает исполняться. Логично: WASM-модуль без разрешённых импортов физически не может быть создан, у рантайма просто нет функции, на которую указывал бы `env.host_log` внутри таблицы вызовов.

**Способ 1 — «ленивая» ошибка вместо немедленного отказа.** `Linker::define_unknown_imports_as_traps` заполняет все неразрешённые импорты заглушками, которые сами по себе ничего не делают до тех пор, пока их реально не вызовут:

```
Инстанцирование прошло успешно (!).
А вот вызов run() -- trap: unknown import: `env::host_log` has not been defined
```

Инстанцирование прошло, `run()` даже начал исполняться — и только в момент фактического вызова `host_log` внутри плагина случился trap. Это полезно, если у вас, скажем, десять импортов и хочется загрузить модуль даже когда не все десять реализованы — плагин может вообще не дойти до вызова недостающей функции в конкретном сценарии использования, и тогда всё отработает нормально.

**Способ 2 — настоящее дефолтное поведение.** `Linker::define_unknown_imports_as_default_values` — то, что вы, судя по всему, и имели в виду:

```
Инстанцирование прошло успешно.
Вызов run() тоже прошёл успешно -- host_log молча ничего не сделала
```

Для функций с возвращаемым значением дефолт — это, по документации самого Wasmtime, «функция, которая возвращает нули» (`a function that returns zeros`); для `void`-функции вроде нашей `host_log` это просто безобидный no-op. Плагин вызвал её, ничего не случилось, выполнение продолжилось как ни в чём не бывало.

**Практический вывод для вашей плагинной архитектуры.** Если хотите, чтобы плагин мог опционально пользоваться какой-то возможностью хоста (например, логированием) без обязательного требования её реализовывать — `define_unknown_imports_as_default_values` даёт это «из коробки», без необходимости писать собственную систему feature-флагов на стороне плагина. Если же вы, наоборот, хотите обнаруживать использование нереализованных возможностей (для отладки, например), `define_unknown_imports_as_traps` даст явную ошибку именно в момент вызова, с понятным сообщением — что удобнее, чем молчаливый no-op, если такие вызовы означают баг, а не штатную опциональность.Это, кстати, естественно подводит к Дню 6 плана — версионирование интерфейса плагина: `define_unknown_imports_as_default_values` — как раз один из практических инструментов для обратной совместимости, когда старый плагин не знает про новую опциональную возможность хоста, а новый хост не должен из-за этого падать.
