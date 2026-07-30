#include "domain.hpp"

#include <algorithm>
#include <ranges>
#include <format>

int TaskRepository::addTask(const std::string& title) {
    const int ID{nextId_++};
    tasks_.push_back({.id = ID, .title = title, .done = false});

    return ID;
}

void TaskRepository::insertTaskAt(const std::size_t index, const Task& task) {
    const auto POS{
        tasks_.begin() +
        static_cast<std::ptrdiff_t>(std::min(index, tasks_.size()))
    };
    tasks_.insert(POS, task);
}

std::pair<Task, std::size_t> TaskRepository::removeTask(const int id) {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const Task& task) { return task.id == id; })
    };
    if (IT == tasks_.end()) throw std::runtime_error{std::format("task not found: {}", id)};
    const std::size_t INDEX{static_cast<std::size_t>(std::distance(tasks_.begin(), IT))};
    Task copy = *IT;
    tasks_.erase(IT);

    return {copy, INDEX};
}

void TaskRepository::removeTaskById(const int id) {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const Task& task) { return task.id == id; })};
    if (IT != tasks_.end()) tasks_.erase(IT);
}

void TaskRepository::setDone(const int id, const bool done) {
    const auto IT{std::ranges::find_if(
        tasks_,
        [id](const Task& task) { return task.id == id; })};
    if (IT != tasks_.end()) IT->done = done;
}
