#include <iostream>
#include <ranges>
#include <vector>

struct CustomView: public std::ranges::view_base {
private:
    int arr[3]{10, 20, 30};

public:
    int* begin() {
        return arr;
    }

    int *end() {
        return arr + 3;
    }
};

template<std::ranges::view T>
void test(T _view) {
    for (auto item: _view) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::views::iota(1, 4));

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7};
    test(vec | std::views::filter(
        [](int x) {return x % 2 == 0;}
    ));

    test(CustomView());

    return 0;
}
