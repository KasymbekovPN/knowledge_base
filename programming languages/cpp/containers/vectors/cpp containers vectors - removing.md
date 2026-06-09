---
tags:
  - programming-language
  - cpp
  - containers
  - vector
---
[[_cpp containers vectors|<=]]

#### Метод pop_back

Удаляет последний элемент вектора. Этот метод не принимает аргументов и не возвращает удаленный элемент.

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3};
    print_vector(numbers);

    numbers.pop_back();
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 
1 2
```

#### Метод erase

Удаляет один или несколько элементов из вектора. Принимает итератор или диапазон итераторов.

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5, 6, 7};
    print_vector(numbers);

    numbers.erase(numbers.begin() + 1);
    print_vector(numbers);

    numbers.erase(numbers.begin() + 2, numbers.begin() + 4);
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 4 5 6 7 
1 3 4 5 6 7
1 3 6 7
```

#### Метод clear

Удаляет все элементы из вектора, оставляя его пустым.

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers {1, 2, 3};
    std::cout << "numbers size: " << numbers.size() << std::endl;

    numbers.clear();
    std::cout << "numbers size: " << numbers.size() << std::endl;

    return 0;
}
```

```
numbers size: 3
numbers size: 0
```

#### Удаление элементов по условию

Для удаления элементов, удовлетворяющих определенному условию, можно использовать идиому __erase-remove__.

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void print_vector(const std::vector<int>&);

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {1, 2, 3, 4, 5, 3, 7};
    print_vector(numbers);

    std::vector<int>::iterator it
		= std::remove(numbers.begin(), numbers.end(), 3);
    numbers.erase(it, numbers.end());
    print_vector(numbers);
  
    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 4 5 3 7 
1 2 4 5 7
```

####  Метод std::remove_if

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5, 3, 7};
    print_vector(numbers);

    std::vector<int>::iterator it = std::remove_if(
        numbers.begin(),
        numbers.end(),
        [](int x) {return x % 2 != 0;}
    );
    numbers.erase(it, numbers.end());
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 4 5 3 7 
2 4
```

#### Удаление через resize

```cpp
#include <iostream>
#include <vector>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5, 3, 7};
    print_vector(numbers);

    numbers.resize(5);
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
1 2 3 4 5 3 7 
1 2 3 4 5
```

#### Удаление элементов с использованием shrink_to_fit

Метод _shrink_to_fit_ в C++ используется для уменьшения емкости (`capacity`) вектора до его текущего размера (`size`). Это может быть полезно, если вы хотите освободить лишнюю память, занимаемую вектором, после удаления элементов.

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void print_vector(const std::vector<int>&);

int main() {
    std::vector<int> numbers {1, 2, 3, 4, 5, 3, 7};
    print_vector(numbers);

    std::vector<int>::iterator it = std::remove_if(
        numbers.begin(),
        numbers.end(),
        [](int x) {return x % 2 != 0;}
    );
    numbers.erase(it, numbers.end());
    print_vector(numbers);
  
    numbers.shrink_to_fit();
    print_vector(numbers);

    return 0;
}

void print_vector(const std::vector<int>& numbers) {
    std::cout << "size: " << numbers.size()
        << ", capacity: " << numbers.capacity()
        << " :: ";
    for (const auto& number: numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;
}
```

```
size: 7, capacity: 7 :: 1 2 3 4 5 3 7 
size: 2, capacity: 7 :: 2 4
size: 2, capacity: 2 :: 2 4
```

---
[Операции с векторами](https://metanit.com/cpp/tutorial/7.4.php)