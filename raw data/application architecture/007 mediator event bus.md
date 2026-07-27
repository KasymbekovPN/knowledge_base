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
        {            "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }    ],    "buildPresets": [  
        {            "name": "debug",  
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
// Event bus (Observer поверх глобальной шины) как практическая форма Mediator:  
// компоненты общаются через типы событий, а не через ссылки друг на друга.  
// Отдельно демонстрируется RAII-подписка (Connection), которая решает  
// классическую проблему event bus - dangling handler на уже уничтоженный объект.  
  
#include <algorithm>  
#include <functional>  
#include <iostream>  
#include <format>  
#include <ranges>  
#include <string>  
#include <typeindex>  
#include <unordered_map>  
#include <vector>  
  
// ---------------------------------------------------------------------------  
// EventBus: центральный посредник. Publisher вызывает bus.publish(Event{...}),  
// subscriber вызывает bus.subscribe<Event>(handler) - они никогда не держат  
// указателей друг на друга, оба знают только про EventBus и про тип события.  
// ---------------------------------------------------------------------------  
class EventBus {  
public:  
    using SubscriptionId = std::size_t;  
  
    // RAII-обёртка над подпиской. Разрушение Connection автоматически  
    // отписывает handler от шины - без этого лямбда с захваченным `this`    // осталась бы висеть в bus и получила бы вызов на уже мёртвый объект    // при следующем publish() (use-after-free).    
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
        }        
        Connection(const Connection&) = delete;  
        Connection& operator=(const Connection&) = delete;  
        ~Connection() { disconnect(); }  
  
        void disconnect() {  
            if (!bus_) return;  
            bus_->unsubscribe_raw(type_, id_);  
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
    template <typename EventT>  
    Connection subscribe(std::function<void(const EventT&)> handler) {  
        const auto type{std::type_index(typeid(EventT))};  
        SubscriptionId id{next_++};  
        // Стираем тип: снаружи хранится std::function<void(const void*)>,  
        // а привязка к конкретному EventT спрятана внутри лямбды.        
        handlers_[type].push_back({  
            id,  
            [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }  
        });  
        return Connection{this, type, id};  
    }  
    template <typename EventT>  
    void publish(const EventT& event) {  
        const auto it{handlers_.find(std::type_index(typeid(EventT)))};  
        if (it == handlers_.end()) return;  
        // Копия списка: handler может внутри себя подписаться/отписаться  
        // (например temp-подписчик из демо ниже), и это не должно        // инвалидировать текущий обход исходного вектора.        
        for (const auto list_copy = it->second;  
            const auto &fn: list_copy | std::views::values) fn(&event);  
    }  
private:  
    void unsubscribe_raw(const std::type_index type, const SubscriptionId id) {  
        const auto it{handlers_.find(type)};  
        if (it == handlers_.end()) return;  
        auto& list = it->second;  
        list.erase(  
            std::ranges::remove_if(list , [id](const auto& entry){ return entry.first == id; }).begin(),  
            list.end());  
    }  
    std::unordered_map<  
        std::type_index,  
        std::vector<  
            std::pair<  
                SubscriptionId, std::function<void(const void*)>>>  
    > handlers_;  
    SubscriptionId next_{1};  
};  
  
// ---------------------------------------------------------------------------  
// События приложения - обычные struct'ы, никакой базовой иерархии не нужно.  
// ---------------------------------------------------------------------------  
struct UserLoggedIn {  
    std::string username;  
};  
  
struct OrderPlaced {  
    std::string order_id;  
    double amount;  
};  
  
// ---------------------------------------------------------------------------  
// Компоненты-подписчики. Ни один не знает о существовании других - каждый  
// знает только про EventBus и про типы событий, которые его интересуют.  
// ---------------------------------------------------------------------------  
class AuditLog {  
public:  
    explicit AuditLog(EventBus& bus) {  
        login_conn_ = bus.subscribe<UserLoggedIn>([this](const UserLoggedIn& e) { on_login(e); });  
        order_conn_ = bus.subscribe<OrderPlaced>([this](const OrderPlaced& e) { on_order(e); });  
    }  
private:  
    void on_login(const UserLoggedIn& e) {  
        std::cout << std::format("  [audit] login: {}\n", e.username);  
    }    
    void on_order(const OrderPlaced& e) {  
        std::cout << std::format("  [audit] order {} with $ {}\n", e.order_id, e.amount);  
    }  
    EventBus::Connection login_conn_, order_conn_;  
};  
  
class NotificationService {  
public:  
    explicit NotificationService(EventBus& bus) {  
        conn_ = bus.subscribe<OrderPlaced>([this](const OrderPlaced& e) { on_order(e); });  
    }  
private:  
    void on_order(const OrderPlaced& e) {  
        std::cout << std::format("  [notify] order {} confirmed\n", e.order_id);  
    }    
    EventBus::Connection conn_;  
};  
  
class LoginCounter {  
public:  
    explicit LoginCounter(EventBus& bus) {  
        conn_ = bus.subscribe<UserLoggedIn>([this](const UserLoggedIn& e) { ++count_; });  
    }    
    int count() const { return count_; }  
  
private:  
    int count_{0};  
    EventBus::Connection conn_;  
};  
  
int main() {  
    EventBus bus;  
    AuditLog audit(bus);  
    NotificationService notify(bus);  
    LoginCounter counter(bus);  
  
    std::cout << "--- publish UserLoggedIn(\"pablo\") ---\n";  
    bus.publish(UserLoggedIn("pablo"));  
  
  
    std::cout << "--- publish OrderPlaced(\"ORD-42\", 19.99) ---\n";  
    bus.publish(OrderPlaced("ORD-42", 19.99));  
  
    std::cout << std::format("logins so far: {}\n\n", counter.count());  
  
    {        
	    std::cout << "--- temporary subscriber in nested scope ---\n";  
        LoginCounter temp(bus);  
        bus.publish(UserLoggedIn("guest"));  
        std::cout << std::format("temp.count: {}\n", temp.count());  
  
    }  
    std::cout << "--- publish after temp destroy ---\n";  
    bus.publish(UserLoggedIn("paul"));  
    std::cout << std::format("counter: {}\n", counter.count());  
  
    return 0;  
}
```

**Mediator vs Event bus — разница в степени связанности**

Классический Mediator (GoF) — это объект, который явно знает про своих "коллег" и имеет именованные методы под конкретные взаимодействия: `ChatMediator::sendMessage(User& from, string text)`, `DialogMediator::onButtonClicked()`. Компоненты не знают друг про друга, но каждый знает про медиатора, а медиатор знает про них всех и про конкретные сценарии их общения. Связанность не исчезает, а стягивается в одну точку — что уже прогресс по сравнению с N компонентами, которые держат ссылки друг на друга, но медиатор всё равно приходится менять при добавлении нового типа взаимодействия.

Event bus — более радикальный шаг: медиатор превращается в generic pub/sub-хаб, который вообще не знает про типы событий заранее (в моём примере — через `std::type_index`, у Qt — через сигнатуру сигнала). Publisher вызывает `publish(SomeEvent{...})`, subscriber вызывает `subscribe<SomeEvent>(handler)` — ни то, ни другое не требует правки самого `EventBus`. Это, по сути, Observer, где subject — не конкретный объект с конкретным состоянием, а сама шина, а "интересующее событие" заменяет привычную пару subject/observer один-к-одному на модель многие-ко-многим по типу события.

**Что видно в коде**

`AuditLog`, `NotificationService`, `LoginCounter` в примере не держат ссылок друг на друга и не знают о существовании друг друга вообще — только про `EventBus&` в конструкторе и про типы событий, которые их интересуют. `AuditLog` подписан на оба события, `NotificationService` — только на `OrderPlaced`, `LoginCounter` — только на `UserLoggedIn`. Добавить четвёртый компонент (например `FraudDetector`, слушающий `OrderPlaced`) не требует трогать ни один существующий класс — это ровно то расширение "без изменения старого кода", о котором был весь блок про модульность.

**Опасность, которую решает `Connection`**

Главная практическая проблема event bus — время жизни. Лямбда-подписчик захватывает `this`, и эта лямбда физически хранится внутри шины, а не внутри самого объекта — то есть шина держит "заимствованную" ссылку на объект (по духу — тот же паттерн raw-указателя "не владею", что обсуждали в разделе про ownership), но без явного контракта "не переживи меня" объект может быть уничтожен, а подписка останется висеть и вызовется на мёртвую память при следующем `publish()`. `Connection` в примере — прямое применение RAII к этой проблеме: подписка — это ресурс, которым владеет объект-подписчик через поле `EventBus::Connection`; когда подписчик уничтожается, деструктор `Connection` сам вызывает `unsubscribeRaw()`. Тест в коде это доказывает буквально: `temp`-подписчик создаётся во вложенном scope, а после выхода из scope и следующего `publish()` программа не падает — Connection отписалась автоматически.

**Компромиссы, которые стоит проговорить на собеседовании**

Событийная шина отлично снимает compile-time связанность, но платит за это debugging-стоимостью: "кто вообще слушает это событие?" больше не видно по call stack'у — нужно грепать по кодовой базе все `subscribe<EventType>`. Порядок вызова обработчиков часто не гарантирован (в примере — порядок подписки, но это деталь реализации, на неё нельзя полагаться архитектурно). А если shared event bus используется бесконтрольно — легко получить "invisible coupling": формально классы не зависят друг от друга, а по факту вся бизнес-логика размазана по десяткам разрозненных `subscribe`-обработчиков, и понять сквозной сценарий (например "что происходит при оформлении заказа") можно только пройдя все подписки руками. Поэтому event bus хорошо подходит для действительно независимых side-effects (аудит, логирование, аналитика, уведомления — как в примере), но не для основного бизнес-workflow, где явный поток управления (тот же Command/pipeline) читается прямолинейнее.

Связь с Qt-сигналами из первого блока: `QueuedConnection` — это, по сути, тот же принцип, только доставка события идёт не мгновенно (синхронный вызов `fn()` в `publish`), а через очередь конкретного event loop потока-получателя. При желании этот же `EventBus::publish` можно сделать асинхронным — заворачивать вызов handler'а в `asio::post()` вместо прямого вызова, и тогда шина станет ещё и границей между потоками, а не только между модулями.
