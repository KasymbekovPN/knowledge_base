
#include <iostream>

class Person {

private:
    std::string name;

public:
    explicit Person(std::string) noexcept;
    virtual ~Person() noexcept;
    void print() const;
};

Person::Person(std::string name) noexcept : name{name} {
    std::cout << "Created" << std::endl;
}

Person::~Person() noexcept{
    std::cout << "Deleted" << std::endl;
}

void Person::print() const {
    throw std::exception("Print error");
}

int main(int argc, char const *argv[]) {
    try {
        Person p = Person("Tom");
        p.print();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}
