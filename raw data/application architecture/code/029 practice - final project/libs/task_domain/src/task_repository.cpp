#include "task_domain/task_repository.hpp"

#include <algorithm>
#include <format>
#include <stdexcept>

namespace domain {

int TaskRepository::addTask(const std::string& title, const int priority) {
    const int ID{nextId_++};
    tasks_.push_back({.id = ID, .title = title, .priority = priority, .done = false});
    bus_.publish(TaskAdded{.id = ID, .title = title, .priority = priority});

    return ID;
}

void TaskRepository::removeTask(int id) {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const auto& task) { return task.id == id; })};

    if (IT == tasks_.end())
        throw std::runtime_error{std::format("task not found: {}", id)};

    const auto REMOVED = *IT;
    tasks_.erase(IT);
    bus_.publish(TaskRemoved{.id = id, .removed = REMOVED});
}

void TaskRepository::completeTask(const int id) {
    setDone(id, true);
    bus_.publish(TaskCompleted{.id = id});
}

void TaskRepository::reopenTask(const int id) {
    setDone(id, false);
    bus_.publish(TaskCompleted{.id = id});
}

void TaskRepository::restoreTask(const Task& task) {
    tasks_.push_back(task);
    // nextId_ не трогаем - специально восстанавливаем СТАРЫЙ id, не выдаём новый
    bus_.publish(TaskRestored{.id = task.id});
}

std::optional<Task> TaskRepository::find(const int id) const {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const auto& task) { return task.id == id; })};

    return IT == tasks_.end() ? std::nullopt : static_cast<std::optional<Task>>(*IT);
}

void TaskRepository::setDone(const int id, const bool done) {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const auto& task) { return task.id == id; })};
    if (IT != tasks_.end()) IT->done = done;
}

}
