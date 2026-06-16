#include <iostream>

class Person {

public:
    std::string name;
    unsigned age;

    Person(std::string p_name = "NoName", unsigned p_age = 42) {
        name = p_name;
        age = p_age;
    }

    void print() {
        std::cout << "{name: '" << name << "', age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 21};
    Person bob {"Bob"};
    Person sam;

    tom.print();
    bob.print();
    sam.print();

    return 0;
}
