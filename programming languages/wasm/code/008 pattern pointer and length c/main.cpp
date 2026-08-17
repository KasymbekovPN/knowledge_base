/*

C++ хост: копирует строку в память WASM-плагина, вызывает функцию
to_upper прямо там же (in-place), читает результат обратно.
Это тот же сценарий, что и host_str.py, но уже на "боевом" API,
который и пойдёт в реальный C++ хост плагинной системы.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=to_upper -Wl,--export=malloc -Wl,--export=free -o plugin_str.wasm plugin_str.c

*/

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

#include <wasmtime.hh>

//<
// using namespace wasmtime;

namespace {
    // Читаем скомпилированный .wasm файл как сырые байты
    std::vector<uint8_t> read_wasm_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>());
    }
}

int main(int argc, char *argv[]) {
    // 1) Engine создаёт компилятор/рантайм, Store -- изолированный "мир"
    //    для одного набора инстансов (у настоящего хоста обычно свой Store
    //    на каждый вызов плагина или на каждую сессию).
    wasmtime::Engine engine;
    auto wasmBytes{read_wasm_file("plugin_str.wasm")};
    wasmtime::Module module{wasmtime::Module::compile(engine, wasmBytes).unwrap()};

    wasmtime::Store store{engine};
    // Модуль ничего не импортирует (мы сами в этом убедились через
    // wasm-objdump -x -- секции Import нет), поэтому список импортов пуст.
    wasmtime::Instance instance{wasmtime::Instance::create(store, module, {}).unwrap()};

    // 2) Достаём нужные экспорты плагина по имени.
    auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};
    auto malloc_fn{std::get<wasmtime::Func>(*instance.get(store, "malloc"))};
    auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "free"))};
    auto to_upper_fn{std::get<wasmtime::Func>(*instance.get(store, "to_upper"))};

    const std::string text{"hello from C++ host!"};
    std::string::size_type length{static_cast<std::string::size_type>(text.size())};

    // 3) Просим ГОСТЯ выделить буфер -- хост не выбирает смещение сам.
    int32_t guest_ptr{malloc_fn.call(store, {static_cast<int32_t>(length)}).unwrap()[0].i32()};
    std::cout << std::format("The guest allocated a buffer at its own offset: {}\n", guest_ptr);

    // 4) Копируем байты строки в linear memory гостя по этому смещению.
    //    memory.data(store) -- это Span<uint8_t>, указывающий на реальный
    //    буфер в АДРЕСНОМ ПРОСТРАНСТВЕ ХОСТА, где рантайм хранит память
    //    гостя. guestPtr -- это просто индекс внутри этого span.
    auto data{memory.data(store)};
    std::memcpy(data.data() + guest_ptr, text.data(), length);

    // 5) Вызываем функцию плагина с классической парой (указатель, длина).
    to_upper_fn.call(store, {guest_ptr, static_cast<int32_t>(length)}).unwrap();

    // 6) Читаем результат обратно из той же области памяти гостя.
    //    ВАЖНО: data() нужно получить заново -- если бы между шагами вызывался
    //    memory.grow (или WASM-код внутри сам рос по памяти), старый span
    //    мог инвалидироваться, указывая на уже неактуальный буфер.
    auto result_data{memory.data(store)};
    std::string result{reinterpret_cast<char*>(result_data.data() + guest_ptr), length};
    std::cout << std::format("Result after plugin calling: {}\n", result);

    // 7) Освобождаем буфер в госте -- как и в Python-версии, free принимает
    //    только указатель (обычный C ABI free(void*)).
    free_fn.call(store, {guest_ptr}).unwrap();
    std::cout << "Buffer free by guest (free-method called form host) !!!\n";

    return 0;
}

