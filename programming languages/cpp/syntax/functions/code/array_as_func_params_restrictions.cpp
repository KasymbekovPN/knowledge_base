#include <iostream>

using std::cout;
using std::endl;

void test_sizeof(int numbers[]);
void test_size(int numbers[]);
void test_for(int numbers[]);

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2};

    test_sizeof(numbers);
    test_size(numbers);
    test_for(numbers);

    return 0;
}

void test_sizeof(int numbers[]) {
    int size = sizeof(numbers); // Warning
    cout << "[test_sizeof] " << size << endl;
}

void test_size(int numbers[]) {
    // size_t size = std::size(numbers); // Error
}

void test_for(int numbers[]) {
    // for (int n: numbers) {} // Error
}