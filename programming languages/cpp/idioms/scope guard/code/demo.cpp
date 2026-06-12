#include <iostream>
#include <functional>

class SGuard {
private:
    std::function<void()> action;
public:
    explicit SGuard(std::function<void()> _action):
        action{std::move(_action)} {}
    SGuard(const SGuard&) = delete;
    SGuard& operator=(const SGuard&) = delete;
    SGuard(SGuard&& _other) noexcept:
        action{std::exchange(_other.action, nullptr)} {}
    ~SGuard() {
        if (!action) { return; }
        action();
    }
};

void test();

int main() {
    test();

    return 0;
}

void test() {
    SGuard sg([&]() {
        std::cout << "Close file" << std::endl;
    });
}
