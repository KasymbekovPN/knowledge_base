[[raw data/cpp/interview/_|<=]]

## Kth Largest Element in an Array

**Условие:** дан массив целых чисел `nums` и число `k`. Найти `k`-й по величине элемент в отсортированном порядке (не обязательно уникальный — считается порядковая позиция после сортировки, а не k-е уникальное значение).

```cpp
!!!
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

---
---
---

- [x] Array/String - Two Sum (2026.07.18)
- [x] Array/String - Best Time to Buy and Sell Stock (2026.07.18)
- [x] Array/String - Maximum Subarray (Kadane's algorithm) (2026.07.18)
- [x] Array/String - Merge Intervals (2026.07.18)
- [x] Array/String - Product of Array Except Self (2026.07.19)
- [x] Array/String - Longest Substring Without Repeating Characters (2026.07.19)
- [x] Array/String - Group Anagrams (2026.07.19)
- [x] Array/String - Valid Parentheses (2026.07.19)
- [x] Array/String - Container With Most Water (2026.07.19)
- [x] Array/String - 3Sum (2026.07.19)
- [x] LinkedList - Reverse Linked List (2026.07.19)
- [x] LinkedList - Detect Cycle in Linked List (Floyd's) (2026.07.19)
- [x] LinkedList - Merge Two Sorted Lists (2026.07.21)
- [x] LinkedList - Remove Nth Node From End of List (2026.07.21)
- [x] LinkedList - LRU Cache (список + hash map) (2026.07.21)
- [x] Trees/Graphs - Maximum Depth of Binary Tree (2026.07.21)
- [x] Trees/Graphs - Validate Binary Search Tree (2026.07.21)
- [x] Trees/Graphs - Binary Tree Level Order Traversal (2026.07.21)
- [x] Trees/Graphs - Lowest Common Ancestor of a BST/BT (2026.07.21)
- [x] Trees/Graphs - Serialize and Deserialize Binary Tree (2026.07.22)
- [x] Trees/Graphs - Number of Islands (BFS/DFS) (2026.07.22)
- [x] Trees/Graphs - Clone Graph (2026.07.22)
- [x] Trees/Graphs - Course Schedule (topological sort) (2026.07.22)
- [x] Trees/Graphs - Word Ladder (BFS) (2026.07.22)
- [x] Dynamic - Climbing Stairs (2026.07.22)
- [x] Dynamic - Coin Change (2026.07.22)
- [x] Dynamic - Longest Common Subsequence (2026.07.23)
- [x] Dynamic - Longest Increasing Subsequence (2026.07.23)
- [x] Dynamic - Edit Distance (2026.07.23)
- [x] Dynamic - House Robber (2026.07.23)
- [x] Dynamic - Word Break (2026.07.23)
- [x] Dynamic - 0/1 Knapsack (2026.07.23)
- [x] head/stack/queue - Min Stack (2026.07.23)
- [ ] head/stack/queue - Kth Largest Element in an Array
- [ ] head/stack/queue - Top K Frequent Elements
- [ ] head/stack/queue - Sliding Window Maximum
- [ ] backtracking - Permutations
- [ ] backtracking - Subsets
- [ ] backtracking - N-Queens
- [ ] backtracking - Combination Sum
- [ ] other - Binary Search
- [ ] other - Search in Rotated Sorted Array
- [ ] other - Trapping Rain Water
- [ ] other - Merge K Sorted Lists
- [ ] other - Design a Trie (Prefix Tree)


