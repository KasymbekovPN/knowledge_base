// Кроссплатформенный загрузчик observer-плагинов - тот же platform::
// слой (dlopen/LoadLibrary), что и в предыдущей практике по plugin
// architecture, только теперь заточен под ITaskObserver.

#pragma once
#include <cstdint>
#include <itask_observer.hpp>
#include <iostream>
#include <format>
#include <memory>
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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
        const DWORD CODE = ::GetLastError();
        return std::format("Win32 error code {}", CODE);
    }
    inline std::string pluginFileName(const std::string& baseName) { return std::format("{}.dll", baseName); }
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
};

class LoadedObserver {
public:
    static std::unique_ptr<LoadedObserver> load(const std::string& path) {
        const platform::LibraryHandle HANDLE{platform::openLibrary(path)};
        if (HANDLE == platform::K_INVALID_HANDLE) {
            std::cerr << std::format("Can't load {} : {}\n", path, platform::lastError());
            return nullptr;
        }

        const auto GET_VERSION{reinterpret_cast<GetObserverApiVersionFn>(
            platform::getSymbol(HANDLE, OBSERVER_API_VERSION_FN_NAME))};
        const auto CREATE{reinterpret_cast<CreateObserverFn>(
            platform::getSymbol(HANDLE, OBSERVER_CREATE_FN_NAME))};
        const auto DESTROY{reinterpret_cast<DestroyObserverFn>(
            platform::getSymbol(HANDLE, OBSERVER_DESTROY_FN_NAME))};

        if (!GET_VERSION || !CREATE || !DESTROY) {
            std::cerr << std::format("  {} : plugin does not export needed entry points\n", path);
            platform::closeLibrary(HANDLE);
            return nullptr;
        }

        if (const auto version{GET_VERSION()};
            version != TASK_OBSERVER_API_VERSION ) {
            std::cerr << std::format("  {} : API vertion mismatching [host = {}] [plugin = {}]\n]",
                path, TASK_OBSERVER_API_VERSION, version);
            platform::closeLibrary(HANDLE);

            return nullptr;
        }

        return std::unique_ptr<LoadedObserver>(new LoadedObserver{HANDLE, CREATE(), DESTROY});
    }

    LoadedObserver(const LoadedObserver&) = delete;
    LoadedObserver& operator=(const LoadedObserver&) = delete;
    ~LoadedObserver() {
        if (instance_) destroy_(instance_);
        if (handle_ != platform::K_INVALID_HANDLE) platform::closeLibrary(handle_);
    }

    [[nodiscard]] ITaskObserver* get() const { return instance_; }

private:
    LoadedObserver(const platform::LibraryHandle handle, ITaskObserver* instance, const DestroyObserverFn destroy):
        handle_(handle), instance_(instance), destroy_(destroy) {}

    platform::LibraryHandle handle_{platform::K_INVALID_HANDLE};
    ITaskObserver* instance_{nullptr};
    DestroyObserverFn destroy_{nullptr};
};
