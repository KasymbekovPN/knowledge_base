#include <iostream>
#include <type_traits>
#include <vector>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_swappable_v<T> << "\n"

struct Point {
    int x, y;
};

class NonSwappable {
private:
    ~NonSwappable()  = default;
};

struct Widget {
    int id;
    std::string name;
};

void swap(Widget& _a, Widget& _b) noexcept {
    using std::swap;
    swap(_a.id, _b.id);
    swap(_a.name, _b.name);
}

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonSwappable);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(Widget);
    
    return 0;
}
