#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

int main() {
    const vector<int> source {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    vector<int> sample_data (3);

    sample(
        source.begin(),
        source.end(),
        sample_data.begin(),
        sample_data.size(),
        mt19937(random_device()())
    );

    cout << "{";
    for(auto& item: sample_data) {
        cout << " " << item;
    }
    cout << " }" << endl;

    return 0;
}
