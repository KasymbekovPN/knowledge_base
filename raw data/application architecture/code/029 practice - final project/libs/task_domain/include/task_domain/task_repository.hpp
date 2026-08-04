#pragma once

#include "task_domain/task.hpp"
#include <event_bus/event_bus.hpp>

#include <optional>
#include <string>
#include <vector>

#include "task.hpp"

namespace domain {
    // Model / источник истины. Единственная зависимость "наружу" - EventBus
    // (мы через него уведомляем о своих изменениях). НИЧЕГО не знает про
    // Command, EventLoop или View - ни ссылок, ни forward-деклараций, ни
    // callback'ов на них. Это направление зависимости и проверяется ниже
    // через `nm` (символы Command/View не должны попасть в libtask_domain.a).
    class TaskRepository {
    public:
        explicit TaskRepository(EventBus& bus): bus_(bus) {}
        int addTask(const std::string&, int);
        // бросает std::runtime_error, если id не найден
        void removeTask(int);
        void completeTask(int);
        void reopenTask(int);

        // Нужно командам для undo(remove): вернуть задачу с ЕЁ ЖЕ id, а не
        // создать новую через addTask (который выдал бы новый id).
        void restoreTask(const Task&);

        [[nodiscard]] std::optional<Task> find(int) const;
        [[nodiscard]] const std::vector<Task>& all() const { return tasks_; }

    private:
        void setDone(int, bool);
        EventBus& bus_;
        std::vector<Task> tasks_;
        int nextId_{1};
    };

}
