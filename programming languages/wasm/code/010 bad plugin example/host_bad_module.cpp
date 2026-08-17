// // Хост, который намеренно скармливает себе "плохой" плагин с
// // бесконечным циклом -- и не падает благодаря топливному лимиту (fuel)
// // Wasmtime. Это последний пункт мини-проекта: доказать, что WASM даёт то,
// // чего dlopen() дать не может -- крашнутый/зависший плагин не роняет и
// // не подвешивает сам хост-процесс.
//
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <vector>
// #include <wasmtime.hh>
//
// using namespace wasmtime;
//
// std::vector<uint8_t> readWasmFile(const char *name) {
//   std::ifstream file(name, std::ios::binary);
//   return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
//                                std::istreambuf_iterator<char>());
// }
//
// int main() {
//   // 1) Включаем расход топлива на уровне Config -- без этого set_fuel
//   //    ниже просто вернёт ошибку "fuel is not configured".
//   Config config;
//   config.consume_fuel(true);
//   Engine engine(std::move(config));
//
//   auto wasmBytes = readWasmFile("bad_plugin.wasm");
//   Module module = Module::compile(engine, wasmBytes).unwrap();
//
//   Store store(engine);
//   Instance instance = Instance::create(store, module, {}).unwrap();
//
//   auto infiniteLoop = std::get<Func>(*instance.get(store, "infinite_loop"));
//   auto ping = std::get<Func>(*instance.get(store, "ping"));
//
//   // 2) Даём этому конкретному вызову ограниченный бюджет "топлива" --
//   //    условных единиц выполнения WASM-инструкций. Без явного set_fuel
//   //    выполнение вообще не началось бы (0 топлива по умолчанию).
//   const uint64_t FUEL_BUDGET = 10'000'000;
//   store.context().set_fuel(FUEL_BUDGET).unwrap();
//
//   std::cout << "Вызываю infinite_loop() с бюджетом " << FUEL_BUDGET
//             << " единиц топлива...\n";
//
//   auto result = infiniteLoop.call(store, {});
//
//   if (result) {
//     // Сюда мы попасть не должны -- бесконечный цикл не может завершиться
//     // сам по себе.
//     std::cout << "Неожиданно: вызов завершился успешно.\n";
//   } else {
//     // А вот сюда -- обязаны. Wasmtime оборвал выполнение trap'ом, как
//     // только топливо кончилось, и вернул управление хосту как обычную
//     // ошибку, а не убил процесс.
//     std::cout << "Плагин остановлен рантаймом: " << result.err().message()
//               << "\n";
//   }
//
//   std::cout << "\n--- Хост всё ещё жив после этого. Доказываю: ---\n";
//
//   // 3) Тот же Store, тот же Instance -- просто пополняем топливо и
//   //    вызываем СОСЕДНЮЮ функцию из ТОГО ЖЕ модуля. Если бы плагин
//   //    реально уронил процесс, до этой строчки мы бы не дошли вообще.
//   store.context().set_fuel(FUEL_BUDGET).unwrap();
//   auto pingResult = ping.call(store, {});
//   std::cout << "Повторный вызов ping() в том же Store: "
//             << pingResult.unwrap()[0].i32() << " (ожидали 42)\n";
//
//   return 0;
// }