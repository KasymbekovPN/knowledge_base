#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

struct Person {
    string name;
    unsigned age;
};

const ostream& operator<<(ostream&, const Person&);
void print_vector(const vector<Person>&, const string&&);

int main() {
    vector<Person> people = {
        {"Alice", 70}, {"Bob", 75}, {"Charlie", 80},
        {"Diana", 65}, {"Eve", 85}
    };
    print_vector(people, "original");

    inplace_merge(
        people.begin(),
        people.begin() + 3,
        people.end(),
        [](const Person& _p0, const Person& _p1) {
            return _p0.age < _p1.age;
        }
    );
    print_vector(people, "after");

    return 0;
}

const ostream& operator<<(ostream& _os, const Person& _person) {
    return _os
        << "{name: " << _person.name
        << ", age: " << _person.age << "}";
}

void print_vector(const vector<Person>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "] {";
    string delimiter {""};
    for (auto &&item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
