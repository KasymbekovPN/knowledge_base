#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Person {string name; int age;};

ostream& operator<<(ostream&, const Person&);
void print_vector(const vector<Person>&);

int main() {
    vector<Person> people = {
        {"Alice", 25},
        {"Alice", 25},
        {"Bob", 30},
        {"Bob", 30}
    };
    print_vector(people);

    auto new_end = unique(
        people.begin(),
        people.end(),
        [](const Person& a, const Person& b) {
            return a.name == b.name && a.age == b.age;
        }
    );
    print_vector(people);

    people.erase(new_end, people.end());
    print_vector(people);

    return 0;
}

ostream& operator<<(ostream& os, const Person& person) {
    return os
        << "{name: " << person.name
        << ", " << person.age << "}";
}

void print_vector(const vector<Person>& container) {
    cout << "{";
    for (auto &&item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
