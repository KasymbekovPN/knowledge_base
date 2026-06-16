#include <iostream>

class Person {

private:
    static inline unsigned count {};

    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print();
    static void print_count();
};

Person::Person(std::string name, unsigned age): name(name), age(age) {
    count++;
}

void Person::print() {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << ", count: " << count
        << "}" << std::endl;
}

void Person::print_count() {
    std::cout << "Count <= " << count << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    Person bob {"Bob", 43};
    Person sam {"Sam", 44};

    tom.print();
    bob.print();
    sam.print();

    tom.print_count();
    Person::print_count();

    return 0;
}
