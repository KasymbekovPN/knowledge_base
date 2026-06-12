#include <iostream>
#include <concepts>
#include <vector>
#include <string>

struct Person {
    std::string name;
    size_t age;

    Person(std::string _name, size_t _age):
        name{_name},
        age{_age} {}
    
    void swap(Person& _other) noexcept {
        name.swap(_other.name);
        std::swap(age, _other.age);
    }
};

void swap(Person& _target, Person& _source) {
    _target.swap(_source);
}

std::ostream& operator<<(std::ostream& _stream, const Person& _person)  {
    _stream
        << "{name: " << _person.name
        << ", age: "  << _person.age << "}";
    return _stream;
}

template<std::swappable T>
void test(T&, T&);

int main() {
    int x{10};
    int y{20};
    test(x, y);
    std::cout
        << "x: " << x 
        << " y: " << y << std::endl;

    auto&& print_vector = [](std::vector<int>& _vector) -> std::string {
        std::string result{""};
        std::string delimiter{""};
        for (auto&& item: _vector) {
            result += delimiter + std::to_string(item);
            delimiter = ", ";
        }
        return result;
    };
    std::vector<int> v0 {1, 2};
    std::vector<int> v1 {3, 4};
    test(v0, v1);
    std::cout
        << "v0: " << print_vector(v0)
        << " v1: " << print_vector(v1) << std::endl;

    Person p0 {"p0", 40};
    Person p1 {"p1", 41};
    test(p0, p1);
    std::cout
        << "p0: " << p0
        << " p1: " << p1 << std::endl;

    return 0;
}

template<std::swappable T>
void test(T& _target, T& _source) {
    std::swap(_target, _source);
}
