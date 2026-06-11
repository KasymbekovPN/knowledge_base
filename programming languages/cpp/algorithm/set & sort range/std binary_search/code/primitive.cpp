#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {1, 3, 5, 7, 9, 11};
    bool found = binary_search(numbers.begin(), numbers.end(), 7);
    cout << "0) " << boolalpha << found << endl;

    reverse(numbers.begin(), numbers.end());
    found = binary_search(
        numbers.begin(), numbers.end(),
        7, greater<int>()
    );
    cout << "0) " << found << noboolalpha << endl;

    return 0;
}
