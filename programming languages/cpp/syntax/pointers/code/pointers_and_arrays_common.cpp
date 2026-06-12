#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2, 3, 4};
    cout << "numbers[0] address <= " << numbers << endl;
    cout << "numbers[0] value   <= " << *numbers << endl;
    cout << "---" << endl;

    for (size_t i = 0; i < std::size(numbers); i++){
        cout << "numbers[" << i << "] | address <= " << numbers + i << " | value <= " << *(numbers + i) << endl;
    }
    

    return 0;
}
