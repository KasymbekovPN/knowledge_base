---
tags:
  - programming-language
  - cpp
  - move-semantic
---
[[programming languages/cpp/move semantic/_|<=]]

В __C++__ понятия **rvalue** и **move-семантика** — это ключевые элементы, введённые в **C++11**, которые позволяют писать более эффективный код за счёт **избегания лишних копирований объектов**.

### rvalue

_rvalue_ — это временный объект, который:
- создаётся внутри выражения,
- живёт недолго,
- может быть использован для инициализации другого объекта.

```cpp
int a = 5; // 5 -- rvalue
std::string str = "Hello"; // "Hello" -- rvalue
std::vector<size_t> vec = getVector(); // method 'getVector' return rvalue
```

### lvalue
_lvalue_ - это именованный объект, который существует дольше, чем одно выражение.

```cpp
// x -- lvalue
int x {42};
x = x + 10; 
```

### Move семантика: зачем она нужна?

Цель _move-семантики_ — **не копировать, а "забрать" ресурсы у временного объекта**.

Функция `std::move()` ничего не перемещает. Она просто **приводит объект к типу `T&&`**, чтобы компилятор выбрал **move-конструктор или move-оператор присваивания**.

```cpp
template<typename T>
typename std::remove_reference<T>::type&&
move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}
```

### Move конструктор

```cpp
MyClass(MyClass&& other) noexcept {
    data = other.data;
    other.data = nullptr; // обнуляем исходный объект
}
```

### Move оператор присваивания

```cpp
MyClass& operator=(MyClass&& other) noexcept {
    if (this != &other) {
        delete[] data;
        data = other.data;
        other.data = nullptr;
    }
    return *this;
}
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

class CustomContainer {

private:
    int* data;
    size_t size;

public:
    CustomContainer(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "ctor" << endl;
    }

    ~CustomContainer() {
        delete[] data;
        cout << "dtor" << endl;
    }

    CustomContainer(CustomContainer&& other) noexcept:
        data(other.data),
        size(other.size) {
        other.data = nullptr;
        cout << "mv ctor" << endl;
    }

    CustomContainer& operator=(CustomContainer&& other) noexcept {
        cout << "mv operator=" << endl;
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
        }
        return *this;
    }

    CustomContainer(const CustomContainer& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "cpy ctor" << endl;
    }

    CustomContainer& operator=(const CustomContainer& other) {
        if (this != &other) {
            int* new_data = new int[other.size];
            std::copy(other.data, other.data + other.size, new_data);
            delete[] data;
            data = new_data;
            size = other.size;
            cout << "cpy operator=" << endl;
        }
        return *this;
    }
};

CustomContainer _create_tmp_container(const size_t size) {
    return CustomContainer(size);
}

int main() {
    const size_t SIZE = 1'000'000;

    cout << "### step 0" << endl;
    CustomContainer a(SIZE);

    cout << "### step 1" << endl;
    CustomContainer b = a;

    cout << "### step 2" << endl;
    CustomContainer c = std::move(a);

    cout << "### step 3" << endl;
    CustomContainer d = _create_tmp_container(SIZE);

    return 0;
}
```

```
### step 0
ctor
### step 1
cpy ctor
### step 2
mv ctor
### step 3
ctor
dtor
dtor
dtor
dtor
```

### Важно помнить

| Факт                                                                    | Объяснение                                                                   |
| ----------------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| После `std::move(obj)` объект валиден, но состояние не определено       | Нельзя использовать `obj` до следующего присваивания                         |
| Move быстрее copy только если есть ресурс (например, выделенная память) | Для POD типов (int, float, структур без указателей) move не даёт преимуществ |
| Move может примениться автоматически (RVO/NRVO)                         | Компилятор может оптимизировать возврат объекта из функции                   |

### Когда move применяется автоматически?

| Случай | Применяется ли move? |
|--------|-----------------------|
| Возврат временного объекта из функции | ✅ Да (или даже RVO/NRVO) |
| Передача временного объекта в функцию принимающую `T&&` | ✅ Да |
| Возврат `std::move(local_var)` из функции | ❌ Не рекомендуется — мешает RVO |
| Вызов `std::move(obj)` вручную | ✅ Явное применение move |
###  Лучшая практика

| Ситуация | Рекомендация |
|----------|--------------|
| Пишете свой класс с ресурсами | Реализуйте move конструктор и оператор |
| Хотите передать владение объектом | Используйте `std::move` |
| Работаете с STL контейнерами | Используйте `emplace_back`, а не `push_back(std::move(x))` |
| Пишете шаблонную функцию | Используйте `std::forward<T>()` |
