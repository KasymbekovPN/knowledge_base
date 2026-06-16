#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string p_name, unsigned p_age): name(p_name){
        age = p_age > 0 && p_age < 110 ? p_age : 18;
    }

    void setName(std::string p_name){
        name = p_name;
    }

    std::string getName() {
        return name;
    }

    void setAge(unsigned p_age) {
        if (p_age > 0 && p_age < 110) {
            age = p_age;
        }
    }

    unsigned getAge() {
        return age;
    }

    void print() {
        std::cout
            << "{name: " << name
            << ", age: " << age
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.print();

    tom.setName(tom.getName() + " !!!");
    tom.setAge(tom.getAge() + 1);
    tom.print();

    return 0;
}
