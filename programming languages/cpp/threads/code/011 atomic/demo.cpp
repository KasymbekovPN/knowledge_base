#include <iostream>
#include <thread>
#include <atomic>

struct NonAtomicValue {
    inline static constexpr size_t SIZE{1000000};
    int value{};

    void increment() {
        for (size_t i{}; i < SIZE; ++i) {
            ++value;
        }
    }
};

struct AtomicValue {
    inline static constexpr size_t SIZE{1000000};
    std::atomic<int> value{0};

    void increment() {
        for (size_t i{}; i < SIZE; ++i) {
            ++value;
        }
    }
};

template<typename T>
concept HasValue = requires(const T& t) {
    t.value;
};

template<HasValue T>
std::ostream& operator<<(std::ostream& _os, const T& _input) {
    return _os << "{" << _input.value << "}";
}

int main() {
    NonAtomicValue nav;
    std::thread t0 {&NonAtomicValue::increment, std::ref(nav)};
    std::thread t1 {&NonAtomicValue::increment, std::ref(nav)};
    t0.join();
    t1.join();

    AtomicValue av;
    std::thread t2 {&AtomicValue::increment, std::ref(av)};
    std::thread t3 {&AtomicValue::increment, std::ref(av)};
    t2.join();
    t3.join();

    std::cout << "nav: " << nav << std::endl;
    std::cout << "av: " << av << std::endl;

    return 0;
}
