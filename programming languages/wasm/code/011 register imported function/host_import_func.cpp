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
