#pragma once

#inlcude "domain.hpp"
#include <eventbus/event_bus.hpp>
#include <memory>
#include <optional>
#include <stack>
#include <string>

struct TaskAdded { int id; std::string title; };
struct TaskRemoved { int id; std::string title; };
struct TaskCompleted { int id; };
struct TaskReopened { int id; };

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class AddTask : public ICommand {
public:
    //     AddTaskCommand(TaskRepository& repo, EventBus& bus, std::string title);
    //     void execute() override;
    //     void undo() override;
    //     int id() const { return id_.value(); }
private:

    //     TaskRepository& repo_;
    //     EventBus& bus_;
    //     std::string title_;
    //     std::optional<int> id_;
};

// class CompleteTaskCommand : public ICommand {
// public:
//     CompleteTaskCommand(TaskRepository& repo, EventBus& bus, int id);
//     void execute() override;
//     void undo() override;
//
// private:
//     TaskRepository& repo_;
//     EventBus& bus_;
//     int id_;
// };
//
// class RemoveTaskCommand : public ICommand {
// public:
//     RemoveTaskCommand(TaskRepository& repo, EventBus& bus, int id);
//     void execute() override;
//     void undo() override;
//
// private:
//     TaskRepository& repo_;
//     EventBus& bus_;
//     int id_;
//     Task removedTask_{};
//     std::size_t removedIndex_ = 0;
// };
//
// class CommandManager {
// public:
//     void execute(std::unique_ptr<ICommand> cmd);
//     bool undo();
//
// private:
//     std::stack<std::unique_ptr<ICommand>> undoStack_;
// };