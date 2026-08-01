#pragma once

#include "config.hpp"
#include "model.hpp"

#include <iostream>
#include <format>

// ---------------------------------------------------------------------------
// View. Держит const-ссылки на Model и Config - может ЧИТАТЬ их состояние,
// но ни разу не мутирует ни то, ни другое. Подписывается на события Model
// сама - Model про View не знает вообще, ни ссылки, ни интерфейса, ни callback'а.
// Никто не вызывает View.render() напрямую - она перерисовывается САМА
// в ответ на любое опубликованное событие.
// ---------------------------------------------------------------------------
class TaskBoardView {
public:
    TaskBoardView(const TaskRepository& repo, const Config& config, EventBus& bus):
        repo_{repo}, config_{config} {
        addedConn_ = bus.subscribe<TaskAdded>([this](const TaskAdded& info) { render(); });
        removedConn_ = bus.subscribe<TaskRemoved>([this](const TaskRemoved& info) { render(); });
        completedConn_ = bus.subscribe<TaskCompleted>([this](const TaskCompleted& info) { render(); });
        reopenedConn_ = bus.subscribe<TaskReopened>([this](const TaskReopened& info) { render(); });
    }

    void render() const {
        std::cout << std::format("=== {} ===\n", config_.appTitle());
        for (const auto&[id, title, priority, done]: repo_.all()) {
            std::cout << std::format("  [{}] #{} '{}' (priority: {})\n", done ? "x" : " ", id, title, priority);
        }
        std::cout << '\n';
    }

private:
    const TaskRepository& repo_;
    const Config& config_;
    EventBus::Connection addedConn_, removedConn_, completedConn_, reopenedConn_;
};
