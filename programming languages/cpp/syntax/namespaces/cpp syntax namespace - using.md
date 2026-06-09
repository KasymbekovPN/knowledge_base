---
tags:
  - programming-language
  - cpp
  - syntax
  - namespace
  - using
---
[[__cpp syntax namespaces__|<=]]

[using. Подключение пространств имен и определение псевдонимов|metanit.com](https://metanit.com/cpp/tutorial/2.11.php)

Использование оператора __using__ имеет следующий формат:
```cpp
using space_name::object;
```

Без __using__:
```cpp
#include <iostream>

int main() {
	int age;
	
	std::cout << "Input age <= ";
	std::cin >> age;

	std::cout << "Age: " << age << std::endl;

	return 0;
}
```

Здесь используются сразу три объекта из пространства имен _std_:  _cout_, _cin_ и _endl_. 
С использованием using:
```cpp
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

int main(int argc, char const *argv[]) {
    int age;

    cout << "Input age <= ";
    cin >> age;

    cout << "Age: " << age << std::endl;

    return 0;
}
```
