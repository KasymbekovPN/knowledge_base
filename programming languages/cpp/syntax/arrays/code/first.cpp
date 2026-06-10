#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const size_t size = 4;

    int undef_numbers[size];
    for (size_t i = 0; i < size; i++) {
        cout << "undef_numbers[" << i << "] <= " << undef_numbers[i] << endl;
    }
    cout << endl;
    
    int zero_numbers[size] {};
    for (size_t i = 0; i < size; i++) {
        cout << "zero_numbers[" << i << "] <= " << zero_numbers[i] << endl;
    }
    cout << endl;
    
    int set_numbers[size] {0, 1, 2, 3};
    for (size_t i = 0; i < size; i++) {
        cout << "set_numbers[" << i << "] <= " << set_numbers[i] << endl;
    }
    cout << endl;

    int not_completed_set_numbers[size] {100, 101};
    for (size_t i = 0; i < size; i++) {
        cout << "not_completed_set_numbers[" << i << "] <= " << not_completed_set_numbers[i] << endl;
    }

    // int bad_size_array[size] {1, 2, 3, 4, 5};

    int nums[] {1, 2, 3};
    // int nums_copy = nums;

    return 0;
}
