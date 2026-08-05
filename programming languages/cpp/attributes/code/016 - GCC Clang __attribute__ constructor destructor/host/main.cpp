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
        }
        handlers.push_back(handle);
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
