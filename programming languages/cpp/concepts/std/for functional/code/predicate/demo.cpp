#include <iostream>
#include <concepts>

bool is_possitive(const int x) {
    return x > 0;
}

auto&& is_negative = [](const int x) {
    return x < 0;
};

struct IsOdd {
    bool operator()(const int x) const {
        return x % 2 != 0;
    }
};

struct IsEven {
    int operator()(const int x) const {
        return x % 2 ? 0 : 1;
    }
};

template<typename F, typename... Args>
requires std::predicate<F, Args...>
void test(std::string&& lbl, F&& func, Args&&... args) {
    std::cout << lbl;
    if (func(args...)) {
        std::cout << " TRUE";
    } else {
        std::cout << " FALSE";
    }
    std::cout << std::endl;
}

int main() {
    test("is_possitive 1", is_possitive, 1);
    test("is_possitive -1", is_possitive, -1);

    test("is_negative 1", is_negative, 1);
    test("is_negative -1", is_negative, -1);

    test("IsOdd 1", IsOdd(), 1);
    test("IsOdd 2", IsOdd(), 2);

    test("IsEven 1", IsEven(), 1);
    test("IsEven 2", IsEven(), 2);

    return 0;
}

