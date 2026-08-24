// Хост для Варианта 1 -- гость (plugin_ping_import.wasm) сам сети не
// касается, только зовёт импортированную host_tcp_ping. Реальная сеть
// -- здесь, на хосте, через настоящий boost::asio (это обычный нативный
// C++, boost::asio компилируется тут без всяких оговорок -- проблема
// была именно с ЕГО КОМПИЛЯЦИЕЙ ПОД WASI ГОСТЯ, не с хостом).

#include <boost/asio.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <vector>

#include <wasmtime.hh>

namespace {

    const std::string TARGET{"example.com"};

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    // Настоящий асинхронный TCP-коннект через boost::asio: резолвинг и
    // коннект гоняются как async-операции на одном io_context, а
    // steady_timer используется как асинхронный таймаут -- гонка двух
    // async-операций на одном event loop'е, ровно то, ради чего вообще
    // берут asio, а не blocking connect().
    double asio_tcp_ping(const std::string& host, int port) {
        boost::asio::io_context io;
        boost::asio::ip::tcp::resolver resolver{io};
        boost::asio::ip::tcp::socket socket{io};
        boost::asio::steady_timer timer{io};

        bool done{false};
        bool ok{false};
        double elapsed_ms{-1.0};
        auto t0{std::chrono::steady_clock::now()};

        timer.expires_after(std::chrono::seconds(5));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (ec || done) return;
            done = true;
            boost::system::error_code ignored;
            socket.close(ignored);
        });

        resolver.async_resolve(
            host,
            std::to_string(port),
            [&](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
                if (ec || done) {
                    done = true;
                    timer.cancel();
                    return;
                }

                boost::asio::async_connect(
                    socket,
                    results,
                    [&](const boost::system::error_code& ec2, const boost::asio::ip::tcp::endpoint&) {
                        if (done) return;
                        done = true;
                        timer.cancel();
                        if (ec2) return;

                        ok = true;
                        elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
                    });

            });

        io.run();
        return ok ? elapsed_ms : -1.0;
    }

}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto bytes{read_file("plugin_ping_import.wasm")};

    auto module_result{wasmtime::Module::compile(engine, bytes)};
    if (!module_result) {
        std::cerr << std::format("Module::compile error: {}\n", module_result.err().message());
        return 1;
    }
    wasmtime::Module module{module_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::WasiConfig wasi_config;
    wasi_config.inherit_stdout();
    wasi_config.inherit_stderr();
    auto wasi_result{store.context().set_wasi(std::move(wasi_config))};
    if (!wasi_result) {
        std::cerr << std::format("set_wasi error: {}\n", wasi_result.err().message());
        return 1;
    }

    wasmtime::Linker linker{engine};
    auto define_wasi_result{linker.define_wasi()};
    if (!define_wasi_result) {
        std::cerr << std::format("define_wasi error: {}\n", define_wasi_result.err().message());
        return 1;
    }

    // Регистрируем host_tcp_ping как обычную импортированную функцию --
    // тот же паттерн, что host_log в Дне 5, просто внутри лямбды теперь
    // настоящий сетевой ввод-вывод вместо std::cout.
    wasmtime::Func host_tcp_ping{wasmtime::Func::wrap(
        store,
        [](wasmtime::Caller caller, int32_t ptr, int32_t len, int32_t port) -> double {
            const auto memory{std::get<wasmtime::Memory>(*caller.get_export("memory"))};
            auto data{memory.data(caller.context())};
            std::string host{reinterpret_cast<char*>(data.data() + ptr), static_cast<std::string::size_type>(len)};
            std::cout << std::format("  [host] boost::asio resolve and connect to {}:{} ...\n",
                host, port);
            return asio_tcp_ping(host, port);
        })};
    auto define_result{linker.define(store, "env", "host_tcp_ping", host_tcp_ping)};
    if (!define_result) {
        std::cerr << std::format("linker.define error: {}\n", define_result.err().message());
        return 1;
    }

    auto instance_result{linker.instantiate(store, module)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n", instance_result.err().message());
        return 1;
    }
    wasmtime::Instance instance{instance_result.unwrap()};

    const auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};
    const auto init_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_init"))};
    const auto alloc_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_alloc"))};
    const auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_free"))};
    const auto progress_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_process"))};
    const auto shutdown_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_shutdown"))};

    (void)init_fn.call(store, {});

    const int32_t in_ptr{alloc_fn.call(store, {(int32_t)TARGET.size()}).unwrap()[0].i32()};
    std::memcpy(memory.data(store).data() + in_ptr, TARGET.data(), TARGET.size());

    auto res{progress_fn.call(store, {in_ptr, (int32_t)TARGET.size()})};
    if (!res) {
        std::cerr << std::format("plugin_process() -- TRAP: {}\n", res.err().message());
        return 1;
    }

    uint64_t packed{static_cast<uint64_t>(res.unwrap()[0].i64())};
    int32_t out_ptr{static_cast<int32_t>(packed >> 32)};
    int32_t out_len{static_cast<int32_t>(packed & 0xFFFFFFFFu)};
    std::string result{
        reinterpret_cast<char*>(memory.data(store).data() + out_ptr),
        static_cast<std::string::size_type>(out_len)};
    std::cout << std::format("Result of plugin_process(): {}\n", result);

    (void)free_fn.call(store, {out_ptr});
    (void)shutdown_fn.call(store, {});

    return 0;
}
