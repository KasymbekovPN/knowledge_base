---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - class
---
[[_cpp syntax template class|<=]]

Параметры шаблонов могут иметь значения по умолчанию - тип по умолчанию, который будет использоваться.

```cpp
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
```

```
{id: 123, name: Tom}
{id: qwerty, name: Bob}
```

---
[Шаблон класса](https://metanit.com/cpp/tutorial/9.1.php)