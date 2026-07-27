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
    }
};

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
    }
};

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
    }
};

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
    // а не на этапе компиляции.
     ServiceLocator freshLocator;  // локальный, чтобы не задеть глобальный instance()
     try {
         freshLocator.resolve<IPaymentGateway>();
         std::cout << "[FAIL] must be exception\n";
     } catch (const std::runtime_error& e) {
         std::cout << "[OK] test_ServiceLocator_forgotten_registration (caught: "
                   << e.what() << ")\n";
     }
}

int main() {
    test_DI_isolated();
    test_DI_failure_case();
    test_ServiceLocator_requires_global_setup();
    test_ServiceLocator_forgotten_registration();

    std::cout << "\nDone\n";
    return 0;
}