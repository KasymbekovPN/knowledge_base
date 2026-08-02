#pragma once

#include "task_domain/task.hpp"
#include <event_bus/event_bus.hpp>

#include <optional>
#include <vector>

namespace domain {
    // Model / источник истины. Единственная зависимость "наружу" - EventBus
    // (мы через него уведомляем о своих изменениях). НИЧЕГО не знает про
    // Command, EventLoop или View - ни ссылок, ни forward-деклараций, ни
    // callback'ов на них. Это направление зависимости и проверяется ниже
    // через `nm` (символы Command/View не должны попасть в libtask_domain.a).
    class TaskRepository {
    public:
        explicit TaskRepository(EventBus& bus): bus_(bus) {}

        //         int addTask(const std::string& title, int priority);
        //         void removeTask(int id);   // бросает std::runtime_error, если id не найден
        //         void completeTask(int id);
        //         void reopenTask(int id);
        //
        //         // Нужно командам для undo(remove): вернуть задачу с ЕЁ ЖЕ id, а не
        //         // создать новую через addTask (который выдал бы новый id).
        //         void restoreTask(const Task& task);
        //
        //         std::optional<Task> find(int id) const;
        //         const std::vector<Task>& all() const { return tasks_; }
    private:
        void setDone(int, bool);
        EventBus& bus_;
        int nextId_{1};
    };

}
