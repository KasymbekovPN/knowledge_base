#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Task {
    int id{0};
    std::string title;
    bool done{false};
};

class TaskRepository {
public:
    int addTask(const std::string& title);
    void insertTaskAt(std::size_t index, const Task& task);
    std::pair<Task, std::size_t> removeTask(int id);
    void removeTaskById(int id);
    void setDone(int id, bool done);
    [[nodiscard]] const std::vector<Task>& all() const { return tasks_; }
private:
    std::vector<Task> tasks_;
    int nextId_{1};
};
