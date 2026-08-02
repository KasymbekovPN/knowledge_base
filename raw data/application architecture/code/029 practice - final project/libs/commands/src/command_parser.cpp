// #include "commands/command_parser.hpp"
// #include "commands/task_commands.hpp"
// #include <sstream>
//
// namespace commands {
//
// namespace {
// // trim + разбить строку на первое слово (команду) и остаток (аргументы)
// std::pair<std::string, std::string> splitHead(const std::string& line) {
//     auto start = line.find_first_not_of(" \t");
//     if (start == std::string::npos) return {"", ""};
//     auto spacePos = line.find_first_of(" \t", start);
//     if (spacePos == std::string::npos) return {line.substr(start), ""};
//     std::string head = line.substr(start, spacePos - start);
//     auto restStart = line.find_first_not_of(" \t", spacePos);
//     std::string rest = (restStart == std::string::npos) ? "" : line.substr(restStart);
//     return {head, rest};
// }
// }  // namespace
//
// ParsedLine parseCommandLine(const std::string& line, domain::TaskRepository& repo) {
//     auto [head, rest] = splitHead(line);
//
//     if (head.empty()) return {ParseAction::Unknown, nullptr, ""};  // пустая строка - молча игнорируем в EventLoop
//     if (head == "quit" || head == "exit") return {ParseAction::Quit, nullptr, ""};
//     if (head == "undo") return {ParseAction::Undo, nullptr, ""};
//     if (head == "redo") return {ParseAction::Redo, nullptr, ""};
//     if (head == "help") return {ParseAction::Help, nullptr, ""};
//
//     if (head == "add") {
//         std::istringstream ss(rest);
//         int priority = 0;
//         if (!(ss >> priority)) {
//             return {ParseAction::Unknown, nullptr, "add: ожидается 'add <priority> <title>'"};
//         }
//         std::string title;
//         std::getline(ss, title);
//         auto titleStart = title.find_first_not_of(' ');
//         title = (titleStart == std::string::npos) ? "" : title.substr(titleStart);
//         if (title.empty()) return {ParseAction::Unknown, nullptr, "add: пустой title"};
//         return {ParseAction::Dispatch, makeAddTaskCommand(repo, title, priority), ""};
//     }
//
//     if (head == "remove" || head == "complete" || head == "reopen") {
//         std::istringstream ss(rest);
//         int id = 0;
//         if (!(ss >> id)) return {ParseAction::Unknown, nullptr, head + ": ожидается '" + head + " <id>'"};
//
//         if (head == "remove") return {ParseAction::Dispatch, makeRemoveTaskCommand(repo, id), ""};
//         if (head == "complete") return {ParseAction::Dispatch, makeCompleteTaskCommand(repo, id), ""};
//         return {ParseAction::Dispatch, makeReopenTaskCommand(repo, id), ""};
//     }
//
//     return {ParseAction::Unknown, nullptr, "неизвестная команда: " + head + " (наберите 'help')"};
// }
//
// }  // namespace commands
