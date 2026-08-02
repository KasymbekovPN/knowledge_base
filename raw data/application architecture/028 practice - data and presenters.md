---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            
	        "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        },        
        {            
	        "name": "release",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release"  
        }  
    ],    
    "testPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "output": {  
                "outputOnFailure": true  
            }  
        }    
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.40)  
project(task_board CXX)  
  
add_executable(task_board app/config.cpp app/main.cpp)  
target_include_directories(task_board PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)  
target_compile_features(task_board PUBLIC cxx_std_23)
```

### app/main.cpp
```cpp
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
```

### app/config.hpp
```cpp
#pragma once  
  
#include <string>  
#include <vector>  
  
// Config - то, что раньше было захардкожено (заголовок приложения, подписи  
// приоритетов) вынесено во внешний app.cfg. Ничего из этого не требует  
// пересборки при изменении - см. data-driven demo с enemies.csv.  
class Config {  
public:  
    static Config loadFromFile(const std::string& path);  
  
    std::string priorityLabel(int priority);  
  
    const std::string& appTitle() const { return appTitle_; }  
    const std::vector<std::string>& priorityLabels() const { return priorityLabels_; }  
  
    // priority хранится как int (0..N-1) в домене - Config отвечает только  
    // за перевод его в человекочитаемую подпись для View.    
    std::string priorityLabel(int priority) const;  
  
private:  
    std::string appTitle_{"Task board"};  
    std::vector<std::string> priorityLabels_ = {"Low", "Medium", "High"};  
};
```

### app/config.cpp
```cpp
#include "config.hpp"  
  
#include <format>  
#include <fstream>  
#include <sstream>  
#include <stdexcept>  
  
namespace {  
    std::vector<std::string> splitCsvList(const std::string& s) {  
        std::istringstream ss{s};  
        std::string item;  
        std::vector<std::string> result;  
        while (std::getline(ss, item, ',')) result.push_back(item);  
  
        return result;  
    }}  
  
  
Config Config::loadFromFile(const std::string& path) {  
    Config cfg;  
    std::ifstream file{path};  
    if (!file) throw std::runtime_error{std::format("Cannot open file: {}", path)};  
  
    std::string line;  
    while (std::getline(file, line)) {  
        if (line.empty() || line[0] == '#') continue;  
        const auto EQ{line.find('=')};  
        if (EQ == std::string::npos) continue;  
        const std::string KEY{line.substr(0, EQ)};  
        const std::string VALUE{line.substr(EQ + 1)};  
  
        if (KEY == "app_title") cfg.appTitle_ = VALUE;  
        else if (KEY == "priority_labels") cfg.priorityLabels_ = splitCsvList(VALUE);  
        // неизвестные ключи молча игнорируются - конфиг может расти,  
        // не ломая старые сборки движка (та же идея, что versioning интерфейсов)    
    }  
  
    return cfg;  
}  
  
std::string Config::priorityLabel(const int priority) {  
    return priority < 0 || static_cast<size_t>(priority) >= priorityLabels_.size()  
        ? "?"  
        : priorityLabels_[priority];  
}
```

### app/eventbus.hpp
```cpp
#pragma once  
  
#include <algorithm>  
#include <functional>  
#include <typeindex>  
#include <unordered_map>  
#include <vector>  
#include <ranges>  
  
class EventBus {  
public:  
    using SubscriptionId = std::size_t;  
  
    class Connection {  
    public:  
        Connection() = default;  
        Connection(Connection&& other) noexcept { *this = std::move(other); }  
        Connection& operator=(Connection&& other) noexcept {  
            if (this != &other) {  
                disconnect();  
                bus_ = other.bus_;  
                type_ = other.type_;  
                id_ = other.id_;  
                other.bus_ = nullptr;  
            }            return *this;  
        }        Connection(const Connection&) = delete;  
        Connection& operator=(const Connection&) = delete;  
        ~Connection() { disconnect(); }  
  
        void disconnect() {  
            if (!bus_) return;  
            bus_->unsubscribeRaw(type_, id_);  
            bus_ = nullptr;  
        }  
    private:  
        friend class EventBus;  
        Connection(EventBus* bus, const std::type_index type, const SubscriptionId id):  
            bus_{bus}, type_{type}, id_{id} {}  
  
        EventBus* bus_{nullptr};  
        std::type_index type_{typeid(void)};  
        SubscriptionId id_{0};  
    };  
    template<typename EventT>  
    Connection subscribe(std::function<void(const EventT&)> handler) {  
        const auto TYPE{std::type_index(typeid(EventT))};  
        const SubscriptionId ID{nextId_++};  
        handlers_[TYPE].push_back({  
            ID,  
            [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }  
        });  
        return {this, TYPE, ID};  
    }  
    template<typename EventT>  
    void publish(const EventT& event) {  
        const auto IT{handlers_.find(std::type_index(typeid(EventT)))};  
        if (IT == handlers_.end()) return;  
        auto listCopy = IT->second;  
        for (const auto& fn: listCopy | std::views::values) fn(&event);  
    }  
private:  
    void unsubscribeRaw(const std::type_index type, const SubscriptionId id) {  
        const auto IT{handlers_.find(type)};  
        if (IT == handlers_.end()) return;  
        auto& list = IT->second;  
        list.erase(  
            std::ranges::remove_if(list, [id](const auto& entry) { return entry.first == id; }).begin(),  
            list.end());  
    }  
    std::unordered_map<  
        std::type_index,  
        std::vector<std::pair<  
            SubscriptionId,  
            std::function<void(const void*)>>>  
    > handlers_;  
    SubscriptionId nextId_{1};  
};
```

### app/model.hpp
```cpp
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
```

### app/view.hpp
```cpp
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
```

### config/app.cfg
```cfg
app_title=My Task Board  
priority_labels=Low,Medium,High
```

### config/task_seeds.csv
```csv
title,priority  
Написать план архитектуры,2  
Собрать event bus,1  
Перевести действия в команды,0
```

1. Конфиги (данные — не код)
2. `Config` — загрузчик данных, отдельный от логики
3. `Model` (домен, ничего не знает о View)
4. `EventBus` (переиспользуем без изменений)
5. `View` — только подписка, ни одного прямого вызова со стороны Model
6. `main.cpp` — composition root, грузит конфиг и сид-данные, ни разу не вызывает `View` напрямую

`View` перерисовывается сама на каждое событие, заголовок и подписи приоритетов — из `app.cfg`, начальные задачи — из `seed_tasks.csv`. 

Кратко по архитектуре: `TaskRepository` (Model) публикует события в своих же методах мутации и ни разу не ссылается на `TaskBoardView`; `View` подписывается сама в конструкторе и перерисовывается по любому событию — `main()` ни разу не вызывает `view.render()` напрямую, даже начальный сид идёт через тот же событийный путь, что и обычные изменения.
