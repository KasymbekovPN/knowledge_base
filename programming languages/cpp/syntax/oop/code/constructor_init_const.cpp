#include <iostream>

class Person {

public:
    const std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age): name(p_name) {
        age = p_age;
    }

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    return 0;
}
