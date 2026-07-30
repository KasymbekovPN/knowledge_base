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

    constexpr LibraryHandle K_INVALID_HANDLE{};
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
        if (handle == platform::K_INVALID_HANDLE) {
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
        if (handle_ != platform::K_INVALID_HANDLE) platform::closeLibrary(handle_);
    }

    IPlugin* get() { return instance_; }

    LoadedPlugin(const LoadedPlugin&) = delete;
    LoadedPlugin& operator=(const LoadedPlugin&) = delete;

private:
    LoadedPlugin(platform::LibraryHandle handle, IPlugin* instance, DestroyPluginFn destroy):
        handle_{handle}, instance_{instance}, destroy_{destroy} {}

    platform::LibraryHandle handle_ = platform::K_INVALID_HANDLE;
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
