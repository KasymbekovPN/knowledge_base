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
// Model как источник истины: сравниваем ПАССИВНУЮ модель (кто-то снаружи  
// должен не забыть вручную уведомить о изменении) и АКТИВНУЮ модель  
// (уведомление встроено прямо в методы мутации - забыть невозможно).  
// View ничего не знает о Model напрямую - только подписывается на события.  
  
#include <algorithm>  
#include <functional>  
#include <iostream>  
#include <format>  
#include <typeindex>  
#include <unordered_map>  
#include <vector>  
#include <ranges>  
  
// ---------------------------------------------------------------------------  
// EventBus (тот же механизм, что использовали во всех прошлых примерах).  
// ---------------------------------------------------------------------------  
namespace {  
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
            }            
            return *this;  
        }        
        Connection(const Connection&) = delete;  
        Connection& operator=(const Connection&) = delete;  
        ~Connection() { disconnect(); }  
        void disconnect() {  
            if (!bus_) return;  
            bus_->unsubscribeRaw(type_, id_);  
            bus_ = nullptr;  
        }  
    private:  
        friend class EventBus;  
  
        Connection(EventBus* bus, const std::type_index type, SubscriptionId id):  
            bus_{bus}, type_{type}, id_{id} {}  
  
        EventBus* bus_{nullptr};  
        std::type_index type_{typeid(void)};  
        SubscriptionId id_{0};  
    };  
    template <typename EventT>  
    Connection subscribe(std::function<void(const EventT&)> handler) {  
        const auto TYPE{std::type_index(typeid(EventT))};  
        const SubscriptionId ID{nextId_++};  
        handlers_[TYPE].push_back({  
            ID,  
            [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }  
        });  
        return  {this, TYPE, ID};  
    }  
    template <typename EventT>  
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
            std::ranges::remove_if(list, [id](const auto& entry){ return entry.first == id; }).begin(),  
            list.end());  
    }  
    std::unordered_map<  
        std::type_index,  
        std::vector<std::pair<SubscriptionId, std::function<void(const void*)>>>  
    > handlers_;  
    SubscriptionId nextId_{1};  
};  
}  
  
  
namespace {  
    struct BalanceChanged { double newBalance; };  
}  
  
namespace {  
    // ---------------------------------------------------------------------------  
    // View - не знает о Model НИЧЕГО, только подписывается на событие.    // ---------------------------------------------------------------------------    class BalanceView {  
    public:  
        explicit BalanceView(EventBus& bus) {  
            conn_ = bus.subscribe<BalanceChanged>([this](const BalanceChanged& e) {  
                lastSeenBalance = e.newBalance;  
                std::cout << std::format("  [View] balance on display: ${}\n", lastSeenBalance);  
            });        
        }  
        double lastSeenBalance{0};  
  
    private:  
        EventBus::Connection conn_;  
    };}  
  
// ============================================================  
// Вариант 1 (проблемный): пассивная Model - сама ничего не публикует.  
// Уведомление - отдельный шаг, который вызывающий код должен не забыть.  
// ============================================================  
namespace passive_model {  
  
    namespace {  
        class Account {  
        public:  
            void deposit(const double amount) { balance_ += amount; }  
            double balance() const { return balance_; }  
  
        private:  
            double balance_{0};  
        };        
    }

    // Путь A (например Presenter из UI) - аккуратно помнит про notify  
    static void depositViaUI(Account& acc, EventBus& bus, const double amount) {  
        acc.deposit(amount);  
        bus.publish(BalanceChanged{acc.balance()});  
    }
  
    // Путь B (например фоновый обработчик начисления процентов, или Command,  
    // добавленный позже другим разработчиком, который не знал про конвенцию    // "не забудь опубликовать событие") - реалистичный источник бага    
    static void depositViaBackgroundJob(Account& acc, const double amount) {  
        acc.deposit(amount);  
        // публикация события ЗАБЫТА - View не узнает об этом изменении  
    }  
  
    static void demo() {  
        std::cout << "-- passive model --\n";  
        EventBus bus;  
        Account acc;  
        BalanceView view{bus};  
  
        // View корректно увидит 100  
        depositViaUI(acc, bus, 100.0);  
        // реальный баланс = 150, View об этом не узнает!  
        depositViaBackgroundJob(acc, 150.0);  
  
        std::cout << std::format("  real model balance: ${}\n", acc.balance());  
        std::cout << std::format("  what see View: ${}\n", view.lastSeenBalance);  
    }
}  
  
// ============================================================  
// Вариант 2: активная Model - уведомление встроено В САМИ методы мутации.  
// Кто бы ни вызвал deposit() - View узнает об этом гарантированно.  
// ============================================================  
namespace active_model {  
  
    class Account {  
    public:  
        explicit Account(EventBus& bus) : bus_{bus} {}  
  
        void deposit(const double amount) {  
            balance_ += amount;  
            bus_.publish(BalanceChanged{balance_});  
        }  
        double balance() const { return balance_; }  
  
    private:  
        EventBus& bus_;  
        double balance_{0};  
    };  

    // Оба пути мутации используют один и тот же deposit() - уведомление  
    // невозможно забыть, оно не зависит от того, кто вызывает.    
    static void depositViaUI(Account& acc, double amount) { acc.deposit(amount); }  
    static void depositViaBackgroundJob(Account& acc, double amount) { acc.deposit(amount); }  
  
    static void demo() {  
        std::cout << "\n-- active_model --\n";  
        EventBus bus;  
        Account acc{bus};  
        BalanceView view{bus};  
  
        depositViaUI(acc, 100.0);  
        depositViaBackgroundJob(acc, 50.0);  
  
        std::cout << std::format("  real model balance: ${}\n", acc.balance());  
        std::cout << std::format("  what see View: ${}\n", view.lastSeenBalance);  
    }  
}  
  
int main() {  
    passive_model::demo();  
    active_model::demo();  
  
    return 0;  
}
```

**Ключевой вопрос: кто отвечает за уведомление — Model или вызывающий код**

В прошлом примере (MVP/MVVM) `CounterModel` была пассивной: `Presenter`/`ViewModel` мутировали её и вручную вызывали `view.showCount()`/`notify()` отдельным шагом сразу после. Это работает, пока есть только один путь мутации через один Presenter. Реальные приложения так не живут: баланс счёта может меняться и через UI-действие пользователя, и через фоновую задачу (начисление процентов), и через Command, добавленный полгода спустя другим разработчиком, который вообще не в курсе конвенции "не забудь опубликовать событие после мутации".

Тест это воспроизводит конкретно: `passive_model::depositViaBackgroundJob()` честно мутирует `balance_`, но забывает вызвать `bus.publish(...)` — и `View` молча остаётся показывать `$100`, пока реальный баланс модели уже `$150`. Никакой ошибки, никакого исключения — просто тихое расхождение источника истины и того, что видит пользователь. Это ровно тот класс багов, который в реальных проектах обнаруживается через "почему у пользователя на экране одно, а в базе другое" тикеты, а не через падение программы.

**Решение: уведомление — не отдельный шаг, а часть самой операции мутации**

В `active_model::Account` конструктор принимает `EventBus&`, и `deposit()` сам публикует `BalanceChanged` внутри себя — `bus_.publish(...)` находится в теле метода, а не рядом с местом вызова. Отсюда прямое следствие: **любой** код, вызывающий `acc.deposit(amount)` — хоть из UI, хоть из фоновой задачи, хоть из Command, хоть из сетевого обработчика — гарантированно триггерит уведомление, потому что публикация не является отдельным действием, которое можно забыть, а встроена в единственный способ изменить состояние. Тест подтверждает: тот же самый сценарий вызовов (`depositViaUI` + `depositViaBackgroundJob`) даёт `View` корректные `$100` → `$150`, без единого расхождения.

**Это и есть суть "Model как источник истины"**

Model — не просто структура данных, а единственное место, где определено, что значит "состояние изменилось" и кто должен об этом узнать. Если ответственность за уведомление размазана по вызывающим сторонам (Presenter, Command, фоновая задача — каждый должен помнить сделать `publish` сам) — источник истины на самом деле не Model, а негласная договорённость между всеми, кто её вызывает, что неизбежно в какой-то момент нарушается. Если уведомление — часть контракта самого метода мутации Model, вызывающему коду вообще не нужно ничего знать о том, что кто-то следит за изменениями — он просто вызывает `deposit()`, и это уже достаточно.

`View` в обоих вариантах устроена одинаково — подписывается на `BalanceChanged` и вообще не хранит указателя/ссылки на `Account`. Она в буквальном смысле не может напрямую прочитать состояние Model, только получить то, что Model сама решила опубликовать — то есть Model полностью управляет тем, какая информация и когда становится видна снаружи, а View пассивно реагирует, не зная о существовании Model вообще.
