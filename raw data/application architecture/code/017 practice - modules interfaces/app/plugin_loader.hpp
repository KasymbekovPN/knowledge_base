// // Кроссплатформенный загрузчик observer-плагинов - тот же platform::
// // слой (dlopen/LoadLibrary), что и в предыдущей практике по plugin
// // architecture, только теперь заточен под ITaskObserver.
// #pragma once
// #include <task_observer_api/itask_observer.hpp>
// #include <iostream>
// #include <memory>
// #include <string>
//
// #if defined(_WIN32)
// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>
// #else
// #include <dlfcn.h>
// #endif
//
// namespace platform {
//
// #if defined(_WIN32)
// using LibraryHandle = HMODULE;
// inline LibraryHandle openLibrary(const std::string& path) { return ::LoadLibraryA(path.c_str()); }
// inline void* getSymbol(LibraryHandle handle, const char* name) {
//     return reinterpret_cast<void*>(::GetProcAddress(handle, name));
// }
// inline void closeLibrary(LibraryHandle handle) { ::FreeLibrary(handle); }
// inline std::string lastError() {
//     DWORD code = ::GetLastError();
//     return "Win32 error code " + std::to_string(code);
// }
// inline std::string pluginFileName(const std::string& baseName) { return baseName + ".dll"; }
// #else
// using LibraryHandle = void*;
// inline LibraryHandle openLibrary(const std::string& path) {
//     return ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
// }
// inline void* getSymbol(LibraryHandle handle, const char* name) { return ::dlsym(handle, name); }
// inline void closeLibrary(LibraryHandle handle) { ::dlclose(handle); }
// inline std::string lastError() {
//     const char* err = ::dlerror();
//     return err ? err : "unknown error";
// }
// inline std::string pluginFileName(const std::string& baseName) { return "lib" + baseName + ".so"; }
// #endif
//
// constexpr LibraryHandle kInvalidHandle{};
//
// }  // namespace platform
//
// class LoadedObserver {
// public:
//     static std::unique_ptr<LoadedObserver> load(const std::string& path) {
//         platform::LibraryHandle handle = platform::openLibrary(path);
//         if (handle == platform::kInvalidHandle) {
//             std::cerr << "  не удалось загрузить " << path << ": " << platform::lastError() << "\n";
//             return nullptr;
//         }
//         auto getVersion = reinterpret_cast<GetObserverApiVersionFn>(
//             platform::getSymbol(handle, OBSERVER_API_VERSION_FN_NAME));
//         auto create =
//             reinterpret_cast<CreateObserverFn>(platform::getSymbol(handle, OBSERVER_CREATE_FN_NAME));
//         auto destroy = reinterpret_cast<DestroyObserverFn>(
//             platform::getSymbol(handle, OBSERVER_DESTROY_FN_NAME));
//         if (!getVersion || !create || !destroy) {
//             std::cerr << "  " << path << ": плагин не экспортирует нужные точки входа\n";
//             platform::closeLibrary(handle);
//             return nullptr;
//         }
//         if (getVersion() != TASK_OBSERVER_API_VERSION) {
//             std::cerr << "  " << path << ": несовместимая версия API (host="
//                       << TASK_OBSERVER_API_VERSION << ", плагин=" << getVersion() << ")\n";
//             platform::closeLibrary(handle);
//             return nullptr;
//         }
//         return std::unique_ptr<LoadedObserver>(new LoadedObserver(handle, create(), destroy));
//     }
//
//     ~LoadedObserver() {
//         if (instance_) destroy_(instance_);
//         if (handle_ != platform::kInvalidHandle) platform::closeLibrary(handle_);
//     }
//
//     ITaskObserver* get() { return instance_; }
//
//     LoadedObserver(const LoadedObserver&) = delete;
//     LoadedObserver& operator=(const LoadedObserver&) = delete;
//
// private:
//     LoadedObserver(platform::LibraryHandle handle, ITaskObserver* instance, DestroyObserverFn destroy)
//         : handle_(handle), instance_(instance), destroy_(destroy) {}
//
//     platform::LibraryHandle handle_ = platform::kInvalidHandle;
//     ITaskObserver* instance_ = nullptr;
//     DestroyObserverFn destroy_ = nullptr;
// };