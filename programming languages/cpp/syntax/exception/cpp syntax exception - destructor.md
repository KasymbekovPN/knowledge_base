---
tags:
  - programming-language
  - cpp
  - syntax
  - exception
---
[[_cpp syntax - exception|<=]]

Если в блоке _try_ создаются некоторые объекты, то при возникновении исключения (перед обработкой _catch_) у них вызываются деструкторы.

```cpp
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
```

```
Created
Deleted
Print error
```

---
[Обработка исключений](https://metanit.com/cpp/tutorial/6.1.php)