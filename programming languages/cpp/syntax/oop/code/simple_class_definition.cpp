#include <iostream>
#include <string>

class Person {
public:
    std::string name {"NoName"};
    unsigned age {42};

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person first_person;
    Person second_person;
    second_person.age++;
    second_person.name = "Tom";

    std::cout << "First => " << std::endl;
    first_person.print();
    std::cout << "Second => " << std::endl;
    second_person.print();
    
    return 0;
}
