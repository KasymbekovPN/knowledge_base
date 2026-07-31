// MVP vs MVVM на одном и том же примере (счётчик), чтобы структурная
// разница была видна прямо в коде: кто на кого ссылается и кто кого зовёт.

#include <functional>
#include <iostream>
#include <format>
#include <string>

// ---------------- Model - общий для обоих вариантов ----------------
class CounterModel {
public:
    int value() const { return value_; }
    void increment() { ++value_; }
    void decrement() { if (value_ > 0) --value_; }

private:
    int value_{0};
};

// ============================================================
// MVP: Presenter АКТИВНО вызывает методы View через интерфейс IView.
// View пассивна - только отображает то, что ей явно сказали показать.
// ============================================================
namespace mvp {

class IView {
public:
    virtual ~IView() = default;
    virtual void showCount(int value) = 0;
    virtual void showMessage(const std::string& msg) = 0;
};

// Presenter содержит ВСЮ логику "что и когда показывать" - ни Model,
// ни View сами по себе этого не знают.
class Presenter {
public:
    Presenter(CounterModel& model, IView& view): model_{model}, view_{view} {
        view_.showCount(model_.value());
    }

    void onIncrementClicked() {
        model_.increment();
        view_.showCount(model_.value());
    }

    void onDecrementClicked() {
        if (model_.value() == 0) {
            view_.showMessage("Value is zero.");
            return;
        }
        model_.decrement();
        view_.showCount(model_.value());
    }

private:
    CounterModel& model_;
    IView& view_;
};

class ConsoleView: public IView {
public:
    void showCount(int value) override {
        std::cout << std::format("  [MVP View] counter = {}\n", value);
    }
    void showMessage(const std::string& msg) override {
        std::cout << std::format("  [MVP View] message: {}\n", msg);
    }
};

// Fake для юнит-теста Presenter БЕЗ какого-либо реального UI
class FakeView: public IView {
public:
    void showCount(const int value) override {
        lastCount = value;
        ++countCalls;
    }
    void showMessage(const std::string& msg) override {
        lastMessage = msg;
    }

    int lastCount{-1};
    int countCalls{0};
    std::string lastMessage{""};
};

void demo() {
    std::cout << "-- MVP: real View --\n";
    CounterModel model;
    ConsoleView view;
    Presenter presenter{model, view};
    presenter.onIncrementClicked();
    presenter.onDecrementClicked();

    std::cout << "\n--- MVP: unit0test Presenter without UI ---\n";
    CounterModel testModel;
    FakeView fakeView;
    Presenter testPresenter{testModel, fakeView};
    // проверяем бизнес-правило "не в минус"
    testPresenter.onDecrementClicked();
    std::cout << std::format("  countCalls = {}, lastMessage = '{}'\n", fakeView.countCalls, fakeView.lastMessage);
}

}

// ============================================================
// MVVM: ViewModel НЕ ЗНАЕТ о существовании View вообще - ни ссылки,
// ни интерфейса. Вместо явных вызовов view.showX() - публикует изменения
// через "точки привязки" (тот же принцип, что Signal/Slot), на которые
// View подписывается сама.
// ============================================================
namespace mvvm {

class CounterViewModel {
public:
    explicit CounterViewModel(CounterModel& model): model_{model} {}

    void increment() {
        model_.increment();
        notify();
    }

    void decrement() {
        if (model_.value() == 0) {
            std::cout << std::format("value is zero");
            return;
        }
        model_.decrement();
        notify();
    }

    // Точки привязки (binding points) - View сама решает на них подписаться.
    // ViewModel никогда не вызывает ничего у конкретного View - он даже
    // не знает, что такое View, не то что какой у него тип.
    std::function<void(int)> onCountChanged;
    std::function<void(const std::string&)> onMessage;

private:
    void notify() {
        if (onCountChanged) onCountChanged(model_.value());
    }

    CounterModel& model_;
};

// View сама устанавливает биндинг при создании - направление подписки
// противоположно направлению вызова в MVP.
class ConsoleView {
public:
    explicit ConsoleView(CounterViewModel& vm) {
        vm.onCountChanged = [](int value) {
            std::cout << std::format("  [MVVM View] counter = {}\n", value);
        };
        vm.onMessage = [](const std::string& msg) {
            std::cout << std::format("  [MVVM View] message: {}\n", msg);
        };
    }
};

void demo() {
    std::cout << "\n--- MVVM --\n";
    CounterModel model;
    CounterViewModel vm{model};
    // View подписывается на ViewModel, а не наоборот
    const ConsoleView view{vm};
    (void) view;

    vm.increment();
    vm.increment();
    vm.decrement();
}

}

int main() {
    mvp::demo();
    mvvm::demo();

    return 0;
}
