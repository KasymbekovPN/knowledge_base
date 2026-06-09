---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - class
---
[[_cpp syntax template class|<=]]

__Шаблон класса__ (__class template__) позволяет задать внутри класса объекты, тип которых на этапе написания кода неизвестен. Шаблоны классов позволяют уменьшить повторяемость кода. Для определения шаблона класса применяется следующий синтаксис.
```cpp
template <parameters-list>
class class_name {
	// ...
}
```

Параметр в угловых скобках представляет произвольный идентификатор, перед которым указывается слово typename или class.
```cpp
template <typename T>

// OR

template <class T>
```

```cpp
#include <iostream>

template <class T>
class Person {

private:
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

int main(int argc, char const *argv[]) {
    Person<int> tom {12345, "Tom"};
    tom.print();

    Person<std::string> bob {"qwery", "Bob"};
    bob.print();

    return 0;
}
```

```
{id: 12345, name: Tom}
{id: qwery, name: Bob}
```

---
[Шаблон класса](https://metanit.com/cpp/tutorial/9.1.php)