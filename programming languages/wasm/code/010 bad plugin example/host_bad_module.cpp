// Хост, который намеренно скармливает себе "плохой" плагин с
// бесконечным циклом -- и не падает благодаря топливному лимиту (fuel)
// Wasmtime. Это последний пункт мини-проекта: доказать, что WASM даёт то,
// чего dlopen() дать не может -- крашнутый/зависший плагин не роняет и
// не подвешивает сам хост-процесс.

#include <iostream>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "wasmtime.hh"

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
    // 1) Включаем расход топлива на уровне Config -- без этого set_fuel
    //    ниже просто вернёт ошибку "fuel is not configured".
    wasmtime::Config config;
    config.consume_fuel(true);
    wasmtime::Engine engine{std::move(config)};

    auto wasmBytes{read_wasm_file("bad_plugin.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasmBytes).unwrap()};

    wasmtime::Store store{engine};
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    const auto infinite_loop_fn{std::get<wasmtime::Func>(*instance.get(store, "infinite_loop"))};
    const auto ping_fn{std::get<wasmtime::Func>(*instance.get(store, "ping"))};

    // 2) Даём этому конкретному вызову ограниченный бюджет "топлива" --
    //    условных единиц выполнения WASM-инструкций. Без явного set_fuel
    //    выполнение вообще не началось бы (0 топлива по умолчанию).
    constexpr uint64_t FUEL_BUDGET{10'000'000};
    store.context().set_fuel(FUEL_BUDGET).unwrap();
    std::cout << std::format("Call infinite_loop with budget: {}\n", FUEL_BUDGET);

    if (const auto result{infinite_loop_fn.call(store, {})}) {
        // Сюда мы попасть не должны -- бесконечный цикл не может завершиться
        // сам по себе.
        std::cout << "Unexpected: calling is finished successfully!\n";
    } else {
        // А вот сюда -- обязаны. Wasmtime оборвал выполнение trap'ом, как
        // только топливо кончилось, и вернул управление хосту как обычную
        // ошибку, а не убил процесс.
        std::cout << std::format("Plugin is stopped by runtime: '{}'\n",
            result.err().message());
    }

    std::cout << "Host is alve\n";

    // 3) Тот же Store, тот же Instance -- просто пополняем топливо и
    //    вызываем СОСЕДНЮЮ функцию из ТОГО ЖЕ модуля. Если бы плагин
    //    реально уронил процесс, до этой строчки мы бы не дошли вообще.
    store.context().set_fuel(FUEL_BUDGET).unwrap();
    auto ping_result{ping_fn.call(store, {})};
    std::cout << std::format("Ping result: {}\n",
        ping_result.unwrap()[0].i32());

    return 0;
}
