---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Container

Boost.Container — это набор контейнеров: частью повторяющих стандартные (но с дополнительными возможностями), частью уникальных. Все header-only. Разберу по группам.

CMake: `find_package(Boost REQUIRED)` + `target_link_libraries(app PRIVATE Boost::boost)` (header-only интерфейс).

## Обзор: что входит

|Контейнер|Назначение|
|---|---|
|`vector`|Как `std::vector`, но с расширенными возможностями|
|`static_vector<T, N>`|Вектор фиксированной ёмкости, **вся память на стеке**|
|`small_vector<T, N>`|Первые N элементов на стеке, дальше — в куче|
|`flat_map` / `flat_set`|Ассоциативные, но хранятся в отсортированном массиве|
|`flat_multimap` / `flat_multiset`|То же с дубликатами|
|`stable_vector`|Vector со стабильными ссылками/итераторами|
|`devector`|Двусторонний вектор (эффективная вставка с обоих концов)|
|`deque`, `list`, `slist`|Аналоги стандартных|
|`string`|Аналог `std::string` с оптимизациями|

## 1. `static_vector<T, Capacity>`

Последовательный контейнер фиксированной максимальной ёмкости. Память — внутри объекта (на стеке), без обращений к куче.

### Особенности

- Ёмкость задаётся **на этапе компиляции** и неизменна.
- Превышение ёмкости → `std::bad_alloc` (или UB у небезопасных операций).
- Идеален для real-time / embedded, где аллокации запрещены.

### Основные методы

|Метод|Описание|
|---|---|
|`push_back(val)` / `emplace_back(args...)`|Добавить в конец|
|`pop_back()`|Удалить последний|
|`size()` / `capacity()`|Текущий размер / фиксированная ёмкость|
|`max_size()`|Равна `Capacity`|
|`operator[]`, `at()`, `front()`, `back()`|Доступ|
|`begin()`, `end()`|Итераторы|
|`clear()`, `resize()`, `insert()`, `erase()`|Изменение|

```cpp
#include <boost/container/static_vector.hpp>
namespace bc = boost::container;

bc::static_vector<int, 8> sv;  // ёмкость 8, всё на стеке
sv.push_back(1);
sv.push_back(2);
// sv.push_back(...) свыше 8 раз → bad_alloc
```

## 2. `small_vector<T, N>`

Гибрид: первые `N` элементов хранятся inline (на стеке), при превышении переключается на динамическую память. Наследник интерфейса `vector`.

### Особенности

- Оптимизация для случая «обычно мало элементов, иногда много».
- Полный интерфейс `std::vector` (растёт без ограничений).
- `N` — лишь размер inline-буфера, не жёсткий лимит.

```cpp
#include <boost/container/small_vector.hpp>
namespace bc = boost::container;

bc::small_vector<int, 4> v{1, 2, 3};  // на стеке
v.push_back(4);                        // ещё на стеке
v.push_back(5);                        // превышен буфер → переезд в кучу
```

Также существует `small_vector_base<T>` — тип-стиратель размера буфера, удобно для передачи в функции без шаблонизации по `N`.

## 3. `flat_map` / `flat_set` (и multi-варианты)

Ассоциативные контейнеры, реализованные поверх **отсортированного непрерывного массива** (вместо дерева). Интерфейс совместим с `std::map`/`std::set`.

### Плюсы и минусы

|Преимущества|Недостатки|
|---|---|
|Cache-friendly (данные подряд)|Вставка/удаление O(n) — сдвиг элементов|
|Меньше памяти (нет узлов/указателей)|Инвалидация итераторов при вставке|
|Быстрый поиск и итерация|Дорогая модификация в больших контейнерах|

> Правило: `flat_map` выгоден, когда **много чтений/итераций и мало вставок**, либо контейнер заполняется один раз и потом только читается.

### Основные методы (как у std::map)

|Метод|Описание|
|---|---|
|`operator[](key)`|Доступ/вставка по ключу|
|`at(key)`|Доступ с проверкой (бросает `out_of_range`)|
|`insert(...)` / `emplace(...)`|Вставка|
|`find(key)`|Поиск → итератор|
|`count(key)` / `contains(key)`|Наличие ключа|
|`erase(...)`|Удаление|
|`lower_bound` / `upper_bound` / `equal_range`|Диапазонный поиск|
|`reserve(n)`|Зарезервировать ёмкость (важная оптимизация!)|

```cpp
#include <boost/container/flat_map.hpp>
namespace bc = boost::container;

bc::flat_map<int, std::string> m;
m.reserve(100);              // избегаем повторных перевыделений
m[3] = "three";
m[1] = "one";
m[2] = "two";
for (auto& [k, v] : m)       // итерация в отсортированном порядке, последовательно в памяти
    std::cout << k << "=" << v << "\n";
```

## 4. `stable_vector`

Vector-подобный контейнер, гарантирующий, что **ссылки, указатели и итераторы на элементы остаются валидными** при вставках/удалениях (кроме удаления самого элемента).

### Особенности

- Достигается ценой дополнительного уровня косвенности (медленнее и затратнее по памяти, чем `vector`).
- Произвольный доступ O(1), но с накладными расходами.
- Применять, когда нужны и индексация, и стабильность ссылок.

```cpp
#include <boost/container/stable_vector.hpp>
namespace bc = boost::container;

bc::stable_vector<int> sv{1, 2, 3};
int* p = &sv[1];     // указатель на элемент
sv.insert(sv.begin(), 0);
// p всё ещё валиден! (у std::vector был бы инвалидирован)
```

## 5. `devector<T>`

«Double-ended vector»: непрерывный буфер со свободным местом с **обоих** концов — эффективные `push_front` и `push_back` без структуры блоков, как у `deque`.

| Метод                                | Описание                                     |
| ------------------------------------ | -------------------------------------------- |
| `push_front()` / `push_back()`       | Вставка с обоих концов, амортизированно O(1) |
| `emplace_front()` / `emplace_back()` | Конструирование на месте                     |
| `reserve_front()` / `reserve_back()` | Резервирование с конкретной стороны          |
| `operator[]`, `front()`, `back()`    | Доступ                                       |

> Отличие от `deque`: данные **непрерывны** в памяти (можно получить указатель на сплошной блок), а `deque` хранит сегментами.

## 6. Прочие контейнеры

- `boost::container::vector` — как `std::vector`, но с поддержкой incomplete-типов, расширенными аллокаторами и др.
- `boost::container::string` — аналог `std::string`.
- `boost::container::deque` / `list` / `slist` (односвязный список) / `map` / `set` — аналоги стандартных с дополнительными возможностями (например, работа с неполными типами для рекурсивных структур).

## Сквозная тема: расширенные аллокаторы

Сильная сторона Boost.Container — продвинутая модель аллокаторов (совместима с Boost.Interprocess для размещения контейнеров в разделяемой памяти). Поддерживаются stateful-аллокаторы и аллокаторы для shared memory — то, что часто проблематично со стандартными контейнерами.

```cpp
// Контейнеры могут хранить incomplete types — удобно для рекурсивных структур:
struct Tree {
    int value;
    boost::container::vector<Tree> children; // std::vector тут технически UB до C++17
};
```

## Как выбрать контейнер

|Сценарий|Контейнер|
|---|---|
|Мало элементов, аллокации нежелательны|`small_vector<T, N>`|
|Жёсткий лимит, без кучи (embedded/realtime)|`static_vector<T, N>`|
|Map/set с упором на чтение и итерацию|`flat_map` / `flat_set`|
|Нужны стабильные ссылки + индексация|`stable_vector`|
|Частые вставки с обоих концов, но непрерывность|`devector`|
|Контейнер в разделяемой памяти|любой + Boost.Interprocess-аллокатор|
|Рекурсивные структуры (incomplete types)|`boost::container::vector` и др.|

## Отличия от стандартных контейнеров

- `static_vector`, `small_vector`, `flat_map`/`flat_set`, `stable_vector`, `devector` — **в `std` прямых аналогов в основном нет** (это главная причина брать Boost.Container).
- Исключение: `std::flat_map` / `std::flat_set` появились **в C++23**, но boost-версии старше, шире по возможностям и доступны на старых компиляторах.
- Стандартные аналоги (`vector`, `deque`, `list`, `map`, `set`, `string`) дублируются ради расширенных аллокаторов, поддержки incomplete types и совместимости с Interprocess.
- `small_vector` концептуально близок к `llvm::SmallVector`; в стандарте такого нет.

### include/test_container.h
```cpp
#pragma once  
  
namespace test_container {  
    void test();  
}
```

### src//test_container.cpp
```cpp
#include "test_container.h"  
  
#include <iostream>  
#include <format>  
#include <boost/container/static_vector.hpp>  
#include <boost/container/small_vector.hpp>  
#include <boost/container/flat_map.hpp>  
#include <boost/container/stable_vector.hpp>  
  
namespace test_container {  
  
namespace bc = boost::container;  
  
static void test_static_vector() {  
    bc::static_vector<int, 3> sv;  
    sv.push_back(1);  
    sv.push_back(2);  
    sv.push_back(3);  
  
    std::string delimiter{};  
    std::cout << "[boost::container::static_vector] {";  
    for (auto item: sv) {  
        std::cout << delimiter << item;  
        delimiter = ", ";  
    }    std::cout << "}\n";  
}  
  
static void test_small_vector() {  
    bc::small_vector<int, 2> sv{1, 2, 3};  
    sv.push_back(1);  
    sv.push_back(2);  
  
    std::string delimiter{};  
    std::cout << "[boost::container::small_vector] {";  
    for (auto item: sv) {  
        std::cout << delimiter << item;  
        delimiter = ", ";  
    }    std::cout << "}\n";  
}  
  
static void test_flat() {  
    bc::flat_map<int, std::string> flatm;  
    flatm.reserve(100);  
    flatm[3] = "three";  
    flatm[1] = "one";  
    flatm[2] = "two";  
  
    std::string delimiter{};  
    std::cout << "[boost::container::flat_map] {";  
    for (auto [k, v]: flatm) {  
        std::cout << std::format("{}[{}, {}]", delimiter, k, v);  
        delimiter = ", ";  
    }    std::cout << "}\n";  
}  
  
static void test_stable_vector() {  
    bc::stable_vector<int> sv{1, 2, 3};  
    int* p = &sv[1];  
    std::cout << std::format("[boost::container::stable_vector] *p = {}\n", *p);  
  
    sv.insert(sv.begin(), 42);  
    std::cout << std::format("[boost::container::stable_vector] *p = {}\n", *p);  
}  

void test() {  
    test_static_vector();  
    test_small_vector();  
    test_flat();  
    test_stable_vector();  
}  
  
}
```

```
[boost::container::static_vector] {1, 2, 3}
[boost::container::small_vector] {1, 2, 3, 1, 2}
[boost::container::flat_map] {[1, one], [2, two], [3, three]}
[boost::container::stable_vector] *p = 2
[boost::container::stable_vector] *p = 2
```
