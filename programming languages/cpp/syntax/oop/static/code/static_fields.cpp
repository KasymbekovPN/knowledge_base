#include <iostream>

class Person {

private:
    static inline unsigned count {};

    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print_count();
};

Person::Person(std::string name, unsigned age): name(name), age(age) {
    count++;
}

void Person::print_count() {
    std::cout << "Count <= " << count << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print_count();

    Person bob {"Bob", 43};
    tom.print_count();

    Person sam {"Sam", 44};
    tom.print_count();

    return 0;
}
