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
project(abs_temp CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// Протекающая абстракция (leaky abstraction): интерфейс формально скрывает  
// реализацию, но детали конкретного backend'а всё равно "просачиваются"  
// наружу - через тип исключения или через то, что "геттер" на самом деле  
// отдаёт мутабельную ссылку на внутреннее состояние.  
  
#include <iostream>  
#include <format>  
#include <map>  
#include <memory>  
#include <stdexcept>  
#include <string>  
#include <vector>  
#include <ranges>  
  
struct User { int id; std::string name; };  
  
// ============================================================  
// Вариант 1 (протекающая абстракция)  
// ============================================================  
  
namespace leaky {  
  
    class IUserRepository {  
    public:  
        virtual ~IUserRepository() = default;  
        // какое исключение бросит при "не найдено"? не определено контрактом  
        virtual User findById(int id) = 0;  
        // "геттер", отдающий мутабельную ссылку на внутреннее хранилище  
        virtual std::vector<User>& getAllMutable() = 0;  
    };  
    // "SQL"-подобная реализация - симулируем типичную для конкретной БД ошибку  
    class SqlNotFoundError: public std::runtime_error {  
    public:  
        explicit SqlNotFoundError(const int id):  
            std::runtime_error(std::format("SQL: row not found, id = {}", id)) {}  
    };  
    class SqlUserRepository: public IUserRepository {  
    public:  
        SqlUserRepository() {  
            users_ = {{.id = 1, .name = "Pablo"}, {.id = 2, .name = "Anna"}};  
        }  
        User findById(const int id) override {  
            for (const auto& user : users_) if (user.id == id) return user;  
            // деталь конкретного backend'а "утекла" в публичный контракт  
            throw SqlNotFoundError(id);  
        }  
        std::vector<User>& getAllMutable() override { return users_; }  
  
    private:  
        std::vector<User> users_;  
    };  
    class InMemoryUserRepository: public IUserRepository {  
    public:  
        User findById(const int id) override {  
            const auto IT{users_.find(id)};  
            // ДРУГОЙ тип исключения!  
            if (IT == users_.end()) throw std::out_of_range(std::format("id not found = {}", id));  
  
            return IT->second;  
        }  
        std::vector<User>& getAllMutable() override {  
            cache_.clear();  
            for (const auto& u: users_ | std::views::values) cache_.push_back(u);  
            // мутация этой ссылки НЕ трогает users_ - поведение расходится с SqlUserRepository  
            return cache_;  
        }  
    private:  
        std::map<int, User> users_ = {{1, {.id = 1, .name = "Pablo"}}};  
        std::vector<User> cache_;  
    };  
    void demo() {  
        std::cout << "--- leaky ---\n";  
  
        // Проблема 1: клиентский код, написанный и протестированный против SQL...  
        std::unique_ptr<IUserRepository> repo = std::make_unique<SqlUserRepository>();  
        try {  
            repo->findById(999);  
        } catch (const SqlNotFoundError& e) {  
            std::cout << std::format("  [sql] caught waited exception: {}\n", e.what());  
        }  
        // ...а теперь backend поменяли на InMemory - тот же catch-блок молча перестаёт ловить  
        repo = std::make_unique<InMemoryUserRepository>();  
        try {  
            repo->findById(999);  
        } catch (const SqlNotFoundError& e) {  
            std::cout << std::format("  [InMemory] caught (SQL): \n", e.what());  
        } catch (const std::exception& e) {  
            std::cout << std::format("  [InMemory] did not catch wiated type: {}\n", e.what());  
        }  
        // Проблема 2: "просто геттер" на деле отдаёт мутабельную ссылку на источник правды  
        repo = std::make_unique<SqlUserRepository>();  
        std::cout << std::format("  [sql] size before: {}\n", repo->getAllMutable().size());  
        // выглядит как безобидное чтение, а на деле - мутация состояния  
        repo->getAllMutable().clear();  
        std::cout << std::format("  [sql] size after getAllMutable().clear(): {}\n", repo->getAllMutable().size());  
    }  
}  
  
// ============================================================  
// Вариант 2: интерфейс сам объявляет доменный контракт ошибок и владения  
// ============================================================  
namespace clean {  
    // Интерфейс объявляет СВОЁ исключение - любая реализация обязана  
    // переводить в него специфичные для backend'а ошибки на границе.    class UserNotFoundException: public std::runtime_error {  
    public:  
        explicit UserNotFoundException(const int id):  
            std::runtime_error(std::format("User not found id: = {}", id)) {}  
    };  
    class IUserRepository {  
    public:  
        virtual ~IUserRepository() = default;  
        // контракт: бросает ТОЛЬКО UserNotFoundException  
        virtual User findById(int id) = 0;  
        // ПО ЗНАЧЕНИЮ - внутреннее хранилище недостижимо снаружи  
        virtual std::vector<User> getAll() const = 0;  
    };  
    class SqlUserRepository: public IUserRepository {  
    public:  
        SqlUserRepository() { users_ = {{.id = 1, .name = "Pablo"}, {.id = 2, .name = "Anna"}}; }  
        User findById(const int id) override {  
            for (auto& u: users_) if (u.id == id) return u;  
            // SQL-специфичная ошибка переведена в доменную ЗДЕСЬ, на границе  
            throw UserNotFoundException(id);  
        }  
        // copy  
        std::vector<User> getAll() const override { return users_; }  
  
    private:  
        std::vector<User> users_;  
    };  
    class InMemoryUserRepository: public IUserRepository {  
    public:  
        User findById(const int id) override {  
            const auto IT{users_.find(id)};  
            if (IT == users_.end()) throw UserNotFoundException(id);  
  
            return IT->second;  
        }  
        std::vector<User> getAll() const override {  
            std::vector<User> result;  
            for (const auto& u: users_ | std::views::values) result.push_back(u);  
  
            return result;  
        }  
    private:  
        std::map<int, User> users_ = {{1, {.id = 1, .name = "Pablo"}}};  
    };  
    static void testFindNotFound(const std::string& label, const std::unique_ptr<IUserRepository>& repo) {  
        try {  
            repo->findById(999);  
        } catch (const UserNotFoundException& e) {  
            std::cout << std::format("  [{}] caught (one type, independently form backend): {}\n", label, e.what());  
        }    }  
    void demo() {  
        std::cout << "--- clean ---\n";  
  
        testFindNotFound("Sql", std::make_unique<SqlUserRepository>());  
        testFindNotFound("InMemory", std::make_unique<InMemoryUserRepository>());  
  
        const auto repo{std::make_unique<SqlUserRepository>()};  
        auto users{repo->getAll()};  
        // это ЛОКАЛЬНАЯ копия - внутреннее состояние repo не тронуто  
        users.clear();  
        std::cout << std::format("  repo->getAll().size() = {}\n", repo->getAll().size());  
    }  
}  
  
int main() {  
    leaky::demo();  
    clean::demo();  
  
    return 0;  
}
```

**Что такое протекающая абстракция**

Термин Джоэла Спольски: абстракция обещает скрыть детали реализации, но эти детали всё равно "просачиваются" наружу — через сообщения об ошибках, через побочные эффекты, через названия методов, через гарантии производительности, которые фактически зависят от того, чем интерфейс реализован. Формально контракт соблюдён (компилятор доволен, интерфейс реализован полностью), но по факту вызывающий код вынужден знать про конкретный backend, чтобы работать правильно — а это и есть провал самой идеи абстракции.

**Проблема 1 из демо: тип исключения — часть реализации, а не контракта**

`SqlUserRepository::findById()` бросает `SqlNotFoundError`, `InMemoryUserRepository::findById()` — `std::out_of_range`. Формально оба честно реализуют `IUserRepository`, оба документированы как "функция, которая может не найти пользователя" — но клиентский код, написанный и протестированный против одной реализации, при подмене backend'а перестаёт ловить исключение **молча**: программа не падает при компиляции, просто нужный `catch` больше не срабатывает, и ошибка утекает выше по стеку туда, где её никто не ожидал. Тест это показал буквально — тот же самый `catch (const SqlNotFoundError&)`, который прекрасно работал для `SqlUserRepository`, стал бесполезным после подмены на `InMemoryUserRepository`.

Решение — интерфейс должен объявлять **свой собственный** словарь ошибок (`UserNotFoundException`), и каждая реализация обязана транслировать backend-специфичную ошибку в доменную ровно на границе, внутри своего метода — снаружи никто никогда не видит, что там на самом деле было: SQL-исключение, отсутствие ключа в `std::map`, таймаут сети. Это прямое продолжение темы DIP: домен владеет не только интерфейсом операций, но и интерфейсом ошибок этих операций.

**Проблема 2: "геттер" — это не всегда просто чтение**

`getAllMutable()` возвращает `std::vector<User>&` — синтаксически выглядит как безобидный доступ на чтение, а по факту отдаёт прямую ссылку на внутреннее хранилище. Тест показал: `repo->getAllMutable().clear()` реально и необратимо стёр данные из `SqlUserRepository` — притом что имя метода звучит как "получить всех", а не "получить доступ на запись". Хуже того: `InMemoryUserRepository` в том же методе отдаёт ссылку на пересобираемый кэш, а не на реальное хранилище — то есть одна и та же сигнатура интерфейса ведёт себя по-разному в зависимости от backend'а: где-то мутация ссылки разрушает состояние навсегда, где-то — не имеет долгосрочного эффекта. Это ещё опаснее чистого "дырявого" геттера — расхождение семантики между реализациями одного и того же интерфейса.

Решение простое и уже встречавшееся раньше (в примере с `CompletionStats`/`TaskRepository`): отдавать данные **по значению** (`std::vector<User> getAll() const`) там, где нет веской причины отдавать ссылку, либо, если производительность копирования критична, — возвращать `const&`, а не мутабельную `&`, явно запрещая мутацию через саму сигнатуру типа.

**Общие правила проектирования против протечки**

Называть методы в терминах поведения предметной области, а не механизма реализации — `save()`, а не `writeToFile()`; `find()`, а не `runQuery()` — если название метода выдаёт, чем он реализован, при смене реализации либо название станет враньём, либо придётся его менять, ломая всех потребителей. Не отдавать наружу мутабельный доступ к внутреннему состоянию без явной, осознанной причины — та же логика, что была в теме ownership: если объект должен быть источником правды, ничто снаружи не должно случайно испортить его инвариант через то, что выглядело как "просто чтение". И — то, что язык не может проверить компилятором, но стоит фиксировать явно в документации интерфейса — гарантии сложности и поведения (например, если `find()` у одной реализации O(1), а у другой O(n), и интерфейс никак это не оговаривает, кто-то обязательно напишет код, который был бы приемлем для одной реализации и станет квадратичным при подмене на другую).

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
- [ ] Type erasure (`std::function`, `std::any`, кастомные обёртки) как способ дать полиморфизм без наследования.
- [ ] Versioning интерфейсов в долгоживущем проекте (что делать, когда интерфейс нужно менять, а есть внешние потребители).
- [ ] Практика: взять один "толстый" класс из пет-проекта и разбить на 2–3 узких интерфейса.

Материалы:

- Sean Parent, "Inheritance Is The Base Class of Evil" (talk) — про value semantics и type erasure.
- Herb Sutter, статьи про интерфейсы и pimpl.

---

## Неделя 3: Данные, представление, закрепление

### День 9–10. Отделение данных от представления

Что разобрать:

- [ ] MVC / MVP / MVVM — различия, где какой уместен (десктоп GUI, игровой UI, сервер).
- [ ] Модель как источник истины, не знающая о UI; view подписывается на изменения (снова Observer/сигналы).
- [ ] Data-Driven design: конфиги/данные вместо хардкода логики (актуально и для геймдева, и для enterprise).
- [ ] Сериализация как граница слоя данных (DTO vs доменная модель — не смешивать).
- [ ] Immutable data / value objects — почему это упрощает рассуждение о состоянии приложения.
- [ ] Практика: в пет-проекте выделить Model отдельно от View, связать через сигналы/observer; данные, которые сейчас захардкожены, вынести в конфиг.

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