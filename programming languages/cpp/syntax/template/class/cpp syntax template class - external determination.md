---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - class
---
[[_cpp syntax template class|<=]]

Синтаксис определения функций вне шаблона класса может немного отличаться от их определения внутри шаблона. В частности, определения функций вне шаблона класса должны определяться как шаблон, даже если они не используют параметры шаблона.

```cpp
#include <iostream>

template<class T>
class Person {

private:
    T id;
    std::string name;

public:
    explicit Person(T, std::string) noexcept;
    Person(const Person&) noexcept;
    virtual ~Person() noexcept;
    virtual Person& operator=(const Person&) noexcept;
    virtual void print() const noexcept;
};

template<class T>
Person<T>::Person(T id, std::string name) noexcept:
	id{id},
	name{name} {}

template<class T> Person<T>::Person(const Person& other) noexcept: id{other.id}, name{other.name} {}

template<class T> Person<T>::~Person() noexcept {
    std::cout << "Person deleted" << std::endl;
}

template<class T>
Person<T>& Person<T>::operator=(const Person<T>& other) noexcept{
    if (this != &other) {
        id = other.id;
        name = other.name;
    }
    return *this;
}

template<class T> void Person<T>::print() const noexcept {
    std::cout
        << "{id: " << id
        << ", name: " << name
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person<int> tom {12345, "Tom"};
    tom.print();

    Person<int> tomas {tom};
    tomas.print();

    Person<int> tommy = tom;
    tommy.print();

    return 0;
}
```

```
{id: 12345, name: Tom}
{id: 12345, name: Tom}
{id: 12345, name: Tom}
Person deleted
Person deleted
Person deleted
```

---
[Шаблон класса](https://metanit.com/cpp/tutorial/9.1.php)