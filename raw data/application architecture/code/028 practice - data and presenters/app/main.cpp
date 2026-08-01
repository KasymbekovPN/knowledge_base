#include "config.hpp"
#include "eventbus.hpp"
#include "model.hpp"
#include "view.hpp"

#include <fstream>
#include <iostream>
#include <format>
#include <sstream>

// Загрузка "сида" начальных задач из внешнего CSV - то, что раньше было
// хардкодом в main() (`app.addTask("Написать план архитектуры")` и т.п.),
// теперь данные, а не код.
namespace {
    void loadSeedTasks(TaskRepository& repo, const std::string& path) {
        std::ifstream file{path};
        if (!file) {
            std::cerr << std::format("cannot open file '{}'\n", path);
            return;
        }

        std::string line;
        std::getline(file, line); // header

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::istringstream ss{line};
            std::string title, priority_s;
            std::getline(ss, title, ',');
            std::getline(ss, priority_s, ',');
            repo.addTask(title, std::stoi(priority_s));
            // repo.addTask() публикует TaskAdded -> View сама перерисуется -
            // даже начальное состояние доски идёт через тот же самый
            // событийный механизм, что и любое последующее изменение.
        }
    }
}

int main(const int argc, char *argv[]) {
    const std::string CONFIG_PATH{
        argc > 1
        ? argv[1]
        : "C:\\projects\\knowledge_base\\raw data\\application architecture\\code\\028 practice - data and presenters\\config\\app.cfg"
    };
    const std::string SEED_PATH{
        argc > 2
        ? argv[2]
        : "C:\\projects\\knowledge_base\\raw data\\application architecture\\code\\028 practice - data and presenters\\config\\seed_tasks.csv"
    };

    const auto config{Config::loadFromFile(CONFIG_PATH)};

    EventBus bus;
    TaskRepository repo{bus};
    // View подписывается на bus здесь
    const TaskBoardView view{repo, config, bus};

    std::cout << std::format("-- seed-task loading from {} --\n", SEED_PATH);
    // View уже показала начальное состояние сама
    loadSeedTasks(repo, SEED_PATH);

    std::cout << "-- task completed #2 --\n";
    repo.completeTask(2);

    std::cout << "-- add new task with high priority --\n";
    repo.addTask("NEW TASK ", 2);

    std::cout << "-- task removing #1 --\n";
    repo.removeTask(1);

    // Обратите внимание: main() ни разу не вызвал view.render() напрямую -
    // View сама решает, когда перерисоваться, реагируя на события Model.
    return 0;
}
