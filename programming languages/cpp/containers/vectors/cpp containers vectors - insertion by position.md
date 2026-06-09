---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers vectors|<=]]

В C++ для вставки элементов в вектор (`std::vector`) существует несколько методов.

#### insert

Позволяет вставлять один, несколько или диапазон элементов в указанную позицию. __Элемент/элементы__ попадают в метод уже созданными либо __копия__, либо __перемещенный__ объект.

_Использовать_:
- когда есть готовый объект, который нужно вставить
- когда нужно вставить несколько копий или диапазон

```cpp
#include <iostream>
#include <vector>
  
class Point {


private:
    float x;
    float y;
  
public:
    Point(float x, float y) noexcept:
        x{x},
        y{y} {}
    Point(const Point& point):
        x{point.x},
        y{point.y} {
        std::cout << "Copy!" << std::endl;
    }
    void print() const {
        std::cout
            << "{" << x
            << ", " << y
            << "}" << std::endl;
    }
};

void print_vector(const std::vector<Point>&);

int main(int argc, char const *argv[]) {
    std::vector<Point> points {{1, 2}, {2, 3}};
    print_vector(points);

    points.insert(points.begin() + 1, {3, 4});
    print_vector(points);
  
    points.insert(points.begin() + 1, {{4, 5}, {5, 6}});
    print_vector(points);

    std::vector<Point> others {{6, 7}, {7, 8}};
    points.insert(points.end(), others.begin(), others.end());
    print_vector(points);

    return 0;
}

void print_vector(const std::vector<Point>& vector) {
    std::cout << "#######" << std::endl;
    for (const auto &pointer: vector) {
        pointer.print();
    }
}
```

```
Copy!
Copy!
#######
{1, 2}
{2, 3}
Copy!
Copy!
Copy!
#######
{1, 2}
{3, 4}
{2, 3}
Copy!
Copy!
Copy!
Copy!
Copy!
#######
{1, 2}
{4, 5}
{5, 6}
{3, 4}
{2, 3}
Copy!
Copy!
Copy!
Copy!
Copy!
Copy!
Copy!
Copy!
Copy!
#######
{1, 2}
{4, 5}
{5, 6}
{3, 4}
{2, 3}
{6, 7}
{7, 8}
```
#### emplace
- Создает элемент непосредственно в указанной позиции, используя переданные аргументы для вызова конструктора
- Может приводить реаллокиции, если если не хватает ёмкости вектора

Использовать:
- для того, чтобы избежать лишнего копирования или перемещения
- подходит для сложных типов

```cpp
#include <iostream>
#include <vector>

class Point {

private:
    float x;
    float y;

public:
    Point(float x, float y) noexcept:
        x{x},
        y{y} {}
    Point(const Point& point):
        x{point.x},
        y{point.y} {
        std::cout << "Copy!" << std::endl;
    }
    void print() const {
        std::cout
            << "{" << x
            << ", " << y
            << "}" << std::endl;
    }
};

void print_vector(const std::vector<Point>&);

int main(int argc, char const *argv[]) {
    std::vector<Point> points {{1, 2}, {2, 3}};
    print_vector(points);

    points.emplace(points.begin() + 1, 3, 4);
    print_vector(points);

    return 0;
}

void print_vector(const std::vector<Point>& vector) {
    std::cout << "#######" << std::endl;
    for (const auto &point: vector) {
        point.print();
    }
}
```

```
Copy!
Copy!
#######
{1, 2}
{2, 3}
Copy!
Copy!
#######
{1, 2}
{3, 4}
{2, 3}
```

#### assign
Заменяет содержимое вектора новыми элементами. Может использоваться для вставки диапазона элементов.
- Замена содержимого вектора другим диапазоном
- Заполнение вектора повторяющимися значениями
- Замена содержимого вектора списком инициализации
- Очистка вектора и заполнение его новыми элементами

```cpp
#include <iostream>
#include <vector>

class Point {

private:
    float x;
    float y;

public:
    Point(float x, float y) noexcept:
        x{x},
        y{y} {}
    void print(bool rn = false) const {
        std::cout << "{" << x << ", " << y << "} ";
        if (rn) {
            std::cout << std::endl;
        }
    }
};

void print_vector(const std::vector<Point>&);

int main(int argc, char const *argv[]) {
    std::vector<Point> original_vector {{1, 2}, {3, 4}};
    std::vector<Point> target_vector;

    target_vector.assign(original_vector.begin(), original_vector.end());
    print_vector(target_vector);

    target_vector.assign(7, {42, 12});
    print_vector(target_vector);

    target_vector.assign({{1, 1}, {2, 2}, {3, 3}});
    print_vector(target_vector);

    return 0;
}

void print_vector(const std::vector<Point>& vector) {
    for (const auto &point: vector) {
        point.print();
    }
    std::cout << std::endl;
}
```

```
{1, 2} {3, 4} 
{42, 12} {42, 12} {42, 12} {42, 12} {42, 12} {42, 12} {42, 12}
{1, 1} {2, 2} {3, 3}
```

---
[Операции с векторами](https://metanit.com/cpp/tutorial/7.4.php)