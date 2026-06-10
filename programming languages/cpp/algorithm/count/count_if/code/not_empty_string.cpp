#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int main() {
    vector<string> data = {"Hello", "", "World", " ", "C++", ""};
    int count = count_if(data.begin(), data.end(), [](const string& s){
        return !s.empty();
    });
    cout << "Count: " << count << endl;

    return 0;
}
