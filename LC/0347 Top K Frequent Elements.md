## Суть задачи

**LeetCode 347 (Top K Frequent Elements)** — дан массив, нужно найти k элементов с наибольшей частотой встречаемости.

Частный случай общей проблемы "top-K selection", и она масштабируется прямо в продакшен-задачи ранжирования.

## Почему heap, а не сортировка

Наивный путь — отсортировать весь массив (`O(n log n)`) и взять первые k. Но если у вас миллионы кандидатов, а нужно top-10, сортировать всё — расточительно.

**Идея с heap:** поддерживать min-heap размера k.

- Проходим по всем n элементам.
- Если heap меньше k — просто добавляем.
- Если heap уже размера k — сравниваем новый элемент с минимумом кучи (root). Если новый больше — выкидываем минимум, вставляем новый.

Сложность: `O(n log k)` вместо `O(n log n)`. Когда `k << n` (а в поиске top-10/top-50 из миллионов документов — это ровно ваш случай), разница огромная.

Почему **min-heap**, а не max-heap, для поиска top-K _наибольших_: в куче мы держим текущих "финалистов", и на каждом шаге нам нужно быстро узнать, кто из финалистов слабейший (кандидат на вылет) — это и есть root min-heap. Достать минимум — `O(1)`, заменить его — `O(log k)`.

```cpp
#include <iostream>
#include <format>
#include <queue>
#include <vector>
#include <unordered_map>

namespace {
    template<typename T>
    void display_vector(const std::vector<T>& vec, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        for (const auto& e : vec) {
            std::cout << e << " ";
        }
        std::cout << ")\n";
    }

    std::vector<int> top_k_freq(const std::vector<int>& nums, const int k) {
        std::unordered_map<int, int> freq;
        for (const int n: nums) freq[n]++;

        auto cmp = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second; // greater => min-heap
        };
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(cmp)> heap(cmp);

        for (auto& [num, count]: freq) {
            heap.emplace(num, count);
            if (heap.size() > k) heap.pop();
        }

        std::vector<int> result;
        while (!heap.empty()) {
            result.push_back(heap.top().first);
            heap.pop();
        }

        return result;
    }
}

int main() {
    const std::vector<int> nums = {1, 2, 3, 4, 5, 5, 4, 3, 1, 1, 2, 1, 5};
    const std::vector<int> result = top_k_freq(nums, 5);
    display_vector(result, "result");
}

```

## Параллель с ранжированием поиска

В поисковом рантайме у вас типичный сценарий:

- Есть N документов-кандидатов (после первичной фильтрации/пересечения posting-листов) — это могут быть тысячи или миллионы.
- Нужно вернуть top-K (обычно 10–100) по релевантности (BM25, ML-score, etc).
- Считать полный score для всех N и сортировать — дорого и часто не нужно.

Решения, которые реально используются в поисковых системах и это explicit развитие темы top-K:

1. **Heap-based top-K** — ровно то, что выше. Держим min-heap размера K по score, отбрасываем слабых кандидатов на лету, пока стримим документы из индекса.
2. **WAND / Block-Max WAND** — более продвинутый алгоритм именно для поиска: использует верхние границы score по блокам posting-листа, чтобы вообще пропускать документы, которые заведомо не попадут в top-K, не считая им score целиком. Это следующий уровень, если в интервью зайдёт глубже — стоит быть готовым хотя бы концептуально объяснить.
3. **Distributed top-K (merge from shards)** — если у вас шардированный индекс: каждый шард считает свой локальный top-K (heap size K), а потом coordinator сливает результаты с разных шардов через ещё один heap/k-way merge (это как раз связка с Merge k Sorted Lists, которую тоже упомянули в списке).

## На что обратить внимание на собеседовании

Вероятные усложнения, которые часто добавляют именно под профиль "поисковый рантайм":

- **"А что если K намного меньше N, но данные приходят потоком (streaming) и не помещаются в память?"** → heap остаётся тем же решением, потому что он online по своей природе — не нужно видеть весь массив сразу.
- **"А как распараллелить между потоками/шардами?"** → merge partial top-K heaps.
- **"А если нужно top-K, но с поддержкой обновлений (документ мог измениться)?"** → indexed heap / упомянуть, что чистый heap этого не поддерживает эффективно, нужна доп. структура (например, hash map от id к позиции в heap).

Хотите, разберём Merge k Sorted Lists следующим — это прямое продолжение темы (слияние top-K с шардов)?
