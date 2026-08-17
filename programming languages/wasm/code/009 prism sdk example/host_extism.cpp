/*

C++ хост на Extism -- тот же сценарий, что и host_str.cc на голом
Wasmtime C++ API, но обрати внимание, НАСКОЛЬКО короче стал сам вызов:
ни memory.data(), ни ручного malloc/free, ни арифметики со смещениями.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -o plugin_extism.wasm plugin_extism.c

*/

#include <extism.h>

#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <vector>

namespace {
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
        };
    }
}

int main(int argc, char *argv[]) {
    const auto wasm{read_wasm_file("plugin_extism.wasm")};

    // Создаём плагин напрямую из байт .wasm. with_wasi=false -- мы же сами
    // убедились через wasm-objdump, что плагину WASI не требуется.
    char* errmsg{nullptr};
    ExtismPlugin* plugin{extism_plugin_new(
        wasm.data(),
        wasm.size(),
        /*functions=*/nullptr,
        /*n_functions=*/0,
        /*with_wasi=*/false,
        &errmsg)};

    if (!plugin) {
        std::cerr << std::format("Could not load plugin: {}\n",
            (errmsg != nullptr ? errmsg : "unknown error"));
        if (errmsg) {
            extism_plugin_new_error_free(errmsg);
        }

        return 1;
    }

    const std::string input = "hello from the extism host!";
    // Один вызов: имя функции + указатель на входные байты + длина.
    // Extism сам копирует их внутрь плагина, освобождает буфер после вызова --
    // хосту не нужно ничего знать про malloc/free плагина.
    const int32_t rc{extism_plugin_call(
        plugin,
        "to_upper",
        reinterpret_cast<const uint8_t*>(input.data()),
        input.size())};

    if (rc != 0) {
        std::cerr << std::format("Plugin called with error: {}\n",
            extism_plugin_error(plugin));
        extism_plugin_free(plugin);

        return 1;
    }

    const ExtismSize out_len{extism_plugin_output_length(plugin)};
    const uint8_t* out_data{extism_plugin_output_data(plugin)};
    const std::string result{reinterpret_cast<const char*>(out_data), out_len};

    std::cout << std::format("Result after plugin calling: {}\n", result);

    extism_plugin_free(plugin);

    return 0;
}
