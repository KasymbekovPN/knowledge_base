#include <iostream>
#include <list>
#include <string>

struct Person {
    std::string name;
    unsigned age;

    Person(std::string name, unsigned age):
        name{name},
        age{age} {}

    std::string to_string() const {
        return "{name: " + name + ", age: " + std::to_string(age) + "}";
    }
};

void print_list(const std::list<Person>&);

int main() {
    std::list<Person> people = {
        {"Alice", 25},
        {"Bob", 30},
        {"John", 20}
    };
    print_list(people);

    people.sort([](const Person& a, const Person& b) {
        return a.age < b.age;
    });
    print_list(people);

    people.sort([](const Person& a, const Person& b) {
        return a.name > b.name;
    });
    print_list(people);   

    return 0;
}

void print_list(const std::list<Person>& list) {
    for (auto &item: list) {
        std::cout << item.to_string() << " ";
    }
    std::cout << std::endl;
}
