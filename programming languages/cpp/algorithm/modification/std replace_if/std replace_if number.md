---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std replace_if/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _print_vector(const vector<int>&);

int main() {
    vector<int> v {1, 2, 3, 4, 5, 6, 7};

    replace_if(v.begin(), v.end(), [](int x) {
       return x % 2 == 0;
    }, -1);
    _print_vector(v);

    return 0;
}

void _print_vector(const vector<int>& vector) {
    cout << "{ ";
    for (auto &i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
```

```
{ 1 -1 3 -1 5 -1 7 }
```

---

## ✅ Пример 3: работа с пользовательским типом

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people = {
        {"Alice", 17},
        {"Bob", 25},
        {"Charlie", 15},
        {"Diana", 18}
    };

    // Заменяем подростков на возраст 18
    std::replace_if(people.begin(), people.end(), [](const Person& p) {
        return p.age < 18;
    }, {"Adult", 18});

    std::cout << "Updated ages:\n";
    for (const auto& p : people)
        std::cout << p.name << " (" << p.age << ")\n";
}
```

### Вывод:
```
Adult (18)
Bob (25)
Adult (18)
Diana (18)
```

---


---

## 🛠 Как работает внутри?

Алгоритм проходит по каждому элементу и проверяет его через предикат:

```cpp
for (; first != last; ++first) {
    if (pred(*first)) {
        *first = new_value;
    }
}
```

Таким образом, он позволяет менять значения **в зависимости от сложного условия**, а не просто точного совпадения.

---

## 📌 Когда использовать `std::replace_if`?

| Ситуация | Почему |
|----------|--------|
| Нужно заменить элементы по сложному условию | Например, числа > N, строки короче M и т.д. |
| Хотите работать с STL вместо циклов | Более читаемый код |
| Работаете с любыми итерируемыми структурами | `std::vector`, `std::list`, массивами и строками |
| Нужно модифицировать данные "на месте" | Без создания копии |
| Условие зависит от нескольких полей или свойств | Например, `x > 10 && y < 5` для структур |

---

## 🧩 Альтернативы

| Функция | Что делает |
|--------|------------|
| `std::replace(b, e, old, new)` | Меняет конкретное значение |
| `std::transform` | Применяет функцию ко всем элементам |
| `std::replace_copy_if` | То же, но копирует результат в другой контейнер |
| `std::for_each` | Выполняет действие над каждым элементом |
| `std::remove_if` / `std::copy_if` | Удаляет или копирует подходящие элементы |

---

## 📝 Сводка

| Метод | Что делает |
|-------|------------|
| `std::replace(b, e, old, new)` | Меняет все `old` на `new` |
| `std::replace_if(b, e, pred, new)` | Меняет элементы, удовлетворяющие `pred` |
| `std::replace_copy_if(...)` | То же, но копирует в новый контейнер |
| `std::replace_if(...)` | Изменяет исходный контейнер "на месте" |

---


---

## 🧰 Бонус: пример с `std::replace_copy_if`

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

int main() {
    std::vector<int> src = {1, 2, 3, 4, 5, 6};
    std::vector<int> dest;

    // Копируем, заменяя нечётные числа на -1
    std::replace_copy_if(src.begin(), src.end(), std::back_inserter(dest),
                         [](int x) { return x % 2 != 0; }, -1);

    std::cout << "Original: ";
    for (int x : src) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "Modified: ";
    for (int x : dest) std::cout << x << " ";
    std::cout << "\n";
}
```

### Вывод:
```
Original: 1 2 3 4 5 6 
Modified: -1 2 -1 4 -1 6 
```

---

Если хочешь — могу показать:
- Как реализовать свою версию `std::replace_if`
- Как использовать `std::replace_if` с `std::string_view`
- Как комбинировать `std::replace_if` с `std::transform`, `std::count_if` и другими алгоритмами

📌 Просто напиши, что тебе интересует!