// #pragma once
// #include "commands/command.hpp"
// #include <task_domain/task_repository.hpp>
// #include <memory>
// #include <string>
//
// namespace commands {
//
// // Каждая команда знает, КАК сделать операцию над Model и КАК её отменить.
// // EventLoop и CommandHistory работают только через интерфейс ICommand -
// // им не нужно знать, что "undo для complete" - это reopen, а "undo для
// // remove" - это restoreTask с тем же id.
//
// class AddTaskCommand : public ICommand {
// public:
//     AddTaskCommand(domain::TaskRepository& repo, std::string title, int priority);
//     void execute() override;
//     void undo() override;
//     std::string description() const override;
//
// private:
//     domain::TaskRepository& repo_;
//     std::string title_;
//     int priority_;
//     int assignedId_ = -1;
// };
//
// class RemoveTaskCommand : public ICommand {
// public:
//     RemoveTaskCommand(domain::TaskRepository& repo, int id);
//     void execute() override;
//     void undo() override;
//     std::string description() const override;
//
// private:
//     domain::TaskRepository& repo_;
//     int id_;
//     domain::Task removedSnapshot_;
// };
//
// class CompleteTaskCommand : public ICommand {
// public:
//     CompleteTaskCommand(domain::TaskRepository& repo, int id);
//     void execute() override;
//     void undo() override;
//     std::string description() const override;
//
// private:
//     domain::TaskRepository& repo_;
//     int id_;
// };
//
// class ReopenTaskCommand : public ICommand {
// public:
//     ReopenTaskCommand(domain::TaskRepository& repo, int id);
//     void execute() override;
//     void undo() override;
//     std::string description() const override;
//
// private:
//     domain::TaskRepository& repo_;
//     int id_;
// };
//
// std::unique_ptr<ICommand> makeAddTaskCommand(domain::TaskRepository& repo, std::string title, int priority);
// std::unique_ptr<ICommand> makeRemoveTaskCommand(domain::TaskRepository& repo, int id);
// std::unique_ptr<ICommand> makeCompleteTaskCommand(domain::TaskRepository& repo, int id);
// std::unique_ptr<ICommand> makeReopenTaskCommand(domain::TaskRepository& repo, int id);
//
// }  // namespace commands
