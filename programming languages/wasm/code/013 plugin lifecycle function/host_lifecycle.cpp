// C++ хост, который проводит плагин через полный жизненный цикл
// контракта из plugin_abi.h:
//   plugin_abi_version -> plugin_init -> plugin_process (x2) -> plugin_shutdown
//
// Два вызова plugin_process подряд специально сделаны, чтобы показать,
// зачем вообще нужно состояние (plugin_call_count) -- это то, что
// невозможно было продемонстрировать на предыдущих stateless-примерах
// вроде plugin_str.c.

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

    // Вызывает plugin_process для одной строки, читает и печатает результат.
    void call_process(wasmtime::Store& store,
                      wasmtime::Instance& instance,
                      const std::string& input) {
        const auto alloc_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_alloc"))};
        const auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_free"))};
        const auto process_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_process"))};
        const auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};

        const int32_t in_len{static_cast<int32_t>(input.size())};
        const int32_t in_ptr{alloc_fn.call(store, {in_len}).unwrap()[0].i32()};
        std::memcpy(memory.data(store).data() + in_ptr, input.data(), in_len);

        const int64_t packed{process_fn.call(store, {in_ptr, in_len}).unwrap()[0].i64()};
        // входной буфер больше не нужен
        free_fn.call(store, {in_ptr}).unwrap();

        // Распаковка контракта: старшие 32 бита -- указатель, младшие -- длина.
        const int32_t out_ptr{static_cast<int32_t>(static_cast<int64_t>(packed) >> 32)};
        std::string::size_type out_len{static_cast<std::string::size_type>(
            static_cast<int32_t>(static_cast<int64_t>(packed) & 0xFFFFFFFFu))};

        if (out_ptr == 0 && out_len == 0) {
            std::cout << "  plugin_process returns error (0, 0)\n";
            return;
        }

        const auto data{memory.data(store)};
        std::string result{reinterpret_cast<char*>(data.data() + out_ptr), out_len};
        std::cout << std::format("  {} -> {}\n", input, result);

        // выходной буфер тоже за хостом
        free_fn.call(store, {out_ptr}).unwrap();
    }
}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto wasm_bytes{read_wasm_file("plugin_lifecycle.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};
    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    // 1) Проверяем версию контракта ДО чего-либо ещё.
    const auto abi_version_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_abi_version"))};
    const int32_t version{abi_version_fn.call(store, {}).unwrap()[0].i32()};
    std::cout << std::format("ABI version: {}\n", version);
    if (version != 1) {
        std::cerr << "Mismatching ABI version\n";
        return 1;
    }

    // 2) init -- ровно один раз.
    const auto init_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_init"))};
    const int32_t init_status{init_fn.call(store, {}).unwrap()[0].i32()};
    std::cout << std::format("plugin_init -> {} (0 = success)\n\n", init_status);

    // 3) process -- сколько угодно раз между init и shutdown.
    std::cout << "Calling #1\n";
    call_process(store, instance, "first call");
    std::cout << "Calling #2\n";
    call_process(store, instance, "second call, same instance");

    const auto call_count_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_call_count"))};
    std::cout << std::format("plugin_call_count -> {}\n\n",
        call_count_fn.call(store, {}).unwrap()[0].i32());

    // 4) shutdown -- ровно один раз, в конце.
    const auto shutdown{std::get<wasmtime::Func>(*instance.get(store, "plugin_shutdown"))};
    shutdown.call(store, {}).unwrap();
    std::cout << "plugin_shutdown called\n";

    return 0;
}
