#pragma once

#include "eventbus.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <ranges>
#include <format>

// ---------------------------------------------------------------------------
// Model. Не включает ничего про View/консоль/рендеринг - вообще не
// подозревает, что кто-то может на неё "смотреть". Публикует события через
// EventBus (Model как источник истины - см. тему пару шагов назад:
// уведомление встроено В САМИ методы мутации, а не в вызывающий код).
// ---------------------------------------------------------------------------
struct Task {
    int id{};
    std::string title;
    // 0..N-1, подпись переводится через Config, а не хардкодится тут
    int priority{};
    bool done{false};
};

struct TaskAdded { int id{}; std::string title; int priority{}; };
struct TaskRemoved { int id{}; };
struct TaskCompleted { int id{}; };
struct TaskReopened { int id{}; };

class TaskRepository {
public:
    explicit TaskRepository(EventBus& bus) : bus_(bus) {}

    int addTask(const std::string& title, const int priority) {
        const int ID{nextId_++};
        tasks_.push_back(Task{.id = ID, .title = title, .priority = priority});
        bus_.publish(TaskAdded{.id = ID, .title = title, .priority = priority});

        return ID;
    }

    void removeTask(const int id) {
        const auto IT{std::ranges::find_if(
            tasks_,
            [id](const Task& t) { return t.id == id; })};
        if (IT == tasks_.end()) throw std::runtime_error{std::format("Task not found: {}", id)};
        tasks_.erase(IT);
        bus_.publish(TaskRemoved{.id = id});
    }

    void completeTask(const int id) {
        setDone(id, true);
        bus_.publish(TaskCompleted{.id = id});
    }

    void reopenTask(const int id) {
        setDone(id, false);
        bus_.publish(TaskReopened{.id = id});
    }

    const std::vector<Task>& all() const { return tasks_; }

private:
    void setDone(const int id, const bool done) {
        const auto IT{std::ranges::find_if(
            tasks_,
            [id](const Task& t) { return t.id == id; })};
        if (IT != tasks_.end()) IT->done = done;
    }

    EventBus& bus_;
    std::vector<Task> tasks_;
    int nextId_{1};
};
