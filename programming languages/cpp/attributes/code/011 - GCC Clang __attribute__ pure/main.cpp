#include <iostream>

namespace {

    int counter{};

    __attribute__((pure))
    int badExample() {
        // это pure — читает состояние, но не меняет его
        return counter;
    }

    __attribute__((pure))
    int stringLength(const char* s) {
        int len{};
        // читает из памяти по указателю — не "const", но "pure"
        while (s[len]) ++len;

        return len;
    }

    void loop(const char* s) {
        for (int i{}; i < stringLength(s); ++i) {
            // без pure компилятор обязан пересчитывать stringLength(s) каждую итерацию
            // с pure — может вынести вызов за пределы цикла, если знает, что s не меняется
            std::cout << s[i];
        }
        std::cout << std::endl;
    }
}

int main() {
    constexpr auto line{"Hello"};
    loop(line);

    std::cout << badExample() << std::endl;

    return 0;
}


