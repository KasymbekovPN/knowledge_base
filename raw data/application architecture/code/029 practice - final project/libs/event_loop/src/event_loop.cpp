// #include "event_loop/event_loop.hpp"
// #include <stdexcept>
//
// namespace app {
//
//     EventLoop::EventLoop(domain::TaskRepository& repo, commands::CommandHistory& history,
//                           std::istream& input, std::ostream& output)
//         : repo_(repo), history_(history), input_(input), output_(output) {}
//
//     void EventLoop::run() {
//         output_ << "-- event loop запущен, наберите 'help' для списка команд --\n";
//         std::string line;
//         while (running_ && std::getline(input_, line)) {
//             handleLine(line);
//         }
//         output_ << "-- event loop остановлен --\n";
//     }
//
//     void EventLoop::handleLine(const std::string& line) {
//         using commands::ParseAction;
//         commands::ParsedLine parsed = commands::parseCommandLine(line, repo_);
//
//         switch (parsed.action) {
//             case ParseAction::Dispatch:
//                 try {
//                     history_.execute(std::move(parsed.command));
//                 } catch (const std::exception& e) {
//                     output_ << "ошибка: " << e.what() << "\n";
//                 }
//                 break;
//             case ParseAction::Undo:
//                 if (!history_.undo()) output_ << "нечего отменять\n";
//                 break;
//             case ParseAction::Redo:
//                 if (!history_.redo()) output_ << "нечего повторять\n";
//                 break;
//             case ParseAction::Help:
//                 output_ << "команды: add <priority> <title> | remove <id> | complete <id> | "
//                            "reopen <id> | undo | redo | quit\n";
//                 break;
//             case ParseAction::Quit:
//                 running_ = false;
//                 break;
//             case ParseAction::Unknown:
//                 if (!parsed.error.empty()) output_ << "ошибка: " << parsed.error << "\n";
//                 break;
//         }
//     }
//
// }  // namespace app
