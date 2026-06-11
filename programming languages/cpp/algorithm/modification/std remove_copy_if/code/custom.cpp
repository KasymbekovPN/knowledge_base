#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

struct Person {
    string name;
    unsigned age;
};

ostream& operator<<(ostream&, const Person&);
void print_vector(const vector<Person>&);

int main() {
    const vector<Person> people = {
        {"Alice", 25},
        {"Bob", 17},
        {"Charlie", 19},
        {"Diana", 16}
    };
    print_vector(people);

    vector<Person> adults;
    remove_copy_if(
        people.begin(),
        people.end(),
        back_inserter(adults),
        [](const Person& p) { return p.age < 18; });
    print_vector(adults);

    return 0;
}

ostream& operator<<(ostream& os, const Person& person) {
    return os
        << "{name: " << person.name
        << ", age: " << person.age << "}";
}

void print_vector(const vector<Person>& people) {
    cout << "{";
    for (auto &&person: people) {
        cout << person << " ";
    }
    cout << "}" << endl;
}
