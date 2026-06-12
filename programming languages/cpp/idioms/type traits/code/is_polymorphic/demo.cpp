#include <iostream>
#include <type_traits>

struct V3 {
    float x, y, z;
};

struct Animal {
    virtual void speak() = 0;
};

struct Dog: Animal {
    void speak() override {
        std::cout << "Woof" << std::endl;
    }
};

template<typename T>
void test(const std::string&&);

int main() {
    test<V3>("V3");
    test<Animal>("Animal");
    test<Dog>("Dog");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    std::cout << "[" << _lbl << "]: ";
    if constexpr (std::is_polymorphic_v<T>) {
        std::cout << "+";
    } else {
        std::cout << "-";
    }
    std::cout << std::endl;
}
