// Composition root. Единственное место, где встречаются: библиотека
// eventbus_lib, domain/commands и
// динамическая загрузка observer-плагинов

#include "commands.hpp"
#include "domain.hpp"
#include "plugin_loader.hpp"

#include <iostream>
#include <format>
#include <vector>

class App {
public:
    // Плагины передаются как уже загруженные извне - App лишь подписывает
    // их на EventBus, переводя типизированные C++ события в вызовы
    // ABI-стабильного интерфейса ITaskObserver (int, const char*).
    void attachObserver(ITaskObserver* observer) {
        addedConn_.push_back(bus_.subscribe<TaskAdded>(
            [observer](const TaskAdded& task) {
                observer->onTaskAdded(task.id, task.title.c_str());
            }));
        removedConn_.push_back(bus_.subscribe<TaskRemoved>(
            [observer](const TaskRemoved& task) {
                observer->onTaskRemoved(task.id, task.title.c_str());
            }));
        completedConn_.push_back(bus_.subscribe<TaskCompleted>(
            [observer](const TaskCompleted& task) {
                observer->onTaskCompleted(task.id);
            }));
        reopenedConn_.push_back(bus_.subscribe<TaskReopened>(
            [observer](const TaskReopened& task) {
                observer->onTaskReopened(task.id);
            }));
    }

    int addTask(const std::string& title) {
        auto cmd{std::make_unique<AddTaskCommand>(repo_, bus_, title)};
        const AddTaskCommand* const raw{cmd.get()};
        commands_.execute(std::move(cmd));

        return raw->id();
    }

    void completeTask(const int id) {
        commands_.execute(std::make_unique<CompleteTaskCommand>(repo_, bus_, id));
    }

    void removeTask(const int id) {
        commands_.execute(std::make_unique<RemoveTaskCommand>(repo_, bus_, id));
    }

    void undo() { commands_.undo(); }

private:
    EventBus bus_;
    TaskRepository repo_;
    CommandManager commands_;
    std::vector<EventBus::Connection> addedConn_, removedConn_, completedConn_, reopenedConn_;
};

int main(const int argc, char *argv[]) {
    std::vector<std::string> plugin_paths;
    if (argc > 1) {
        plugin_paths.assign(argv + 1, argv + argc);
    } else {
        for (const auto& base: {"console_logger", "stats_logger"})
            plugin_paths.push_back(std::format("./{}", platform::pluginFileName(base)));
    }

    std::vector<std::unique_ptr<LoadedObserver>> observers;
    App app;

    std::cout << "--- observer-plugin load ---\n";
    for (const auto& path: plugin_paths) {
        std::cout << std::format("== {} ==\n", path);
        if (auto obs = LoadedObserver::load(path)) {
            app.attachObserver(obs->get());
            observers.push_back(std::move(obs));
        }
    }

    std::cout << "\n--- scenario ---\n";
    const int ID1{app.addTask("Architecture plan")};
    const int ID2{app.addTask("build eventbus_lib separately")};
    app.completeTask(ID2);
    app.removeTask(ID1);
    std::cout << "\n--- undo ---\n";
    app.undo();

    // observers уничтожаются здесь -> destroyTaskObserver() внутри каждого
    // плагина -> closeLibrary(). App с его EventBus умирает раньше, поэтому
    // подписки Connection отключаются ДО выгрузки библиотек - порядок важен.
    return 0;
}

