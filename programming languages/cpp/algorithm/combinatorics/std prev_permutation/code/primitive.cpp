#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {3, 2, 1};
    do {
        cout << "0 ) ";
        for (auto& item: numbers) {
            cout << item << " ";
        }
        cout << endl;
    } while (prev_permutation(numbers.begin(), numbers.end()));

    reverse(numbers.begin(), numbers.end());    
    do {
        cout << "1 ) ";
        for (auto& item: numbers) {
            cout << item << " ";
        }
        cout << endl;
    } while (prev_permutation(numbers.begin(), numbers.end(), greater<int>()));

    return 0;
}
