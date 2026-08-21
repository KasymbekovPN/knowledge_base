/*

Хост поверх РЕАЛЬНОГО Component Model API Wasmtime (wasmtime/component/*.hh),
а не Core-модуля через Module/Instance, как во всех предыдущих днях.
Вызывает init -> info -> process -> process(без init была бы ошибка,
поэтому здесь порядок правильный) -> shutdown на одном и том же
инстансе -- то, что через `wasmtime run --invoke` сделать нельзя,
т.к. каждый его вызов -- это новый инстанс.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -I. -o plugin_core.wasm generated/plugin.c impl.c generated/plugin_component_type.o

wasm-tools component new plugin_core.wasm -o plugin_component.wasm

*/

#include <fstream>
#include <iostream>
#include <format>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>

namespace {

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file(name, std::ios::binary);
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    wasmtime::component::ExportIndex must_get_index(const wasmtime::component::Instance& instance,
                                                    const wasmtime::Store::Context cx,
                                                    const wasmtime::component::ExportIndex* parent,
                                                    const char* name) {
        const auto idx{instance.get_export_index(cx, parent, name)};
        if (!idx) {
            std::cerr << std::format("Export not found: {}\n", name);
            std::exit(1);
        }

        return *idx;
    }

    wasmtime::component::Func must_get_func(const wasmtime::component::Instance& instance,
                                            const wasmtime::Store::Context cx,
                                            const wasmtime::component::ExportIndex& idx) {
        const auto f{instance.get_func(cx, idx)};
        if (!f) {
            std::cerr << "Export found, but it is not function\n";
            std::exit(1);
        }

        return *f;
    }

}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto bytes{read_file("plugin_component.wasm")};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Compilation error of component: {}\n",
            component_result.err().message());
        return 1;

    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::component::Linker linker{engine};
    // У нашего компонента нет импортов вообще (не подключали WASI/wasip2),
    // но define_unknown_imports_as_traps на всякий случай не помешает.
    (void)linker.define_unknown_imports_as_traps(component);

    auto instance_result{linker.instantiate(store.context(), component)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n",
            instance_result.err().message());
        return 1;
    }

    wasmtime::component::Instance instance{instance_result.unwrap()};

    const wasmtime::Store::Context ctx = store.context();

    // Сначала находим индекс интерфейса-экспорта "example:plugin/transform@0.1.0",
    // затем внутри него -- индексы отдельных функций.
    const wasmtime::component::ExportIndex transform_idx{must_get_index(
        instance,
        ctx,
        nullptr,
        "example:plugin/transform@0.1.0")};

    wasmtime::component::Func init_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "init"))};
    wasmtime::component::Func info_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "info"))};
    wasmtime::component::Func process_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "process"))};
    wasmtime::component::Func shutdown_fn{must_get_func(
        instance,
        ctx,
        must_get_index(instance, ctx, &transform_idx, "shutdown"))};

    // 1) init() -- без аргументов, без результата.
    std::vector<wasmtime::component::Val> no_args, no_returns;
    if (auto init_res{init_fn.call(ctx, no_args, no_returns)}; !init_res) {
        std::cerr << std::format("init() failed: {}\n", init_res.err().message());
        return 1;
    }
    std::cout << "init() done\n";

    // 2) info() -> record { name: string, abi-version: u32 }
    // плейсхолдер, перезапишется вызовом
    std::vector<wasmtime::component::Val> info_results{wasmtime::component::Val{false}};
    std::vector<wasmtime::component::Val> info_args;
    if (auto info_res{info_fn.call(ctx, info_args, info_results)}; !info_res) {
        std::cerr << std::format("info() failed: {}\n", info_res.err().message());
        return 1;
    }

    const wasmtime::component::Record& record{info_results[0].get_record()};
    std::cout << "info(): {";
    std::string delimiter;
    for (const auto& field: record) {
        std::cout << std::format("{}{}=", delimiter, field.name());
        delimiter = ", ";
        if (const auto& value{field.value()};
            value.is_string()) {
            std::cout << std::format("\"{}\"", value.get_string());
        } else if (value.is_u32()) {
            std::cout << value.get_u32();
        }
    }
    std::cout << "}\n";

    // 3) process(input: string) -> result<string, process-error>
    auto call_process{[&](const std::string& text) {
        std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(text)};
        std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};
        if (auto result{process_fn.call(ctx, args, results)};
            !result) {
            std::cerr << std::format("  TRAP: {}\n", result.err().message());
            return;
        }

        if (const wasmtime::component::WitResult& wr{results[0].get_result()};
            wr.is_ok()) {
            std::cout << std::format("  Ok(\"{}\")", wr.payload()->get_string());
        } else {
            const wasmtime::component::Variant& err_variant{wr.payload()->get_variant()};
            std::cout << std::format("  Err({})\n", err_variant.discriminant());
        }
    }};
    const std::string line0{"hello wit component"};
    std::cout << std::format("process(\"{}\")", line0);
    call_process(line0);


    // 4) shutdown()
    std::vector<wasmtime::component::Val> shutdown_args, shutdown_results;
    if (auto shutdown_res{shutdown_fn.call(ctx, shutdown_args, shutdown_results)}; !shutdown_res) {
        std::cerr << std::format("shutdown() failed: {}\n", shutdown_res.err().message());
        return 1;
    }
    std::cout << "shutdown() done\n";

    // 5) Вызов после shutdown -- у нас plugin_shutdown() просто сбрасывает
    // g_initialized, так что process() после него снова должен вернуть
    // Err(not-initialized) -- проверим, что состояние действительно общее
    // на весь инстанс (тот самый плюс к ручному ABI: здесь это staticная
    // переменная в WASM-памяти инстанса, а не что-то, что хост обязан
    // сериализовать сам).
    const std::string line1{"after shutdown"};
    std::cout << std::format("process(\"{}\")", line1);
    call_process(line1);

    return 0;
}
