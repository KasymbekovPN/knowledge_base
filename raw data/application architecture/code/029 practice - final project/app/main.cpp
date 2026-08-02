// #include <config/config.hpp>
// #include <event_bus/event_bus.hpp>
// #include <event_loop/event_loop.hpp>
// #include <task_domain/task_repository.hpp>
// #include <commands/command_history.hpp>
// #include <view/task_board_view.hpp>
// #include <fstream>
// #include <iostream>
// #include <sstream>
//
// namespace {
// // Загрузка начального состояния. Это НЕ команды пользователя (не должны
// // попадать в undo-историю) - сид напрямую дергает Model, как и раньше.
// void loadSeedTasks(domain::TaskRepository& repo, const std::string& path) {
//     std::ifstream file(path);
//     if (!file) {
//         std::cerr << "не удалось открыть " << path << ", пропускаем сид\n";
//         return;
//     }
//     std::string line;
//     std::getline(file, line);  // заголовок csv
//     while (std::getline(file, line)) {
//         if (line.empty()) continue;
//         std::istringstream ss(line);
//         std::string title, priorityStr;
//         std::getline(ss, title, ',');
//         std::getline(ss, priorityStr, ',');
//         repo.addTask(title, std::stoi(priorityStr));
//     }
// }
// }  // namespace
//
// // Композиционный корень (composition root). ЕДИНСТВЕННОЕ место во всём
// // приложении, которое знает про все модули сразу: config, event_bus,
// // task_domain (Model), commands (+ CommandHistory), event_loop (вход),
// // view (выход). Каждый из модулей по отдельности не знает о соседях
// // больше необходимого - граф зависимостей ниже.
// //
// //                     ┌───────────┐
// //   stdin ──────────▶ │ EventLoop │
// //                     └─────┬─────┘
// //                           │ parseCommandLine (commands)
// //                           ▼
// //                 ┌───────────────────┐
// //                 │  CommandHistory    │  undo/redo стек
// //                 └─────────┬──────────┘
// //                           │ execute()/undo()
// //                           ▼
// //                 ┌───────────────────┐        ┌─────────┐
// //                 │  TaskRepository    │◀──────▶│  Config │ (только View)
// //                 │      (Model)       │        └─────────┘
// //                 └─────────┬──────────┘
// //                           │ publish(TaskAdded/...)  через EventBus
// //                           ▼
// //                 ┌───────────────────┐
// //                 │   TaskBoardView    │ ─────▶ stdout
// //                 └───────────────────┘
// //
// // EventLoop и TaskBoardView НИКОГДА не ссылаются друг на друга напрямую -
// // единственная связь между "входом" и "выходом" идёт через Model + EventBus.
// int main(int argc, char** argv) {
//     std::string configPath = (argc > 1) ? argv[1] : "app.cfg";
//     std::string seedPath = (argc > 2) ? argv[2] : "seed_tasks.csv";
//
//     Config config = Config::loadFromFile(configPath);
//
//     EventBus bus;
//     domain::TaskRepository repo(bus);
//     commands::CommandHistory history;
//     view::TaskBoardView boardView(repo, config, bus);  // подписывается сама, в конструкторе
//
//     std::cout << "-- загрузка сид-задач из " << seedPath << " --\n";
//     loadSeedTasks(repo, seedPath);  // View уже отрисовала стартовое состояние сама
//
//     app::EventLoop loop(repo, history, std::cin, std::cout);
//     loop.run();  // единственная точка, где приложение реагирует на внешние события
//
//     return 0;
// }
