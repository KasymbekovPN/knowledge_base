---
tags:
  - programming-language
  - cpp
  - syntax
  - exception
---
[[_cpp syntax - exception|<=]]

В процессе работы программы могут возникать различные ошибки. Например, при передаче файла по сети оборвется сетевое подключение или будут введены некорректные и недопустимые данные, которые вызовут падение программы. Такие ошибки еще называются __исключениями__. 

__Исключение__ представляют временный объект любого типа, который используется для сигнализации об ошибке. Цель __объекта-исключения__ состоит в том, чтобы передать информацию из точки, в которой произошла ошибка, в код, который должен ее обработать. _Если исключение не обработано, то при его возникновении программа прекращает свою работу_. Как программа ниже, в которой возникает ошибка деления на ноль, но при этом не происходит перехват исключения.

```cpp
#include <iostream>

int divide(int, int);

int main(int argc, char const *argv[]) {
    int result {divide(100, 0)};
    std::cout << "result <= " << result << std::endl;

    return 0;
}

int divide(int a, int b) {
    return a / b;
}
```

В случае возникновения исключительной ситуации нужно уведомить систему.
```cpp
throw <object-exception>;
```
Оператор throw генерирует исключение. Через оператор throw можно передать информацию об ошибке.

Исключение так же нужно обработать.
```cpp
try {
	// code which may throw exception
} catch (<exception>) {
	// exception handling
}
```

В блоке catch идет обработка исключения. Причем многоточие в скобках после оператора catch (`catch(...)`) позволяет обработать любое исключение.

```cpp
#include <iostream>

void test(bool);
int divide(int, int);
double divide(double, double);

int main(int argc, char const *argv[]) {
    const bool arr[] {false, true};
    for (auto f: arr) {
        ::test(f);
    }

    return 0;
}

void test(bool f) {
    try {
        if (f) {
            divide(100, 0);
        } else {
            divide(100.0, 0.0);
        }
    }
    catch(const char* error_message) {
        std::cerr << error_message << std::endl;
    } catch (...) {
        std::cerr << "EXCEPTION" << std::endl;
    }
}

int divide(int a, int b) {
    if (b) {
        return a / b;
    }
    throw "Division by zero";
}

double divide(double a, double b) {
    if (b) {
        return a / b;
    }
    throw -1;
}
```

```
EXCEPTION
Division by zero
```
Если же исключение не обработано, то вызывается функция std::terminate() (из модуля `<exception>` стандартной библиотеки C++), которая, в свою очередь, по умолчанию вызывает другую функцию - std::abort() (из `<cstdlib>`), которая собственно и завершает программу.

При обработке исключения стоит помнить, что при передаче объекта оператору `throw` блок _catch_ получает копию этого объекта. И эта копия существует только в пределах блока _catch_.

Для значений примитивных типов, например, `int`, копирование значения может не влиять на производительность программы. Однако при передаче объектов классов издержки могут выше. Поэтому в этом случае объекты обычно передаются по ссылке.

---
[Обработка исключений](https://metanit.com/cpp/tutorial/6.1.php)