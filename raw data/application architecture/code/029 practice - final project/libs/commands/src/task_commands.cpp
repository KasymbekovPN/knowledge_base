#include "commands/task_commands.hpp"

#include <format>
#include <stdexcept>

namespace commands {

// BEGIN AddTaskCommand

AddTaskCommand::AddTaskCommand(domain::TaskRepository& repo, std::string title, const int priority):
    repo_{repo}, title_{std::move(title)}, priority_{priority} {}

void AddTaskCommand::execute() { assignId_ = repo_.addTask(title_, priority_); }

void AddTaskCommand::undo() { if (assignId_ != -1) repo_.removeTask(assignId_); }

std::string AddTaskCommand::description() const { return std::format("add '{}'", title_); }

// END AddTaskCommand

// BEGIN RemoveTaskCommand

RemoveTaskCommand::RemoveTaskCommand(domain::TaskRepository& repo, const int id):
    repo_{repo}, id_{id} {}

void RemoveTaskCommand::execute() {
    const auto FOUND{repo_.find(id_)};
    if (!FOUND) throw std::runtime_error{std::format("task not found: {}", id_)};
    removedSnapshot_ = *FOUND; // снимок ДО удаления - иначе undo будет нечем восстанавливать
    repo_.removeTask(id_);
}

void RemoveTaskCommand::undo() { repo_.restoreTask(removedSnapshot_); }

std::string RemoveTaskCommand::description() const { return std::format("remove #{}", id_); }

// END RemoveTaskCommand

// BEGIN CompleteTaskCommand

CompleteTaskCommand::CompleteTaskCommand(domain::TaskRepository& repo, const int id):
    repo_{repo}, id_{id} {}

void CompleteTaskCommand::execute() { repo_.completeTask(id_); }

void CompleteTaskCommand::undo() { repo_.reopenTask(id_); }

std::string CompleteTaskCommand::description() const { return std::format("complete #{}", id_); }

// END CompleteTaskCommand

// BEGIN ReopenTaskCommand

ReopenTaskCommand::ReopenTaskCommand(domain::TaskRepository& repo, const int id):
    repo_{repo}, id_{id} {}

void ReopenTaskCommand::execute() { repo_.reopenTask(id_); }

void ReopenTaskCommand::undo() { repo_.completeTask(id_); }

std::string ReopenTaskCommand::description() const { return std::format("reopen #{}", id_); }

// END ReopenTaskCommand

std::unique_ptr<ICommand> makeAddTaskCommand(domain::TaskRepository& repo, std::string title, const int priority) {
    return std::make_unique<AddTaskCommand>(repo, std::move(title), priority);
}
std::unique_ptr<ICommand> makeRemoveTaskCommand(domain::TaskRepository& repo, const int id) {
    return std::make_unique<RemoveTaskCommand>(repo, id);
}
std::unique_ptr<ICommand> makeCompleteTaskCommand(domain::TaskRepository& repo, const int id) {
    return std::make_unique<CompleteTaskCommand>(repo, id);
}
std::unique_ptr<ICommand> makeReopenTaskCommand(domain::TaskRepository& repo, const int id) {
    return std::make_unique<ReopenTaskCommand>(repo, id);
}

}
