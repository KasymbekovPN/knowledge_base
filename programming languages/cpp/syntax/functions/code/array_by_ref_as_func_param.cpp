#include <iostream>

void func(const int (&)[5]);

int main(int argc, char const *argv[]) {
    int numbers[] {1, 2, 3, 4, 5};
    func(numbers);

    return 0;
}

void func(const int (&numbers)[5]) {
    for (size_t i {}; i < 5; i++) {
        std::cout << "[func] " << numbers[i] << std::endl;
    }
}
