// Тот же bad_plugin.wasm, что и в host_bad_module.cc (fuel), но теперь
// прерывание по ЭПОХАМ -- принципиально другой механизм: не count
// инструкций, а тики реального времени, приходящие из ФОНОВОГО потока.

#include <iostream>
#include <format>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <wasmtime.hh>

#include "tools.h"

int main(int argc, char *argv[]) {
    // 1) epoch_interruption вместо consume_fuel -- другой флаг Config,
    //    оба механизма можно включить и одновременно, но для наглядности
    //    держим их раздельно.
    wasmtime::Config config;
    config.epoch_interruption(true);
    wasmtime::Engine engine{std::move(config)};

    auto wasm_bytes{read::wasm_file("bad_plugin.wasm")};
    wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};

    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    const auto infinite_loop_fn{std::get<wasmtime::Func>(*instance.get(store, "infinite_loop"))};
    const auto ping{std::get<wasmtime::Func>(*instance.get(store, "ping"))};

    // 2) По умолчанию дедлайн -- ТЕКУЩАЯ эпоха движка, то есть выполнение
    //    прервалось бы немедленно. set_epoch_deadline(50) говорит: разреши
    //    ещё 50 "тиков" вперёд, прежде чем прерывать.
    store.context().set_epoch_deadline(50);

    // 3) Тики создаёт ОТДЕЛЬНЫЙ поток, по таймеру -- реальное время, а не
    //    подсчёт исполненных инструкций. Именно в этом ключевое отличие
    //    от fuel: fuel детерминирован по количеству работы, epoch -- по
    //    факту "прошло какое-то время снаружи", независимо от того, что
    //    именно WASM-код успел сделать за это время.
    std::atomic<bool> stop_ticket{false};
    std::thread ticker{[&]() {
        while (!stop_ticket.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            engine.increment_epoch();
        }
    }};

    const auto start{std::chrono::steady_clock::now()};
    std::cout << "Calling infinite_loop with epoch-deadline 50 ticks (1 tick ~10ms)\n";

    const auto result{infinite_loop_fn.call(store, {})};
    const auto elapsed_ms{std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count()};

    stop_ticket.store(true);
    ticker.join();

    if (!result) {
        std::cout << std::format("Stopped by epoch ~{} ms4 {}\n",
            elapsed_ms,
            result.err().message());
    } else {
        std::cout << "Unexpected!\n";
    }

    std::cout << "\n--- Host is lived ---\n";
    store.context().set_epoch_deadline(50);
    auto ping_result{ping.call(store, {})};

    std::cout << std::format("ping() => {}\n", ping_result.unwrap()[0].i32());

    return 0;
}
