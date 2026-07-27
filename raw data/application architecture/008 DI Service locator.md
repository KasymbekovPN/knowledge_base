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
// Dependency Injection vs Service Locator - одна и та же задача (OrderService  
// нужен IPaymentGateway) решена двумя способами. Тесты внизу наглядно  
// показывают разницу в тестируемости.  
  
#include <cassert>  
#include <iostream>  
#include <format>  
#include <memory>  
#include <stdexcept>  
#include <string>  
#include <typeindex>  
#include <unordered_map>  
  
// ---------------------------------------------------------------------------  
// Общий интерфейс (Dependency Inversion: оба варианта зависят от абстракции,  
// не от конкретного StripeGateway).  
// ---------------------------------------------------------------------------  
class IPaymentGateway {  
public:  
    virtual ~IPaymentGateway() = default;  
    virtual bool change(const double amount) = 0;  
};  
  
class StripeGateway: public IPaymentGateway {  
public:  
    bool change(const double amount) override {  
        std::cout << std::format("  [Stripe] net request ${}\n", amount);  
        // в реальности - HTTP-вызов к платёжному провайдеру  
        return true;  
    }};  
  
// ============================================================  
// Fake для тестов - не ходит в сеть, просто запоминает вызовы.  
// ============================================================  
class FakeGateway: public IPaymentGateway {  
public:  
    double last_amount{0.0};  
    int call_count{0};  
    bool should_success{true};  
  
    bool change(const double amount) override {  
        last_amount = amount;  
        ++call_count;  
        return should_success;  
    }};  
  
// ============================================================  
// Вариант 1: Service Locator - глобальный реестр, зависимость  
// достаётся "по требованию" изнутри метода.  
// ============================================================  
class ServiceLocator {  
public:  
    static ServiceLocator& instance() {  
        static ServiceLocator instance;  
        return instance;  
    }  
    template<typename T>  
    void register_service(std::shared_ptr<T> service) {  
        services_[std::type_index(typeid(T))] = service;  
    }  
    template<typename T>  
    std::shared_ptr<T> resolve() {  
        const auto it{services_.find(std::type_index(typeid(T)))};  
        if (it == services_.end()) throw std::runtime_error("service not registered");  
        return std::static_pointer_cast<T>(it->second);  
    }  
private:  
    std::unordered_map<std::type_index, std::shared_ptr<IPaymentGateway>> services_;  
};  
  
class OrderServiceLocatorBased {  
public:  
    // Проблема видна уже здесь: конструктор ничего не говорит о том,  
    // что классу вообще нужен IPaymentGateway.    
    OrderServiceLocatorBased() = default;  
  
    bool place_order(const double amount) {  
        const auto gw{ServiceLocator::instance().resolve<IPaymentGateway>()};  
        return gw->change(amount);  
    }};  
  
// ============================================================  
// Вариант 2: Dependency Injection - зависимость передаётся явно,  
// через конструктор.  
// ============================================================  
class OrderServiceDI {  
public:  
    // Зависимость - часть публичного контракта класса: нельзя создать  
    // объект, не предоставив IPaymentGateway. Компилятор это проверяет.    
    explicit OrderServiceDI(IPaymentGateway& gw): gw_{gw} {}  
  
    bool place_order(const double amount) { return gw_.change(amount); }  
  
private:  
    IPaymentGateway& gw_;  
};  
  
// ============================================================  
// "Тесты" (assert-based, без фреймворка - достаточно для демонстрации)  
// ============================================================  
void test_DI_isolated() {  
    // Ничего глобального не трогаем. Fake создан прямо в теле теста,  
    // время жизни очевидно, teardown не нужен - fake умрёт вместе со стеком.    
    FakeGateway fake;  
    OrderServiceDI service{fake};  
  
    const bool result = service.place_order(100.0);  
  
    assert(result == true);  
    assert(fake.last_amount == 100.0);  
    assert(fake.call_count == 1);  
    std::cout << "[OK] test_DI_isolated\n";  
}  
  
void test_DI_failure_case() {  
    FakeGateway fake;  
    fake.should_success = false;  
    OrderServiceDI service{fake};  
  
    const bool result = service.place_order(50.0);  
  
    assert(result == false);  
    std::cout << "[OK] test_DI_failure_case\n";  
}  
  
void test_ServiceLocator_requires_global_setup() {  
    // Чтобы протестировать OrderServiceLocatorBased, ПРИХОДИТСЯ подменять  
    // глобальное состояние ServiceLocator до создания объекта - тест    
    // больше не изолирован, порядок и параллельность тестов начинают иметь    
    // значение, а тестовый fake "просачивается" во всё, что resolve'ит    
    // IPaymentGateway после этой строки, пока кто-то его не заменит обратно.    
    auto fake = std::make_shared<FakeGateway>();  
    ServiceLocator::instance().register_service<IPaymentGateway>(fake);  
  
    OrderServiceLocatorBased service;  // сигнатура не намекает на зависимость  
    const bool result = service.place_order(75.0);  
  
    assert(result == true);  
    assert(fake->last_amount == 75.0);  
    std::cout << "[OK] test_ServiceLocator_requires_global_setup\n";  
}  
  
void test_ServiceLocator_forgotten_registration() {  
    // Демонстрация другого риска: если сервис забыли зарегистрировать -  
    // об этом узнаём только в рантайме, в глубине call stack'а,    
    // а не на этапе компиляции.     ServiceLocator freshLocator;  
    // локальный, чтобы не задеть глобальный instance()  
     try {  
         freshLocator.resolve<IPaymentGateway>();  
         std::cout << "[FAIL] must be exception\n";  
     } catch (const std::runtime_error& e) {  
         std::cout << "[OK] test_ServiceLocator_forgotten_registration (caught: "  
                   << e.what() << ")\n";  
     }}  
  
int main() {  
    test_DI_isolated();  
    test_DI_failure_case();  
    test_ServiceLocator_requires_global_setup();  
    test_ServiceLocator_forgotten_registration();  
  
    std::cout << "\nDone\n";  
    return 0;  
}
```

**Что видно из теста напрямую**

`test_DI_isolated` и `test_DI_failure_case` не трогают ничего, кроме своего стека: `FakeGateway fake;` создаётся, передаётся в конструктор `OrderServiceDI`, тест выполняется, всё умирает при выходе из функции. Никакого teardown, никакого общего состояния между тестами — их можно гонять в любом порядке, параллельно, в разных потоках.

`test_ServiceLocator_requires_global_setup` вынужден мутировать глобальный синглтон (`ServiceLocator::instance()`) до создания объекта — а после этого теста `IPaymentGateway` в глобальном реестре так и останется `FakeGateway`, и если следующий тест в файле тоже резолвит `IPaymentGateway` через локатор, он невольно получит чужой fake с уже "грязным" состоянием (`lastAmount`, `callCount` из предыдущего теста), если явно не подменить или не почистить реестр. Порядок тестов внезапно стал иметь значение — это именно та скрытая связанность, о которой обычно предупреждают на словах, а тут видно в коде буквально.

**Плюсы и минусы по существу**

DI делает зависимость частью публичного контракта класса — сигнатура конструктора `OrderServiceDI(IPaymentGateway&)` говорит всё, что нужно знать, не читая реализацию. Компилятор физически не даст создать объект без зависимости. Service Locator прячет это: `OrderServiceLocatorBased()` выглядит так, будто классу вообще ничего не нужно, а на самом деле внутри `placeOrder()` спрятан `resolve<IPaymentGateway>()`, который упадёт в рантайме, если кто-то забыл зарегистрировать сервис (как в `test_ServiceLocator_forgotten_registration` — узнаём об ошибке только когда реально дошли до вызова, а не на этапе сборки объекта).

DI также снимает зависимость от глобального изменяемого состояния — можно спокойно держать в одной программе два `OrderServiceDI` с разными gateway одновременно (например, один настоящий, один тестовый, для canary-запросов), тогда как `ServiceLocator::instance()` — обычно синглтон, и подменить сервис "только для одного места использования" неудобно без городить дополнительный механизм скоупинга.

У DI своя цена — при росте графа зависимостей конструктор может разрастись (5-6 параметров), и решение — не тянуть Service Locator обратно, а вынести сборку всего графа объектов в одно место, "composition root" (обычно `main()` или отдельная `App::create()`), которое явно строит всё дерево зависимостей один раз при старте. Именно так стоит проектировать `Application`/`Context`, о котором говорили раньше: он должен именно строить и передавать зависимости через конструкторы модулей (это и есть DI вручную, без фреймворка), а не превращаться в глобальный `resolve<T>()` по требованию — это и был тот "God Object" риск, о котором упоминали в первом дне блока.

Service Locator оправдан в узких случаях: когда набор доступных сервисов действительно определяется в рантайме и не известен статически (плагинная система, где плагины регистрируют себя динамически), или в легаси-коде, где полноценный рефакторинг конструкторов слишком дорог прямо сейчас. Но как архитектура по умолчанию для нового кода — DI выигрывает почти всегда именно из-за тестируемости и явности контрактов.
