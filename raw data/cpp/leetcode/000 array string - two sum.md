[[raw data/cpp/interview/_|<=]]


## Two Sum

**Условие:** дан массив целых чисел `nums` и число `target`. Найти индексы двух элементов, сумма которых равна `target`. Предполагается, что решение единственно, и один элемент нельзя использовать дважды.

### Идея

Прямой перебор пар — O(n²). Оптимально: пока идём по массиву, для каждого `nums[i]` проверяем, встречалось ли раньше число `target - nums[i]`. Для этого храним уже просмотренные значения в хеш-таблице `значение → индекс`. Тогда каждый элемент обрабатывается за O(1) в среднем — весь проход за O(n).

### Решение

```cpp
#include "two_sum.h"  
  
#include <unordered_map>  
#include <iostream>  
#include <format>  
  
namespace leetcode {  
  
std::vector<int> two_sum(const std::vector<int>& numbers, const int target) {  
    std::unordered_map<int, int> seen; // value -> index  
    seen.reserve(numbers.size());  
  
    for (int i{}; i < static_cast<int>(numbers.size()); ++i) {  
        int complement{target - numbers[i]};  
        if (auto it = seen.find(complement); it != seen.end()) {  
            return {it->second, i};  
        }  
        seen[numbers[i]] = i;  
    }  
    return {};  
}  
  
void demo() {  
    const std::vector<int> NUMBERS = {1, 2, 3, 4, 5, 6, 7, 8, 9};  
    std::cout << "two_sum:";  
    for (const std::vector<int> result = two_sum(NUMBERS, 10); int item: result) {  
        std::cout << std::format(" {}", item);  
    }
    std::cout << std::endl;  
}  
  
}
```

### Разбор

- `seen` хранит пары "число → его индекс" для уже пройденных элементов.
- На шаге `i` вычисляем `complement = target - nums[i]` — то, чего не хватает до `target`.
- Если `complement` уже встречался (есть в `seen`), значит пара найдена: возвращаем `{индекс complement, i}`.
- Если нет — записываем текущий `nums[i]` в `seen` и идём дальше.
- Важно: сначала проверяем наличие complement, потом добавляем текущий элемент — это гарантирует, что один и тот же индекс не используется дважды (например, если `target = nums[i] * 2`).

### Сложность

- Время: **O(n)** — один проход, поиск/вставка в hash map амортизированно O(1).
- Память: **O(n)** — под хеш-таблицу.

### Альтернатива

Если массив можно сортировать и не нужны исходные индексы — two pointers после сортировки: O(n log n) по времени, O(1) доп. памяти. Но с исходными индексами это не работает напрямую (нужно хранить пары `(значение, индекс)` перед сортировкой).
