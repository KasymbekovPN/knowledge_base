// Signal/Slot (Qt-стиль) вне Qt - через boost::signals2. Никакого moc,
// никакого общего интерфейса Observer - слотом может быть что угодно
// с подходящей сигнатурой: лямбда, свободная функция, member function.

#include <boost/signals2.hpp>
#include <iostream>
#include <format>
#include <string>

// ---------------------------------------------------------------------------
// Button - "источник событий". У него просто есть сигнал как обычное
// поле-член - никакого наследования от QObject/Observer не требуется.
// ---------------------------------------------------------------------------
class Button {
public:
    // Тип сигнала описывает сигнатуру слотов: void(const std::string&).
    // Это и есть типобезопасность - в отличие от нашего EventBus, где
    // тип события стирался через const void*, здесь всё типизировано
    // на этапе компиляции самим boost::signals2::signal<Sig>.
    boost::signals2::signal<void(const std::string&)> clicked;

    explicit Button(const std::string &name): name_{std::move(name)} {}
    void click() { clicked(name_); }
private:
    std::string name_;
};

// Слот как обычная свободная функция - никакого общего интерфейса не нужно.
void onButtonFunctionFreeFunction(const std::string& name) {
    std::cout << std::format("  [free fn] click: {}\n", name);
}

// ---------------------------------------------------------------------------
// Logger - компонент, подписывающийся на сигнал чужого объекта.
// scoped_connection - RAII: как наш ручной Connection из примера с EventBus,
// только уже встроен в саму библиотеку, отключается сам в деструкторе.
// ---------------------------------------------------------------------------
class Logger {
public:
    void subscribe(Button& button) {
        conn_ = button.clicked.connect([this](const std::string& name) {onClick(name); });
    }
private:
    void onClick(const std::string& name) {
        std::cout << std::format("  [Logger] click holt: {}\n", name);
    }

    boost::signals2::scoped_connection conn_;
};

int main() {
    Button okButton("OK");

    // 1. Slot - lambda
    auto conn1 = okButton.clicked.connect([](const std::string& name) {
        std::cout << std::format("  [lambda] clicked: {}\n", name);
    });

    // 2. Slot - free function
    okButton.clicked.connect(&onButtonFunctionFreeFunction);

    // 3. Slot - via a separate component with automatic shutdown based on lifetime
    {
        Logger logger;
        logger.subscribe(okButton);

        std::cout << "--- click #1 (3 subscribers: lambda, free fn, Logger) ---\n";
        okButton.click();
    } // logger уничтожается -> scoped_connection сам отключает слот

    std::cout << "\n--- click #2 (Logger already unsubscribed) ---\n";
    okButton.click();

    // отключение и вручную работает как обычно
    conn1.disconnect();
    std::cout << "\n--- click #3 (lambda is also disabled; only the free function remains) ---\n";
    okButton.click();

    return 0;
}
