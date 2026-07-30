// Плагин 1: печатает каждое событие в консоль (аналог ActivityLog из
// предыдущей версии пет-проекта, только теперь это отдельно собираемая
// и отдельно загружаемая библиотека).

#include "itask_observer.hpp"

#include <iostream>
#include <format>

class ConsoleLoggerObserver: public ITaskObserver {
public:
    void onTaskAdded(int id, const char* title) override {
        std::cout << std::format("  [console_logger] task #{} '{}' added\n", id, title);
    }
    void onTaskRemoved(int id, const char* title) override {
        std::cout << std::format("  [console_logger] task #{} '{}' removed\n", id, title);
    }
    void onTaskCompleted(int id) override {
        std::cout << std::format("  console_logger task #{} completed\n", id);
    }
    void onTaskReopened(int id) override {
        std::cout << std::format("  console_logger task #{} reopened\n", id);
    }
};

extern "C" {

int taskObserverApiVersion() { return TASK_OBSERVER_API_VERSION; }
ITaskObserver* createTaskObserver() { return new ConsoleLoggerObserver(); }
void destroyTaskObserver(ITaskObserver* p) { delete p; }

}