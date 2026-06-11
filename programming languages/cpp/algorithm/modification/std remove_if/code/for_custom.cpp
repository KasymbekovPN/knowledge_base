#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Person {
    string name;
    unsigned age;
};

ostream& operator<<(ostream&, const Person&);
void print_vector(const vector<Person>&);

int main() {
    vector<Person> people = {
        {"Alice", 25},
        {"Bob", 17},
        {"Charlie", 19},
        {"Diana", 16}
    };
    print_vector(people);

    auto new_end = remove_if(
        people.begin(),
        people.end(),
        [](const Person& p) { return p.age > 18; }
    );
    print_vector(people);

    people.erase(new_end, people.end());
    print_vector(people);
    
    return 0;
}


ostream& operator<<(ostream& os, const Person& person) {
    return os << "{ name: " << person.name << ", age: " << person.age << "}";
}

void print_vector(const vector<Person>& vec) {
    cout << "{";
    for (auto &&i: vec) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
