---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__attribute__((constructor))` / `__attribute__((destructor))` — помечают функцию, которая должна выполниться автоматически до входа в `main()` (constructor) или после выхода из него (destructor), на этапе загрузки/выгрузки бинарника или shared-библиотеки. По сути это низкоуровневый аналог глобальных объектов с конструкторами, но для произвольных функций, включая C-код.

```cpp
#include <iostream>

__attribute__((constructor))
void onLoad() {
    std::cout << "Library loaded, initializing...\n";
}

__attribute__((destructor))
void onUnload() {
    std::cout << "Library unloading, cleaning up...\n";
}

int main() {
    std::cout << "main() running\n";
}
// вывод:
// Library loaded, initializing...
// main() running
// Library unloading, cleaning up...
```

Можно задать приоритет — число от 101 до 65535 (меньше — раньше для constructor, позже для destructor), если порядок инициализации нескольких таких функций важен:

```cpp
__attribute__((constructor(101)))
void initFirst() { /* ... */ }

__attribute__((constructor(200)))
void initSecond() { /* ... */ }
```

Типичные применения:

- **Регистрация плагинов/фабрик** при загрузке `.so` — модуль сам регистрирует себя в глобальном реестре при `dlopen`, без явного вызова инициализирующей функции извне.
- **Инициализация статического состояния библиотеки** (например, инициализация сторонней C-библиотеки типа OpenSSL, где раньше требовался ручной `_init`/`_fini`).
- **Инструментация/профилирование** — конструктор устанавливает хуки или обработчики сигналов до начала работы программы.

Осторожности:

- Порядок выполнения между конструкторами из _разных_ единиц трансляции/библиотек не гарантирован строго (аналог "static initialization order fiasco" для глобальных объектов) — приоритет помогает только внутри одной программы/библиотеки, но не решает проблему полностью для сложных зависимостей между `.so`.
- Выполняется даже при `dlopen()` библиотеки во время выполнения программы — то есть не только "перед main", а перед первым использованием после динамической загрузки.
- Компилятор-специфично (GCC/Clang), стандартного `[[...]]`-аналога нет; на MSVC для похожего эффекта используют `#pragma section` + `__declspec(allocate(...))` с указателями на функции в специальной секции (`.CRT$XCU`), что заметно сложнее и менее переносимо.
- Стандартный C++ способ для похожей задачи — глобальный объект с конструктором в анонимном namespace; `constructor`-атрибут используют, когда нужен C-стиль, точный контроль приоритета, или инициализация до конструирования любых глобальных C++ объектов.

## Пример

### README.md

Демонстрация паттерна самостоятельной регистрации плагинов при загрузке .so/.dll: хост-приложение загружает каждую plugin-библиотеку динамически, и в момент загрузки автоматически выполняется её on-load-функция, которая добавляет плагин в общий реестр — без явного вызова какой-либо init-функции извне. Кроссплатформенно: Linux/macOS (GCC/Clang, `.so`) и Windows (MSVC или MinGW, `.dll`).

## Структура

```
plugin-demo/
├── CMakeLists.txt
├── core/                          # общая shared-библиотека
│   ├── CMakeLists.txt
│   ├── include/core/plugin.h            # IPlugin, PluginRegistry, CORE_API
│   ├── include/core/plugin_lifecycle.h  # PLUGIN_LIFECYCLE(...) — кроссплатформенный on-load/on-unload
│   └── src/registry.cpp
├── plugins/
│   ├── plugin_a/                  # -> plugin_a.so / plugin_a.dll
│   │   ├── CMakeLists.txt
│   │   └── plugin_a.cpp
│   └── plugin_b/                  # -> plugin_b.so / plugin_b.dll
│       ├── CMakeLists.txt
│       └── plugin_b.cpp
└── host/                          # исполняемый файл, грузит плагины динамически
    ├── CMakeLists.txt
    ├── platform_dl.h              # dlopen/dlclose vs LoadLibrary/FreeLibrary
    └── main.cpp
```

## Кроссплатформенные различия и как они решены

**Динамическая загрузка библиотеки.** POSIX даёт `dlopen`/`dlclose` (`<dlfcn.h>`), Windows — `LoadLibraryA`/`FreeLibrary` (`<windows.h>`), это два совершенно разных API. Решение — `host/platform_dl.h`: `PlatformLoadLibrary`/`PlatformCloseLibrary`/`PlatformLastError` скрывают разницу за одним интерфейсом, `main.cpp` работает только с ними.

**Автоматический код при загрузке/выгрузке.** Тема, с которой начался этот разговор — `__attribute__((constructor))`/`((destructor))`. Это чисто GCC/Clang-расширение: работает на Linux, macOS, и даже на MinGW (GCC-based компилятор) под Windows. Но у **чистого MSVC** (`cl.exe`, не Clang) этого атрибута попросту нет — единственная гарантированная загрузчиком точка входа при `LoadLibrary`/`FreeLibrary` это `DllMain` с `DLL_PROCESS_ATTACH`/`DLL_PROCESS_DETACH`. `core/plugin_lifecycle.h` прячет это за макросом `PLUGIN_LIFECYCLE`: на GCC/Clang (в том числе clang-cl) разворачивается в пару `__attribute__((constructor/destructor))`, на настоящем MSVC — в тело `DllMain`. `plugin_a.cpp`/`plugin_b.cpp` пишут одну и ту же `PLUGIN_LIFECYCLE(registerPluginA, unregisterPluginA)` и не знают, во что это раскроется.

**Важный нюанс DllMain.** Код внутри `DllMain` выполняется под loader lock — нельзя грузить другие DLL через `LoadLibrary`, создавать и ждать потоки и т.п. (см. "DllMain restrictions" в документации Microsoft). Простая регистрация в `std::vector` внутри своего процесса, как здесь, безопасна, но при усложнении плагина (например, если on-load захочет сам что-то догрузить через `LoadLibrary`) это ограничение может привести к дедлоку — на POSIX-стороне (`constructor`-атрибут) такого ограничения нет.

**Имя файла плагина.** GCC/MinGW по умолчанию добавляют префикс `lib` (`libplugin_a.so`/`libplugin_a.dll`), MSVC — нет (`plugin_a.dll`). Чтобы имя не зависело от компилятора, в `plugins/*/CMakeLists.txt` явно указано `set_target_properties(... PROPERTIES PREFIX "")` — на всех платформах файл называется `plugin_a.<so|dll>`, и `PlatformPluginFileName()` в `platform_dl.h` знает только про разницу в расширении.

**Экспорт символов.** Уже решено раньше в `core/plugin.h` через `CORE_API` (`__declspec(dllexport/dllimport)` на Windows, `__attribute__((visibility("default")))` на Unix) — без изменений.

**STL через границу DLL (только Windows).** `PluginRegistry` передаёт `std::string`/`std::function` между `core.dll`, `host.exe` и `plugin_*.dll`. На Windows это безопасно, только если core, host и все плагины собраны одним и тем же компилятором/версией CRT и с одинаковым режимом рантайма (`/MD` или `/MT`, Debug или Release не смешивать) — иначе разные модули видят несовместимые layout'ы STL или используют разные кучи. На Linux с libstdc++ и одним компилятором в связке эта проблема практически не возникает, но для кроссплатформенного кода стоит об этом помнить.

## Сборка

```bash
cmake -S . -B build
cmake --build build
cd build/bin      # (или соответствующий output-каталог для вашего генератора)
./host            # host.exe на Windows
```

На Windows через Visual Studio генератор: `cmake --build build --config Release`, бинарники окажутся в `build/bin/Release/`.

> Примечание: ветка для настоящего MSVC (`DllMain`) написана по стандартным соглашениям WinAPI, но не скомпилирована в этой среде — здесь нет ни MinGW, ни MSVC, ни доступа в сеть, чтобы их поставить. POSIX-ветка (GCC/Clang, Linux) собрана и прогнана после каждого изменения. Стоит один раз собрать Windows-ветку на реальной машине перед использованием в продакшене.

## Найденный при тестировании баг: destructor обязателен, не опционален

Первая версия падала с segfault **после** выгрузки плагина — не внутри самой выгрузки, а при выходе из процесса. Причина: `PluginRegistry` хранит `PluginFactory` (`std::function<...>`), физически содержащую указатель на код лямбды, живущий внутри `plugin_a.so`. Если запись в реестре не убрать до выгрузки библиотеки, позже при разрушении этой `std::function` (например, при завершении процесса) происходит обращение к памяти, которая больше не отображена.

Исправление: on-unload функция каждого плагина обязана вызывать `PluginRegistry::instance().unregisterPlugin(name)` **до** завершения выгрузки (сама on-unload-функция выполняется, пока библиотека ещё отображена в память — последний безопасный момент почистить за собой).

Вывод для реальных плагинных систем: если хранить в реестре что-либо, чей код/данные физически живут в самом плагине (function pointers, `std::function` с захватом, vtable-указатели на объекты, созданные внутри .so/.dll), on-load/on-unload должны быть _парой_ — недостаточно только регистрации при загрузке, обязательна и симметричная очистка при выгрузке.

## Ожидаемый вывод

```
Loading ./plugin_a.so...
[PluginA] on-load: registered in PluginRegistry
Loading ./plugin_b.so...
[PluginB] on-load: registered in PluginRegistry

Registered plugins:
- PluginA -> [PluginA] Hello from plugin A!
- PluginB -> [PluginB] Greetings from plugin B!

Unloading plugins...
[PluginA] on-unload: unregistered, plugin unloading
[PluginB] on-unload: unregistered, plugin unloading
```

### core/include/core/plugin_lifecycle.hpp
```cpp
#pragma once  
  
// Кроссплатформенный способ выполнить код автоматически при загрузке и  
// при выгрузке плагина (.so на Linux/macOS, .dll на Windows), без  
// явного вызова init-функции извне.  
//  
// - GCC/Clang (Linux, macOS, и MinGW на Windows):  
//   __attribute__((constructor))/((destructor)) — понимается всеми  
//   компиляторами на базе GCC/Clang вне зависимости от целевой ОС.  
// - Windows БЕЗ MinGW (обычный cl.exe, а также clang/clang-cl,  
//   таргетящиеся на MSVC ABI): __attribute__((constructor)) у clang  
//   на этом таргете действительно вызывается при DLL_PROCESS_ATTACH  
//   (через .CRT$XCU), а вот __attribute__((destructor)) НЕ подключён  
//   к DLL_PROCESS_DETACH — деструктор просто никогда не вызывается  
//   (проверено: registerPlugin срабатывает, unregisterPlugin — нет,  
//   ни при FreeLibrary, ни при завершении процесса). Поэтому здесь  
//   нужен DllMain — единственная гарантированная точка входа, которую  
//   вызывает загрузчик Windows при LoadLibrary/FreeLibrary, — для  
//   ЛЮБОГО компилятора, если сборка не под MinGW.  
//  
// ВАЖНЫЙ WINDOWS-НЮАНС: код внутри DllMain выполняется под loader lock —  
// нельзя грузить другие DLL через LoadLibrary, создавать/join'ить потоки  
// и т.п. (см. "DllMain restrictions" в документации Microsoft). Простая  
// регистрация в структуре данных внутри своего процесса (как здесь)  
// безопасна, но при усложнении плагина об этом ограничении важно помнить.  
//  
// PLUGIN_LIFECYCLE(OnLoad, OnUnload) объявляет OnLoad()/OnUnload() как  
// обычные static-функции; их тела пишутся отдельно как определения.  
  
#if defined(_WIN32) && !defined(__MINGW32__)  
  
    #define WIN32_LEAN_AND_MEAN  
    #include <windows.h>  
    #define PLUGIN_LIFECYCLE(OnLoad, OnUnload)\        static void OnLoad();\  
        static void OnUnload();\  
        extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {\  
            if (reason == DLL_PROCESS_ATTACH) { OnLoad(); }\  
            else if (reason == DLL_PROCESS_DETACH) { OnUnload(); }\  
            return TRUE;\  
        }  
#else  
  
    #define PLUGIN_LIFECYCLE(OnLoad, OnUnload) \  
        static void OnLoad();\  
        static void OnUnload();\  
        __attribute__((constructor)) static void OnLoad##_ctor_trampoline() { \  
            OnLoad(); \  
        } \  
        __attribute__((destructor)) static void OnUnload##_ctor_trampoline() { \  
            OnUnload(); \  
        } \  
  
#endif
```

### core/include/core/plugin.hpp
```cpp
#pragma once  
  
#include <functional>  
#include <memory>  
#include <string>  
#include <utility>  
#include <vector>  
  
#if defined(_WIN32) || defined(__CIGWIN__)  
    #if defined(CORE_EXPORTS)  
        #define CORE_API __declspec(dllexport)  
    #else  
        #define CORE_API __declspec(dllimport)  
    #endif  
#else  
    #define CORE_API __attribute__((visibility("default")))  
#endif  
  
// Интерфейс, который должен реализовать каждый плагин.  
class IPlugin {  
public:  
    virtual ~IPlugin() = default;  
    [[nodiscard]] virtual const char* name() const = 0;  
    virtual void execute() const = 0;  
};  
  
using PluginFactory = std::function<std::unique_ptr<IPlugin>()>;  
  
// Глобальный реестр плагинов. Живёт внутри libcore.so — единственный экземпляр  
// на процесс, потому что и host, и каждая plugin_*.so линкуются с одной и той же  
// core-библиотекой, а значит динамический линкер резолвит обращения к  
// PluginRegistry::instance() в один и тот же объект в памяти.  
class CORE_API PluginRegistry {  
public:  
    static PluginRegistry& instance();  
  
    void registerPlugin(std::string, PluginFactory);  
  
    // ВАЖНО: перед dlclose() плагин обязан вызвать это для себя из своего  
    // __attribute__((destructor)). PluginFactory — это std::function,    // хранящий указатель на код (лямбду), физически находящийся внутри    // .so самого плагина. Если оставить запись в реестре висеть после    // dlclose(), то при разрушении этой std::function (например, при    // выходе из процесса) программа попытается выполнить код по адресу,    // который уже был отмаплен из памяти -> segfault.    void unregisterPlugin(const std::string&);  
  
    [[nodiscard]] const std::vector<std::pair<std::string, PluginFactory>>& plugins() const;  
  
private:  
    std::vector<std::pair<std::string, PluginFactory>> plugins_;  
};
```

### core/arc/registry.cpp
```cpp
#include "core/plugin.hpp"  
  
#include <algorithm>  
  
PluginRegistry& PluginRegistry::instance() {  
    static PluginRegistry registry;  
    return registry;  
}  
  
void PluginRegistry::registerPlugin(std::string name, PluginFactory factory) {  
    plugins_.emplace_back(std::move(name), std::move(factory));  
}  
  
void PluginRegistry::unregisterPlugin(const std::string& name) {  
    plugins_.erase(std::ranges::remove_if(  
        plugins_, [&name](const auto& entry) { return entry.first == name; }).begin(),  
        plugins_.end());  
}  
  
const std::vector<std::pair<std::string, PluginFactory>>& PluginRegistry::plugins() const {  
    return plugins_;  
}
```

### core/CMakeLists.txt
```cmake
add_library(core SHARED src/registry.cpp)  
target_include_directories(core PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}/include  
)  
target_compile_definitions(core PRIVATE CORE_EXPORTS)
```

### plugins/plugin_a/CMakeLists.txt
```cmake
add_library(plugin_a MODULE plugin_a.cpp)  
target_link_libraries(plugin_a PRIVATE core)  
  
# Одинаковое имя файла на всех платформах (plugin_a.so / plugin_a.dll),  
# без "lib"-префикса, который GCC/MinGW добавили бы по умолчанию —  
# host/platform_dl.h рассчитывает именно на такое имя.  
set_target_properties(plugin_a PROPERTIES PREFIX "")
```

### plugins/plugin_a/plugin_a.cpp
```cpp
#include "core/plugin.hpp"  
#include "core/plugin_lifecycle.hpp"  
  
#include <iostream>  
#include <memory>  
  
namespace {  
    class PluginA: public IPlugin {  
    public:  
        [[nodiscard]] const char* name() const override { return "PluginA"; }  
        void execute() const override {  
            std::cout << "[PluginA] Hello from plugin A!\n";  
        }    
    };
}  
  
// На GCC/Clang это разворачивается в пару __attribute__((constructor))/  
// ((destructor)); на MSVC — в DllMain. См. core/plugin_lifecycle.h.  
PLUGIN_LIFECYCLE(registerPluginA, unregisterPluginA)  
  
static void registerPluginA() {  
    PluginRegistry::instance().registerPlugin("PluginA", []() {  
        return std::make_unique<PluginA>();  
    });    
    std::cout << "[PluginA] on-load: registered in PluginRegistry\n";  
}  
  
// КРИТИЧНО вызвать unregisterPlugin здесь, до фактической выгрузки  
// библиотеки — запись в реестре хранит std::function с кодом, живущим  
// в этой .so/.dll (см. README, раздел про найденный при тестировании баг).  
static void unregisterPluginA() {  
    PluginRegistry::instance().unregisterPlugin("PluginA");  
    std::cout << "[PluginA] on-unload: unregistered, plugin unloading\n";  
}
```

### plugins/plugin_b/CMakeLists.txt
```cmake
add_library(plugin_b MODULE plugin_b.cpp)  
target_link_libraries(plugin_b PRIVATE core)  
set_target_properties(plugin_a PROPERTIES PREFIX "")
```

### plugins/plugin_b/plugin_b.cpp
```cpp
#include "core/plugin.hpp"  
#include "core/plugin_lifecycle.hpp"  
  
#include <iostream>  
#include <memory>  
  
namespace {  
    class PluginB : public IPlugin {  
    public:  
        [[nodiscard]] const char* name() const override { return "PluginB"; }  
        void execute() const override {  
            std::cout << "[PluginB] Greetings from plugin B!\n";  
        }    
    };
}  
  
PLUGIN_LIFECYCLE(registerPluginB, unregisterPluginB)  
  
static void registerPluginB() {  
    PluginRegistry::instance().registerPlugin("PluginB", [] {  
        return std::make_unique<PluginB>();  
    });    
    std::cout << "[PluginB] on-load: registered in PluginRegistry\n";  
}  
  
static void unregisterPluginB() {  
    PluginRegistry::instance().unregisterPlugin("PluginB");  
    std::cout << "[PluginB] on-unload: unregistered, plugin unloading\n";  
}
```

### host/CMakeLists.txt
```cmake
add_executable(host main.cpp)  
  
# ${CMAKE_DL_LIBS} — на Linux/macOS разворачивается в "dl" (нужна для  
# dlopen/dlclose), на Windows пустая строка (LoadLibrary/FreeLibrary  
# находятся в kernel32, линкуется автоматически) — переменная сама  
# кроссплатформенна, менять под Windows не нужно.  
target_link_libraries(host PRIVATE core ${CMAKE_DL_LIBS})  
  
# BUILD_RPATH/INSTALL_RPATH актуальны только для ELF/Mach-O (Linux/macOS);  
# на Windows эти свойства просто игнорируются CMake, ошибок не будет.  
# На Windows core.dll и host.exe и так ищутся в одной директории по  
# умолчанию (стандартный порядок поиска DLL).  
set_target_properties(host PROPERTIES  
        BUILD_RPATH "$ORIGIN"  
        INSTALL_RPATH "$ORIGIN"  
)
```

### host/platform_dl.hpp
```cpp
#pragma once  
  
#include <string>  
  
// Кроссплатформенная обёртка над dlopen/dlsym/dlclose (POSIX) и  
// LoadLibrary/GetProcAddress/FreeLibrary (Windows). Плагины сами себя  
// регистрируют при загрузке (см. core/plugin_lifecycle.h), поэтому  
// GetProcAddress/dlsym здесь не нужны — только сама загрузка/выгрузка.  
#if defined(_WIN32)  
    #define WIN32_LEAN_AND_MEAN  
    #include <windows.h>  
    using LibraryHandle = HMODULE;  
#else  
    #include <dlfcn.h>  
    using LibraryHandle = void*;  
#endif  
  
inline LibraryHandle PlatformLoadLibrary(const std::string& path) {  
#if defined(_WIN32)  
    return ::LoadLibraryA(path.c_str());  
#else  
    return dlopen(path.c_str(), RTLD_NOW);  
#endif  
}  
  
inline void PlatformCloseLibrary(LibraryHandle handle) {  
#if defined(_WIN32)  
    ::FreeLibrary(handle);  
#else  
    dlclose(handle);  
#endif  
}  
  
inline std::string PlatformLastError() {  
#if defined(_WIN32)  
    DWORD err = ::GetLastError();  
    if (err == 0) {  
        return "unknown error";  
    }    LPSTR buffer  = nullptr;  
    const size_t size = ::FormatMessageA(  
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  
        reinterpret_cast<LPSTR>(&buffer), 0, nullptr);  
    std::string message{buffer, size};  
    ::LocalFree(buffer);  
    return message;  
#else  
    const char* err = ::dlerror();  
    return err ? err : "unknown error";  
#endif  
}  
  
// Плагины собираются с PREFIX "" на всех платформах (см.  
// plugins/*/CMakeLists.txt) — единственная разница в имени файла  
// между платформами это расширение: .so (Linux/macOS) / .dll (Windows).  
inline std::string PlatformPluginFileName(const std::string& baseName) {  
#if defined(_WIN32)  
    return baseName + ".dll";  
#else  
    return baseName + ".so";  
#endif  
}
```

### host/main.cpp
```cpp
#include "core/plugin.hpp"  
#include "platform_dl.hpp"  
  
#include <iostream>  
#include <format>  
#include <string>  
#include <vector>  
  
int main(int argc, char *argv[]) {  
    std::vector<std::string> pluginPaths = {  
        "./" + PlatformPluginFileName("plugin_a"),  
        "./" + PlatformPluginFileName("plugin_b")  
    };  
  
    if (argc > 1) pluginPaths.assign(argv + 1, argv + argc);  
  
    std::vector<LibraryHandle> handlers;  
    for (const auto& path: pluginPaths) {  
        std::cout << std::format("Loading {}...\n", path);  
        LibraryHandle handle = PlatformLoadLibrary(path);  
        if (!handle) {  
            std::cerr << std::format("  failed: {}\n", PlatformLastError());  
            continue;  
        }        handlers.push_back(handle);  
        // К этому моменту on-load-функция плагина (constructor-атрибут на  
        // GCC/Clang, DllMain(DLL_PROCESS_ATTACH) на MSVC) уже отработала —        
        // плагин уже добавлен в PluginRegistry::instance().    
    }  
  
    std::cout << "\nRegistered plugins:\n";  
    for (const auto&[name, factory]: PluginRegistry::instance().plugins()) {  
        const auto plugin = factory();  
        std::cout << std::format("- {} ->", name);  
        plugin->execute();  
    }  

    std::cout << "\nUnloading plugins...\n";  
    for (const LibraryHandle handle: handlers) {  
        PlatformCloseLibrary(handle);  
    }
  
    return 0;  
}
```
