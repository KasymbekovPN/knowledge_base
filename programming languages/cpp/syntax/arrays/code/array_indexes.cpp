#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const size_t SIZE = 4;
    int nums[SIZE] {100, 101, 102, 103};

    int &first {nums[0]};
    cout << "first <= " << first << endl;

    first++;
    cout << "first after increment <= " << first << endl;

    int seventh = nums[7];
    cout << "seventh <= " << seventh << endl;

    return 0;
}
