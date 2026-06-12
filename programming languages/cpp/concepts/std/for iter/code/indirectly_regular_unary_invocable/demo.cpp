#include <iostream>
#include <vector>
#include <concepts>

struct Person {
    int age{42};

    const Person* operator*() const {
        return this;
    }
};

auto&& good_lambda = [](const Person& _p) {
    std::cout << "good_lambda: " << _p.age << std::endl;
};

auto&& bad_lambda = [](Person& _p) {
    std::cout << "bad_lambda: " << _p.age << std::endl;
};

void handle_int(int _input) {
    std::cout << "handle_int: " << _input << std::endl;
}

template<typename F, typename I>
requires std::indirectly_regular_unary_invocable<F, I>
void test(F _func, I _i) {
    _func(*_i);
}

int main(int argc, char const *argv[]) {
    std::vector<int> v {1, 2, 3};
    test(handle_int, v.begin());

    Person p = Person();
    test(good_lambda, *p);

    return 0;
}
