[[raw data/cpp/interview/_|<=]]

## Binary Search

**Условие:** дан отсортированный по возрастанию массив `nums` и число `target`. Найти индекс `target` в массиве за O(log n). Если элемента нет — вернуть -1.

### Идея

Классический бинарный поиск: поддерживаем диапазон `[left, right]`, в котором потенциально может находиться искомый элемент. На каждом шаге смотрим на средний элемент `mid`. Если он равен `target` — нашли. Если `nums[mid] < target` — искомый элемент (если есть) находится строго правее, сдвигаем `left`. Если `nums[mid] > target` — сдвигаем `right` влево. Каждая итерация отбрасывает **половину** оставшегося диапазона — отсюда логарифмическая сложность.

### Решение

```cpp
#include "binary_search.h"  
  
#include <algorithm>  
#include <vector>  
#include <iostream>  
#include <format>  
  
namespace binary_search {  
  
static constexpr int BAD_RESULT{-1};  
  
int binary_search(const std::vector<int>& sorted_container, const int target) {  
    int left{0};  
    int right{static_cast<int>(sorted_container.size() - 1)};  
  
    while (left <= right) {  
        // защита от переполнения  
        const int mid{left  + (right - left) / 2 };  
  
        if (sorted_container[mid] == target) {  
            return mid;  
        }  
        if (sorted_container[mid] < target) {  
            left = mid + 1;  
        } else {  
            right = mid - 1;  
        }    
    }  
    return BAD_RESULT;  
}  
  
int binary_search_recursive(const std::vector<int>& sorted_container,  
                            const int target,  
                            const int left,  
                            const int right) {  
    if (left > right) return BAD_RESULT;  
  
    const int mid{left  + (right - left) / 2 };  
  
    if (sorted_container[mid] == target) return mid;  
  
    return  
        sorted_container[mid] < target  
        ? binary_search_recursive(sorted_container, target, mid + 1, right)  
        : binary_search_recursive(sorted_container, target, left, mid - 1);  
}  
  
int binary_search_stl(const std::vector<int>& sorted_container, const int target) {  
    if (const auto it = std::ranges::lower_bound(sorted_container, target);  
        it != sorted_container.end() && *it == target) {  
        return static_cast<int>(it - sorted_container.begin());  
    }  
    return BAD_RESULT;  
}  
  
void demo() {  
    constexpr int BAD_TARGET{2};  
    constexpr int GOOD_TARGET{9};  
    const std::vector<int> SORTED_COINTAINER{-1, 0, 3, 5, 9, 12};  
  
    std::cout << std::format("target: {} => {}\n", BAD_TARGET, binary_search(SORTED_COINTAINER, BAD_TARGET));  
    std::cout << std::format("target: {} => {}\n", GOOD_TARGET, binary_search(SORTED_COINTAINER, GOOD_TARGET));  
  
    std::cout << std::format("target rec: {} => {}\n", BAD_TARGET, binary_search_recursive(  
        SORTED_COINTAINER,  
        BAD_TARGET,  
        0,  
        static_cast<int>(SORTED_COINTAINER.size()) - 1));  
    std::cout << std::format("target rec: {} => {}\n", GOOD_TARGET, binary_search_recursive(  
        SORTED_COINTAINER,  
        GOOD_TARGET,  
        0,  
        static_cast<int>(SORTED_COINTAINER.size()) - 1));  
  
    std::cout << std::format("target stl: {} => {}\n", BAD_TARGET, binary_search_stl(  
        SORTED_COINTAINER,  
        BAD_TARGET));  
    std::cout << std::format("target stl: {} => {}\n", GOOD_TARGET, binary_search_stl(  
        SORTED_COINTAINER,  
        GOOD_TARGET));  
}  
  
}
```

### Разбор

- `mid = left + (right - left) / 2` — вычисляется именно так, а не `(left + right) / 2`, чтобы избежать **переполнения** (UB для `int`), если `left` и `right` оба близки к `INT_MAX`. Формула `left + (right - left) / 2` математически эквивалентна, но не создаёт промежуточной суммы, которая могла бы превысить диапазон `int`. Это классический вопрос на собеседовании — "а что не так с `(left+right)/2`?".
- Условие цикла `left <= right` (не `<`) — важно: диапазон `[left, right]` включает оба конца, и когда `left == right`, там ещё остаётся ровно один непроверенный элемент, который нужно проверить.
- `left = mid + 1` / `right = mid - 1` — **строго** исключают уже проверенный `mid` из дальнейшего поиска; без `+1`/`-1` возможен бесконечный цикл (например, если `left == right == mid` и условие не выполнилось бы).
- Возврат `-1` после выхода из цикла — когда `left > right`, диапазон поиска стал пустым, значит элемента в массиве нет.

### Пример

```
nums = [-1, 0, 3, 5, 9, 12], target = 9

left=0, right=5
  mid=0+(5-0)/2=2, nums[2]=3 < 9 -> left=3
left=3, right=5
  mid=3+(5-3)/2=4, nums[4]=9 == 9 -> return 4

Результат: 4
```

```
nums = [-1, 0, 3, 5, 9, 12], target = 2

left=0, right=5
  mid=2, nums[2]=3 > 2 -> right=1
left=0, right=1
  mid=0+(1-0)/2=0, nums[0]=-1 < 2 -> left=1
left=1, right=1
  mid=1, nums[1]=0 < 2 -> left=2
left=2, right=1 -> left > right, цикл завершён

Результат: -1
```

### Сложность

- Время: **O(log n)** — диапазон поиска уменьшается вдвое на каждой итерации.
- Память: **O(1)** — итеративная версия использует только несколько переменных.

### Рекурсивная версия (для сравнения)

Время: O(log n), память: **O(log n)** на стек рекурсии (в отличие от итеративной версии с O(1)) — частый доп. вопрос "почему рекурсивная версия хуже по памяти при той же временной сложности".

### `std::lower_bound` / `std::upper_bound` — стандартная библиотека уже это умеет

На практике в C++ часто не пишут бинарный поиск руками, а используют готовые функции:

`std::lower_bound` находит первую позицию, где элемент `>= target` — если там реально стоит `target`, значит нашли; иначе элемента нет. `std::upper_bound` — аналогично, но находит первую позицию **строго больше** `target`, полезно для поиска границ диапазона одинаковых значений.

### Частые вариации (это один из самых "разветвлённых" топиков на собеседованиях)

- **Search in Rotated Sorted Array** — массив отсортирован, но "сдвинут" (повёрнут) — на каждом шаге нужно определить, какая половина (левая или правая от `mid`) остаётся отсортированной, и уже внутри неё решать, сужать ли диапазон в эту сторону.
- **Find First and Last Position of Element in Sorted Array** — два отдельных бинарных поиска: один ищет левую границу (первое вхождение), другой — правую (последнее вхождение) — модификация условий `<=`/`<` внутри стандартного binary search.
- **Find Minimum in Rotated Sorted Array** — поиск точки поворота — сравнение `nums[mid]` с `nums[right]` определяет, в какой половине находится минимум.
- **Search a 2D Matrix** — если матрица отсортирована и по строкам, и по столбцам определённым образом, можно свести к одному бинарному поиску через пересчёт индекса `mid` в пару `(row, col)`.
- **Koko Eating Bananas / Capacity To Ship Packages** — "бинарный поиск по ответу" — не поиск элемента в массиве, а поиск минимального/максимального значения параметра, удовлетворяющего некоторому монотонному условию (частый паттерн продвинутых задач).
