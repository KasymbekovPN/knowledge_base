
/*

Хост для Варианта 2 -- Component Model, гость сам делает сеть через
wasi:sockets. Единственное отличие от host_component.cc из Дня 9 --
WasiConfig теперь явно разрешает сеть (inherit_network +
allow_ip_name_lookup), без этого guest получил бы permission-denied
уже на этапе резолвинга адреса.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip2 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -I. -o plugin_core.wasm generated/netcheck_plugin.c impl.c generated/netcheck_plugin_component_type.o

под wasip2 clang уже сразу выдаёт готовый компонент
cp plugin_core.wasm plugin_component.wasm

*/

#include <iostream>
#include <format>
#include <fstream>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>
#include <wasmtime/wasi.hh>

namespace {
    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }
}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    const char* PATH{argc > 1 ? argv[1] : "plugin_component.wasm"};
    auto bytes{read_file(PATH)};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Compilation error: {}\n", component_result.err().message());
        return 1;
    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::WasiConfig wasi;
    wasi.inherit_stdout();
    wasi.inherit_stderr();
    wasi.inherit_env();
    // Без этих двух строк -- инстанцирование пройдёт (импорты
    // wasi:sockets всё равно резолвятся), но реальный tcp-ping вернёт
    // permission-denied: сеть в WASI -- capability, которую хост обязан
    // выдать явно, а не то, что гость получает по умолчанию.
    wasi.inherit_network();
    wasi.allow_ip_name_lookup(true);
    auto ctx{store.context()};
    ctx.set_wasi(std::move(wasi)).unwrap();

    wasmtime::component::Linker linker{engine};
    (void)linker.add_wasip2();
    (void)linker.define_unknown_imports_as_traps(component);

    auto instance_result{linker.instantiate(ctx, component)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n", instance_result.err().message());
        return 1;
    }
    wasmtime::component::Instance instance{instance_result.unwrap()};

    auto check_idx{instance.get_export_index(
        ctx,
        nullptr,
        "example:netcheck/check@0.1.0")};

    if (!check_idx) {
        std::cerr << "Interface 'example:netcheck/check@0.1.0' not found";
        return 1;
    }

    auto tcp_ping_idx{instance.get_export_index(
        ctx,
        &*check_idx,
        "tcp-ping")};
    if (!tcp_ping_idx) {
        std::cerr << "function tcp-ping not found";
        return 1;
    }
    wasmtime::component::Func tcp_ping_fn{*instance.get_func(ctx, *tcp_ping_idx)};

    std::string host{"example.com"};
    uint16_t port{80};
    std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(host), wasmtime::component::Val{port}};
    std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};

    if (auto res{tcp_ping_fn.call(ctx, args, results)}; !res) {
        std::cerr << std::format("TRAP: {}\n", res.err().message());
        return 1;
    }

    if (const wasmtime::component::WitResult& wr{results[0].get_result()}; wr.is_ok()) {
        std::cout << std::format("OK: TCP-connection to {}:{} took {} ms\n",
            host,
            port,
            wr.payload()->get_f64());
    } else {
        const wasmtime::component::Variant& errVariant{wr.payload()->get_variant()};
        const auto value{errVariant.value()};
        std::cout << std::format("FAIL ('{}'): ", errVariant.discriminant());
        if (value && value->is_string()) {
            std::cout << value->get_string();
        }
        std::cout << "\n";
    }

    return 0;
}
