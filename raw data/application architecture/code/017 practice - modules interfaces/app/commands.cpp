// #include "commands.hpp"
//
// AddTaskCommand::AddTaskCommand(TaskRepository& repo, EventBus& bus, std::string title)
//     : repo_(repo), bus_(bus), title_(std::move(title)) {}
//
// void AddTaskCommand::execute() {
//     if (!id_.has_value()) {
//         id_ = repo_.addTask(title_);
//     } else {
//         repo_.insertTaskAt(repo_.all().size(), Task{*id_, title_, false});
//     }
//     bus_.publish(TaskAdded{*id_, title_});
// }
// void AddTaskCommand::undo() {
//     bus_.publish(TaskRemoved{*id_, title_});
//     repo_.removeTaskById(*id_);
// }
//
// CompleteTaskCommand::CompleteTaskCommand(TaskRepository& repo, EventBus& bus, int id)
//     : repo_(repo), bus_(bus), id_(id) {}
//
// void CompleteTaskCommand::execute() {
//     repo_.setDone(id_, true);
//     bus_.publish(TaskCompleted{id_});
// }
// void CompleteTaskCommand::undo() {
//     repo_.setDone(id_, false);
//     bus_.publish(TaskReopened{id_});
// }
//
// RemoveTaskCommand::RemoveTaskCommand(TaskRepository& repo, EventBus& bus, int id)
//     : repo_(repo), bus_(bus), id_(id) {}
//
// void RemoveTaskCommand::execute() {
//     auto [task, index] = repo_.removeTask(id_);
//     removedTask_ = task;
//     removedIndex_ = index;
//     bus_.publish(TaskRemoved{id_, task.title});
// }
// void RemoveTaskCommand::undo() {
//     repo_.insertTaskAt(removedIndex_, removedTask_);
//     bus_.publish(TaskAdded{removedTask_.id, removedTask_.title});
// }
//
// void CommandManager::execute(std::unique_ptr<ICommand> cmd) {
//     cmd->execute();
//     undoStack_.push(std::move(cmd));
// }
//
// bool CommandManager::undo() {
//     if (undoStack_.empty()) return false;
//     auto cmd = std::move(undoStack_.top());
//     undoStack_.pop();
//     cmd->undo();
//     return true;
// }