---
tags:
  - programming-language
  - cpp
  - syntax
  - inheritance
  - oop
---
[[__cpp syntax oop__|<==]]

Производный класс может иметь несколько прямых базовых классов. Подобный тип наследования называется множественным наследованием в отличие от одиночного наследования, при котором используется один базовый класс. 

```cpp
#include <iostream>

class Camera {

public:
    void makePhoto() const;
};

void Camera::makePhoto() const {
    std::cout << "Making photo" << std::endl;
}


class Phone {

public:
    void makeCall() const;
};

void Phone::makeCall() const {
    std::cout << "Making call" << std::endl;
}


class Smartpone: public Phone, public Camera{};


int main(int argc, char const *argv[]) {
    Smartpone sp;
    sp.makeCall();
    sp.makePhoto();

    return 0;
}
```

```
Making call
Making photo
```

Стоит обратить внимание, что при установке наследования для каждого базового класса указывается спецификатор доступа.

[[multiple inheritance constructors and destructors]]
[[duality with equal names]]
[[multiple inheritance and virtual base class]]

---
[Множественное наследование](https://metanit.com/cpp/tutorial/5.24.php)