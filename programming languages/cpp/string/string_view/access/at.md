---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/access/_|<=]]

Метод `at()` позволяет осуществить доступ с проверкой границ (бросает исключение при выходе за пределы)

```cpp
#include <iostream>
#include <string>
#include <string_view>

void _test_at(const std::string_view&, size_t);

int main() {
    const std::string_view sv {"ABC"};
    for (size_t i {}; i <= sv.size(); i++) {
        _test_at(sv, i);
    }

    return 0;
}

void _test_at(const std::string_view& sv, size_t i) {
    try {
        std::cout << sv.at(i) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
```

```
A
B
C
invalid string_view position
```

---
---

## 🧩 Методы `std::string_view`

| Метод              | Описание                                                             |
| ------------------ | -------------------------------------------------------------------- |
| `substr(pos, len)` | Возвращает подстроку                                                 |
| `find(str, pos)`   | Ищет подстроку                                                       |
| `rfind()`          | Поиск справа                                                         |
| `compare()`        | Сравнение строк                                                      |

---
---

## 🧪 Пример: использование `string_view` в функциях

```cpp
#include <iostream>
#include <string_view>

void print_prefix(std::string_view sv, size_t n) {
    std::cout << sv.substr(0, n) << std::endl;
}

int main() {
    print_prefix("Hello World", 5); // выводит "Hello"
    std::string s = "Modern C++ is great!";
    print_prefix(s, 7);             // выводит "Modern "
    return 0;
}
```

---

## ⚠️ Ограничения

---

## 🛠 Как безопасно получить `const char*`

Если вам нужен `const char*` и вы уверены, что данные null-terminated:

```cpp
std::string str = "Hello";
std::string_view sv = str;

// Убедитесь, что null-terminated
if (sv.size() + 1 <= str.size()) {
    const char* cstr = sv.data();
    std::cout << cstr << std::endl;
}
```

Или используйте `std::string`, если нужно гарантированное завершение нулём.

---

## 📌 

---

## 📝 Вывод

> `std::string_view` — это мощный инструмент для работы с строками без лишних копирований.  
> Он идеально подходит для **чтения строк**, **передачи в функции**, **парсинга текста** и других задач, где **не требуется владеть данными**.

---

Если хочешь, могу показать:
- Как использовать `string_view` с UTF-8, JSON парсерами,
- Как реализовать свой "safe substring parser",
- Или как совмещать `string_view` с WinAPI / Qt.

Пишите, интересует ли вас конкретный пример!