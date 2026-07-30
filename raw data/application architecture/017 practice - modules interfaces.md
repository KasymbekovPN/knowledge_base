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
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
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
project(module_interface CXX)  
  
set(CMAKE_CXX_STANDARD 23)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
# --- eventbus_lib: header-only библиотека с чётким публичным интерфейсом ---  
add_library(eventbus_lib INTERFACE)  
target_include_directories(eventbus_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/eventbus_lib/include)  
  
# --- task_observer_api: контракт между host и плагинами ---  
add_library(task_observer_api INTERFACE)  
target_include_directories(task_observer_api INTERFACE  
        ${CMAKE_CURRENT_SOURCE_DIR}/task_observer_api/include  
)  
  
# --- domain: Task/TaskRepository ---  
add_library(domain STATIC app/domain.cpp)  
target_include_directories(domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app)  
  
# --- commands: Command/CommandManager, зависит от domain и eventbus_lib ---  
add_library(commands STATIC app/commands.cpp)  
target_include_directories(commands PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app)  
target_link_libraries(commands PUBLIC domain eventbus_lib)  
  
# --- observer-плагины: MODULE, грузятся в рантайме, а не линкуются в host ---  
function(add_observer_plugin name)  
    add_library(${name} MODULE plugins/${name}/${name}.cpp)  
    target_link_libraries(${name} PRIVATE task_observer_api)  
    set_target_properties(${name} PROPERTIES  
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin  
            WINDOWS_EXPORT_ALL_SYMBOLS ON    )  
endfunction()  
  
add_observer_plugin(console_logger)  
add_observer_plugin(stats_logger)  
  
# --- host ---  
add_executable(app_host app/main.cpp)  
target_link_libraries(app_host PRIVATE commands task_observer_api ${CMAKE_DL_LIBS})  
set_target_properties(app_host PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```

### app/main.cpp
```cpp
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
    // их на EventBus, переводя типизированные C++ события в вызовы    // ABI-стабильного интерфейса ITaskObserver (int, const char*).    void attachObserver(ITaskObserver* observer) {  
        addedConn_.push_back(bus_.subscribe<TaskAdded>(  
            [observer](const TaskAdded& task) {  
                observer->onTaskAdded(task.id, task.title.c_str());  
            }));        removedConn_.push_back(bus_.subscribe<TaskRemoved>(  
            [observer](const TaskRemoved& task) {  
                observer->onTaskRemoved(task.id, task.title.c_str());  
            }));        completedConn_.push_back(bus_.subscribe<TaskCompleted>(  
            [observer](const TaskCompleted& task) {  
                observer->onTaskCompleted(task.id);  
            }));        reopenedConn_.push_back(bus_.subscribe<TaskReopened>(  
            [observer](const TaskReopened& task) {  
                observer->onTaskReopened(task.id);  
            }));    }  
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
    // плагина -> closeLibrary(). App с его EventBus умирает раньше, поэтому    // подписки Connection отключаются ДО выгрузки библиотек - порядок важен.    
    return 0;  
}
```

### app/plugin_loader.hpp
```cpp
// Кроссплатформенный загрузчик observer-плагинов - тот же platform::  
// слой (dlopen/LoadLibrary), что и в предыдущей практике по plugin  
// architecture, только теперь заточен под ITaskObserver.  
  
#pragma once  
#include <cstdint>  
#include <itask_observer.hpp>  
#include <iostream>  
#include <format>  
#include <memory>  
#include <string>  
  
#if defined(_WIN32)  
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>  
#else  
#include <dlfcn.h>  
#endif  
  
namespace platform {  
#if defined(_WIN32)  
    using LibraryHandle = HMODULE;  
    inline LibraryHandle openLibrary(const std::string& path) {  
        return ::LoadLibrary(path.c_str());  
    }    
    inline void* getSymbol(const LibraryHandle handle, const char* name) {  
        return reinterpret_cast<void*>(::GetProcAddress(handle, name));  
    }    
    inline void closeLibrary(const LibraryHandle handle) { ::FreeLibrary(handle); }  
    inline std::string lastError() {  
        const DWORD CODE = ::GetLastError();  
        return std::format("Win32 error code {}", CODE);  
    }    
    inline std::string pluginFileName(const std::string& baseName) { return std::format("{}.dll", baseName); }  
#else  
    using LibraryHandle = void*;  
    inline LibraryHandle openLibrary(const std::string& path) {  
        return ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);  
    }    
    inline void* getSymbol(LibraryHandle handle, const char* name) { return ::dlsym(handle, name); }  
    inline void closeLibrary(LibraryHandle handle) { ::dlclose(handle); }  
    inline std::string lastError() {  
        const char* err = ::dlerror();  
        return err ? err : "unknown error";  
    }    
    inline std::string pluginFileName(const std::string& baseName) { return "lib" + baseName + ".so"; }  
#endif  
  
    constexpr LibraryHandle K_INVALID_HANDLE{};  
};  
  
class LoadedObserver {  
public:  
    static std::unique_ptr<LoadedObserver> load(const std::string& path) {  
        const platform::LibraryHandle HANDLE{platform::openLibrary(path)};  
        if (HANDLE == platform::K_INVALID_HANDLE) {  
            std::cerr << std::format("Can't load {} : {}\n", path, platform::lastError());  
            return nullptr;  
        }  
        const auto GET_VERSION{reinterpret_cast<GetObserverApiVersionFn>(  
            platform::getSymbol(HANDLE, OBSERVER_API_VERSION_FN_NAME))};  
        const auto CREATE{reinterpret_cast<CreateObserverFn>(  
            platform::getSymbol(HANDLE, OBSERVER_CREATE_FN_NAME))};  
        const auto DESTROY{reinterpret_cast<DestroyObserverFn>(  
            platform::getSymbol(HANDLE, OBSERVER_DESTROY_FN_NAME))};  
  
        if (!GET_VERSION || !CREATE || !DESTROY) {  
            std::cerr << std::format("  {} : plugin does not export needed entry points\n", path);  
            platform::closeLibrary(HANDLE);  
            return nullptr;  
        }  
        if (const auto version{GET_VERSION()};  
            version != TASK_OBSERVER_API_VERSION ) {  
            std::cerr << std::format("  {} : API vertion mismatching [host = {}] [plugin = {}]\n]",  
                path, TASK_OBSERVER_API_VERSION, version);  
            platform::closeLibrary(HANDLE);  
  
            return nullptr;  
        }  
        return std::unique_ptr<LoadedObserver>(new LoadedObserver{HANDLE, CREATE(), DESTROY});  
    }  
    LoadedObserver(const LoadedObserver&) = delete;  
    LoadedObserver& operator=(const LoadedObserver&) = delete;  
    ~LoadedObserver() {  
        if (instance_) destroy_(instance_);  
        if (handle_ != platform::K_INVALID_HANDLE) platform::closeLibrary(handle_);  
    }  
    [[nodiscard]] ITaskObserver* get() const { return instance_; }  
  
private:  
    LoadedObserver(const platform::LibraryHandle handle, ITaskObserver* instance, const DestroyObserverFn destroy):  
        handle_(handle), instance_(instance), destroy_(destroy) {}  
  
    platform::LibraryHandle handle_{platform::K_INVALID_HANDLE};  
    ITaskObserver* instance_{nullptr};  
    DestroyObserverFn destroy_{nullptr};  
};
```

### app/domain.hpp
```cpp
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
```

### app/domain.cpp
```cpp
#include "domain.hpp"  
  
#include <algorithm>  
#include <ranges>  
#include <format>  
  
int TaskRepository::addTask(const std::string& title) {  
    const int ID{nextId_++};  
    tasks_.push_back({.id = ID, .title = title, .done = false});  
  
    return ID;  
}  
  
void TaskRepository::insertTaskAt(const std::size_t index, const Task& task) {  
    const auto POS{  
        tasks_.begin() +  
        static_cast<std::ptrdiff_t>(std::min(index, tasks_.size()))  
    };    
    tasks_.insert(POS, task);  
}  
  
std::pair<Task, std::size_t> TaskRepository::removeTask(const int id) {  
    const auto IT{std::ranges::find_if(  
        tasks_,  
        [id](const Task& task) { return task.id == id; })  
    };    
    if (IT == tasks_.end()) throw std::runtime_error{std::format("task not found: {}", id)};  
    const std::size_t INDEX{static_cast<std::size_t>(std::distance(tasks_.begin(), IT))};  
    Task copy = *IT;  
    tasks_.erase(IT);  
  
    return {copy, INDEX};  
}  
  
void TaskRepository::removeTaskById(const int id) {  
    const auto IT{std::ranges::find_if(  
        tasks_,  
        [id](const Task& task) { return task.id == id; })};  
    if (IT != tasks_.end()) tasks_.erase(IT);  
}  
  
void TaskRepository::setDone(const int id, const bool done) {  
    const auto IT{std::ranges::find_if(  
        tasks_,  
        [id](const Task& task) { return task.id == id; })};  
    if (IT != tasks_.end()) IT->done = done;  
}
```

### app/commands.hpp
```cpp
#pragma once  
  
#include "domain.hpp"  
#include <event_bus.hpp>  
#include <memory>  
#include <optional>  
#include <stack>  
#include <string>  
  
#include "domain.hpp"  
  
struct TaskAdded { int id; std::string title; };  
struct TaskRemoved { int id; std::string title; };  
struct TaskCompleted { int id; };  
struct TaskReopened { int id; };  
  
class ICommand {  
public:  
    virtual ~ICommand() = default;  
    virtual void execute() = 0;  
    virtual void undo() = 0;  
};  
  
class AddTaskCommand : public ICommand {  
public:  
    AddTaskCommand(TaskRepository& repo, EventBus& bus, std::string title);  
    void execute() override;  
    void undo() override;  
    int id() const { return id_.value(); }  
private:  
    TaskRepository& repo_;  
    EventBus& bus_;  
    std::string title_;  
    std::optional<int> id_;  
};  
  
class CompleteTaskCommand : public ICommand {  
public:  
    CompleteTaskCommand(TaskRepository& repo, EventBus& bus, int id);  
    void execute() override;  
    void undo() override;  
  
private:  
    TaskRepository& repo_;  
    EventBus& bus_;  
    int id_{0};  
};  
  
class RemoveTaskCommand : public ICommand {  
public:  
    RemoveTaskCommand(TaskRepository& repo, EventBus& bus, int id);  
    void execute() override;  
    void undo() override;  
  
private:  
    TaskRepository& repo_;  
    EventBus& bus_;  
    int id_{0};  
    Task removedTask_{};  
    std::size_t removedIndex_{0};  
};  
  
class CommandManager {  
public:  
    void execute(std::unique_ptr<ICommand> cmd);  
    bool undo();  
  
private:  
    std::stack<std::unique_ptr<ICommand>> undoStack_;  
};
```

### app/commands.cpp
```cpp
#include "commands.hpp"  
  
AddTaskCommand::AddTaskCommand(TaskRepository& repo, EventBus& bus, std::string title):  
    repo_{repo}, bus_{bus}, title_{std::move(title)} {}  
  
void AddTaskCommand::execute() {  
    if (!id_.has_value()) {  
        id_ = repo_.addTask(title_);  
    } else {  
        repo_.insertTaskAt(repo_.all().size(), Task{*id_, title_, false});  
    }    bus_.publish(TaskAdded{*id_, title_});  
}  
void AddTaskCommand::undo() {  
    bus_.publish(TaskRemoved{.id = *id_, .title = title_});  
    repo_.removeTaskById(*id_);  
}  
  
CompleteTaskCommand::CompleteTaskCommand(TaskRepository& repo, EventBus& bus, const int id):  
    repo_{repo}, bus_{bus}, id_{id} {}  
  
void CompleteTaskCommand::execute() {  
    repo_.setDone(id_, true);  
    bus_.publish(TaskCompleted{id_});  
}  
void CompleteTaskCommand::undo() {  
    repo_.setDone(id_, false);  
    bus_.publish(TaskReopened{id_});  
}  
  
RemoveTaskCommand::RemoveTaskCommand(TaskRepository& repo, EventBus& bus, const int id):  
    repo_{repo}, bus_{bus}, id_{id} {}  
  
void RemoveTaskCommand::execute() {  
    const auto [task, index] = repo_.removeTask(id_);  
    removedTask_ = task;  
    removedIndex_ = index;  
    bus_.publish(TaskRemoved{.id = id_, .title = task.title});  
}  
void RemoveTaskCommand::undo() {  
    repo_.insertTaskAt(removedIndex_, removedTask_);  
    bus_.publish(TaskRemoved{.id = removedTask_.id, .title = removedTask_.title});  
}  
  
void CommandManager::execute(std::unique_ptr<ICommand> cmd) {  
    cmd->execute();  
    undoStack_.push(std::move(cmd));  
}  
  
bool CommandManager::undo() {  
    if (undoStack_.empty()) return false;  
    const auto cmd{std::move(undoStack_.top())};  
    undoStack_.pop();  
    cmd->undo();  
  
    return true;  
}
```

### eventbus_lib/include/event_bus.hpp
```cpp
// eventbus_lib - отдельная переиспользуемая библиотека. Публичный API -  
// ровно этот заголовок, ничего больше. Не знает НИЧЕГО про задачи,  
// плагины или что угодно предметно-специфичное - универсальная типобезопасная  
// шина, которую можно унести в любой другой проект без изменений.  
  
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
            if (bus_) {  
                bus_->unsubscribeRaw(type_, id_);  
                bus_ = nullptr;  
            }        }    private:  
        friend class EventBus;  
        Connection(EventBus* bus, std::type_index type, SubscriptionId id)  
            : bus_(bus), type_(type), id_(id) {}  
        EventBus* bus_ = nullptr;  
        std::type_index type_ = typeid(void);  
        SubscriptionId id_ = 0;  
    };  
    template <typename EventT>  
    Connection subscribe(std::function<void(const EventT&)> handler) {  
        auto type = std::type_index(typeid(EventT));  
        SubscriptionId id = nextId_++;  
        handlers_[type].push_back(  
            {id, [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }});  
        return {this, type, id};  
    }  
    template <typename EventT>  
    void publish(const EventT& event) {  
        auto it = handlers_.find(std::type_index(typeid(EventT)));  
        if (it == handlers_.end()) return;  
        auto listCopy = it->second;  
        for (auto& [id, fn] : listCopy) fn(&event);  
    }private:  
    void unsubscribeRaw(const std::type_index type, const SubscriptionId id) {  
        const auto it{handlers_.find(type)};  
        if (it == handlers_.end()) return;  
        auto& list = it->second;  
        list.erase(  
            std::ranges::remove_if(list, [id](const auto& entry){ return entry.first == id; }).begin(),  
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

### task_observer_api/include/itask_observer.hpp
```cpp
// Контракт между host-приложением и плагинами-наблюдателями за задачами.  
// Сигнатуры сознательно используют только POD-типы (int, const char*),  
// а не std::string/std::vector - через границу dlopen/LoadLibrary лучше  
// не пересекать STL-типы, чей layout зависит от версии/настроек  
// стандартной библиотеки, с которой собран host, и может НЕ совпадать  
// с той, с которой собран плагин (особенно если это разные компиляторы  
// или версии компилятора). const char* - "наименьший общий знаменатель",  
// который одинаково понимают любые двое.  
  
#pragma once  
  
constexpr int TASK_OBSERVER_API_VERSION{1};  
  
class ITaskObserver {  
public:  
    virtual ~ITaskObserver() = default;  
    virtual void onTaskAdded(int id, const char* title) = 0;  
    virtual void onTaskRemoved(int id, const char* title) = 0;  
    virtual void onTaskCompleted(int id) = 0;  
    virtual void onTaskReopened(int id) = 0;  
};  
  
extern "C" {  
    using CreateObserverFn = ITaskObserver* (*)();  
    using DestroyObserverFn = void (*)(ITaskObserver*);  
    using GetObserverApiVersionFn = int (*)();  
}  
  
#define OBSERVER_CREATE_FN_NAME "createTaskObserver"  
#define OBSERVER_DESTROY_FN_NAME "destroyTaskObserver"  
#define OBSERVER_API_VERSION_FN_NAME "taskObserverApiVersion"
```

### plugins/stats_logger/stats_logger.cpp
```cpp
// Плагин 2: считает завершённые задачи (аналог CompletionStats).  
// Host о его существовании узнаёт только в рантайме - можно добавлять  
// новые observer-плагины, не трогая и не пересобирая host вообще.  
#include "itask_observer.hpp"  
  
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
```

### plugins/console_logger/console_logger.cpp
```cpp
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
    }};  
  
extern "C" {  
  
int taskObserverApiVersion() { return TASK_OBSERVER_API_VERSION; }  
ITaskObserver* createTaskObserver() { return new ConsoleLoggerObserver(); }  
void destroyTaskObserver(ITaskObserver* p) { delete p; }  
  
}
```

---


1. Библиотека `eventbus_lib` (тот же `EventBus`, но теперь отдельным модулем с чётким публичным заголовком):
2. Контракт плагина-наблюдателя (стабильный ABI, без STL-типов в сигнатурах — как обсуждали):
3. Два плагина:
4. Domain (`Task`/`TaskRepository`) как отдельный модуль:
5. Command'ы (используют `eventbus_lib`, работают через типизированные события):
6. Кроссплатформенный загрузчик плагинов (переиспользую `platform::` из прошлого раза):
7. `main.cpp` — composition root, мостик между типизированной `EventBus` и ABI-стабильным `ITaskObserver`:
8. `CMakeLists.txt` (корневой + под каждый модуль):Собираю и запускаю.

`eventbus_lib` вынесена отдельной библиотекой с единственным публичным заголовком — тот же код, что был у нас раньше зашит прямо в `task_app.cpp`, теперь переиспользуем без изменений. `task_observer_api/itask_observer.hpp` — новый ABI-контракт для наблюдателей, сознательно на POD-типах (`int`, `const char*`), а не `std::string`, чтобы плагин можно было собрать даже другим компилятором. `console_logger` и `stats_logger` — то, что раньше было `ActivityLog`/`CompletionStats` внутри одного файла, теперь два независимых `.so`, которые host находит и грузит в рантайме через уже проверенный кроссплатформенный `platform::` слой.

Ключевой архитектурный момент — в `App::attachObserver`: это мост между двумя разными мирами по обе стороны от plugin-границы — типизированными C++ событиями `EventBus` внутри host и стабильным ABI `ITaskObserver` снаружи. Именно host отвечает за перевод одного в другое, а не плагин — плагину не нужно ничего знать про `EventBus` вообще.
