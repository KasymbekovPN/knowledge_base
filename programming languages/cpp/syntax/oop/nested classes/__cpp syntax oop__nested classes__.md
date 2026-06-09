---
tags:
  - programming-language
  - cpp
  - syntax
  - nested
---
[[__cpp syntax oop__|<=]]

__Вложенный класс__ (__nested class__) — это класс, определение которого находится внутри другого класса. Обычно вложенные классы применяются для описания таких сущностей, которые могут существовать только в рамках объекта внешнего класса, особенно когда внешний класс работает с набором объектов вложенного класса.

```cpp
#include <iostream>

class Person {

private:
    class Account {

    public:
        std::string email;
        Account(std::string email = ""): email(email) {}
    };

    std::string name;
    Account account {};

public:
    Person(std::string, std::string);
    void print() const;
};

Person::Person(std::string name, std::string email):
    name{name},
    account{Account{email}} {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << "email: " << account.email
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", "t@localhost.su"};
    tom.print();

    return 0;
}
```

```
{name: Tomemail: t@localhost.su}
```

Во вложенных классах также можно использовать спецификаторы доступа. 

Функции вложенного класса могут напрямую ссылаться на статические члены внешнего класса, а также на любые другие типы, определенные во внешнем классе. 

Доступ к другим членам внешнего класса можно получить из вложенного класса стандартными способами: через объект класса, указатель или ссылку на объект класса. При этом функции вложенного класса могут обращаться в том числе к приватным переменным и константам, которые определены во внешнем классе.

---
[Вложенные классы](https://metanit.com/cpp/tutorial/5.19.php)