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
    }
    LPSTR buffer  = nullptr;
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
