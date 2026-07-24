[[raw data/cpp/interview/_|<=]]

## Top K Frequent Elements

**Условие:** дан массив целых чисел `nums` и число `k`. Вернуть `k` элементов с наибольшей частотой встречаемости в массиве (порядок в ответе не важен).

### Идея

Первый шаг всегда одинаковый — посчитать частоту каждого элемента через hash map (`unordered_map<int, int>`), O(n). Дальше есть несколько путей выбрать top-k по частоте:

1. Полная сортировка по частоте — O(n log n).
2. **Min-heap размера k** по частоте — O(n log k), тот же паттерн, что и в Kth Largest Element.
3. **Bucket sort по частоте** — O(n), используя тот факт, что частота элемента не может превышать длину массива `n`.

Разберём 2 и 3, как наиболее часто ожидаемые на собеседовании.

```cpp
#include "top_k_freq_elements.h"  
  
#include <vector>  
#include <unordered_map>  
#include <queue>  
#include <iostream>  
#include <format>  
  
namespace top_k_freq_elements {  
  
// min-heap solution  
std::vector<int> top_k_freq(const std::vector<int>& nums, const int k) {  
    std::unordered_map<int, int> freq;  
    for (const auto& num: nums) ++freq[num];  
  
    // min-heap по частоте: {frequency, value}  
    std::priority_queue<  
        std::pair<int, int>,  
        std::vector<std::pair<int, int>>,  
        std::greater<std::pair<int, int>>  
    > min_heap;  
  
    for (const auto& [value, count]: freq) {  
        min_heap.push({count, value});  
        // выбрасываем наименее частый, если размер превысил k  
        if (static_cast<int>(min_heap.size()) > k) min_heap.pop();  
    }  
    std::vector<int> result;  
    result.reserve(k);  
    while (!min_heap.empty()) {  
        result.push_back(min_heap.top().second);  
        min_heap.pop();  
    }  
    return result;  
}  
  
// bucket sort solution  
std::vector<int> top_k_freq_bucket(const std::vector<int>& nums, const int k) {  
    std::unordered_map<int, int> freq;  
    for (const auto& num: nums) ++freq[num];  
  
    const int N{static_cast<int>(nums.size())};  
    // buckets[f] = значения с частотой f  
    std::vector<std::vector<int>> buckets(N + 1);  
  
    for (const auto& [value, count]: freq) {  
        buckets[count].push_back(value);  
    }  
    std::vector<int> result;  
    for (int f{N}; f >= 1 && static_cast<int>(result.size()) < k; --f) {  
        for (const auto& value: buckets[f]) {  
            result.push_back(value);  
            if (static_cast<int>(result.size()) == k) break;  
        }    
    }  
    return result;  
}  
  
static void print(std::vector<int>&& nums, std::string&& label) {  
    std::cout << label << ": ";  
    for (const auto& num: nums) std::cout << num << " ";  
    std::cout << std::endl;  
}  
  
void demo() {  
    constexpr int K{2};  
    const std::vector<int> NUMS{1,1,1,2,2,3};  
  
    print(top_k_freq(NUMS, K), "min-heap solution");  
    print(top_k_freq_bucket(NUMS, K), "bucket sort solution");  
}  
  
}
```

### Решение 1: min-heap размера k

### Разбор

- `freq` — подсчёт количества вхождений каждого уникального значения.
- Куча хранит пары `{частота, значение}`; сравнение пар по умолчанию идёт по первому элементу (`std::pair` сравнивается лексикографически), то есть именно по частоте — что и нужно.
- `std::greater<>` делает кучу min-heap (по умолчанию `priority_queue` — max-heap) — вершина всегда наименее частый элемент из текущих top-k кандидатов.
- Как и в Kth Largest Element: держим в куче не более `k` элементов; при превышении размера выбрасываем наименее частый — он гарантированно не входит в глобальный top-k, раз уже есть `k` элементов не менее частых.

**Сложность:** время **O(n log k)** — подсчёт частот O(n), затем `|freq|` вставок/удалений в кучу размера `k`, каждая O(log k). Память: **O(n + k)** — хеш-таблица частот плюс куча.

### Решение 2: Bucket Sort по частоте (O(n))

**Идея:** частота любого элемента лежит в диапазоне `[0, n]`. Создаём массив "корзин" `buckets[freq]` — список всех значений с частотой ровно `freq`. Заполняем корзины за один проход по `freq`-таблице, затем идём по корзинам **от наибольшей частоты к наименьшей**, собирая значения, пока не наберём `k` элементов.

### Разбор

- `buckets` — индексируется по значению частоты (от 0 до `n` включительно), в каждой корзине — список элементов с именно такой частотой. Максимальная возможная частота ограничена `n` (весь массив состоит из одного значения), поэтому размер `buckets` заведомо достаточен.
- Заполнение корзин — один проход по `freq` (не по `nums`), то есть по уникальным значениям.
- Идём по корзинам от `f = n` вниз до `1`, собирая значения — так они естественным образом обрабатываются в порядке убывания частоты, без явной сортировки.
- Останавливаемся, как только набрали `k` элементов — не нужно проходить оставшиеся корзины с меньшей частотой.

### Пример

```
nums = [1,1,1,2,2,3], k = 2

freq: {1:3, 2:2, 3:1}
n = 6, buckets размера 7 (индексы 0..6)

buckets[3] = [1]
buckets[2] = [2]
buckets[1] = [3]
остальные buckets пусты

Проход f=6..1:
  f=6,5,4: пусто
  f=3: buckets[3]=[1] -> result=[1]
  f=2: buckets[2]=[2] -> result=[1,2] -> size==k=2 -> стоп

Результат: [1, 2]
```

### Сложность

- Время: **O(n)** — подсчёт частот O(n), заполнение корзин O(количество уникальных значений) ≤ O(n), финальный проход по корзинам суммарно посещает не более `n+1` корзин и не более `n` элементов внутри них.
- Память: **O(n)** — под `freq` и `buckets`.

### Сравнение подходов

- **Min-heap (O(n log k))** — хорош, когда `k` мало относительно `n`, интуитивно понятен, обобщается на streaming-сценарий (можно поддерживать top-k "на лету", не имея сразу всех данных).
- **Bucket sort (O(n))** — строго быстрее асимптотически, использует специфику задачи (ограниченный диапазон частот), но требует, чтобы все данные были доступны сразу (не подходит для потоковой обработки).

### Частые вариации

- **Kth Largest Element in an Array** — уже разобранная задача, идентичный паттерн min-heap размера k, но по значению, а не по частоте.
- **Sort Characters By Frequency** — та же идея bucket sort, но применительно к символам строки, с последующим построением результирующей строки.
- **Top K Frequent Words** — аналогично, но при равной частоте нужна **лексикографическая** сортировка — куча сравнивает пары `(частота, слово)` с кастомным компаратором (по частоте по возрастанию для min-heap, но по слову — в обратном лексикографическом порядке, чтобы при извлечении получить правильный порядок).
- **Task Scheduler** — использует похожую идею группировки по частоте (какая задача встречается чаще всего), но дальше решается жадным алгоритмом с учётом cooldown-периода, а не просто top-k выборкой.
