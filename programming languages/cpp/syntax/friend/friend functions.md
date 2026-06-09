---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - friend
---
[[__cpp syntax friend__|<==]]

__Дружественные функции__ - это функции, которые не являются членами класса, однако имеют доступ к его закрытым членам - переменным и функциям, которые имеют спецификатор private.

Для определения дружественных функций используется ключевое слово _friend_.

```cpp
#include <iostream>

class Auto {

friend void drive(const Auto&);
friend void setPrice(Auto&, unsigned);

private:
    std::string name;
    unsigned price;

public:
    Auto(std::string name, unsigned price);
    void print();
};

Auto::Auto(std::string name, unsigned price): name(name), price(price) {}

void Auto::print() {
    std::cout
        << "{name: " << name
        << ", " << price
        << "}" << std::endl;
}

void drive(const Auto& car) {
    std::cout <<  car.name << " is driven" << std::endl;
}

void setPrice(Auto& car, unsigned price) {
    car.price = price;
}

int main(int argc, char const *argv[]) {
    Auto tesla{"Tesla", 5000};
    tesla.print();

    drive(tesla);
    setPrice(tesla, 4000);
    tesla.print();

    return 0;
}
```

```
{name: Tesla, 5000}
Tesla is driven
{name: Tesla, 4000}
```

Здесь определен класс _Auto_, который представляет автомобиль. У этого класса определены приватные закрытые переменные . Также в классе объявлены две дружественные функции: _drive_ и _setPrice_. Обе этих функции принимают в качестве параметра ссылку на объект _Auto_.

Когда мы объявляем дружественные функции, то фактически мы говорим компилятору, что это друзья класса и они имеют доступ ко всем членам этого класса, в том числе закрытым.

При этом для дружественных функций не важно, определяются они под спецификатором _public_ или _private_. Для них это не имеет значения.

Определение этих функций производится вне класса. И поскольку эти функции являются дружественными, то внутри этих функций мы можем через переданную ссылку _Auto_ обратиться ко всем его закрытым переменным.

---

__Дружественные функции__ могут определяться в другом классе.

```cpp
#include <iostream>

class Auto;

class Person {

private:
    std::string name;

public:
    Person(std::string);
    void drive(const Auto&);
    void setPrice(Auto&, unsigned);
};

class Auto {

friend void Person::drive(const Auto&);
friend void Person::setPrice(Auto&, unsigned);

private:
    std::string name;
    unsigned price;

public:
    Auto(std::string, unsigned);
    void print();
};

Person::Person(std::string name): name(name) {}

void Person::drive(const Auto& car) {
    std::cout << name << " drives " << car.name << std::endl;
}

void Person::setPrice(Auto& car, unsigned price) {
    car.price = price;
}

Auto::Auto(std::string name, unsigned price): name(name), price(price){}

void Auto::print() {
    std::cout
        << "{name: " << name
        << ", price: " << price
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Auto tesla {"Tesla", 5000};
    Person tom {"Tom"};
    tesla.print();

    tom.drive(tesla);
    tom.setPrice(tesla, 5550);
    tesla.print();

    return 0;
}
```

```
{name: Tesla, price: 5000}
Tom drives Tesla
{name: Tesla, price: 5550}
```

---
[Дружественные функции и классы](https://metanit.com/cpp/tutorial/5.5.php)