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


---
## Неделя 1: Ядро — event loop и управление объектами

### День 1–2. Event loop и его виды

Что разобрать:

- [x] Зачем нужен event loop: разница между блокирующим последовательным кодом и реактивным приложением (GUI, сеть, игровой цикл). (2026.07.26)
- [x] Модели: single-threaded loop (Node.js-стиль), reactor pattern (select/poll/epoll в основе), proactor pattern (async I/O, Windows IOCP), multi-reactor / thread-per-core (nginx, Seastar). (2026.07.26)
- [x] Как устроены реальные реализации: Qt event loop (QEventLoop, сигналы/слоты как очередь событий), boost::asio::io_context (async_*, strand для потокобезопасности), libuv (событие в основе Node.js), игровой game loop (fixed timestep vs variable). (2026.07.26)
- [x] Таймеры, очереди задач, приоритеты событий, "starvation" одних обработчиков другими. (2026.07.26)
- [x] Практика: написать свой мини event loop на boost::asio — обработка нескольких таймеров + сокетов в одном потоке. (2026.07.26)

Материалы:

- Boost.Asio docs + примеры (asynchronous model, `io_context::run`).
- Статья/исходники "Reactor pattern" (оригинал — Douglas C. Schmidt, ACE framework).
- Исходники Qt: `QEventLoop`, `QAbstractEventDispatcher` (посмотреть, не обязательно вчитываться построчно).

### День 3–4. Системное управление функциями приложения и объектами

Что разобрать:

- [x] Жизненный цикл объектов: кто владеет (ownership) — `unique_ptr`/`shared_ptr`/raw + RAII как основа управления ресурсами. (2026.07.26)
- [x] Command pattern — обёртка действий приложения в объекты (undo/redo, очереди команд, макросы). (2026.07.27)
- [x] Mediator / Event bus (Observer поверх глобальной шины) — как компоненты общаются, не зная друг о друге напрямую. (2026.07.27)
- [x] Dependency Injection vs Service Locator — плюсы/минусы, почему DI обычно предпочтительнее для тестируемости. (2026.07.27)
- [x] Signal/Slot (Qt-стиль) как частный случай Observer, применимость вне Qt (boost::signals2). (2026.07.27)
- [x] Application/Context объект — единая точка доступа к сервисам приложения (но осторожно с антипаттерном "God Object"). (2026.07.27)
- [x] Практика: в пет-проекте выделить `Application`/`Context`, завести event bus, перевести 2–3 действия в Command-объекты. (2026.07.28)

Материалы:

- GoF: главы Command, Mediator, Observer.
- Статьи про Dependency Injection в C++ (без фреймворков — через конструкторы и интерфейсы).

---

## Неделя 2: Модульность и интерфейсы

### День 5–6. Модульность и расширяемость

Что разобрать:

- [x] SOLID применительно к C++ (особенно Dependency Inversion — от него зависит вся модульность). (2026.07.28)
- [x] Разделение на слои/модули: как резать приложение на независимо собираемые куски (библиотеки), API между модулями. (2026.07.28)
- [x] Plugin architecture: динамическая загрузка (`dlopen`/`LoadLibrary`), абстрактный интерфейс плагина, versioning, factory-функция как точка входа. (2026.07.28)
- [x] pImpl idiom — сокрытие деталей реализации, стабильность ABI, ускорение пересборки. (2026.07.28)
- [x] Component-based design / ECS (Entity-Component-System) — альтернатива классической иерархии наследования для расширяемости (актуально для игр/симуляций, но принцип полезен шире). (2026.07.29)
- [x] Практика: вынести один модуль пет-проекта в отдельную библиотеку с чётким интерфейсом; по желанию — сделать простой плагин через `dlopen/LoadLibrary`. (2026.07.30)

Материалы:

- Исходники LLVM или Chromium (структура модулей, интерфейсы между компонентами) — просто полистать оглавление/README архитектуры.
- Статьи про pImpl и binary compatibility в C++.

### День 7–8. Проектирование интерфейсов

Что разобрать:

- [x] Абстрактные классы vs шаблоны (compile-time polymorphism) — когда что выбирать, стоимость виртуальных вызовов. (2026.07.30)
- [x] Интерфейсная сегрегация (ISP) — узкие интерфейсы вместо одного "толстого". (2026.07.30)
- [x] Как проектировать интерфейс так, чтобы он не тянул за собой детали реализации (не протекающая абстракция). (2026.07.30)
- [x] Type erasure (`std::function`, `std::any`, кастомные обёртки) как способ дать полиморфизм без наследования. (2026.07.31)
- [x] Versioning интерфейсов в долгоживущем проекте (что делать, когда интерфейс нужно менять, а есть внешние потребители). (2026.07.31)
- [x] Практика: взять один "толстый" класс из пет-проекта и разбить на 2–3 узких интерфейса.

Материалы:

- Sean Parent, "Inheritance Is The Base Class of Evil" (talk) — про value semantics и type erasure.
- Herb Sutter, статьи про интерфейсы и pimpl.



---

## Неделя 3: Данные, представление, закрепление

### День 9–10. Отделение данных от представления

Что разобрать:

- [x] MVC / MVP / MVVM — различия, где какой уместен (десктоп GUI, игровой UI, сервер). (2026.07.31)
- [x] Модель как источник истины, не знающая о UI; view подписывается на изменения (снова Observer/сигналы).  (2026.07.31)
- [x] Data-Driven design: конфиги/данные вместо хардкода логики (актуально и для геймдева, и для enterprise). (2026.07.31)
- [x] Сериализация как граница слоя данных (DTO vs доменная модель — не смешивать). (2026.07.31)
- [x] Immutable data / value objects — почему это упрощает рассуждение о состоянии приложения. (2026.08.01)
- [x] Практика: в пет-проекте выделить Model отдельно от View, связать через сигналы/observer; данные, которые сейчас захардкожены, вынести в конфиг. (2026.08.01)

Материалы:

- Статьи про MVVM в контексте Qt (QML + C++ backend — хороший наглядный пример).
- Martin Fowler, "Patterns of Enterprise Application Architecture" (главы про MVC/Presentation Model — общие принципы применимы и к C++).

### День 11–12. Собрать всё вместе

- [ ] Свести пет-проект в цельную архитектуру: event loop на входе → команды/события через bus → модули с чёткими интерфейсами → модель отдельно от представления.
- [ ] Нарисовать диаграмму компонентов своего проекта (от руки или в draw.io) — на собеседовании часто просят объяснять архитектуру именно так.
- [ ] Сформулировать для себя ответы на вопросы: "почему выбрал такой event loop", "как добавить новый модуль без изменения старых", "как тестировать логику отдельно от UI".

### День 13–14. Подготовка к собеседованию

- [ ] Прогнать типичные вопросы: разница Observer/Mediator, когда DI лучше Service Locator, что такое pImpl и зачем, reactor vs proactor, как обеспечить модульность в C++ без фреймворков, SOLID на примерах из C++.
- [ ] Порешать 1–2 "system design"-задачи в духе "спроектируй архитектуру X" (например: текстовый редактор с undo/redo, торговый терминал, игровой движок верхнего уровня) — на бумаге/в голове, 30–40 минут на каждую.
- [ ] Повторить свой пет-проект как готовый кейс — уметь за 3–5 минут рассказать архитектуру.

---

## Итог по неделям

- Неделя 1: event loop + управление объектами/командами → фундамент.
- Неделя 2: модульность, интерфейсы → как резать систему на части.
- Неделя 3: данные/представление + сборка воедино → законченная архитектура и готовность к собеседованию.

## Что получится в результате

- Понимание принципов построения архитектуры приложений.
- Навык управления логикой и взаимодействием компонентов (event bus, DI, команды).
- Умение проектировать гибкий, расширяемый код (модули, интерфейсы, plugin-подход).
- Практика разделения данных, логики и представления (MVC/MVVM, data-driven подход).
- Готовый пет-проект как кейс для собеседования + прогнанные типовые вопросы.