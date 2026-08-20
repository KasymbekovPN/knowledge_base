// Проверяем гипотезу: limiter реально проверяется только когда РОСТ
// памяти инициирует сам WASM-код (инструкция memory.grow внутри гостя),
// а не когда хост дёргает Memory::grow() напрямую со своей стороны.

#include <iostream>
#include <format>
#include <vector>

#include <wasmtime.hh>

#include "tools.h"

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto wasm_bytes{read::wasm_file("grow_test.wasm")};
    const wasmtime::Module module{wasmtime::Module::compile(engine, wasm_bytes).unwrap()};

    wasmtime::Store store{engine};
    // Модуль объявляет initial=2 страницы. Лимит ставим ровно в эти же
    // 2 страницы (131072 байта) -- инстанцирование должно пройти
    // (начальный размер не ограничивается ретроактивно), а вот попытка
    // САМОГО ГОСТЯ вырасти ещё на 1 страницу поверх лимита -- должна упасть.
    store.limiter(131072, -1, -1, -1, -1);

    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};
    const auto grow_from_guest{std::get<wasmtime::Func>(*instance.get(store, "grow_from_guest"))};
    if (auto result{grow_from_guest.call(store, {})}) {
        const int32_t prev_pages{result.unwrap()[0].i32()};
        std::cout << std::format("grow_from_guest() returned {}\n", prev_pages);
    } else {
        std::cout << std::format("grow_from_guest() -- trap: {}\n", result.err().message());
    }

    return 0;
}
