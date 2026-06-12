#include <iostream>
#include <type_traits>

struct Point {
    float x, y;
};

struct Base {
    float a;
};

struct Derived: Base {
    float b;
};

class NonStandard {
private:
    int x;
public:
    int y;
};

union U {
    int i;
    float f;
};

template<typename T>
void test_print(const std::string&&);

int main() {
    test_print<Point>("Point");
    test_print<Base>("Base");
    test_print<Derived>("Derived");
    test_print<NonStandard>("NonStandard");
    test_print<U>("U");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_standard_layout_v<T>) {
        std::cout << " - std";
    } else {
        std::cout << " - no std";
    }
    std::cout << std::endl;
}
