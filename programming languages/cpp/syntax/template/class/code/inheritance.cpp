#include <iostream>

template<typename T>
class Person {

protected:
    T id;
    std::string name;

public:
    explicit Person(T id, std::string name) noexcept: id{id}, name{name} {}
    void print() const noexcept {
        std::cout
            << "{id: " << id
            << ", name: " << name
            << "}" << std::endl;
    }
};

template<typename T>
class Employee: Person<T>{

private:
    std::string company;

public:
    explicit Employee(T id, std::string name, std::string company) noexcept:
        Person<T>{id, name},
        company{company} {}
    void print() const noexcept {
        Person<T>::print();
        std::cout
            << Person<T>::name << " is working in "
            << company << std::endl;
    }
};

class UEmployee: Person<unsigned>{

private:
    std::string company;

public:
    explicit UEmployee(unsigned id, std::string name, std::string company) noexcept:
        Person{id, name},
        company{company} {}
    void print() const noexcept {
        Person::print();
        std::cout
            << name << " is working in "
            << company << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Employee<std::string> tom {"xyz", "Tom", "Company"};
    tom.print();

    UEmployee bob {123, "Bob", "Company"};
    bob.print();

    return 0;
}
