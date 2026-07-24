[[raw data/cpp/interview/_|<=]]

## Kth Largest Element in an Array

**Условие:** дан массив целых чисел `nums` и число `k`. Найти `k`-й по величине элемент в отсортированном порядке (не обязательно уникальный — считается порядковая позиция после сортировки, а не k-е уникальное значение).

```cpp
#include "kth_largest_element.h"  
  
#include <vector>  
#include <queue>  
#include <algorithm>  
#include <cstdlib>  
#include <iostream>  
#include <format>  
  
namespace kth_largest_element {  
// solution 1  
int find_kth_largest(const std::vector<int>& nums, const int k) {  
    std::vector<int> copy{nums.begin(), nums.end()};  
    std::ranges::sort(copy, std::greater<>());  
  
    return copy[k - 1];  
}  
  
// solution 2  
int find_kth_largest_heap(const std::vector<int>& nums, const int k) {  
    std::priority_queue<int, std::vector<int>, std::greater<>> min_heap;  
  
    for (const auto& num : nums) {  
        min_heap.push(num);  
        if (static_cast<int>(min_heap.size()) > k) {  
            // выбрасываем наименьший, если размер превысил k  
            min_heap.pop();  
        }    
    }  
    return min_heap.top();  
}  
  
// solution 3  
  
static int partition(std::vector<int>& nums,  
                     const int left,  
                     const int right,  
                     const int pivot_index) {  
    const int pivot_value{nums[pivot_index]};  
    // прячем pivot в конец  
    std::swap(nums[pivot_index], nums[right]);  
    int store_index{left};  
  
    for (int i{left}; i < right; ++i) {  
        if (nums[i] < pivot_value) {  
            std::swap(nums[i], nums[store_index]);  
            ++store_index;  
        }    
    }  
    // возвращаем pivot на финальную позицию  
    std::swap(nums[store_index], nums[right]);  
    return store_index;  
}  
  
static int quick_select(std::vector<int>& nums, const int left, const int right, const int target_index) {  
    if (left == right) return nums[left];  
  
    // случайный выбор pivot  
    int pivot_index{left + std::rand() % (right - left + 1)};  
    pivot_index = partition(nums, left, right, pivot_index);  
  
    if (target_index == pivot_index) {  
        return nums[target_index];  
    }    
    if (target_index < pivot_index) {  
        return quick_select(nums, left, pivot_index - 1, target_index);  
    }    
    return quick_select(nums, pivot_index + 1, right, target_index);  
}  
  
int find_kth_largest_quick_select(const std::vector<int>& nums, const int k) {  
    const int N{static_cast<int>(nums.size())};  
    std::vector<int> copy{nums.begin(), nums.end()};  
  
    return quick_select(copy, 0, N - 1, N - k);  
}  
  
void demo() {  
    constexpr int K{2};  
    const std::vector<int> NUMS{3,2,1,5,6,4};  
  
    std::cout << std::format("solution 1: {}\n", find_kth_largest(NUMS, K));  
    std::cout << std::format("solution 2: {}\n", find_kth_largest_heap(NUMS, K));  
    std::cout << std::format("solution 3: {}\n", find_kth_largest_quick_select(NUMS, K));  
}  
}
```

### Решение 1: сортировка (простое, но не оптимальное)

Время: **O(n log n)**, память: O(log n)–O(n) под сортировку. Просто, но не использует специфику задачи (не нужен **полный** порядок, только k-й элемент).

### Решение 2: min-heap размера k (эффективнее по времени при малом k)

**Идея:** поддерживаем min-heap (куча с минимумом на вершине) размером ровно `k`. Проходим по массиву; если куча ещё не заполнена — просто добавляем. Если заполнена — сравниваем новый элемент с минимумом кучи: если новый больше — минимум точно не входит в top-k, выбрасываем его и добавляем новый элемент. В конце вершина кучи — это и есть k-й по величине элемент (наименьший среди k наибольших).

**Разбор:**

- `std::priority_queue<int, std::vector<int>, std::greater<int>>` — куча с минимумом на вершине (по умолчанию `priority_queue` — max-heap, `std::greater` инвертирует сравнение).
- Куча хранит ровно `k` наибольших элементов, увиденных на данный момент. Как только размер превышает `k`, наименьший из них (вершина min-heap) удаляется — он гарантированно не входит в top-k глобально, если сейчас в куче уже есть `k` элементов больше него.
- В конце в куче остаются ровно `k` наибольших элементов всего массива, а вершина (минимум среди них) — это k-й по величине элемент.

**Пример:**

```
nums = [3,2,1,5,6,4], k = 2

push(3): heap=[3]
push(2): heap=[2,3]
push(1): heap=[1,2,3] -> size>2 -> pop(1) -> heap=[2,3]
push(5): heap=[2,3,5] -> pop(2) -> heap=[3,5]
push(6): heap=[3,5,6] -> pop(3) -> heap=[5,6]
push(4): heap=[4,5,6] -> pop(4) -> heap=[5,6]

top() = 5  (2-й по величине: 6,5,4,3,2,1 -> 2-й это 5)
```

Время: **O(n log k)** — каждая вставка/удаление в куче размера `k` стоит O(log k), выполняется для всех `n` элементов. Выгоднее сортировки при `k << n`. Память: **O(k)**.

### Решение 3: Quickselect (в среднем самое быстрое, O(n))

**Идея:** модификация Quicksort — вместо того чтобы рекурсивно сортировать **обе** части после разбиения (partition), рекурсивно спускаемся только в ту часть, где заведомо находится искомый элемент. Это отбрасывает половину работы на каждом шаге в среднем случае.

**Разбор:**

- `partition` — стандартная схема Ломуто (Lomuto partition): выбранный pivot временно перемещается в конец, затем все элементы меньше pivot собираются в начало диапазона, после чего pivot ставится на границу между "меньше" и "больше или равно" — эта граница и есть его финальная отсортированная позиция.
- `targetIndex = n - k` — k-й по величине элемент, если бы массив был отсортирован по возрастанию, стоит на индексе `n - k` (например, при `k=1` — это последний элемент, индекс `n-1`, самый большой).
- После partition сравниваем `targetIndex` с финальной позицией pivot: если совпали — нашли ответ; если `targetIndex` левее — рекурсивно ищем только в левой части; если правее — только в правой. **Другая половина полностью отбрасывается**, в отличие от полной сортировки.
- Случайный выбор pivot (`std::rand() % (...)`) защищает от вырожденного O(n²) на уже отсортированных или специально составленных входных данных — без рандомизации выбор всегда первого/последнего элемента как pivot уязвим к состязательным тестам.

**Сложность:**

- Время: **в среднем O(n)** (геометрически убывающая сумма `n + n/2 + n/4 + ... ≈ 2n`), в худшем случае **O(n²)** (при систематически неудачном выборе pivot, что рандомизация делает крайне маловероятным).
- Память: **O(log n)** в среднем на стек рекурсии, O(1) дополнительно если реализовать итеративно.

### Сравнение подходов

- **Сортировка** — проще всего написать, O(n log n), достаточно если `k` и `n` не очень большие или простота важнее производительности.
- **Min-heap размера k** — хорош, когда `k` значительно меньше `n`, или когда данные поступают потоком (streaming) и нужно поддерживать top-k "на лету".
- **Quickselect** — оптимален по среднему времени O(n), но сложнее реализовать корректно (рандомизация pivot, аккуратный partition) и не подходит для streaming-сценария (нужен весь массив сразу).

### Частые вариации

- **Kth Smallest Element** — та же логика quickselect, но `targetIndex = k - 1` напрямую (без пересчёта через `n - k`), либо max-heap размера k вместо min-heap.
- **Top K Frequent Elements** — аналогичная задача, но ключ для кучи/quickselect — частота встречаемости элемента, а не само значение (нужна предварительная агрегация через hash map).
- **Median of Data Stream** — потоковая версия, где нужен k-й элемент (медиана) при постоянно поступающих данных — решается через две кучи (min-heap и max-heap), а не quickselect (который требует статичный массив).
- **Wiggle Sort II** — использует поиск медианы (частный случай Kth Element) как подготовительный шаг перед перестановкой массива.
