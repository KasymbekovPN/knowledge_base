// День 8 -- мини-проект: хост сканирует папку plugins/, загружает каждый
// .wasm как плагин по контракту из plugin_abi.h, единообразно защищает
// КАЖДЫЙ вызов plugin_process топливным лимитом (fuel), изолирует
// сломанный плагин (не падает и не останавливает обработку остальных),
// и замеряет время компиляции модуля и время вызова функции.

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iostream>
#include <format>
#include <ranges>

#include <wasmtime.hh>

// Единый лимит топлива на весь жизненный цикл плагина (abi_version -> init
// -> process -> shutdown). Специально не завышен: реальным плагинам из
// этого проекта его с большим запасом хватает, а "сломанный" плагин с
// while(1) исчерпает его практически мгновенно -- защита одинакова для
// всех, никакого спецкейса под broken.wasm в коде хоста нет.

namespace {
    constexpr uint64_t FUEL_BUDGET{5'000'000};
    const std::string TEST_INPUT{"Hello WASM plugin world, this is a test"};
    const std::filesystem::path PLUGIN_DIR{"plugins"};
    const std::string EXT{".wasm"};

    struct PluginResult {
        bool ok{false};
        std::string name;
        std::string status;
        std::string output;
        double compile_ms{0.0};
        double call_ms{0.0};
    };

    std::vector<uint8_t> read_file(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    std::optional<wasmtime::Func> get_export(wasmtime::Store& store,
                                             wasmtime::Instance& instance,
                                             const char* name) {
        const auto e{instance.get(store, name)};
        if (!e || !std::holds_alternative<wasmtime::Func>(*e)) return std::nullopt;
        return std::get<wasmtime::Func>(*e);
    }

}

int main(int argc, char *argv[]) {
    std::vector<std::filesystem::path> wasm_files;
    for (const auto& entry: std::filesystem::directory_iterator(PLUGIN_DIR)) {
        if (entry.path().extension() != EXT) continue;
        wasm_files.push_back(entry.path());
    }
    std::ranges::sort(wasm_files);

    std::cout << std::format("Found {} .wasm files in {}:\n",
        wasm_files.size(),
        PLUGIN_DIR.string());
    for (const auto& p: wasm_files) std::cout << std::format("  {}\n", p.filename().string());
    std::cout << '\n';

    wasmtime::Config config;
    config.consume_fuel(true);
    wasmtime::Engine engine{std::move(config)};

    std::vector<PluginResult> results;
    for (const auto& path: wasm_files) {
        PluginResult r;
        r.name = path.filename().string();
        std::cout << std::format("==== {} ====\n", r.name);

        auto bytes{read_file(path)};

        auto t0{std::chrono::steady_clock::now()};
        auto module_result{wasmtime::Module::compile(engine, bytes)};
        auto t1{std::chrono::steady_clock::now()};
        r.compile_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        if (!module_result) {
            r.status = std::format("COMPILATION ERROR: {}\n", module_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        wasmtime::Module module{module_result.unwrap()};
        std::cout << std::format("  Module compiled in {:.3f} ms\n", r.compile_ms);

        wasmtime::Store store{engine};
        // Топливный лимит выставляется ОДИНАКОВО для каждого плагина, ДО
        // того как хост знает, "хороший" он или "сломанный" -- это и есть
        // защита по умолчанию, а не отдельная ветка для broken.wasm.
        store.context().set_fuel(FUEL_BUDGET).unwrap();

        // upper_cpp.wasm (C++, libc++) тянет за собой WASI-импорты
        // (fd_write/fd_seek/fd_close) через свои abort/terminate-пути, даже
        // с -fno-exceptions -fno-rtti -- ровно та же история, что с Extism
        // и с impl.cpp в Дне 9. Чисто-сишные плагины этого не делают, но
        // хост не должен полагаться на язык плагина -- поэтому WASI теперь
        // подключается всегда, через Linker вместо голого Instance::create.
        wasmtime::WasiConfig wasi;
        wasi.inherit_stdout();
        wasi.inherit_stderr();
        store.context().set_wasi(std::move(wasi)).unwrap();

        wasmtime::Linker linker{engine};
        linker.define_wasi().unwrap();
        auto instance_result{linker.instantiate(store, module)};
        if (!instance_result) {
            r.status = std::format("INSTANTIATION ERROR: {}\n", instance_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        wasmtime::Instance instance{instance_result.unwrap()};

        auto abi_version_fn{get_export(store, instance, "plugin_abi_version")};
        auto init_fn{get_export(store, instance, "plugin_init")};
        auto alloc_fn{get_export(store, instance, "plugin_alloc")};
        auto free_fn{get_export(store, instance, "plugin_free")};
        auto process_fn{get_export(store, instance, "plugin_process")};
        auto shutdown_fn{get_export(store, instance, "plugin_shutdown")};
        auto memory_export{instance.get(store, "memory")};

        if (!abi_version_fn ||
            !init_fn ||
            !alloc_fn ||
            !free_fn ||
            !process_fn ||
            !shutdown_fn ||
            !memory_export || !std::holds_alternative<wasmtime::Memory>(*memory_export)) {

            r.status = std::format("ERROR: plugin does not implement contract completely");
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        wasmtime::Memory memory{std::get<wasmtime::Memory>(*memory_export)};

        // 1) abi_version
        auto abi_res{abi_version_fn->call(store, {})};
        if (!abi_res) {
            r.status = std::format("TRAP on plugin_abi_version: {}\n", abi_res.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        if (const int32_t abi_version{abi_res.unwrap()[0].i32()}; abi_version != 1) {
            r.status = std::format("ABI VERSION MISMATCHING: {}", abi_version);
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 2) init
        auto init_result{init_fn->call(store, {})};
        if (!init_result) {
            r.status = std::format("TRAP on plugin_init: {}\n", init_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        if (init_result.unwrap()[0].i32() != 0) {
            r.status = "plugin_init returned error code";
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        // 3) записываем тестовый вход в память гостя
        auto in_alloc_result{alloc_fn->call(store, {static_cast<int32_t>(TEST_INPUT.size())})};
        if (!in_alloc_result) {
            r.status = std::format("TRAP on plugin_alloc: {}\n", in_alloc_result.err().message());
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }
        const int32_t in_ptr{in_alloc_result.unwrap()[0].i32()};
        std::memcpy(memory.data(store).data() + in_ptr, TEST_INPUT.data(), TEST_INPUT.size());

        // 4) вызов plugin_process -- под тем же топливным лимитом, что и
        // весь остальной жизненный цикл. Именно здесь "сломанный" плагин
        // споткнётся об исчерпание топлива.
        const auto t2{std::chrono::steady_clock::now()};
        auto process_result{process_fn->call(store, {
            in_ptr,
            static_cast<int32_t>(TEST_INPUT.size()),})};
        const auto t3{std::chrono::steady_clock::now()};
        r.call_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

        if (!process_result) {
            // Именно эта ветка ловит "сломанный" плагин: хост не падает,
            // печатает понятную ошибку и переходит к следующему плагину.
            const std::string full_message{process_result.err().message()};
            // Для сводки вытаскиваем содержательную строку "wasm trap: ..."
            // (сама причина), а не первую строку backtrace'а; полный текст
            // остаётся виден в подробном логе выше.
            std::string short_message{full_message};
            const auto trap_pos{full_message.find("wasm trap:")};
            if (trap_pos != std::string::npos) {
                std::string rest{full_message.substr(trap_pos)};
                short_message = rest.substr(0, rest.find('\n'));
            } else {
                short_message = full_message.substr(0, full_message.find('\n'));
            }
            r.status = std::format("TRAP on plugin_process: {}\n", short_message);
            std::cout << std::format("  plugin_process failed in {:.3f} ms\n", r.call_ms);
            std::cout << std::format(" {}\n\n", full_message);
            results.push_back(r);
            continue;
        }

        const uint64_t packed{static_cast<uint64_t>(process_result.unwrap()[0].i64())};
        const int32_t out_ptr{static_cast<int32_t>(static_cast<uint32_t>(packed >> 32))};
        const int32_t out_len{static_cast<int32_t>(static_cast<uint32_t>(packed & 0xFFFFFFFFu))};
        if (out_ptr == 0) {
            r.status = std::format("plugin_process notify about error (ptr == 0)");
            std::cout << std::format("{}\n\n", r.status);
            results.push_back(r);
            continue;
        }

        std::string output{
            reinterpret_cast<char*>(memory.data(store).data() + out_ptr),
            static_cast<std::string::size_type>(out_len)
        };

        // 5) освобождаем оба буфера
        (void)free_fn->call(store, {in_ptr});
        (void)free_fn->call(store, {out_ptr});

        // 6) shutdown
        (void)shutdown_fn->call(store, {});

        r.ok = true;
        r.status = "OK";
        r.output = output;
        std::cout << std::format("  Calling plugin_process() took {:.3f} ms\n", r.call_ms);
        std::cout << std::format("  Input: '{}'\n", TEST_INPUT);
        std::cout << std::format("  Output: '{}'\n", output);

        results.push_back(r);
    }

    // Итоговая сводка
    std::cout << "===== CONCLUSION =====\n";
    std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n", "Plugin", "Status", "Compilation(ms)", "Call(ms)");
    for (const auto& r : results) {
        std::cout << std::format("|{:20}|{:8}|{:15}|{:8}|\n",
            r.name,
            (r.ok ? "OK" : "FAIL"),
            r.compile_ms,
            r.call_ms);
    }

    std::cout << "\nError details:\n";
    int ok_count{};
    for (const auto& r : results) {
        if (r.ok) ok_count++;
        else std::cout << std::format("  {}: {}\n", r.name, r.status);
    }
    std::cout << std::format("\nProcessed {} out of {}\n", ok_count, results.size());

    return 0;
}
