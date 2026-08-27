// Живая проверка [C++ host] <-> [transform_proxy.so, Rust] <-> [C-плагин
// example:plugin/transform, Component Model]. Этот .cpp не включает ни
// одного заголовка wasmtime -- только transform_proxy.h.


#include "include/transform_proxy.h"

#include <iostream>
#include <format>
#include <string>

namespace {
    // Небольшая C++-обёртка вокруг сырых (ptr,len)-ручек -- то, что в
    // реальном проекте вынесли бы в отдельный класс Plugin, здесь для
    // наглядности прямо в main().
    bool call_process(ProxyComponent* component,
                      const std::string& input,
                      std::string& out,
                      std::string& err) {
        uint8_t* out_ptr{nullptr};
        size_t out_len{0};
        char err_buf[256] = {0};

        const int32_t status{proxy_process(
            component,
            reinterpret_cast<const uint8_t*>(input.data()),
            input.size(),
            &out_ptr,
            &out_len,
            err_buf,
            sizeof(err_buf))};

        if (status == 0) {
            out.assign(reinterpret_cast<char*>(out_ptr), out_len);
            // обязательно -- не free()/delete[]
            proxy_free_string(out_ptr, out_len);
            return true;
        }

        err = err_buf;
        return false;
    }
}

int main(int argc, char *argv[]) {
    std::cout << std::format("proxy ABI version: {}\n", proxy_abi_version());

    ProxyEngine* engine{proxy_engine_new()};
    if (!engine) {
        std::cerr << std::format("proxy_engine_new() failed\n");
        return 1;
    }

    char err[256] = {0};
    ProxyComponent* comp{
        proxy_load(engine, "plugin_component.wasm", err, sizeof(err))
    };
    if (!comp) {
        std::cerr << std::format("proxy_load() failed: {}\n", err);
        proxy_engine_free(engine);
        return 1;
    }

    // process() до init() -- плагин обязан вернуть Err(not-initialized),
    // ровно как в host_component.cc из Дня 9.
    std::string out, call_err;
    if (call_process(comp, "too early", out, call_err)) {
        std::cerr << std::format("unexpected: process() until init() OK('{}')\n", out);
    } else {
        std::cerr << std::format("process() until init(): error (expected) -- {}\n", call_err);
    }

    int32_t status{proxy_init(comp, err, sizeof(err))};
    if (status != 0) {
        std::cerr << std::format("proxy_init() failed: ({}): {}\n", status, err);
        proxy_free(comp);
        proxy_engine_free(engine);
        return 1;
    }
    std::cout << "init() executed\n";

    char name_buf[128] = {0};
    uint32_t abi_version = 0;
    status = proxy_info(comp, name_buf, sizeof(name_buf), &abi_version, err, sizeof(err));
    if (status == 0) {
        std::cerr << std::format("info(): name='{}', abi_version={}\n", name_buf, abi_version);
    }

    if (const std::string line0{"hello from cpp host"};
        call_process(comp, line0, out, call_err)) {
        std::cout << std::format("process('{}') -> Ok('{}')\n", line0, out);
    } else {
        std::cerr << std::format("process() failed: {}\n", call_err);
    }

    if (const std::string line1;
        call_process(comp, line1, out, call_err)) {
        std::cerr << std::format("unexpected: process('{}') -> Ok('{}')\n", line1, out);
    } else {
        std::cout << std::format("process('{}') -> '{}'\n", line1, call_err);
    }

    status = proxy_shutdown(comp, err, sizeof(err));
    std::cout << std::format("shutdown(): code {}\n", status);

    if (const std::string line2{"after shutdown"};
        call_process(comp, line2, out, call_err)) {
        std::cerr << std::format("unexpected: process() after shutdown() -> Ok('{}')", out);
    } else {
        std::cout << std::format("process() after shutdown() -> {}\n", call_err);
    }

    proxy_free(comp);
    proxy_engine_free(engine);

    return 0;
}
