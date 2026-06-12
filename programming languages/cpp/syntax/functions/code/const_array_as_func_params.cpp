#include <iostream>

void print(const int*, const size_t);
void twice(int*, const size_t);

int main(int argc, char const *argv[]) {
    int numbers[] {1, 2, 3, 4, 5};
    size_t size = std::size(numbers);

    print(numbers, size);
    twice(numbers, size);
    print(numbers, size);

    return 0;
}

void print(const int numbers[], const size_t size) {
    for (size_t i {}; i < size; i++) {
        std::cout << "[print] " << numbers[i] << std::endl;
    }
}

void twice(int* numbers, const size_t size) {
    for (size_t i {}; i < size; i++) {
        numbers[i] = 2 * numbers[i];
    }
}
