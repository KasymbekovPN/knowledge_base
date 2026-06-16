#include <iostream>

class Person {

private:
    static inline unsigned count {};

    std::string name {"Unnamed"};
    unsigned age {42};

public:
    static inline const unsigned MAX_AGE {120};

    static void print_count();

    Person(std::string, unsigned);
};

Person::Person(std::string name, unsigned age): name(name), age(age) {
    ++count;
    if (this->age < 0 || this->age > MAX_AGE) {
        this->age = MAX_AGE;
    }
}

void Person::print_count() {
    std::cout << "Counter <= " << count << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    Person bob {"Bob", 43};

    Person::print_count();
    std::cout << "Max age <= " << Person::MAX_AGE << std::endl;

    return 0;
}
