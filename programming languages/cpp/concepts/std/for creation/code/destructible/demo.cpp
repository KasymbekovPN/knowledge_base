#include <iostream>
#include <concepts>
#include <memory>
#include <vector>
#include <string>

class PrivDestructor {
private:
    ~PrivDestructor() = default;
public:
    static PrivDestructor create() { return PrivDestructor(); }
};

class DeletedDestructor {
public:
    ~DeletedDestructor() = delete;
};

template <typename T>
class SafeContainer {
private:
    T* data_;
    size_t size_;

public:
    static SafeContainer<T> create(size_t n_) {
        return SafeContainer<T>(n_);
    }

    explicit SafeContainer(size_t _n):
        data_{new T[_n]},
        size_{_n} {}

    ~SafeContainer() {
        std::cout << "DCTOR" << std::endl;
        delete[] data_;
    }
};

template<std::destructible T>
void test(T);

int main() {
    test(42);
    test(std::string("hello"));
    test(std::vector<int>{1, 2, 3});
    test(SafeContainer<int>::create(5));
    // test(PrivDestructor::create()); // Error

    return 0;
}

template<std::destructible T>
void test(T value) {
    std::cout << typeid(value).name() << std::endl;
}
