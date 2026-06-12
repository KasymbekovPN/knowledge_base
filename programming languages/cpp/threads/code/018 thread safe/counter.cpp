#include <iostream>
#include <mutex>

class Value {
public:
    int inc() {
        std::lock_guard lock{mtx};
        int old_value = value;
        ++value;
        ++counter;

        return old_value;
    }

    int dec() {
        std::lock_guard lock{mtx};
        int old_value = value;
        --value;
        ++counter;

        return old_value;
    }

    int get() const {
        std::lock_guard lock{mtx};
        return value;
    }

    size_t getCounter() const {
        std::lock_guard lock{mtx};
        return counter;
    }

private:
    mutable std::mutex mtx;
    int value{};
    size_t counter{};
};

std::ostream& operator<<(std::ostream& _os, const Value& _value) {
    return _os
        << "{value: " << _value.get()
        << ", counter: " << _value.getCounter() << "}";
}

int main() {
    Value value;
    std::thread{[&]() {value.inc();}}.join();
    std::thread{[&]() {value.dec();}}.join();
    std::thread{[&]() {value.inc();}}.join();

    std::cout << "value: " << value << std::endl;

    return 0;
}
