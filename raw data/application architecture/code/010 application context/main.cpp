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
    }
};

class DataBase {
public:
    void save(const std::string& record) {
        std::cout << std::format("  [db] save: {}\n", record);
    }
};

class EventBusStub {
public:
    void publish(const std::string& event) {
        std::cout << std::format("  [publish] {}\n", event);
    }
};

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
            // а его реальные зависимости растут бесконтрольно.
        }
    };

    void run() {
        std::cout << "--- bad_good_object ---\n";

        // конструктор ничего не говорит о зависимостях
        OrderService service;
        service.placeOrder("gadget");
    }
}

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
        }
    private:
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
        }
    private:
        EventBusStub& bus_;
    };

    // App - единственное место, где всё это собирается вместе. Не синглтон,
    // создаётся один раз в main(), живёт на стеке (RAII, как в наших
    // предыдущих примерах с io_context).
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
