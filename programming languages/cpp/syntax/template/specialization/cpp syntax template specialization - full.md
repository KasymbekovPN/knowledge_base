---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - specialization
---
[[_cpp syntax template specialization|<=]]

При __полной специализации шаблона__ указываются значения для всех параметров шаблона. И тогда для указанного набора аргументов (типов) компилятор будет использовать специализацию шаблона, а не создавать класс на основе шаблона. 

```cpp
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
```

```
{name: Tom, id: 123}
{name: Bob, id: 1}
{name: Bobby, id: 2}
```

Вначале надо определить сам шаблон. В данном случае это шаблон класса _Person_, который принимает один параметр. После шаблона класса идет специализация шаблона.

Cпециализация шаблона класса необязательно должна иметь те же члены, что и сам шаблон: специализация шаблона может изменять, добавлять или опускать члены без ограничений. Так, в данном случае _id_ представляет тип _unsigned_ и генерируется в конструкторе на основе дополнительно добавленной статической переменной. 

Если на экземпляре `Person<unsigned>` вызвать метод _setId_, то получится следующая ошибка
```
.\full_specialization.cpp:63:11: error: no member named 'setId' in 'Person<unsigned int>'
   63 |     bobby.setId(123);
      |     ~~~~~ ^
```

---
[Специализация шаблона](https://metanit.com/cpp/tutorial/9.3.php)