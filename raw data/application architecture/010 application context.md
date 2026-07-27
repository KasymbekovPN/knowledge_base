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
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(app CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// Application/Context: два способа собрать одно и то же приложение.  
// bad_god_object - Context как глобальная точка доступа "достань что угодно".  
// good_composition_root - Context как единственное место, где граф объектов  
// СОБИРАЕТСЯ, а дальше зависимости расходятся по конструкторам явно (DI).  
  
#include <iostream>  
#include <format>  
#include <memory>  
#include <string>  
  
// =====================================================================  
// Сервисы приложения - общие для обоих вариантов.  
// =====================================================================  
class Logger {  
public:  
    void log(const std::string& msg) {  
        std::cout << std::format("  [log] {}\n", msg);  
    }};  
  
class DataBase {  
public:  
    void save(const std::string& record) {  
        std::cout << std::format("  [db] save: {}\n", record);  
    }};  
  
class EventBusStub {  
public:  
    void publish(const std::string& event) {  
        std::cout << std::format("  [publish] {}\n", event);  
    }};  
  
// =====================================================================  
// Вариант 1 (антипаттерн): Context как глобальный God Object.  
// Любой модуль в программе может дотянуться до AppContext::instance()  
// и взять оттуда что захочет - Logger, Database, EventBus, да что угодно.  
// =====================================================================  
namespace bad_god_object {  
    class AppContext {  
    public:  
        static AppContext& instance() {  
            static AppContext ctx;  
            return ctx;  
        }  
        Logger& logger() { return logger_; }  
        DataBase& database() { return database_; }  
        EventBusStub& bus() { return bus_; }  
  
    private:  
        Logger logger_;  
        DataBase database_;  
        EventBusStub bus_;  
    };  
    // OrderService нигде явно не говорит, что ему нужны Logger/Database/EventBus -  
    // он просто тянет их из глобального контекста изнутри метода.    
    class OrderService {  
    public:  
        void placeOrder(const std::string& item) {  
            auto& ctx = AppContext::instance();  
            ctx.logger().log(std::format("placing order: {}", item));  
            ctx.database().save(item);  
            ctx.bus().publish(std::format("OrderPlaced: {}", item));  
            // Со временем сюда так же незаметно добавляются ctx.config(),  
            // ctx.network(), ctx.cache() - сигнатура класса не меняется,            
            // а его реальные зависимости растут бесконтрольно.        }  
    };  
    void run() {  
        std::cout << "--- bad_good_object ---\n";  
  
        // конструктор ничего не говорит о зависимостях  
        OrderService service;  
        service.placeOrder("gadget");  
    }}  
  
// =====================================================================  
// Вариант 2: Context как composition root. Строит граф объектов ОДИН РАЗ,  
// а дальше каждый модуль получает только то, что ему реально нужно,  
// через конструктор - явно, узко, проверяемо компилятором.  
// =====================================================================  
namespace good_composition_root {  
    // OrderService зависит только от Logger& и Database& - именно от того,  
    // что реально использует. Не от "всего Context целиком".    
    class OrderService {  
    public:  
        OrderService(Logger& logger, DataBase& db) : logger_{logger}, db_ {db} {}  
  
        void placeOrder(const std::string& item) {  
            logger_.log(std::format("placing order: {}", item));  
            db_.save(item);  
        }    private:  
        Logger& logger_;  
        DataBase& db_;  
    };  
    // NotificationService зависит только от EventBus - Logger и Database  
    // ему вообще не видны, даже потенциально.    
    class NotificationService {  
    public:  
        explicit NotificationService(EventBusStub& bus) : bus_{bus} {}  
        void notifyOrderPlaced(const std::string& item) {  
            bus_.publish(std::format("OrderPlaced: {}", item));  
        }    private:  
        EventBusStub& bus_;  
    };  
    // App - единственное место, где всё это собирается вместе. Не синглтон,  
    // создаётся один раз в main(), живёт на стеке (RAII, как в наших    // предыдущих примерах с io_context).    
    class App {  
    public:  
        App():  
            orderService_{logger_, database_},  
            notificationService_{bus_} {}  
  
        void placeOrder(const std::string& item) {  
            orderService_.placeOrder(item);  
            notificationService_.notifyOrderPlaced(item);  
        }  
    private:  
        // Владение сервисами - по значению, дерево, как обсуждали в теме RAII/ownership.  
        Logger logger_;  
        DataBase database_;  
        EventBusStub bus_;  
  
        // Модули получают ссылки на сервисы через конструктор при инициализации -  
        // порядок объявления полей здесь важен (инициализируются сверху вниз).        
        OrderService orderService_;  
        NotificationService notificationService_;  
    };  
    void run() {  
        std::cout << "-- good_composition_root --\n";  
        App app;  // единственное место сборки графа объектов за всю программу  
        app.placeOrder("widget");  
    }  
}  
  
int main() {  
    bad_god_object::run();  
    std::cout << "\n";  
    good_composition_root::run();  
  
    return 0;  
}
```

Оба варианта дают одинаковый результат — но структурно они принципиально разные.

**Идея Application/Context**

Любому нетривиальному приложению нужна точка, где создаются и связываются основные подсистемы — логгер, БД, сеть, event bus, конфиг. Вопрос архитектуры не в том, нужен ли такой объект (нужен почти всегда), а в том, **как модули получают доступ к тому, что в нём лежит**. Ровно здесь и расходятся два варианта из примера.

**`bad_god_object`: Context как глобальная точка "достань что угодно"**

`AppContext::instance()` — это `Service Locator`, который мы уже разбирали, просто под другим именем. `OrderService::placeOrder()` тянет `logger()`, `database()`, `bus()` прямо изнутри метода — сигнатура класса (`OrderService()` без параметров) ничего не говорит о реальных зависимостях. Проблема растёт со временем незаметно: сегодня понадобился `ctx.config()`, завтра `ctx.network()`, послезавтра `ctx.cache()` — и всё это добавляется без единого изменения в объявлении класса, то есть без единого сигнала "я стал сложнее". Через полгода любой `OrderService`-подобный класс потенциально знает про весь `AppContext` целиком, хотя реально использует три метода из двадцати.

Отсюда и название "God Object": сам `Context` в этой модели не обязательно разрастается монструозно сам по себе, но он становится **скрытой точкой связанности всего со всем** — теоретически любой модуль приложения может добраться до любого другого через `instance()`, и это разрушает саму идею модульности из следующего блока плана: границы между подсистемами перестают быть архитектурным решением и становятся просто "что кто-то не успел вызвать".

**`good_composition_root`: Context как единственное место сборки**

`App` в примере — не синглтон и не имеет `instance()`. Это обычный объект, создаваемый один раз в `main()` (в примере — внутри `run()`, но по сути composition root), который **строит** граф зависимостей и раздаёт их конструкторам — `OrderService` получает ровно `Logger&` и `Database&`, `NotificationService` — ровно `EventBusStub&`. Ни один из модулей не видит `App` целиком и не может дотянуться до сервисов, которые ему не передали явно. Это прямое продолжение темы DI из предыдущего сообщения: `App` — это и есть composition root, то самое "одно место", куда стоит выносить сложность сборки объектного графа, вместо того чтобы размазывать `resolve<T>()` по всей кодовой базе.

Владение здесь тоже соответствует правилу из блока про RAII: `Logger`, `Database`, `bus_` — поля по значению, дерево владения растёт от `App` вниз, `OrderService`/`NotificationService` держат только заимствованные ссылки, не владение.

**Практические последствия, которые стоит проговаривать явно**

Помимо тестируемости (уже разобрали на примере DI vs Locator — здесь ровно то же самое: `OrderService(logger, database)` можно протестировать с fake-объектами напрямую, `OrderService` из `bad_god_object` — только через глобальный `AppContext::instance()`), есть ещё один практический эффект, который часто упускают: **скорость сборки**. Если `Context`/`AppContext` — это один заголовочный файл, который знает про все подсистемы приложения, то любое изменение в любой подсистеме (даже добавление одного метода в `Database`) заставляет пересобираться каждый файл, который инклудит `AppContext.h`, — а инклудят его отовсюду, потому что это же "точка доступа ко всему". В "хорошем" варианте `OrderService.h` инклудит только `Logger.h` и `Database.h` — изменение в `EventBusStub` его вообще не касается. Это прямой мостик к следующей теме плана — модульности и pImpl, где скорость пересборки и явные границы компиляции — не побочный эффект, а часть архитектурного решения.

**Где глобальный доступ всё же терпим**

Не всё нужно тащить через DI любой ценой — для вещей вроде логирования, которые по природе своей нужны буквально везде и не являются "бизнес-зависимостью" (использование логгера не влияет на корректность бизнес-логики, в отличие от платёжного шлюза), многие проекты сознательно оставляют узкий глобальный доступ (например `Logger::global()`), но это осознанное исключение для одной конкретной, стабильной, узкой службы — а не общий `Context::instance()`, через который можно достать вообще всё.
