// #include "commands/task_commands.hpp"
// #include <stdexcept>
//
// namespace commands {
//
// // -- AddTaskCommand -----------------------------------------------------
// AddTaskCommand::AddTaskCommand(domain::TaskRepository& repo, std::string title, int priority)
//     : repo_(repo), title_(std::move(title)), priority_(priority) {}
//
// void AddTaskCommand::execute() { assignedId_ = repo_.addTask(title_, priority_); }
// void AddTaskCommand::undo() {
//     if (assignedId_ != -1) repo_.removeTask(assignedId_);
// }
// std::string AddTaskCommand::description() const { return "add \"" + title_ + "\""; }
//
// // -- RemoveTaskCommand ----------------------------------------------------
// RemoveTaskCommand::RemoveTaskCommand(domain::TaskRepository& repo, int id) : repo_(repo), id_(id) {}
//
// void RemoveTaskCommand::execute() {
//     auto found = repo_.find(id_);
//     if (!found) throw std::runtime_error("task not found: " + std::to_string(id_));
//     removedSnapshot_ = *found;  // снимок ДО удаления - иначе undo будет нечем восстанавливать
//     repo_.removeTask(id_);
// }
// void RemoveTaskCommand::undo() { repo_.restoreTask(removedSnapshot_); }
// std::string RemoveTaskCommand::description() const { return "remove #" + std::to_string(id_); }
//
// // -- CompleteTaskCommand --------------------------------------------------
// CompleteTaskCommand::CompleteTaskCommand(domain::TaskRepository& repo, int id) : repo_(repo), id_(id) {}
// void CompleteTaskCommand::execute() { repo_.completeTask(id_); }
// void CompleteTaskCommand::undo() { repo_.reopenTask(id_); }
// std::string CompleteTaskCommand::description() const { return "complete #" + std::to_string(id_); }
//
// // -- ReopenTaskCommand ------------------------------------------------------
// ReopenTaskCommand::ReopenTaskCommand(domain::TaskRepository& repo, int id) : repo_(repo), id_(id) {}
// void ReopenTaskCommand::execute() { repo_.reopenTask(id_); }
// void ReopenTaskCommand::undo() { repo_.completeTask(id_); }
// std::string ReopenTaskCommand::description() const { return "reopen #" + std::to_string(id_); }
//
// // -- factory functions ------------------------------------------------------
// std::unique_ptr<ICommand> makeAddTaskCommand(domain::TaskRepository& repo, std::string title, int priority) {
//     return std::make_unique<AddTaskCommand>(repo, std::move(title), priority);
// }
// std::unique_ptr<ICommand> makeRemoveTaskCommand(domain::TaskRepository& repo, int id) {
//     return std::make_unique<RemoveTaskCommand>(repo, id);
// }
// std::unique_ptr<ICommand> makeCompleteTaskCommand(domain::TaskRepository& repo, int id) {
//     return std::make_unique<CompleteTaskCommand>(repo, id);
// }
// std::unique_ptr<ICommand> makeReopenTaskCommand(domain::TaskRepository& repo, int id) {
//     return std::make_unique<ReopenTaskCommand>(repo, id);
// }
//
// }  // namespace commands
