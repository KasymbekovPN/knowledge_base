#include <iostream>

template<typename T>
class Person {

private:
    T id;
    std::string name;

public:
    explicit Person(std::string) noexcept;
    virtual void setId(T) noexcept;
    virtual void print() const noexcept;
};

template<typename T>
Person<T>::Person(std::string name) noexcept:
    name{name} {}

template<typename T>
void Person<T>::setId(T id) noexcept {
    this->id = id;
}

template<typename T>
void Person<T>::print() const noexcept {
    std::cout
        << "{name: " << name
        << ", id: " << id
        << "}" << std::endl;
}

template <>
class Person<unsigned>{

private:
    static inline unsigned counter {};
    unsigned id;
    std::string name;

public:
    explicit Person(std::string name) noexcept: name{name} {
        id = ++counter;
    }
    virtual void print() const noexcept {
        std::cout
            << "{name: " << name
            << ", id: " << id
            << "}" << std::endl;        
    }
};

int main(int argc, char const *argv[]) {
    Person<std::string> tom {"Tom"};
    tom.setId("123");
    tom.print();

    Person<unsigned> bob {"Bob"};
    bob.print();

    Person<unsigned> bobby {"Bobby"};
    bobby.print();
    // bobby.setId(123); // <= Error

    return 0;
}
