/*

Тот же контракт plugin_abi.h (Дни 6/8), что у upper.c/reverse.c/
wordcount.c, но реализация -- на C++: класс с состоянием, std::string
и std::transform вместо ручного цикла по char*. НИКАКОГО Component
Model -- обычный core-модуль, ровно как чисто-сишные плагины из Дня 8.
Цель примера -- показать, что хосту (host_miniproject.cc) совершенно
всё равно, на каком языке был написан плагин: граница -- это ABI
(пять экспортов + соглашение о ptr/len), а не исходный код.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang++.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -fno-exceptions -fno-rtti -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -x c++ upper.cpp -o plugins/upper.wasm


*/

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#define PLUGIN_ABI_VERSION 1

namespace {

    class TransformPlugin {
    public:
        void init() { initialized_ = true; }

        [[nodiscard]] bool initialized() const { return initialized_; }

        [[nodiscard]] std::string toUpper(const std::string& input) const {
            std::string out{input};
            std::transform(out.begin(), out.end(), out.begin(), ::toupper);
            return out;
        }

        void shutdown() { initialized_ = false; }

    private:
        bool initialized_{false};
    };

    TransformPlugin g_plugin;

}

extern "C" {

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_plugin.init();
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_plugin.initialized()) return 0;

    const std::string text{in_ptr, (size_t)in_len};
    const std::string upper{g_plugin.toUpper(text)};

    char* out{static_cast<char*>(plugin_alloc(
        static_cast<int32_t>(text.size())))};
    if (out == NULL) return 0;
    std::memcpy(out, upper.data(), upper.size());

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)upper.size();
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_plugin.shutdown(); }

}
