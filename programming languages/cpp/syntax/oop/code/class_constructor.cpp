#include <iostream>

class Person {
public:
    std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout << "Person '" << name << "'(" << age << ") has deen created." << std::endl;
    }

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl; 
    }
};

int main(int argc, char const *argv[]) {
    Person tom("Tom", 42);
    Person bob{"Bob", 43};
    Person sam = Person("Sam", 44);

    tom.print();
    bob.print();
    sam.print();

    return 0;
}
