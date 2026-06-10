#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    const int ARR[] {5, 10, 5, 15, 5, 20};
    size_t size = std::size(ARR);

    const int TARGET {5};
    cout
        << "Number of " << TARGET
        << " is " << count(ARR, ARR + size, TARGET)
        << endl;

    return 0;
}
