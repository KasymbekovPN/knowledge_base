#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

void _test_find(const int*, const int*, const int);

int main() {
    const int arr[] {10, 20, 30, 40, 50};
    const int* end = arr + sizeof(arr) / sizeof(arr[0]);

    const vector<int> values {29, 30, 31};
    for(auto& value: values) {
        _test_find(arr, end, value);
    }

    return 0;
}

void _test_find(const int* arr, const int* end, const int value) {
    auto it = std::find(arr, end, value);
    if (it != end) {
        cout
            << "Found '" << value << "' at position "
            << (it - arr) << endl;
    } else {
        cout << value << " not found" << endl;
    }
}
