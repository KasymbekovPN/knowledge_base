#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Person {
    string name;
    unsigned age;
};

ostream& operator<<(ostream&, const Person&);
void _print_vector(const vector<Person>&);

ostream& operator<<(ostream& os, const Person& p) {
    return os << "{name: " << p.name << ", " << p.age << "}";
}

int main() {
    vector<Person> people = {
        {"Alice", 17},
        {"Bob", 25},
        {"Charlie", 15},
        {"Diana", 18}
    };

    replace_if(
        people.begin(),
        people.end(),
        [](const Person& p) { return p.age >= 18; },
        Person("Adult", 18)
    );
    _print_vector(people);

    return 0;
}


void _print_vector(const vector<Person>& vector) {
    cout << "{";
    for (const auto &i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
