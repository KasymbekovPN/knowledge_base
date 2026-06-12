#include <iostream>

using std::cout;
using std::endl;

void test_by_closed_item(const char[]);
void test_by_size(const int[], size_t);
void test_by_range(int*, int*);


int main(int argc, char const *argv[]) {
    const char line[] {"Hello"};
    const int numbers[] {0, 1, 2, 4};

    test_by_closed_item(line);
    test_by_size(numbers, std::size(numbers));
    test_by_range((int*) std::begin(numbers), (int*) std::end(numbers));

    return 0;
}

void test_by_closed_item(const char chars[]) {
    for (size_t i = 0; chars[i] != '\0'; i++) {
        cout << "[test_by_closed_item] " << chars[i]  << endl;
    }
}

void test_by_size(const int numbers[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        cout << "[test_by_size] " << numbers[i] << endl;
    }
}

void test_by_range(int* p_begin, int* p_end) {
    for (int *ptr {p_begin}; ptr != p_end; ptr++) {
        cout << "[test_by_range] " << *ptr << endl;
    }
}
