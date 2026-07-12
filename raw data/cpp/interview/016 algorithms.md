[[raw data/cpp/interview/_|<=]]

# `<algorithm>` и итераторные категории

## Итераторные категории

Итератор — абстракция «указателя на элемент последовательности». Категории образуют **иерархию по возможностям**: каждая следующая умеет всё, что предыдущая, плюс больше.

```
Input ──┐
        ├──> Forward ──> Bidirectional ──> Random Access ──> Contiguous (C++20)
Output ─┘
```

### Input iterator

Однопроходное чтение. `++`, `*` (только чтение), `==`/`!=`. После инкремента предыдущие копии невалидны.

```cpp
std::istream_iterator<int> it(std::cin);   // читаем из потока — назад не отмотать
```

### Output iterator

Однопроходная запись. `++`, `*` (только запись).

```cpp
std::ostream_iterator<int> out(std::cout, " ");
std::back_insert_iterator — обёртка над push_back
```

### Forward iterator

Многопроходное чтение/запись, можно сохранять копии и проходить заново. `++` только вперёд.

```cpp
std::forward_list<int>::iterator   // односвязный список — только вперёд
std::unordered_map::iterator       // тоже forward
```

### Bidirectional iterator

Добавляет `--` (движение назад).

```cpp
std::list<int>::iterator     // двусвязный список
std::map<int,int>::iterator  // RB-дерево — можно ходить в обе стороны
std::set::iterator
```

### Random Access iterator

Добавляет арифметику: `it + n`, `it - n`, `it1 - it2`, `it[n]`, сравнения `<`, `>`. **Переход на любую позицию за O(1)**.

```cpp
std::vector<int>::iterator
std::deque<int>::iterator
std::array<int,5>::iterator
T*   // обычный указатель — тоже random access!
```

### Contiguous iterator (C++20)

Random access + гарантия, что элементы лежат **непрерывно** в памяти (`&*(it + n) == &*it + n`). Позволяет получить сырой указатель и передать в C-API/memcpy.

```cpp
std::vector<int>::iterator   // contiguous
std::array, std::string, std::span
// НО: std::deque — random access, но НЕ contiguous!
```

---

## Таблица: контейнер → категория

|Контейнер|Категория итератора|
|---|---|
|`vector`, `array`, `string`|**Contiguous** (C++20) / Random Access|
|`deque`|**Random Access** (но не contiguous!)|
|`list`|**Bidirectional**|
|`map`, `set`, `multimap`, `multiset`|**Bidirectional**|
|`unordered_*`|**Forward**|
|`forward_list`|**Forward**|
|потоки (`istream_iterator`)|**Input**|

Важное: `vector<bool>` — **особый случай**, его итератор **не contiguous** (proxy-объекты вместо `bool&`, битовая упаковка). Классическая ловушка стандарта.

---

## Почему категории важны — сложность алгоритмов зависит от них

Алгоритмы требуют минимальной категории и **работают по-разному** в зависимости от неё.

### `std::advance` / `std::distance`

```cpp
std::advance(it, 100);   // сдвинуть на 100

// Random Access:  it += 100         → O(1)
// Bidirectional:  100 раз ++it      → O(n)
// Forward:        100 раз ++it      → O(n)
```

Библиотека **выбирает реализацию по тегу категории** через tag dispatch (или `if constexpr` в современном коде) — это классический пример compile-time полиморфизма.

```cpp
std::distance(v.begin(), v.end());   // vector: O(1) — вычитание указателей
std::distance(l.begin(), l.end());   // list:   O(n) — считаем шагами!
```

### `std::sort` требует Random Access

```cpp
std::vector<int> v;
std::sort(v.begin(), v.end());   // ✅ O(n log n)

std::list<int> l;
std::sort(l.begin(), l.end());   // ❌ ОШИБКА КОМПИЛЯЦИИ — bidirectional недостаточно
l.sort();                        // ✅ у list есть СВОЙ sort (merge sort, O(n log n))
```

Причина: `std::sort` (introsort — quicksort + heapsort + insertion sort) требует произвольного доступа для выбора pivot и перестановок. Для списка нужен другой алгоритм (merge sort через перестановку указателей).

Аналогично `std::nth_element`, `std::binary_search` (работает на forward, но за O(n) шагов вместо O(log n) — сравнений log n, а вот **переходов** n).

### `std::reverse` требует Bidirectional

```cpp
std::reverse(v.begin(), v.end());   // ✅
std::reverse(fl.begin(), fl.end()); // ❌ forward_list — только Forward
fl.reverse();                        // ✅ свой метод
```

---

## Почему у `list`/`map` есть собственные методы

Когда алгоритм можно сделать **эффективнее**, зная внутреннее устройство, контейнер даёт свой метод:

|Общий алгоритм|Метод контейнера|Почему|
|---|---|---|
|`std::sort` — O(n log n), требует RA|`list::sort()`|merge sort через указатели, без перемещения данных|
|`std::remove` + `erase` — O(n)|`list::remove()`|удаляет узлы напрямую, без сдвигов|
|`std::unique`|`list::unique()`|то же|
|`std::find` — O(n)|`map::find()` — **O(log n)**|использует структуру дерева!|
|`std::find` — O(n)|`unordered_map::find()` — **O(1)**|хеш|
|—|`list::splice()` — **O(1)**|перенос узлов перестановкой указателей|

**Ключевое правило: если у контейнера есть одноимённый метод — используй его, он почти всегда эффективнее общего алгоритма.**

```cpp
std::map<int, int> m;
std::find_if(m.begin(), m.end(), [](auto& p){ return p.first == 42; });   // ❌ O(n)
m.find(42);                                                                // ✅ O(log n)
```

---

## Основные алгоритмы `<algorithm>`

### Немодифицирующие

```cpp
std::find(first, last, value);              // O(n)
std::find_if(first, last, pred);            // O(n)
std::count(first, last, value);             // O(n)
std::count_if(first, last, pred);
std::all_of / any_of / none_of(first, last, pred);   // O(n)
std::equal(f1, l1, f2);                     // сравнение диапазонов
std::mismatch(f1, l1, f2);                  // первое расхождение
std::search(f1, l1, f2, l2);                // подпоследовательность
```

### Модифицирующие

```cpp
std::copy(first, last, dest);
std::copy_if(first, last, dest, pred);
std::move(first, last, dest);               // перемещает элементы (не std::move из <utility>!)
std::transform(first, last, dest, func);    // map/apply
std::fill(first, last, value);
std::generate(first, last, gen);
std::replace(first, last, old, new);
std::reverse(first, last);                  // требует Bidirectional
std::rotate(first, middle, last);
std::swap_ranges(f1, l1, f2);
```

### Удаление — erase-remove идиома

**Критично понимать:** алгоритмы **не могут менять размер контейнера** — у них есть только итераторы, не сам контейнер.

```cpp
std::vector<int> v{1,2,3,2,4,2};

std::remove(v.begin(), v.end(), 2);   // ⚠️ НЕ удаляет! только переставляет
// v == {1,3,4, ?,?,?}   size ВСЁ ЕЩЁ 6
//            ^ возвращённый итератор — "новый конец"
```

`remove` сдвигает нужные элементы влево и возвращает итератор на новый логический конец. «Хвост» — в unspecified состоянии (обычно перемещённые из).

**Erase-remove идиома:**

```cpp
v.erase(std::remove(v.begin(), v.end(), 2), v.end());   // ✅ теперь реально удалено
// C++20 — проще:
std::erase(v, 2);
std::erase_if(v, [](int x){ return x % 2 == 0; });
```

Аналогично `std::unique` — не удаляет, а переставляет (и требует **отсортированной** последовательности, чтобы убрать все дубликаты):

```cpp
std::sort(v.begin(), v.end());
v.erase(std::unique(v.begin(), v.end()), v.end());   // классика: удалить дубликаты
```

### Сортировка и порядок

```cpp
std::sort(first, last);                  // O(n log n), introsort, НЕ стабильная
std::stable_sort(first, last);           // O(n log n) [или O(n log²n) при нехватке памяти], стабильная
std::partial_sort(first, middle, last);  // отсортировать только первые k — O(n log k)
std::nth_element(first, nth, last);      // поставить n-й элемент на место — O(n) в среднем!
std::is_sorted(first, last);
```

**`nth_element` — недооценённый алгоритм:** находит k-й порядковый элемент (медиану!) за **O(n)** в среднем, без полной сортировки. Частичное разбиение (quickselect).

```cpp
// найти медиану за O(n), а не O(n log n)
auto mid = v.begin() + v.size()/2;
std::nth_element(v.begin(), mid, v.end());
int median = *mid;
```

**Стабильность:** `sort` может переставить равные элементы, `stable_sort` — сохраняет их относительный порядок.

### Бинарный поиск (на отсортированном!)

```cpp
std::binary_search(first, last, value);   // bool — есть ли
std::lower_bound(first, last, value);     // первый >= value
std::upper_bound(first, last, value);     // первый > value
std::equal_range(first, last, value);     // пара [lower, upper) — все равные
```

**Тонкость:** на Random Access — O(log n). На Forward/Bidirectional — сравнений O(log n), но **переходов O(n)** → итого O(n). Поэтому на `list` бинарный поиск бессмысленен.

**Обязательное требование:** последовательность должна быть отсортирована по тому же компаратору. Иначе — UB (не ошибка, а тихо неверный результат).

### Куча (heap)

```cpp
std::make_heap(first, last);        // O(n)
std::push_heap(first, last);        // после push_back — O(log n)
std::pop_heap(first, last);         // переносит max в конец — O(log n), потом pop_back
std::sort_heap(first, last);        // heapsort
```

Основа `std::priority_queue`.

### Множества (на отсортированных)

```cpp
std::set_union, set_intersection, set_difference, set_symmetric_difference
std::includes
std::merge          // слияние двух отсортированных
```

### Min/max

```cpp
std::min_element / max_element(first, last);   // O(n)
std::minmax_element(first, last);              // O(n), но за ~1.5n сравнений вместо 2n
std::clamp(v, lo, hi);                         // C++17
```

### Численные (`<numeric>` — не algorithm, но рядом)

```cpp
std::accumulate(first, last, init);            // свёртка (fold)
std::accumulate(first, last, init, op);        // с кастомной операцией
std::reduce(first, last);                      // C++17, может параллелиться, порядок не гарантирован
std::inner_product, std::partial_sum, std::iota
```

```cpp
std::iota(v.begin(), v.end(), 0);   // заполнить 0,1,2,3,...
int sum = std::accumulate(v.begin(), v.end(), 0);
```

---

## Итераторные адаптеры

```cpp
std::back_inserter(v)     // превращает копирование во вставку через push_back
std::front_inserter(d)    // push_front
std::inserter(c, pos)     // insert

std::vector<int> dst;
std::copy(src.begin(), src.end(), std::back_inserter(dst));   // ✅ dst растёт
// std::copy(src.begin(), src.end(), dst.begin());  // ⚠️ UB! dst пустой — пишем за границы
```

Частая ошибка: `copy`/`transform` **не создают элементы**, они **пишут** в существующие. Без `back_inserter` нужно заранее `resize`.

```cpp
std::reverse_iterator     // rbegin()/rend()
std::move_iterator        // разыменование даёт rvalue → элементы перемещаются, а не копируются
```

---

## Ranges (C++20) — современная альтернатива

```cpp
#include <ranges>
namespace rv = std::views;

std::vector<int> v{1,2,3,4,5,6};

// вместо пары итераторов — сам контейнер:
std::ranges::sort(v);
auto it = std::ranges::find(v, 3);

// ленивые вьюхи с композицией:
auto result = v | rv::filter([](int x){ return x % 2 == 0; })
                | rv::transform([](int x){ return x * x; })
                | rv::take(2);
// {4, 16} — вычисляется ЛЕНИВО при обходе, без промежуточных контейнеров
```

Плюсы: короче, композируемо, ленивая оценка (нет временных векторов), проверка через **concepts** (внятные ошибки компиляции вместо «шаблонной простыни»).

Concepts заменили старые теги категорий:

```cpp
std::input_iterator, std::forward_iterator, std::bidirectional_iterator,
std::random_access_iterator, std::contiguous_iterator
```

---

## Параллельные алгоритмы (C++17)

```cpp
#include <execution>

std::sort(std::execution::par, v.begin(), v.end());        // многопоточно
std::for_each(std::execution::par_unseq, v.begin(), v.end(), f);
std::reduce(std::execution::par, v.begin(), v.end());
```

Политики: `seq` (последовательно), `par` (параллельно), `par_unseq` (параллельно + векторизация), `unseq` (C++20, только векторизация).

**Важно:** предикаты/операции должны быть **потокобезопасны** и **без гонок**. Порядок обхода не гарантирован → `reduce` требует **ассоциативной и коммутативной** операции (в отличие от `accumulate`, где порядок строго слева направо).

---

## Формулировки на собеседовании

**«Какие категории итераторов и чем отличаются?»** — Input/Output (однопроходные), Forward (многопроходный, только вперёд), Bidirectional (+`--`), Random Access (+арифметика, O(1) переход), Contiguous (C++20, +непрерывность памяти).

**«Почему `std::sort` не работает с `std::list`?»** — Требует Random Access (для quicksort нужен произвольный доступ), а у `list` — Bidirectional. У `list` есть собственный `sort()` на merge sort через перестановку указателей.

**«Что делает `std::remove`?»** — Не удаляет! Сдвигает «оставляемые» элементы влево и возвращает итератор на новый логический конец. Размер контейнера **не меняется** (алгоритм не имеет доступа к контейнеру). Отсюда erase-remove идиома, или `std::erase`/`erase_if` в C++20.

**«Сложность `std::distance`?»** — O(1) для Random Access (вычитание итераторов), O(n) для остальных (подсчёт шагами). Библиотека выбирает реализацию по тегу категории.

**«В чём разница `sort` и `stable_sort`?»** — `sort` (introsort) — O(n log n), не сохраняет порядок равных элементов. `stable_sort` — сохраняет, но требует доп. памяти (при нехватке деградирует до O(n log²n)).

**«Как найти медиану быстрее, чем сортировкой?»** — `std::nth_element` — O(n) в среднем (quickselect), частичное разбиение без полной сортировки.

**«Почему при `std::copy` в пустой вектор — UB?»** — `copy` пишет в существующие элементы, не создаёт их. Нужен `std::back_inserter` или предварительный `resize`.

---

Отличие от Java: там алгоритмы — методы коллекций (`list.sort()`) или Stream API (`stream().filter().map().collect()`). C++ разделяет **контейнеры и алгоритмы** через итераторы: один `std::sort` работает с любым Random Access диапазоном — вектором, массивом, даже сырыми указателями. Плата — нужно передавать пару итераторов и следить за их категорией. C++20 Ranges сближает подходы: `v | views::filter(...) | views::transform(...)` — прямой аналог Stream API, но **ленивый и без аллокаций** (Java Stream тоже ленив, но боксит и аллоцирует). Это ещё один пример: zero-cost abstraction на шаблонах вместо рантайм-механизмов.
