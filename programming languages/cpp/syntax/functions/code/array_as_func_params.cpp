#include <iostream>

using std::cout;
using std::endl;

void func_as_array(const int numbers[]);
void func_as_ptr(const int* const ptr);

int main(int argc, char const *argv[]) {
    const int numbers[] {0, 1, 2};

    func_as_array(numbers);
    func_as_ptr(numbers);

    return 0;
}

void func_as_array(const int numbers[]) {
    cout << "numbers[0] <= " << numbers[0] << endl;
}

void func_as_ptr(const int* const ptr) {
    cout << "*ptr <= "  << *ptr << endl;
}
