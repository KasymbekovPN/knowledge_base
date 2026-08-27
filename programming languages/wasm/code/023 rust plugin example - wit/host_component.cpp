// Хост поверх РЕАЛЬНОГО Component Model API Wasmtime (wasmtime/component/*.hh),
// а не Core-модуля через Module/Instance, как во всех предыдущих днях.
// Вызывает init -> info -> process -> process(без init была бы ошибка,
// поэтому здесь порядок правильный) -> shutdown на одном и том же
// инстансе -- то, что через `wasmtime run --invoke` сделать нельзя,
// т.к. каждый его вызов -- это новый инстанс.

#include <fstream>
#include <iostream>
#include <format>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>

namespace {

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    wasmtime::component::ExportIndex must_get_index(wasmtime::component::Instance& instance,
                                                    wasmtime::Store::Context& ctx,
                                                    const wasmtime::component::ExportIndex* parent,
                                                    const char* name) {
        const auto idx{instance.get_export_index(ctx, parent, name)};
        if (!idx) {
            std::cerr << std::format("Export '{}' not found\n", name);
            std::exit(1);
        }

        return *idx;
    }

    wasmtime::component::Func must_get_func(wasmtime::component::Instance& instance,
                                            wasmtime::Store::Context& ctx,
                                            const wasmtime::component::ExportIndex& idx) {
        const auto f{instance.get_func(ctx, idx)};
        if (!f) {
            std::cerr << "Export found, but it is not function\n";
            std::exit(1);
        }
        return *f;
    }

    template<typename T>
    bool check_bad_result(const wasmtime::Result<T>& result, const std::string& label) {
        if (result) return false;
        std::cerr << std::format("{}: {}\n", label, result.err().message());

        return true;
    }

}

int main(const int argc, char *argv[]) {
    wasmtime::Engine engine;
    const char* path{argc > 1 ? argv[1] : "plugin_component_rust.wasm"};
    std::cout << std::format("Loading: {}\n", path);
    auto bytes{read_file(path)};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Component compilation error: {}\n", component_result.err().message());
        return 1;
    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::component::Linker linker{engine};;
    (void)linker.add_wasip2();
    // На случай если модуль всё же попросит что-то ещё не покрытое --
    // не падаем при инстанцировании, падаем только если реально позовут.
    (void)linker.define_unknown_imports_as_traps(component);

    auto ctx{store.context()};
    auto instance_result{linker.instantiate(ctx, component)};
    if (check_bad_result(instance_result, "Instantiation error")) return 1;

    wasmtime::component::Instance instance{instance_result.unwrap()};

    // Сначала находим индекс интерфейса-экспорта "example:plugin/transform@0.1.0",
    // затем внутри него -- индексы отдельных функций.
    wasmtime::component::ExportIndex transform_idx{must_get_index(
        instance,
        ctx,
        nullptr,
        "example:plugin/transform@0.1.0")};

    const auto export_func = [&instance, &ctx, &transform_idx](const char* name) {
        return must_get_func( instance, ctx, must_get_index(
            instance,
            ctx,
            &transform_idx,
            name));
    };
    wasmtime::component::Func init_fn{export_func("init")};
    wasmtime::component::Func info_fn{export_func("info")};
    wasmtime::component::Func process_fn{export_func("process")};
    wasmtime::component::Func shutdown_fn{export_func("shutdown")};

    // 1) init() -- без аргументов, без результата.
    std::vector<wasmtime::component::Val> no_args, no_results;
    if (auto init_result{init_fn.call(ctx, no_args, no_results)};
        check_bad_result(init_result, "init() failed")) return 1;
    std::cout << "init() executed\n";

    // 2) info() -> record { name: string, abi-version: u32 }
    std::vector<wasmtime::component::Val> info_results{wasmtime::component::Val{false}};
    std::vector<wasmtime::component::Val> info_args{};
    if (auto info_result{info_fn.call(ctx, info_args, info_results)};
        check_bad_result(info_result, "info() failed")) return 1;

    const wasmtime::component::Record& record{info_results[0].get_record()};
    std::string delimiter;
    std::cout << "info() {";
    for (const wasmtime::component::RecordField& field: record) {
        std::cout << std::format("{}{}=", delimiter, field.name());
        delimiter = ", ";
        if (const auto& value{field.value()}; value.is_string()) {
            std::cout << std::format("'{}'", value.get_string());
        } else if (value.is_u32()) {
            std::cout << value.get_u32();
        }
    }
    std::cout << "}\n";

    // 3) process(input: string) -> result<string, process-error>
    const auto call_process = [&](const std::string& text) {
        std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(text)};
        std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};
        if (const auto result{process_fn.call(ctx, args, results)};
            check_bad_result(result, "  TRAP")) return;

        if (const wasmtime::component::WitResult& wr{results[0].get_result()};
            wr.is_ok()) {
            std::cout << std::format("  Ok('{}')\n", wr.payload()->get_string());
        } else {
            std::cout << std::format("  Err({})\n", wr.payload()->get_variant().discriminant());
        }
    };

    std::cout << "process(\"hello wit component\"):\n";
    call_process("hello wit component");

    // 4) shutdown()
    std::vector<wasmtime::component::Val> shutdown_args, shutdown_results;
    if (const auto shutdown_result{shutdown_fn.call(ctx, shutdown_args, shutdown_results)};
        check_bad_result(shutdown_result, "shutdown() failed")) return 1;
    std::cout << "shutdown() executed\n";

    // 5) Вызов после shutdown -- у нас plugin_shutdown() просто сбрасывает
    // g_initialized, так что process() после него снова должен вернуть
    // Err(not-initialized) -- проверим, что состояние действительно общее
    // на весь инстанс (тот самый плюс к ручному ABI: здесь это staticная
    // переменная в WASM-памяти инстанса, а не что-то, что хост обязан
    // сериализовать сам).
    std::cout << "process(\"after shutdown\"):\n";
    call_process("after shutdown");

    return 0;
}
