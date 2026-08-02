// #pragma once
// #include <config/config.hpp>
// #include <task_domain/task_repository.hpp>
// #include <event_bus/event_bus.hpp>
// #include <iostream>
//
// namespace view {
//
//     // View. Держит const-ссылки на Model и Config - только читает, никогда не
//     // мутирует. Подписывается на события Model САМА в конструкторе - Model
//     // про View не знает вообще (ни ссылки, ни интерфейса, ни callback'а), и
//     // EventLoop про View тоже не знает НИЧЕГО (нет ни единого include).
//     // render() никто не вызывает напрямую - View перерисовывается сама в
//     // ответ на любое опубликованное событие, включая TaskRestored (undo).
//     class TaskBoardView {
//     public:
//         TaskBoardView(const domain::TaskRepository& repo, const Config& config, EventBus& bus)
//             : repo_(repo), config_(config) {
//             addedConn_ = bus.subscribe<domain::TaskAdded>([this](const domain::TaskAdded&) { render(); });
//             removedConn_ = bus.subscribe<domain::TaskRemoved>([this](const domain::TaskRemoved&) { render(); });
//             completedConn_ = bus.subscribe<domain::TaskCompleted>([this](const domain::TaskCompleted&) { render(); });
//             reopenedConn_ = bus.subscribe<domain::TaskReopened>([this](const domain::TaskReopened&) { render(); });
//             restoredConn_ = bus.subscribe<domain::TaskRestored>([this](const domain::TaskRestored&) { render(); });
//         }
//
//         void render() const {
//             std::cout << "=== " << config_.appTitle() << " ===\n";
//             for (const auto& t : repo_.all()) {
//                 std::cout << "  [" << (t.done ? "x" : " ") << "] #" << t.id << " " << t.title
//                            << "  (priority: " << config_.priorityLabel(t.priority) << ")\n";
//             }
//             std::cout << "\n";
//         }
//
//     private:
//         const domain::TaskRepository& repo_;
//         const Config& config_;
//         EventBus::Connection addedConn_, removedConn_, completedConn_, reopenedConn_, restoredConn_;
//     };
//
// }  // namespace view
