---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/rule 350/_|<=]]

Если класс **управляет ресурсом** (например, выделяет память через `new`), то нужно **явно определить три метода**:
1. **Деструктор**
2. **Конструктор копирования**
3. **Оператор присваивания**

По умолчанию C++ генерирует "поверхностное" копирование (`shallow copy`).  
Если не переопределить эти методы — два объекта могут указывать на одну память → `double free`.

### Нарушение rule-3
```cpp
#include <iostream>
#include <cstring>

class BadImpl {
private:
    char* data;

public:
    BadImpl(const char* _str) {
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

    // no copy constructor
    // no destructor
};

void test();

int main() {
    test();

    return 0;
}

void test() {
    BadImpl a{"Hello"};
    auto b = a;
} // will called dezstructor on both instances -> UB
```

### Правильное применение
```cpp
#include <iostream>
#include <cstring>

class GoodImpl {

private:
    char* data;

    void copy_from(const char* _str) {
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

public:
    GoodImpl(const char* _str) {
        std::cout << "ctor" << std::endl;
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

    GoodImpl(const GoodImpl& other) {
        std::cout << "copy ctor" << std::endl;
        copy_from(other.data);
    }

    GoodImpl& operator=(const GoodImpl& other) {
        std::cout << "assign op" << std::endl;
        if (this != &other) {
            delete[] data;
            copy_from(other.data);
        }
        return *this;
    }

    ~GoodImpl() {
        std::cout << "dtor" << std::endl;
        delete[] data;
    }
};

void test();

int main() {
    test();

    return 0;
}

void test() {
    GoodImpl a{"Hello"};
    auto b = a;
}
```

```
ctor
copy ctor
dtor
dtor
```
