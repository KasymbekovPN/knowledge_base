#include <iostream>

template<typename T=int>
class Person {

private:
    T id;
    std::string name;

public:
    explicit Person(T, std::string) noexcept;
    void setId(T) noexcept;
    virtual void print() const noexcept;
};

template<typename T>
Person<T>::Person(T id, std::string name) noexcept:
    id{id},
    name{name} {}

template<typename T>
void Person<T>::setId(T value) noexcept {
    id = value;
}

template<typename T>
void Person<T>::print() const noexcept {
    std::cout
        << "{id: " << id
        << ", name: " << name
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person<int> tom {123, "Tom"};
    tom.print();

    Person<std::string> bob {"qwerty", "Bob"};
    bob.print();

    return 0;
}
