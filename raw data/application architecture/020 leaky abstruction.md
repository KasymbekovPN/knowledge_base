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
