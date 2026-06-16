#include <iostream>

class Person
{
private:
    std::string private_name;
    unsigned private_age;
public:
    std::string public_name;
    unsigned public_age;
    
    Person(std::string p_name, unsigned p_age):
        private_name(p_name),
        public_name(p_name),
        private_age(p_age),
        public_age(p_age) {};

    void print() {
        std::cout
            << "{private_name: " << private_name
            << ", public_name: " << public_name
            << ", private_age: " << private_age
            << ", public_age: " << public_age
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person tom {"Tom", 42};
    tom.public_name += " PUB";
    tom.public_age++;
    // tom.private_name += "PRI"; //< Error
    // tom.private_age += 2; //< Error

    tom.print();

    return 0;
}
