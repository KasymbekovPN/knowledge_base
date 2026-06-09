---
tags:
  - programming-language
  - cpp
  - syntax
  - class
  - abstract
  - abstract-class
  - virtual
  - pure-virtual-method
---
[[__cpp syntax oop__|<=]]

Иногда возникает необходимость определить класс, который не предполагает создания конкретных объектов, т.е. __абстрактный класс__.

__Абстрактные классы__ - это классы, которые содержат или наследуют без переопределения хотя бы одну __чистую виртуальную функцию__. Абстрактный класс определяет интерфейс для переопределения производными классами.

__Чистые виртуальные функции__ (__pure virtual functions__) - это функции, которые не имеют определения. Цель подобных функций - просто определить функционал без реализации, а реализацию определят производные классы. Чтобы определить виртуальную функцию как чистую, ее объявление завершается значением `=0`. 

__Абстрактный класс__ может определять и обычные функции и переменные, может иметь несколько конструкторов.

```cpp
#include <iostream>
#include <numbers>

class Shape {

private:
    double x {};
    double y {};

public:
    Shape(double, double);
    virtual double getSquare() const = 0;
    virtual double getPerimeter() const = 0;
    void printCoords() const;
};

Shape::Shape(double x, double y): x{x}, y{y} {}

void Shape::printCoords() const {
    std::cout
        << "{x: " << x
        << ", y: " << y
        << "}" << std::endl;
}

class Rectange: public Shape {

private:
    double width;
    double height;

public:
    Rectange(double x, double y, double width, double height);
    double getSquare() const override;
    double getPerimeter() const override;
};

Rectange::Rectange(double x, double y, double width, double height):
    Shape{x, y},
    width{width},
    height{height} {}

double Rectange::getSquare() const {
    return height * width;
}

double Rectange::getPerimeter() const {
    return 2 * (height + width);
}

class Circle: public Shape{

private:
    double r;

public:
    Circle(double, double, double);
    double getSquare() const override;
    double getPerimeter() const override;
};

Circle::Circle(double x, double y, double r):
    Shape{x, y},
    r{r} {}

double Circle::getSquare() const {
    return r * r * std::numbers::pi;
}

double Circle::getPerimeter() const {
    return 2 * std::numbers::pi * r;
}

void print(const Shape*);

int main(int argc, char const *argv[]) {
    print(new Rectange{0, 0, 10, 20});
    print(new Circle{1, 2, 5});

    return 0;
}

void print(const Shape* p_shape) {
    p_shape->printCoords();
    std::cout << "square: " << p_shape->getSquare() << std::endl;;
    std::cout << "perimeter: " << p_shape->getPerimeter() << std::endl;
}
```

```
{x: 0, y: 0}
square: 200
perimeter: 60
{x: 1, y: 2}
square: 78.5398
perimeter: 31.4159
```

---
[Чистые виртуальные функции и абстрактные классы](https://metanit.com/cpp/tutorial/5.12.php)