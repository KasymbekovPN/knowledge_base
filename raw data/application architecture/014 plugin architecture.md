---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            
	        "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        }  
    ],    
    "testPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "output": {  
                "outputOnFailure": true  
            }  
        }    
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(plugin_demo CXX)  
  
set(CMAKE_CXX_STANDARD 23)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
# plugin_api - только заголовки, интерфейс между host и плагинами  
add_library(plugin_api INTERFACE)  
target_include_directories(plugin_api INTERFACE  
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin_api/include  
)  
  
# Каждый плагин - MODULE (не SHARED): семантически "грузится через dlopen  
# в рантайме", а не линкуется напрямую ни с чем на этапе сборки.  
function(add_demo_plugin name)  
    add_library(${name} MODULE plugins/${name}/${name}.cpp)  
    target_link_libraries(${name} PRIVATE plugin_api)  
    # Имя файла НЕ переопределяем - CMake по умолчанию уже даёт правильное  
    # для платформы имя (libX.so на Linux/macOS, X.dll на Windows), ровно    # то, что строит platform::pluginFileName() в host/main.cpp.    # Складываем все плагины и host в один output-каталог, чтобы host    # мог находить их по относительному пути "./<имя>"    set_target_properties(${name} PROPERTIES  
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin  
            WINDOWS_EXPORT_ALL_SYMBOLS ON    )  
endfunction()  
  
add_demo_plugin(hello_plugin)  
add_demo_plugin(reverse_plugin)  
add_demo_plugin(bad_version_plugin)  
  
add_executable(host host/main.cpp)  
target_link_libraries(host PRIVATE plugin_api ${CMAKE_DL_LIBS})  
set_target_properties(host PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```

### host/main.cpp
```cpp
// Host: сам ничего не знает о конкретных плагинах на этапе компиляции -  
// только про plugin_api/iplugin.hpp. Список библиотек грузится в рантайме.  
//  
// Кроссплатформенность: dlopen/dlsym/dlclose (POSIX, Linux/macOS) и  
// LoadLibrary/GetProcAddress/FreeLibrary (Win32) - это две РАЗНЫЕ API,  
// у них разные сигнатуры, разная обработка ошибок и разное имя файла  
// библиотеки (libX.so vs X.dll). Вместо #ifdef по всему файлу вся эта  
// разница спрятана в один небольшой namespace platform в начале файла -  
// остальной код (LoadedPlugin, main) платформенно-нейтрален.  
  
#include "iplugin.hpp"  
#include <iostream>  
#include <format>  
#include <memory>  
#include <string>  
#include <vector>  
  
#if defined(_WIN32)  
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>  
#else  
#include <dlfcn.h>  
#endif  
  
// ---------------------------------------------------------------------------  
// platform:: - тонкий слой абстракции над динамической загрузкой библиотек.  
// Единственное место в файле с #ifdef _WIN32.  
// ---------------------------------------------------------------------------  
namespace platform {  
#if defined(_WIN32)  
    using LibraryHandle = HMODULE;  
  
    inline LibraryHandle openLibrary(const std::string& path) {  
        return ::LoadLibrary(path.c_str());  
    }  
    inline void* getSymbol(const LibraryHandle handle, const char* name) {  
        return reinterpret_cast<void*>(::GetProcAddress(handle, name));  
    }  
    inline void closeLibrary(const LibraryHandle handle) { ::FreeLibrary(handle); }  
  
    inline std::string lastError() {  
        const DWORD code = ::GetLastError();  
        if (code == 0) return "unknown error";  
        LPSTR buffer{nullptr};  
        const size_t size = ::FormatMessageA(  
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  
            nullptr,  
            code,  
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  
            reinterpret_cast<LPSTR>(&buffer),  
            0,  
            nullptr);  
        std::string message(buffer, size);  
        ::LocalFree(buffer);  
  
        return message;  
    }  
    // На Windows принято X.dll без префикса "lib" (в отличие от Linux/macOS).  
    inline std::string pluginFileName(const std::string& baseName) { return baseName + ".dll"; }  
#else  
    using LibraryHandle = void*;  
  
    inline LibraryHandle openLibrary(const std::string& path) {  
        return ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);  
    }  
    inline void* getSymbol(LibraryHandle handle, const char* name) { return ::dlsym(handle, name); }  
  
    inline void closeLibrary(LibraryHandle handle) { ::dlclose(handle); }  
  
    inline std::string lastError() {  
        const char* err = ::dlerror();  
        return err ? err : "unknown error";  
    }  
    inline std::string pluginFileName(const std::string& baseName) { return "lib" + baseName + ".so"; }  
#endif  
  
    constexpr LibraryHandle kInvalidHandle{};  
    // constexpr LibraryHandle kInvalidHandle{};  
}  
  
// ---------------------------------------------------------------------------  
// LoadedPlugin - RAII-обёртка над загруженной библиотекой + созданным в ней  
// объектом. Платформенно-нейтральна: работает только через platform::*.  
// ---------------------------------------------------------------------------  
class LoadedPlugin {  
public:  
    static std::unique_ptr<LoadedPlugin> load(const std::string& path) {  
        platform::LibraryHandle handle = platform::openLibrary(path);  
        if (handle == platform::kInvalidHandle) {  
            std::cerr << std::format("  library loading error: {}\n", platform::lastError());  
            return nullptr;  
        }  
        const auto getVersion = reinterpret_cast<GetApiVersionFn>(  
            platform::getSymbol(handle, PLUGIN_API_VERSION_FN_NAME));  
        const auto create = reinterpret_cast<CreatePluginFn>(  
            platform::getSymbol(handle, PLUGIN_CREATE_FN_NAME));  
        const auto destroy = reinterpret_cast<DestroyPluginFn>(  
            platform::getSymbol(handle, PLUGIN_DESTROY_FN_NAME));  
        if (!getVersion || !create || !destroy) {  
            std::cerr << std::format("not enough mandatory entry points: {}\n", path);  
            platform::closeLibrary(handle);  
            return nullptr;  
        }  
        if (const int pluginVersion{getVersion()};  
            pluginVersion != PLUGIN_API_VERSION) {  
            std::cerr << std::format("version mismatching: host {} plugin {}\n", PLUGIN_API_VERSION, pluginVersion);  
            platform::closeLibrary(handle);  
            return nullptr;  
        }  
        IPlugin* instance{create()};  
        return std::unique_ptr<LoadedPlugin>(new LoadedPlugin{handle, instance, destroy});  
    }  
    ~LoadedPlugin() {  
        // уничтожаем ЧЕРЕЗ фабрику плагина, не delete напрямую  
        if (instance_) destroy_(instance_);  
        if (handle_ != platform::kInvalidHandle) platform::closeLibrary(handle_);  
    }  
    IPlugin* get() { return instance_; }  
  
    LoadedPlugin(const LoadedPlugin&) = delete;  
    LoadedPlugin& operator=(const LoadedPlugin&) = delete;  
  
private:  
    LoadedPlugin(platform::LibraryHandle handle, IPlugin* instance, DestroyPluginFn destroy):  
        handle_{handle}, instance_{instance}, destroy_{destroy} {}  
  
    platform::LibraryHandle handle_ = platform::kInvalidHandle;  
    IPlugin* instance_{nullptr};  
    DestroyPluginFn destroy_{nullptr};  
};  
  
int main(int argc, char *argv[]) {  
    std::vector<std::string> paths;  
    if (argc > 1) {  
        paths.assign(argv + 1, argv + argc);  
    } else {  
        // Имя файла зависит от платформы (libX.so на Linux/macOS, X.dll на  
        // Windows) - platform::pluginFileName() скрывает эту разницу.        
        for (const auto& base: {"hello_plugin", "reverse_plugin", "bad_version_plugin"})  
            paths.push_back("./" + platform::pluginFileName(base));  
    }  
    for (const auto& path : paths) {  
        std::cout << std::format("== {}\n", path);  
        const auto plugin{LoadedPlugin::load(path)};  
        if (!plugin) continue;  
        std::cout << std::format("  name: {}\n", plugin->get()->name());  
        std::cout << std::format("  result: {}\n", plugin->get()->execute("world"));  
    }    
    // деструкторы LoadedPlugin отрабатывают здесь -> destroyPlugin() + closeLibrary()  

    return 0;  
}
```

### plugin_api/include/iplugin.hpp
```cpp
// Стабильный контракт между хостом и плагинами. Это ЕДИНСТВЕННОЕ, что  
// хост и плагин обязаны знать друг о друге - никаких общих CMake-целей,  
// никакой совместной сборки, только этот заголовок с двух сторон.  
  
#pragma once  
  
// Версия API плагинов. Меняется ТОЛЬКО при несовместимых изменениях  
// интерфейса IPlugin (добавили чисто виртуальный метод, поменяли  
// сигнатуру существующего). Хост и плагин сверяют её до того, как  
// хоть что-то вызвать друг у друга - это и есть versioning на практике,  
// не просто номер в комментарии.  
constexpr int PLUGIN_API_VERSION{1};  
  
class IPlugin {  
public:  
    virtual ~IPlugin() = default;  
    virtual const char* name() const = 0;  
    virtual const char* execute(const char* input) = 0;  
};  
  
// Сигнатуры фабричных функций - точки входа в плагин. extern "C" убирает  
// C++ name mangling: dlsym ищет функцию по простому строковому имени,  
// а не по мангленному имени, которое зависит от компилятора/ABI и может  
// отличаться даже между двумя сборками одним и тем же компилятором с  
// разными флагами. Без extern "C" dlsym("createPlugin") почти наверняка  
// ничего не найдёт.  
extern "C" {  
    using CreatePluginFn = IPlugin* (*)();  
    using DestroyPluginFn = void (*)(IPlugin*);  
    using GetApiVersionFn = int (*)();  
}  
  
#define PLUGIN_CREATE_FN_NAME "createPlugin"  
#define PLUGIN_DESTROY_FN_NAME "destroyPlugin"  
#define PLUGIN_API_VERSION_FN_NAME "pluginApiVersion"
```

### plugins/bad_version_plugin/bad_version_plugin.cpp
```cpp
// Симулирует плагин, собранный против несовместимой (более старой/новой)  
// версии API - например, кто-то забыл пересобрать плагин после того,  
// как хост обновил интерфейс IPlugin. pluginApiVersion() намеренно  
// возвращает не ту версию, чтобы показать, как host должен на это  
// реагировать - НЕ падением, а явным отказом загрузки.  
  
#include "iplugin.hpp"  
  
#include <string>  
  
class BadVersionPlugin : public IPlugin {  
public:  
    const char* name() const override {  
        return "BadVersionPlugin";  
    }    const char* execute(const char* input) override {  
        return "should never be called";  
    }};  
  
extern "C" {  
    // намеренно несовместимая версия  
    int pluginApiVersion() { return PLUGIN_API_VERSION + 1; }  
    IPlugin* createPlugin() { return new BadVersionPlugin(); }  
    void destroyPlugin(IPlugin* p) { delete p; }  
}
```

### plugins/bad_version_plugin/hello_plugin.cpp
```cpp
#include "iplugin.hpp"  
  
#include <string>  
#include <format>  
  
class HelloPlugin: public IPlugin {  
public:  
    const char* name() const override { return "HelloPlugin"; }  
    const char* execute(const char* input) override {  
        result_ = std::format("Hello, {}!", input);  
        return result_.c_str();  
    }private:  
    std::string result_;  
};  
  
// Точки входа - то, что host найдёт через dlsym по имени.  
extern "C" {  
    int pluginApiVersion() { return PLUGIN_API_VERSION; }  
    IPlugin* createPlugin() { return new HelloPlugin(); }  
    void destroyPlugin(IPlugin* p) { delete p; }  
}
```

### plugins/bad_version_plugin/reverse_plugin.cpp
```cpp
#include "iplugin.hpp"  
  
#include <algorithm>  
#include <string>  
  
class ReversePlugin: public IPlugin {  
public:  
    const char* name() const override { return "ReversePlugin"; }  
    const char* execute(const char* input) override {  
        result_ = input;  
        std::ranges::reverse(result_);  
        return result_.c_str();  
    }private:  
    std::string result_;  
};  
  
extern "C" {  
    int pluginApiVersion() { return PLUGIN_API_VERSION; }  
    IPlugin* createPlugin() { return new ReversePlugin(); }  
    void destroyPlugin(IPlugin* p) { delete p; }  
}
```

**Что происходит на уровне ABI**

`plugin_api/iplugin.hpp` — единственное, что физически связывает `host` и плагины. Никакой общей CMake-цели, никакой совместной сборки: плагин мог бы теоретически появиться на диске от третьей стороны, собранной отдельно, и `host` загрузил бы его, ничего о ней не зная на этапе компиляции. Это принципиально отличает plugin-архитектуру от обычной модульности из прошлого раза — там границы были compile-time (`target_link_libraries`), здесь граница — runtime (`dlopen`).

**Зачем `extern "C"` на фабричных функциях**

`createPlugin`/`destroyPlugin`/`pluginApiVersion` объявлены `extern "C"` — без этого C++ компилятор бы "мангл"ил их имена (кодировал сигнатуру в имя символа для перегрузки функций), и мангленное имя зависит от конкретного компилятора и его версии. `dlsym(handle, "createPlugin")` ищет символ по буквальной строке — с C++ манглингом пришлось бы либо угадывать мангленное имя, либо жёстко привязываться к одному компилятору. `extern "C"` даёт стабильную, предсказуемую точку входа независимо от того, чем именно плагин собран (лишь бы тем же ABI класса `IPlugin` — а вот это уже общий C++ ABI, не C, и здесь скрыт нюанс: сам `IPlugin` с виртуальными методами предполагает совместимость по layout vtable, то есть host и плагин обычно должны быть собраны одним и тем же компилятором/ABI, особенно критично на Windows между разными версиями MSVC).

**Почему `destroyPlugin(instance)`, а не `delete instance` в host**

В `LoadedPlugin::~LoadedPlugin()` уничтожение идёт строго через `destroy_(instance_)` — то есть через функцию, живущую в самой .so, а не через `delete` в коде хоста. Причина — плагин мог быть собран с другим аллокатором/рантаймом (особенно актуально для Windows, где разные .dll могут иметь разные экземпляры CRT — heap, в котором `new` выделил память, и heap, в котором `delete` пытается её освободить, физически разные кучи, и вызов `delete` из чужого модуля на объект, созданный в другом — undefined behavior). Правило простое: что создано фабрикой внутри модуля — должно уничтожаться деструктором внутри того же модуля.

**Как работает конкретная последовательность в `LoadedPlugin::load`**

`dlopen` возвращает `handle` — непрозрачный дескриптор загруженной библиотеки. `dlsym(handle, "pluginApiVersion")` ищет функцию по имени и возвращает `void*`, который приходится приводить через `reinterpret_cast` к нужному типу указателя на функцию — компилятор не может проверить сигнатуру на этапе компиляции, потому что до рантайма мы вообще не знаем, что там за библиотека. Если любая из трёх функций не найдена (`dlsym` вернул `nullptr`) — значит .so не следует контракту плагина, и `host` явно отказывается, а не падает по случайному указателю.

**Versioning на практике, не только в комментарии**

Тест на `bad_version_plugin` показывает именно это: `pluginApiVersion()` возвращает `PLUGIN_API_VERSION + 1` (симулирует ситуацию "плагин собран под старую/новую версию интерфейса, кто-то забыл пересобрать"), и `host` — до того как хоть раз вызвать `createPlugin()` — сверяет версию и вежливо отказывается, вместо того чтобы вызвать фабрику, получить объект, чья vtable не соответствует ожидаемому layout `IPlugin`, и упасть где-то глубоко внутри `execute()` с необъяснимым крашем. Реальный вывод программы это подтверждает: `hello_plugin` и `reverse_plugin` отработали нормально, `bad_version_plugin` был отклонён с понятным сообщением ещё до создания объекта.

Важный практический момент про сам номер версии: он должен увеличиваться при любом несовместимом изменении интерфейса `IPlugin` — добавлении нового чисто виртуального метода, смене сигнатуры, смене порядка методов (порядок в vtable важен!). Обратно совместимые изменения (добавление нового метода не в базовый `IPlugin`, а в отдельный `IPluginV2 : IPlugin` — расширение через дополнительный интерфейс) версию ломать не обязаны — это тот же принцип ISP, применённый к эволюции ABI со временем.
