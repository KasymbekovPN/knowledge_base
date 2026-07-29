// Плагин 2: считает завершённые задачи (аналог CompletionStats).
// Host о его существовании узнаёт только в рантайме - можно добавлять
// новые observer-плагины, не трогая и не пересобирая host вообще.
#include "task_observer_api/itask_observer.hpp"

#include <iostream>
#include <format>

class StatsLogger: public ITaskObserver {
public:
    void onTaskAdded(int, const char*) override { ++totalAdded_; }
    void onTaskRemoved(int, const char*) override { --totalAdded_; }
    void onTaskCompleted(int id) override {
        ++completed_;
        std::cout << std::format(
            "  [stats_logger] completed {} from {} active(last #{})\n",
            completed_,
            totalAdded_,
            id);
    }
    void onTaskReopened(int) override {
        --completed_;
        std::cout << std::format(" [stats_logger] completed {} from {}",
            completed_,
            totalAdded_);
    }

private:
    int totalAdded_{0};
    int completed_{0};
};

extern "C" {

int taskObserverApiVersion() { return TASK_OBSERVER_API_VERSION; }
ITaskObserver* createTaskObserver() { return new StatsLogger(); }
void destroyTaskObserver(ITaskObserver* p) { delete p; }

}
