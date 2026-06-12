#include <iostream>
#include <type_traits>

struct Point {
    int x, y;
};

class Vec3 {
public:
    float x, y, z;
};

class NonAggregate {
public:
    int value;
    NonAggregate(int v): value{v} {}
};

struct Data {
    int a;
    int b = 0;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<Point>("Point");
    test<Vec3>("Vec3");
    test<NonAggregate>("NonAggregate");
    test<int[5]>("int[5]");
    test<Data>("Data");
    test<int>("int");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_aggr = std::is_aggregate_v<T>;
    std::cout << "[" << _lbl << "]: ";
    std::cout
        << std::boolalpha
        << is_aggr
        << std::noboolalpha
        << std::endl;
}
