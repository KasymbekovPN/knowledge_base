#include <iostream>
#include <type_traits>

struct Point {
    float x, y;
};

struct Base {
    int a;
};

struct Derived: Base {
    int b;
};

class NonPod {
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
    test_print<NonPod>("NonPod");
    test_print<U>("U");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_pod_v<T>) {
        std::cout << " - pod";
    } else {
        std::cout << " - no pod";
    }
    std::cout << std::endl;
}
