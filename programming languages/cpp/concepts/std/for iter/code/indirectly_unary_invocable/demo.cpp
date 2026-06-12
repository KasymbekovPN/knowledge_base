#include <iostream>
#include <iterator>
#include <vector>
#include <concepts>

struct Iterator {
    int value{42};

    const int* operator*() const {
        return &value;
    }
};

void handle_int(int _input) {
    std::cout << "handle_int: " << _input << std::endl;
}

void handle_str(std::string _input) {
    std::cout << "handle_str: " << _input << std::endl;
}

struct Square {
    void operator()(int _input) const {
        std::cout << "sqr: " << _input * _input << std::endl;
    }
};

template<typename F, typename I>
requires std::indirectly_unary_invocable<F, I>
void test(F _func, I _it) {
    _func(*_it);
}

int main() {
    std::vector<int> v = {1, 2, 3};
    test(handle_int, v.begin());
    // test(handle_str, v.begin()); // Error

    Iterator it = Iterator();
    test(handle_int, *it);
    test(Square(), *it);

    return 0;
}
