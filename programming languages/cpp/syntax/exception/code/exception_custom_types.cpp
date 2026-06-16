#include <iostream>

class AgeException {

private:
    std::string message;

public:
    explicit AgeException(std::string) noexcept;
    std::string getMessage() const noexcept;
};

AgeException::AgeException(std::string message) noexcept: message{message} {}

std::string AgeException::getMessage() const noexcept {
    return message;
}

class Person {

private:
    std::string name;
    unsigned age;

public:
    explicit Person(std::string, unsigned);
    void print() const noexcept;
};

Person::Person(std::string name, unsigned age): name{name}, age{age} {
    if (!this->age || this->age > 110) {
        throw AgeException{"Invalid age"};
    }
}

void Person::print() const noexcept {
    std::cout
        << "{name: " << name
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    try {
        Person tom {"Tom", 42};
        tom.print();

        Person bob {"Bob", 1234};
        bob.print();
    } catch(const AgeException& e) {
        std::cerr << e.getMessage() << std::endl;
    } catch(...) {
        std::cerr << "Unknown exception" << std::endl;
    }
    
    return 0;
}
