---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - list
---
[[_cpp containers list|<=]]

Метод _sort()_ сортирует элементы списка. Поддерживает компараторы.

##### Без компаратора
```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort();
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
11 2 3 19 3 
2 3 3 11 19
```

##### Сортировка `std::list` со стандартным компаратором

Метод `sort` в `std::list` может принимать компаратор в качестве аргумента. Компаратор — это функция или объект, который определяет порядок сортировки.

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort(); // default, sort ascending
    print_list(numbers);

    numbers.sort(std::greater<int>()); // sort descending
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
11 2 3 19 3 
2 3 3 11 19
19 11 3 3 2
```

##### Сортировка с пользовательским компаратором

Компаратор может быть функцией, лямбда-функцией или функциональным объектом.

###### Использование функции

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

bool compare_ascending(int, int);
bool compare_descending(int, int);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort(compare_descending);
    print_list(numbers);

    numbers.sort(compare_ascending);
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

bool compare_ascending(int a, int b) {
    return a < b;
}

bool compare_descending(int a, int b) {
    return a > b;
}
```

```
11 2 3 19 3 
19 11 3 3 2
2 3 3 11 19
```

###### Использование лямбда-функции

```cpp
#include <iostream>
#include <list>

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort([](int a, int b){return a > b;});
    print_list(numbers);
  
    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
11 2 3 19 3 
19 11 3 3 2
```

###### Использование функционального объекта

```cpp
#include <iostream>
#include <list>

struct CompareDescending {
    bool operator()(int a, int b) const {
        return a > b;
    }
};

template<typename T>
void print_list(const std::list<T>&);

int main() {
    std::list<int> numbers {11, 2, 3, 19, 3};
    print_list(numbers);

    numbers.sort(CompareDescending());
    print_list(numbers);

    return 0;
}

template<typename T>
void print_list(const std::list<T>& list) {
    for (auto &item: list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
11 2 3 19 3 
19 11 3 3 2
```

##### Сортировка списка пользовательских объектов

Если список содержит пользовательские объекты, можно определить компаратор для сортировки по определенному полю.

```cpp
#include <iostream>
#include <list>
#include <string>

struct Person {
    std::string name;
    unsigned age;

    Person(std::string name, unsigned age):
        name{name},
        age{age} {}

    std::string to_string() const {
        return "{name: " + name + ", age: " + std::to_string(age) + "}";
    }
};

void print_list(const std::list<Person>&);

int main() {
    std::list<Person> people = {
        {"Alice", 25},
        {"Bob", 30},
        {"John", 20}
    };
    print_list(people);

    people.sort([](const Person& a, const Person& b) {
        return a.age < b.age;
    });
    print_list(people);

    people.sort([](const Person& a, const Person& b) {
        return a.name > b.name;
    });
    print_list(people);  

    return 0;
}

void print_list(const std::list<Person>& list) {
    for (auto &item: list) {
        std::cout << item.to_string() << " ";
    }
    std::cout << std::endl;
}
```
