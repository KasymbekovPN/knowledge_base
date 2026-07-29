// // Composition root. Единственное место, где встречаются: библиотека
// // eventbus_lib (Day 3-4), domain/commands (Day 3-4 практика) и
// // динамическая загрузка observer-плагинов (Day 5-6 практика).
// #include "commands.hpp"
// #include "domain.hpp"
// #include "plugin_loader.hpp"
// #include <iostream>
// #include <vector>
//
// class App {
// public:
//     // Плагины передаются как уже загруженные извне - App лишь подписывает
//     // их на EventBus, переводя типизированные C++ события в вызовы
//     // ABI-стабильного интерфейса ITaskObserver (int, const char*).
//     void attachObserver(ITaskObserver* observer) {
//         addedConn_.push_back(bus_.subscribe<TaskAdded>(
//             [observer](const TaskAdded& e) { observer->onTaskAdded(e.id, e.title.c_str()); }));
//         removedConn_.push_back(bus_.subscribe<TaskRemoved>(
//             [observer](const TaskRemoved& e) { observer->onTaskRemoved(e.id, e.title.c_str()); }));
//         completedConn_.push_back(bus_.subscribe<TaskCompleted>(
//             [observer](const TaskCompleted& e) { observer->onTaskCompleted(e.id); }));
//         reopenedConn_.push_back(bus_.subscribe<TaskReopened>(
//             [observer](const TaskReopened& e) { observer->onTaskReopened(e.id); }));
//     }
//
//     int addTask(const std::string& title) {
//         auto cmd = std::make_unique<AddTaskCommand>(repo_, bus_, title);
//         AddTaskCommand* raw = cmd.get();
//         commands_.execute(std::move(cmd));
//         return raw->id();
//     }
//     void completeTask(int id) { commands_.execute(std::make_unique<CompleteTaskCommand>(repo_, bus_, id)); }
//     void removeTask(int id) { commands_.execute(std::make_unique<RemoveTaskCommand>(repo_, bus_, id)); }
//     void undo() { commands_.undo(); }
//
// private:
//     EventBus bus_;
//     TaskRepository repo_;
//     CommandManager commands_;
//     std::vector<EventBus::Connection> addedConn_, removedConn_, completedConn_, reopenedConn_;
// };
//
// int main(int argc, char** argv) {
//     std::vector<std::string> pluginPaths;
//     if (argc > 1) {
//         pluginPaths.assign(argv + 1, argv + argc);
//     } else {
//         for (const auto& base : {"console_logger", "stats_logger"})
//             pluginPaths.push_back("./" + platform::pluginFileName(base));
//     }
//
//     std::vector<std::unique_ptr<LoadedObserver>> observers;
//     App app;
//
//     std::cout << "-- загрузка observer-плагинов --\n";
//     for (const auto& path : pluginPaths) {
//         std::cout << "== " << path << " ==\n";
//         if (auto obs = LoadedObserver::load(path)) {
//             app.attachObserver(obs->get());
//             observers.push_back(std::move(obs));
//         }
//     }
//
//     std::cout << "\n-- сценарий --\n";
//     int id1 = app.addTask("Написать план архитектуры");
//     int id2 = app.addTask("Собрать eventbus_lib отдельно");
//     app.completeTask(id2);
//     app.removeTask(id1);
//     std::cout << "\n-- undo (вернуть удалённую задачу) --\n";
//     app.undo();
//
//     // observers уничтожаются здесь -> destroyTaskObserver() внутри каждого
//     // плагина -> closeLibrary(). App с его EventBus умирает раньше, поэтому
//     // подписки Connection отключаются ДО выгрузки библиотек - порядок важен.
//     return 0;
// }
