// // День 8 -- мини-проект: хост сканирует папку plugins/, загружает каждый
// // .wasm как плагин по контракту из plugin_abi.h, единообразно защищает
// // КАЖДЫЙ вызов plugin_process топливным лимитом (fuel), изолирует
// // сломанный плагин (не падает и не останавливает обработку остальных),
// // и замеряет время компиляции модуля и время вызова функции.
// #include <algorithm>
// #include <chrono>
// #include <cstring>
// #include <filesystem>
// #include <fstream>
// #include <iomanip>
// #include <iostream>
// #include <string>
// #include <vector>
// #include <wasmtime.hh>
//
// using namespace wasmtime;
// namespace fs = std::filesystem;
//
// // Единый лимит топлива на весь жизненный цикл плагина (abi_version -> init
// // -> process -> shutdown). Специально не завышен: реальным плагинам из
// // этого проекта его с большим запасом хватает, а "сломанный" плагин с
// // while(1) исчерпает его практически мгновенно -- защита одинакова для
// // всех, никакого спецкейса под broken.wasm в коде хоста нет.
// constexpr uint64_t FUEL_BUDGET = 5'000'000;
//
// const std::string TEST_INPUT = "Hello WASM plugin world, this is a test";
//
// struct PluginResult {
//   std::string name;
//   bool ok = false;
//   std::string status;
//   double compileMs = 0.0;
//   double callMs = 0.0;
//   std::string output;
// };
//
// std::vector<uint8_t> readFile(const fs::path &p) {
//   std::ifstream file(p, std::ios::binary);
//   return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
//                                std::istreambuf_iterator<char>());
// }

int main(int argc, char *argv[]) {
    return 0;
}


// int main() {
//   fs::path pluginsDir = "plugins";
//   std::vector<fs::path> wasmFiles;
//   for (auto &entry : fs::directory_iterator(pluginsDir)) {
//     if (entry.path().extension() == ".wasm") {
//       wasmFiles.push_back(entry.path());
//     }
//   }
//   std::sort(wasmFiles.begin(), wasmFiles.end());
//
//   std::cout << "Найдено " << wasmFiles.size() << " .wasm файлов в '"
//             << pluginsDir.string() << "':\n";
//   for (auto &p : wasmFiles) std::cout << "  " << p.filename().string() << "\n";
//   std::cout << "\n";
//
//   Config config;
//   config.consume_fuel(true);
//   Engine engine(std::move(config));
//
//   std::vector<PluginResult> results;
//
//   for (auto &path : wasmFiles) {
//     PluginResult r;
//     r.name = path.filename().string();
//     std::cout << "==== " << r.name << " ====\n";
//
//     auto bytes = readFile(path);
//
//     auto t0 = std::chrono::steady_clock::now();
//     auto moduleResult = Module::compile(engine, bytes);
//     auto t1 = std::chrono::steady_clock::now();
//     r.compileMs =
//         std::chrono::duration<double, std::milli>(t1 - t0).count();
//
//     if (!moduleResult) {
//       r.status = "ОШИБКА КОМПИЛЯЦИИ: " + moduleResult.err().message();
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     Module module = moduleResult.unwrap();
//     std::cout << "  Модуль скомпилирован за " << std::fixed
//                << std::setprecision(3) << r.compileMs << " мс\n";
//
//     Store store(engine);
//     // Топливный лимит выставляется ОДИНАКОВО для каждого плагина, ДО
//     // того как хост знает, "хороший" он или "сломанный" -- это и есть
//     // защита по умолчанию, а не отдельная ветка для broken.wasm.
//     store.context().set_fuel(FUEL_BUDGET).unwrap();
//
//     auto instanceResult = Instance::create(store, module, {});
//     if (!instanceResult) {
//       r.status = "ОШИБКА ИНСТАНЦИРОВАНИЯ: " + instanceResult.err().message();
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     Instance instance = instanceResult.unwrap();
//
//     auto getExport = [&](const char *name) -> std::optional<Func> {
//       auto e = instance.get(store, name);
//       if (!e) return std::nullopt;
//       if (!std::holds_alternative<Func>(*e)) return std::nullopt;
//       return std::get<Func>(*e);
//     };
//
//     auto abiVersionFn = getExport("plugin_abi_version");
//     auto initFn = getExport("plugin_init");
//     auto allocFn = getExport("plugin_alloc");
//     auto freeFn = getExport("plugin_free");
//     auto processFn = getExport("plugin_process");
//     auto shutdownFn = getExport("plugin_shutdown");
//     auto memoryExport = instance.get(store, "memory");
//
//     if (!abiVersionFn || !initFn || !allocFn || !freeFn || !processFn ||
//         !shutdownFn || !memoryExport ||
//         !std::holds_alternative<Memory>(*memoryExport)) {
//       r.status = "ОШИБКА: плагин не реализует контракт полностью "
//                  "(отсутствует один из экспортов)";
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     Memory memory = std::get<Memory>(*memoryExport);
//
//     // 1) abi_version
//     auto abiRes = abiVersionFn->call(store, {});
//     if (!abiRes) {
//       r.status = "TRAP на plugin_abi_version: " + abiRes.err().message();
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     int32_t abiVersion = abiRes.unwrap()[0].i32();
//     if (abiVersion != 1) {
//       r.status = "НЕСОВМЕСТИМАЯ ВЕРСИЯ ABI: " + std::to_string(abiVersion);
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//
//     // 2) init
//     auto initRes = initFn->call(store, {});
//     if (!initRes) {
//       r.status = "TRAP на plugin_init: " + initRes.err().message();
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     if (initRes.unwrap()[0].i32() != 0) {
//       r.status = "plugin_init вернул код ошибки";
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//
//     // 3) записываем тестовый вход в память гостя
//     auto inAllocRes =
//         allocFn->call(store, {(int32_t)TEST_INPUT.size()});
//     if (!inAllocRes) {
//       r.status = "TRAP на plugin_alloc(вход): " + inAllocRes.err().message();
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//     int32_t inPtr = inAllocRes.unwrap()[0].i32();
//     std::memcpy(memory.data(store).data() + inPtr, TEST_INPUT.data(),
//                 TEST_INPUT.size());
//
//     // 4) вызов plugin_process -- под тем же топливным лимитом, что и
//     // весь остальной жизненный цикл. Именно здесь "сломанный" плагин
//     // споткнётся об исчерпание топлива.
//     auto t2 = std::chrono::steady_clock::now();
//     auto processRes = processFn->call(
//         store, {inPtr, (int32_t)TEST_INPUT.size()});
//     auto t3 = std::chrono::steady_clock::now();
//     r.callMs = std::chrono::duration<double, std::milli>(t3 - t2).count();
//
//     if (!processRes) {
//       // Именно эта ветка ловит "сломанный" плагин: хост не падает,
//       // печатает понятную ошибку и переходит к следующему плагину.
//       std::string fullMsg = processRes.err().message();
//       // Для сводки вытаскиваем содержательную строку "wasm trap: ..."
//       // (сама причина), а не первую строку backtrace'а; полный текст
//       // остаётся виден в подробном логе выше.
//       std::string shortMsg = fullMsg;
//       auto trapPos = fullMsg.find("wasm trap:");
//       if (trapPos != std::string::npos) {
//         std::string rest = fullMsg.substr(trapPos);
//         shortMsg = rest.substr(0, rest.find('\n'));
//       } else {
//         shortMsg = fullMsg.substr(0, fullMsg.find('\n'));
//       }
//       r.status = "TRAP на plugin_process (изолирован): " + shortMsg;
//       std::cout << "  plugin_process() провалился за " << std::fixed
//                  << std::setprecision(3) << r.callMs << " мс\n";
//       std::cout << "  " << fullMsg << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//
//     uint64_t packed = (uint64_t)processRes.unwrap()[0].i64();
//     int32_t outPtr = (int32_t)(uint32_t)(packed >> 32);
//     int32_t outLen = (int32_t)(uint32_t)(packed & 0xFFFFFFFFu);
//
//     if (outPtr == 0) {
//       r.status = "plugin_process сообщил об ошибке (ptr == 0)";
//       std::cout << r.status << "\n\n";
//       results.push_back(r);
//       continue;
//     }
//
//     std::string output(
//         reinterpret_cast<char *>(memory.data(store).data() + outPtr),
//         outLen);
//
//     // 5) освобождаем оба буфера
//     (void)freeFn->call(store, {inPtr});
//     (void)freeFn->call(store, {outPtr});
//
//     // 6) shutdown
//     (void)shutdownFn->call(store, {});
//
//     r.ok = true;
//     r.status = "OK";
//     r.output = output;
//     std::cout << "  Вызов plugin_process() занял " << std::fixed
//                << std::setprecision(3) << r.callMs << " мс\n";
//     std::cout << "  Вход:  \"" << TEST_INPUT << "\"\n";
//     std::cout << "  Выход: \"" << output << "\"\n\n";
//
//     results.push_back(r);
//   }
//
//   // Итоговая сводка
//   // Примечание: setw считает БАЙТЫ, а не отображаемые символы, так что
//   // с кириллицей (2 байта/символ в UTF-8) ручное выравнивание строкой
//   // надёжнее, чем std::setw на самой строке заголовка.
//   std::cout << "==================== ИТОГ ====================\n";
//   std::cout << "Плагин            Статус  Компиляция(мс)   Вызов(мс)\n";
//   for (auto &r : results) {
//     std::cout << std::left << std::setw(18) << r.name << std::setw(8)
//                << (r.ok ? "OK" : "FAIL") << std::right << std::fixed
//                << std::setprecision(3) << std::setw(18) << r.compileMs
//                << std::setw(14) << r.callMs << "\n";
//   }
//   std::cout << "\nДетали ошибок (если были):\n";
//   for (auto &r : results) {
//     if (!r.ok) std::cout << "  " << r.name << ": " << r.status << "\n";
//   }
//
//   int okCount = 0;
//   for (auto &r : results) if (r.ok) okCount++;
//   std::cout << "\nУспешно обработано " << okCount << " из " << results.size()
//             << " плагинов. Хост не упал ни разу.\n";
//
//   return 0;
// }