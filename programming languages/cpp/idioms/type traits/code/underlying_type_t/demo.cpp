#include <iostream>
#include <type_traits>
#include <utility>

enum Status {
    Idle,
    Running,
    Stopped
};

enum Priority : uint8_t {
    Low = 33,
    Medium,
    High
};

template<typename T>
void test(const T&&);

int main() {
    test<Status>(Status::Running);
    test<Priority>(Priority::Low);

    std::cout
        << "to_underlying: "
        << std::to_underlying(Status::Stopped)
        << std::endl;

    return 0;
}

template<typename T>
void test(const T&& _value) {
    std::cout
        << "{ " << _value
        << ", " << typeid(T).name()
        << ", " << typeid(std::underlying_type_t<T>).name()
        << "}" << std::endl;
}
