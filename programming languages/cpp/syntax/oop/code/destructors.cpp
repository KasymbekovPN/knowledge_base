#include <iostream>

class Person {

private:
    static inline unsigned counter {};

    std::string name {"Unnamed"};

public:
    Person(std::string);
    ~Person();
};

Person::Person(std::string name): name(name) {
    counter++;
    std::cout << "Person '" << this->name << "' created. Counter <= " << counter << std::endl;
}

Person::~Person() {
    counter--;
    std::cout << "Person '" << this->name << "' deleted. Counter <= " << counter << std::endl;
}

int main(int argc, char const *argv[]) {
    {
        Person tom ("Tom");
        Person bob ("Bob");
    }
    Person sam ("Sam");

    return 0;
}
