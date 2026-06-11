#include <iostream>
#include <vector>
#include <algorithm>
#include<iterator>

using namespace std;

struct Value {
    int x;

    bool operator<(const Value& other) {
        return x < other.x;
    }
};

ostream& operator<<(ostream&, const Value&);
void print_vector(const vector<Value>&);

int main() {
    vector<Value> values {
        {1},
        {5},
        {2},
        {11},
        {9}
    };
    print_vector(values);

    sort(values.begin(), values.end());
    print_vector(values);

    sort(
        values.begin(),
        values.end(),
        [](const Value& a, const Value& b) {return a.x > b.x;}
    );
    print_vector(values);

    return 0;
}

ostream& operator<<(ostream& os, const Value& p) {
    return os
        << "{x: "<< p.x
        << "}";
}

void print_vector(const vector<Value> &container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
