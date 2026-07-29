#include "domain.hpp"

#include <algorithm>

int TaskRepository::addTask(const std::string& title) {
    //     int id = nextId_++;
    //     tasks_.push_back({id, title, false});
    //     return id;
}

void TaskRepository::insertTaskAt(const std::size_t index, const Task& task) {
    //     auto pos = tasks_.begin() + static_cast<std::ptrdiff_t>(std::min(index, tasks_.size()));
    //     tasks_.insert(pos, task);
}

std::pair<Task, std::size_t> TaskRepository::removeTask(const int id) {
    //     auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
    //     if (it == tasks_.end()) throw std::runtime_error("task not found: " + std::to_string(id));
    //     std::size_t index = static_cast<std::size_t>(std::distance(tasks_.begin(), it));
    //     Task copy = *it;
    //     tasks_.erase(it);
    //     return {copy, index};
}

void TaskRepository::removeTaskById(const int id) {
    //     auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
    //     if (it != tasks_.end()) tasks_.erase(it);
}

void TaskRepository::setDone(const int id, const bool done) {
    //     auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& t) { return t.id == id; });
    //     if (it != tasks_.end()) it->done = done;
}
