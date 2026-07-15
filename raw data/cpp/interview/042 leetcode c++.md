[[raw data/cpp/interview/_|<=]]

# Алгоритмические задачи на C++ — стратегия для собеседования

Как senior с алгоритмической базой ты, скорее всего, знаешь **сами алгоритмы**. На C++-собеседовании проверяют другое: **идиоматичное владение языком** при решении. Разберём именно C++-специфику — что отличает «Java-программиста, пишущего на C++» от «C++ разработчика».

## Что реально оценивают

1. **STL по назначению** — не пишешь свой хеш-мап, когда есть `unordered_map`
2. **Правильный контейнер** — понимаешь сложности (мы разбирали)
3. **Отсутствие лишних копий** — `const&`, move, `emplace`, `reserve`
4. **Корректность с итераторами** — не ловишь инвалидацию
5. **Чистый современный код** — structured bindings, range-for, лямбды, ranges

Давай пройдём по типовым паттернам с акцентом на C++.

---

## Паттерн 1: хеш-таблица (частоты, поиск пар)

**Two Sum** — канонично:

```cpp
std::vector<int> twoSum(const std::vector<int>& nums, int target) {
    std::unordered_map<int, int> seen;   // значение → индекс
    seen.reserve(nums.size());            // ✅ избегаем rehash

    for (int i = 0; i < std::ssize(nums); ++i) {   // ✅ ssize — знаковый (C++20)
        int need = target - nums[i];
        if (auto it = seen.find(need); it != seen.end()) {   // ✅ if-init + find
            return {it->second, i};                           // ✅ list-init возврата
        }
        seen[nums[i]] = i;
    }
    return {};
}
```

C++-детали, которые замечают:

- `const vector&` — не копируем входные данные
- `reserve` — устраняем rehash
- `if (auto it = ...; it != end())` — идиома «найти и использовать» без двойного поиска (не `count()` + `[]`!)
- `std::ssize` вместо `nums.size()` — избегаем signed/unsigned ловушки в цикле

**Частая ошибка Java-стиля:**

```cpp
if (seen.count(need)) return {seen[need], i};   // ⚠️ ДВА поиска в хеше!
```

**Частоты символов:**

```cpp
std::unordered_map<char, int> freq;
for (char c : s) ++freq[c];   // ✅ operator[] создаёт 0 при первом обращении

// или для символов — массив быстрее:
std::array<int, 256> count{};   // ✅ на стеке, ноль аллокаций, {} зануляет
for (unsigned char c : s) ++count[c];
```

---

## Паттерн 2: two pointers / sliding window

**Longest substring without repeating:**

```cpp
int lengthOfLongestSubstring(std::string_view s) {   // ✅ string_view — без копии
    std::array<int, 256> lastSeen;
    lastSeen.fill(-1);
    int maxLen = 0, start = 0;

    for (int i = 0; i < std::ssize(s); ++i) {
        unsigned char c = s[i];
        if (lastSeen[c] >= start) {
            start = lastSeen[c] + 1;
        }
        lastSeen[c] = i;
        maxLen = std::max(maxLen, i - start + 1);
    }
    return maxLen;
}
```

- `string_view` параметр — работает и с `std::string`, и с литералом, без копий (мы разбирали)
- `std::array` + `fill` вместо map — когда алфавит фиксирован, кэш-локальность решает
- `std::max` вместо ручного `if`

---

## Паттерн 3: сортировка + компаратор

Здесь C++-специфика особенно видна.

```cpp
struct Interval { int start, end; };

std::vector<Interval> merge(std::vector<Interval> intervals) {   // по значению — будем сортировать
    if (intervals.empty()) return {};

    // ✅ ranges::sort с проекцией (C++20) — без лямбды-компаратора
    std::ranges::sort(intervals, {}, &Interval::start);

    std::vector<Interval> result;
    result.reserve(intervals.size());              // ✅ верхняя оценка
    result.push_back(intervals[0]);

    for (const auto& iv : intervals | std::views::drop(1)) {   // ✅ пропустить первый
        if (iv.start <= result.back().end) {
            result.back().end = std::max(result.back().end, iv.end);
        } else {
            result.push_back(iv);
        }
    }
    return result;   // ✅ NRVO / move — не копируется
}
```

- `ranges::sort(v, {}, &Interval::start)` — проекция вместо лямбды (мы разбирали в ranges): читается сразу как «сортируй по start»
- Классический вариант компаратора, если ranges недоступны:

```cpp
std::sort(intervals.begin(), intervals.end(),
          [](const Interval& a, const Interval& b) { return a.start < b.start; });
```

- **Компаратор должен быть strict weak ordering** — `<`, не `<=`! Иначе UB:

```cpp
[](int a, int b){ return a <= b; }   // ⚠️ UB в std::sort! нарушает irreflexivity
[](int a, int b){ return a < b; }    // ✅
```

Это любимый вопрос-ловушка.

---

## Паттерн 4: heap / priority_queue

**Top K / K-th largest:**

```cpp
int findKthLargest(std::vector<int>& nums, int k) {
    // min-heap размера k
    std::priority_queue<int, std::vector<int>, std::greater<>> minHeap;
    //                                          ^^^^^^^^^^^^^ min-heap (по умолчанию max)

    for (int x : nums) {
        minHeap.push(x);
        if (std::ssize(minHeap) > k) minHeap.pop();   // держим k наибольших
    }
    return minHeap.top();
}
```

- `std::greater<>` — min-heap (по умолчанию `priority_queue` — max-heap!)
- `std::greater<>` (пустые скобки, C++14) — transparent, выводит тип

**Но часто лучше `nth_element` — O(n) вместо O(n log k):**

```cpp
int findKthLargest(std::vector<int>& nums, int k) {
    auto kth = nums.begin() + (k - 1);
    std::nth_element(nums.begin(), kth, nums.end(), std::greater<>{});   // ✅ O(n)
    return *kth;
}
```

Упомянуть `nth_element` на собеседовании — сильный ход (мы разбирали в алгоритмах): показывает знание STL за пределами базы.

---

## Паттерн 5: обход дерева/графа

**BFS:**

```cpp
int bfs(const std::vector<std::vector<int>>& adj, int start, int target) {
    std::queue<int> q;
    std::vector<bool> visited(adj.size(), false);   // ✅ vector<bool> тут ОК (компактно)
    
    q.push(start);
    visited[start] = true;
    int depth = 0;

    while (!q.empty()) {
        int levelSize = std::ssize(q);
        for (int i = 0; i < levelSize; ++i) {
            int node = q.front();
            q.pop();
            if (node == target) return depth;
            for (int next : adj[node]) {
                if (!visited[next]) {
                    visited[next] = true;
                    q.push(next);
                }
            }
        }
        ++depth;
    }
    return -1;
}
```

**DFS с деревом (unique_ptr для владения):**

```cpp
struct TreeNode {
    int val;
    std::unique_ptr<TreeNode> left, right;   // ✅ владение через unique_ptr
    explicit TreeNode(int v) : val(v) {}
};

int maxDepth(const TreeNode* root) {   // ✅ сырой указатель = НАБЛЮДЕНИЕ (не владение)
    if (!root) return 0;
    return 1 + std::max(maxDepth(root->left.get()),
                        maxDepth(root->right.get()));
}
```

Тонкость владения (мы разбирали): узлы владеют детьми через `unique_ptr`, но функция обхода принимает **сырой указатель** — она не владеет, только читает. `.get()` для перехода. Это идиоматично.

**Ловушка LeetCode:** там `TreeNode` обычно с сырыми указателями (`TreeNode* left`) и без деструктора — там владение неявное. В «правильном» C++ — `unique_ptr`.

---

## Паттерн 6: DP с таблицей

```cpp
int longestCommonSubsequence(std::string_view a, std::string_view b) {
    int n = std::ssize(a), m = std::ssize(b);
    // ✅ 2D через 1D vector или vector<vector> — для читаемости vector<vector>
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (a[i-1] == b[j-1]) {
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = std::max(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    return dp[n][m];
}
```

Оптимизация памяти (две строки вместо всей таблицы) — упомянуть, что можно O(min(n,m)) памяти:

```cpp
std::vector<int> prev(m + 1, 0), curr(m + 1, 0);
// ... swap(prev, curr) после каждой строки — ✅ O(1) swap векторов
```

`std::swap` двух векторов — O(1) (меняются указатели), не копирование. Хороший штрих.

---

## Ключевые C++-идиомы для собеседования

### Возврат нескольких значений

```cpp
std::pair<int, int> minMax(const std::vector<int>& v) {
    auto [minIt, maxIt] = std::minmax_element(v.begin(), v.end());   // ✅ STL + structured binding
    return {*minIt, *maxIt};
}

auto [mn, mx] = minMax(nums);   // ✅ распаковка
```

### Range-based обработка

```cpp
// Сумма квадратов чётных
int result = 0;
for (int x : nums | std::views::filter([](int x){ return x % 2 == 0; })
                  | std::views::transform([](int x){ return x * x; })) {
    result += x;
}
// или std::accumulate
```

### Избегание копий

```cpp
for (const auto& item : container) { }   // ✅ const& — без копий
for (auto& item : container) { }          // ✅ если модифицируем
for (auto item : container) { }           // ⚠️ КОПИЯ каждого элемента!
```

Для `vector<string>` копия каждой строки в цикле — заметная ошибка.

### `std::string` построение

```cpp
// ❌ конкатенация в цикле — квадратичная сложность
std::string result;
for (const auto& s : parts) result = result + s;   // ⚠️ O(n²)

// ✅
std::string result;
size_t total = 0;
for (const auto& s : parts) total += s.size();
result.reserve(total);                              // ✅ одна аллокация
for (const auto& s : parts) result += s;            // ✅ O(n)
```

---

## Типичные C++-ошибки на собеседовании

**1. Модификация контейнера во время итерации** (мы разбирали инвалидацию):

```cpp
for (auto it = v.begin(); it != v.end(); ++it) {
    if (cond(*it)) v.erase(it);   // ⚠️ it инвалидирован!
}
// ✅
for (auto it = v.begin(); it != v.end(); ) {
    if (cond(*it)) it = v.erase(it);
    else ++it;
}
// ✅ ещё лучше (C++20)
std::erase_if(v, cond);
```

**2. `map::operator[]` вместо `find`** (создаёт элемент!):

```cpp
if (m[key] > 0) { }   // ⚠️ вставляет {key, 0}, если ключа не было!
if (auto it = m.find(key); it != m.end() && it->second > 0) { }   // ✅
```

**3. Компаратор с `<=`** (UB в sort) — упомянул выше.

**4. Integer overflow в вычислениях:**

```cpp
int mid = (low + high) / 2;          // ⚠️ low+high может переполнить int
int mid = low + (high - low) / 2;    // ✅ классика binary search
```

Это тот самый signed overflow UB, что мы разбирали — знаменитый баг в `Arrays.binarySearch` был именно здесь.

**5. Dangling из-за возврата ссылки/view:**

```cpp
std::string_view firstWord(const std::string& s);   // ✅ ОК — s жив у вызывающего
std::string_view firstWord(std::string s);          // ⚠️ view на параметр-копию — dangling!
```

---

## Как готовиться конкретно

Раз алгоритмы ты знаешь, фокус на:

1. **Прорешать 20-30 задач medium, ПИСАВ идиоматично** — не «Java на C++», а с STL, ranges, structured bindings. LeetCode + переписывать своё решение «как написал бы C++ эксперт».
    
2. **Знать сложности STL наизусть** (мы разбирали) — на собеседовании спросят «почему unordered_map, а не map».
    
3. **Уметь объяснять выбор** — «взял `nth_element`, потому что O(n) против O(n log n) у сортировки».
    
4. **Написать несколько структур руками** — LRU cache (`list` + `unordered_map`), trie, union-find. Это частые задачи, проверяющие владение указателями/итераторами.
    

**LRU Cache** — почти гарантированная задача, идеально проверяет C++:

```cpp
class LRUCache {
    int capacity_;
    std::list<std::pair<int, int>> items_;                      // ключ-значение, MRU спереди
    std::unordered_map<int, std::list<std::pair<int,int>>::iterator> map_;   // ключ → итератор
    // ⭐ list не инвалидирует итераторы при вставке/удалении — ключевое свойство!
public:
    explicit LRUCache(int cap) : capacity_(cap) {}

    int get(int key) {
        auto it = map_.find(key);
        if (it == map_.end()) return -1;
        items_.splice(items_.begin(), items_, it->second);   // ✅ O(1) перемещение в начало
        return it->second->second;
    }

    void put(int key, int value) {
        if (auto it = map_.find(key); it != map_.end()) {
            it->second->second = value;
            items_.splice(items_.begin(), items_, it->second);
            return;
        }
        if (std::ssize(items_) >= capacity_) {
            map_.erase(items_.back().first);
            items_.pop_back();
        }
        items_.emplace_front(key, value);
        map_[key] = items_.begin();
    }
};
```

Почему это отличная C++-задача: использует `list::splice` (O(1), не инвалидирует итераторы — мы разбирали это уникальное свойство `list`), хранит итераторы в map (что безопасно **только** потому, что `list` их не инвалидирует), демонстрирует понимание сложностей.

---

## Формулировки на собеседовании

**«Почему `unordered_map`, а не `map`?»** — O(1) средняя против O(log n); порядок не нужен. (Если нужен отсортированный обход или worst-case гарантия — `map`.)

**«Почему `nth_element`, а не `sort`?»** — Нужен k-й элемент, не полный порядок → O(n) против O(n log n).

**«Почему храните итераторы `list` в map?»** — `list` не инвалидирует итераторы при вставке/удалении других элементов → можно безопасно хранить; `splice` даёт O(1) перемещение.

**«Почему компаратор `<`, а не `<=`?»** — `std::sort` требует strict weak ordering; `<=` нарушает иррефлексивность → **UB**.

**«Почему `low + (high-low)/2`?»** — `low + high` может переполнить `int` (signed overflow UB) при больших индексах.

---

Раз ты идёшь с сильной алгоритмической базой, главный совет: **на собеседовании проговаривай C++-решения, а не только алгоритм**. Скажи «беру `unordered_map` с `reserve`», «здесь `nth_element` за O(n)», «`list::splice` не инвалидирует итераторы» — это отличает senior C++ от «кандидата, который знает алгоритмы». Твой memory-профиль показывает глубокую подготовку по языку — на алгоритмической секции именно связка «алгоритм + идиоматичный C++» будет твоим преимуществом.

Хочешь разобрать конкретные задачи (LRU, trie, union-find написать полностью), или вернёмся к пропущенному дню 7 (unique_ptr/shared_ptr/vector руками) — это тоже частые «напиши структуру» задачи?

---

## Неделя 1: Core C++ и память

**День 1–2: Управление памятью и RAII**

- [x] Stack vs Heap, lifetime объектов (2026.07.10)
- [x] RAII, правило 0/3/5 (2026.07.10)
- [x] Smart pointers: `unique_ptr`, `shared_ptr`, `weak_ptr` — внутреннее устройство, control block, циклические ссылки (2026.07.10)
- [x] `make_shared` vs `make_unique` (2026.07.10)

**День 3–4: Move-семантика и rvalue**

- [x] lvalue/rvalue/xvalue, `std::move`, `std::forward` (2026.07.10)
- [x] Perfect forwarding, universal (forwarding) references (2026.07.10)
- [x] RVO/NRVO, copy elision (гарантии C++17) (2026.07.10)
- [x] Move constructor/assignment, когда генерируются (2026.07.10)

**День 5: Объектная модель и виртуальность**

- [x] vtable/vptr, как работает виртуальный вызов (2026.07.11)
- [x] Виртуальный деструктор (зачем) (2026.07.11)
- [x] Множественное и виртуальное наследование, ромб (2026.07.11)
- [x] `override`, `final`, slicing (2026.07.11)

**День 6: const и type system**

- [x] `const`/`constexpr`/`consteval`/`constinit` (2026.07.11)
- [x] const correctness, `mutable` (2026.07.11)
- [x] Касты: `static_cast`, `dynamic_cast`, `reinterpret_cast`, `const_cast` (2026.07.11)

**День 7: Повтор + написать руками** unique_ptr, shared_ptr, vector (push_back с реаллокацией).

## Неделя 2: STL, шаблоны, многопоточность

**День 8–9: STL контейнеры и алгоритмы**

- [x] Сложность операций: `vector`, `deque`, `map`/`set` (RB-tree), `unordered_map` (хеши, коллизии, rehash) (2026.07.12)
- [x] Инвалидация итераторов (важный вопрос) (2026.07.12)
- [x] Алгоритмы: `<algorithm>`, итераторные категории (2026.07.12)
- [x] `emplace` vs `insert`, `reserve` (2026.07.12)

**День 10–11: Шаблоны**

- [x] Function/class templates, специализация (полная/частичная) (2026.07.12)
- [x] SFINAE, `enable_if`, базовые concepts (C++20) (2026.07.13)
- [x] Variadic templates, fold expressions (2026.07.13)
- [x] CRTP, type traits (2026.07.13)

**День 12–13: Многопоточность**

- [x] `std::thread`, `jthread`, `async`/`future`/`promise` (2026.07.13)
- [x] `mutex`, `lock_guard`, `unique_lock`, `scoped_lock`, deadlock (2026.07.13)
- [x] `condition_variable`, spurious wakeup (2026.07.13)
- [x] Memory model, `std::atomic`, memory_order (2026.07.13)
- [x] Data race vs race condition (2026.07.13)

**День 14: Повтор** + thread-safe queue, простой thread pool.

## Неделя 3: Современный C++, UB, практика

**День 15: Modern C++ обзор**

- [x] C++11→23: structured bindings (2026.07.14)
- [x] C++11→23: `auto` (2026.07.14)
- [x] C++11→23: lambdas (2026.07.14)
- [x] C++11→23: ranges (2026.07.14)
- [x] C++11→23: `optional` (2026.07.14)
- [x] C++11→23: `variant` (2026.07.14)
- [x] C++11→23: `expected` (2026.07.14)
- [x] C++11→23:`string_view` (2026.07.14)
- [x] C++11→23:`span` (2026.07.14)

**День 16: Undefined Behavior**

- [x] Undefined Behavior: dangling (2026.07.14)
- [x] Undefined Behavior: use-after-free (2026.07.15)
- [x] Undefined Behavior: signed overflow (2026.07.15)
- [x] Undefined Behavior: strict aliasing (2026.07.15)
- [x] Undefined Behavior: нарушение порядка вычислений (2026.07.15)

**День 17–18: Алгоритмические задачи**

- [ ] LeetCode medium на C++ (важно: писать чисто, с STL)
- [ ] Акцент на корректную работу с памятью/итераторами

**День 19: System design / архитектура** (как тимлид это твоя сильная зона — освежи именно C++-специфику: pImpl, header/cpp разделение, ABI, линковка, ODR)

**День 20: Build/tooling**

- [ ] Компиляция/линковка, ODR violations
- [ ] CMake базово, sanitizers (ASan/UBSan/TSan), valgrind
- [ ] `-O` уровни, что компилятор оптимизирует

**День 21: Mock-собеседование** — прорешай вслух, объясняя решения.

---

## Ключевые «любимые» вопросы интервьюеров

- Чем `shared_ptr` thread-safe, а чем нет (счётчик vs объект)
- Что будет при исключении в конструкторе/деструкторе
- `static` инициализация и порядок (static init order fiasco)
- Разница `int*`, `int* const`, `const int*`
- Почему нельзя возвращать ссылку на локальную переменную

## Учитывая твой бэкграунд

Раз последние годы был не на «чистом» C++ — **сделай акцент на том, что отличает C++ от Java**: ручное управление памятью, move-семантика, шаблоны вместо generics (стирание типов vs мономорфизация), отсутствие GC, деструкторы/RAII, UB. Это типичные места, где «java-программиста в C++» ловят.

Хочешь — могу расписать конкретные задачи/материалы под любой из дней или сделать набор флеш-карточек по каверзным вопросам.**