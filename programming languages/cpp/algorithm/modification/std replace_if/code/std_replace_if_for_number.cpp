#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    vector<int> v {1, 2, 3, 4, 5, 6, 7};

    replace_if(v.begin(), v.end(), [](int x) {
       return x % 2 == 0; 
    }, -1);
    _print_vector(v);
    
    return 0;
}


void _print_vector(const vector<int>& vector) {
    cout << "{ ";
    for (auto &i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
