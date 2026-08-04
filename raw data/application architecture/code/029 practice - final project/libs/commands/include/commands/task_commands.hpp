#pragma once

#include "commands/command.hpp"
#include <task_domain/task_repository.hpp>

#include <memory>
#include <string>

namespace commands {

// Каждая команда знает, КАК сделать операцию над Model и КАК её отменить.
// EventLoop и CommandHistory работают только через интерфейс ICommand -
// им не нужно знать, что "undo для complete" - это reopen, а "undo для
// remove" - это restoreTask с тем же id.

class AddTaskCommand: public ICommand {
public:
    AddTaskCommand(domain::TaskRepository&, std::string, int);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;
private:
    domain::TaskRepository& repo_;
    std::string title_{};
    int priority_{};
    int assignId_{-1};
};

class RemoveTaskCommand: public ICommand {
public:
    RemoveTaskCommand(domain::TaskRepository&, int);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;
private:
    domain::TaskRepository& repo_;
    int id_;
    domain::Task removedSnapshot_;
};

class CompleteTaskCommand: public ICommand {
public:
    CompleteTaskCommand(domain::TaskRepository&, int);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    domain::TaskRepository& repo_;
    int id_{};
};

class ReopenTaskCommand: public ICommand {
public:
    ReopenTaskCommand(domain::TaskRepository&, int);
    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;
private:
    domain::TaskRepository& repo_;
    int id_{};
};

std::unique_ptr<ICommand> makeAddTaskCommand(domain::TaskRepository&, std::string, int);
std::unique_ptr<ICommand> makeRemoveTaskCommand(domain::TaskRepository&, int);
std::unique_ptr<ICommand> makeCompleteTaskCommand(domain::TaskRepository&, int);
std::unique_ptr<ICommand> makeReopenTaskCommand(domain::TaskRepository&, int);

}
