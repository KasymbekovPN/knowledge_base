[[raw data/cpp/interview/_|<=]]

# Ranges (C++20)

Мы касались их в теме алгоритмов — теперь подробно.

## Проблема, которую решают

Классические STL-алгоритмы работают с **парой итераторов**. Это громоздко и не композируемо:

```cpp
std::vector<int> v{1,2,3,4,5,6};

// 1. Многословно
std::sort(v.begin(), v.end());

// 2. Нельзя составлять цепочки — нужны промежуточные контейнеры
std::vector<int> evens;
std::copy_if(v.begin(), v.end(), std::back_inserter(evens),
             [](int x){ return x % 2 == 0; });   // ⚠️ аллокация
std::vector<int> squares;
std::transform(evens.begin(), evens.end(), std::back_inserter(squares),
               [](int x){ return x * x; });      // ⚠️ ещё аллокация

// 3. Легко ошибиться
std::sort(v.begin(), w.end());   // ⚠️ итераторы разных контейнеров — UB, компилятор молчит
```

Ranges решают всё три проблемы.

---

## Range — концепт

**Range** — всё, у чего есть `begin()` и `end()`. Формализовано через concepts (`<ranges>`):

```cpp
std::ranges::range              // есть begin/end
std::ranges::sized_range        // + O(1) size()
std::ranges::input_range        // итератор — input
std::ranges::forward_range
std::ranges::bidirectional_range
std::ranges::random_access_range
std::ranges::contiguous_range   // элементы непрерывно в памяти
std::ranges::viewable_range     // можно превратить в view
```

Любой STL-контейнер, массив, `std::span` — это range.

---

## Range-алгоритмы: контейнер вместо пары итераторов

```cpp
#include <algorithm>
namespace rg = std::ranges;

std::vector<int> v{3, 1, 2};

rg::sort(v);                        // ✅ вместо std::sort(v.begin(), v.end())
auto it = rg::find(v, 2);
rg::for_each(v, [](int x){ /*...*/ });
bool ok = rg::all_of(v, [](int x){ return x > 0; });
rg::reverse(v);
int n = rg::count(v, 2);
```

**Плюс: проекции** — извлечь поле перед применением предиката/компаратора:

```cpp
struct Person { std::string name; int age; };
std::vector<Person> people;

rg::sort(people, {}, &Person::age);            // ✅ сортировка ПО ВОЗРАСТУ
//              ^^  ^^^^^^^^^^^^^ проекция
//              компаратор по умолчанию (less)

rg::sort(people, std::greater{}, &Person::age);   // по убыванию возраста

auto it = rg::find(people, "Alice", &Person::name);   // ✅ поиск по имени
```

Раньше приходилось писать лямбду:

```cpp
std::sort(people.begin(), people.end(),
          [](const auto& a, const auto& b){ return a.age < b.age; });   // ❌ многословно
```

Проекция — крайне удобная фича, которую часто недооценивают.

---

## Views — ленивые адаптеры

**View** — легковесный range (O(1) копирование, **не владеет** данными), вычисляемый **лениво**.

```cpp
namespace rv = std::views;

std::vector<int> v{1,2,3,4,5,6};

auto result = v | rv::filter([](int x){ return x % 2 == 0; })
                | rv::transform([](int x){ return x * x; });

// ⚠️ ЗДЕСЬ ЕЩЁ НИЧЕГО НЕ ВЫЧИСЛЕНО!

for (int x : result) std::cout << x;   // ✅ 4 16 36 — вычисляется ПРИ ОБХОДЕ
```

**Ключевые свойства:**

- **Лениво** — элементы вычисляются при обходе, а не заранее
- **Без аллокаций** — нет промежуточных контейнеров
- **Композируемо** — через `operator|` (pipe)
- **Не владеет** — view ссылается на исходный контейнер

Механика: `result` — объект типа вроде `transform_view<filter_view<ref_view<vector<int>>, Lambda1>, Lambda2>`. Обход дёргает цепочку итераторов. Компилятор всё инлайнит → по производительности эквивалентно ручному циклу.

---

## Основные views

### Фильтрация и преобразование

```cpp
rv::filter(pred)       // оставить элементы, удовлетворяющие предикату
rv::transform(func)    // применить функцию к каждому
```

### Взятие/пропуск

```cpp
rv::take(n)            // первые n
rv::take_while(pred)   // пока предикат true
rv::drop(n)            // пропустить первые n
rv::drop_while(pred)   // пропустить, пока предикат true
```

```cpp
auto first3 = v | rv::take(3);                      // 1 2 3
auto rest = v | rv::drop(3);                        // 4 5 6
auto until5 = v | rv::take_while([](int x){ return x < 5; });   // 1 2 3 4
```

### Порядок и структура

```cpp
rv::reverse            // обратный порядок
rv::join               // «расплющить» range of ranges
rv::split(delim)       // разбить по разделителю
rv::elements<N>        // N-й элемент из каждого tuple-like
rv::keys               // = elements<0> — ключи map
rv::values             // = elements<1> — значения map
rv::common             // привести к common_range (begin/end одного типа — для старых алгоритмов)
```

```cpp
std::map<int, std::string> m{{1,"a"}, {2,"b"}};
for (auto& k : m | rv::keys) { }        // ✅ 1, 2
for (auto& val : m | rv::values) { }    // ✅ "a", "b"

std::vector<std::vector<int>> nested{{1,2},{3,4}};
for (int x : nested | rv::join) { }     // ✅ 1 2 3 4 — расплющено
```

### Генераторы

```cpp
rv::iota(start)        // бесконечная последовательность start, start+1, ...
rv::iota(start, end)   // конечная [start, end)
rv::single(value)      // range из одного элемента
rv::empty<T>           // пустой
rv::repeat(v)          // C++23: бесконечное повторение
```

**Бесконечные range работают** — благодаря ленивости:

```cpp
auto first10squares = rv::iota(1)                        // 1, 2, 3, ... ∞
                    | rv::transform([](int x){ return x*x; })
                    | rv::take(10);                       // ✅ обрезали

for (int x : first10squares) std::cout << x << " ";   // 1 4 9 16 25 36 49 64 81 100
```

Без ленивости это было бы невозможно (бесконечный контейнер не создать).

### C++23

```cpp
rv::zip(r1, r2)        // пары из двух ranges
rv::enumerate          // (индекс, значение)
rv::chunk(n)           // разбить на куски по n
rv::slide(n)           // скользящее окно
rv::adjacent<N>        // соседние N элементов
rv::cartesian_product(r1, r2)
rv::stride(n)          // каждый n-й
rv::chunk_by(pred)     // группировка по предикату
```

```cpp
// C++23:
for (auto [i, x] : v | rv::enumerate) {   // ✅ индекс + значение
    std::cout << i << ": " << x << "\n";
}

std::vector<int> a{1,2,3}, b{10,20,30};
for (auto [x, y] : rv::zip(a, b)) {       // ✅ (1,10), (2,20), (3,30)
    std::cout << x + y;
}
```

`enumerate` и `zip` — то, чего долго не хватало (в Python это встроено).

---

## Материализация: view → контейнер

View ленив и не владеет. Чтобы получить контейнер:

**C++23 — `ranges::to`:**

```cpp
auto vec = v | rv::filter(pred)
             | rv::transform(func)
             | rg::to<std::vector>();      // ✅ материализуем
```

**C++20 — вручную:**

```cpp
auto view = v | rv::filter(pred);
std::vector<int> vec(view.begin(), view.end());   // ✅ (если view — common_range)
// или:
std::vector<int> vec;
rg::copy(view, std::back_inserter(vec));
```

Отсутствие `ranges::to` в C++20 — известная недоработка, исправленная в C++23.

---

## Ловушка: dangling views

View **не владеет** данными → не должен переживать источник.

```cpp
auto bad() {
    std::vector<int> v{1,2,3};
    return v | rv::filter(pred);   // ⚠️ v умрёт → view повиснет!
}

// Защита в стандарте: временные range не «прилипают»:
auto view = getVector() | rv::filter(pred);   // ⚠️ getVector() — временный
// Стандарт частично защищает: некоторые операции с rvalue-range дают
// std::ranges::dangling вместо итератора
auto it = rg::find(getVector(), 5);   // it имеет тип std::ranges::dangling
// *it;   // ❌ ОШИБКА КОМПИЛЯЦИИ — вместо UB ✅
```

`std::ranges::dangling` — механизм, превращающий рантайм-UB в ошибку компиляции. Но полной защиты нет:

```cpp
auto view = std::vector<int>{1,2,3} | rv::filter(pred);   // ⚠️ UB, компилятор не поймает всё
```

**Правило: view — временный объект в выражении, не храни его дольше источника.**

Исключение — **owning_view** (C++20): если передать rvalue-контейнер в пайплайн, view может его **забрать** во владение:

```cpp
auto view = std::vector<int>{1,2,3} | rv::transform(f);   // owning_view — владеет вектором
```

Но это работает не для всех адаптеров, поэтому надёжнее не полагаться.

---

## Ловушка: `filter_view` — не всегда O(1) begin

```cpp
auto view = v | rv::filter(expensive_pred);
view.begin();   // ⚠️ ищет ПЕРВЫЙ подходящий элемент — может быть O(n)
```

`filter_view::begin()` кеширует результат, но:

```cpp
for (auto x : v | rv::filter(pred)) { }   // ✅ begin() один раз
```

Плюс `filter_view` **не** является `const`-итерируемым в C++20 (нужно кешировать begin) — иногда всплывает как неожиданная ошибка компиляции.

---

## Производительность

**Теория:** views компилируются в тот же код, что ручной цикл — всё инлайнится, ноль аллокаций.

**Практика:** компиляторы обычно справляются, но:

- Длинные цепочки могут не полностью оптимизироваться (особенно на -O1)
- Время **компиляции** растёт заметно (глубокая шаблонная вложенность)
- Сообщения об ошибках всё ещё тяжёлые, хотя concepts помогли

Сравнение с ручным циклом:

```cpp
// Ranges
auto sum = 0;
for (int x : v | rv::filter(isEven) | rv::transform(square))
    sum += x;

// Ручной цикл — обычно генерирует ИДЕНТИЧНЫЙ ассемблер
auto sum = 0;
for (int x : v)
    if (isEven(x)) sum += square(x);
```

На практике разница чаще в пользу читаемости, чем против производительности.

---

## Сравнение с STL-алгоритмами

| |Классические алгоритмы|Ranges|
|---|---|---|
|Синтаксис|`sort(v.begin(), v.end())`|`rg::sort(v)`|
|Композиция|промежуточные контейнеры|`\|` без аллокаций|
|Ленивость|❌|✅|
|Проекции|❌ (лямбда)|✅ `rg::sort(v, {}, &P::age)`|
|Бесконечные последовательности|❌|✅|
|Проверка типов|шаблонная простыня|**concepts** — внятные ошибки|
|Совместимость с итераторами|нативно|нужен `rv::common` для старых API|
|Время компиляции|быстрее|медленнее|

---

## Практические примеры

```cpp
namespace rv = std::views;
namespace rg = std::ranges;

// Топ-3 самых старых
auto oldest = people
    | rv::transform(&Person::age)
    | rg::to<std::vector>();          // C++23
rg::partial_sort(oldest, oldest.begin() + 3, std::greater{});

// Слова из строки (C++20)
std::string text = "hello world foo";
for (auto word : text | rv::split(' ')) {
    std::string s(word.begin(), word.end());   // C++20: неудобно
    // C++23: auto s = word | rg::to<std::string>();
}

// Чётные квадраты первых 100 чисел
auto result = rv::iota(1, 101)
            | rv::filter([](int x){ return x % 2 == 0; })
            | rv::transform([](int x){ return x * x; });

// Обход map только по значениям
for (auto& v : myMap | rv::values) { }

// Скользящее окно (C++23)
for (auto window : v | rv::slide(3)) { }
```

---

## Формулировки на собеседовании

**«Что такое range?»** — Концепт: всё, у чего есть `begin()` и `end()`. Range-алгоритмы принимают контейнер целиком вместо пары итераторов.

**«Что такое view?»** — Лёгкий range (O(1) копирование), **не владеющий** данными, вычисляемый **лениво**. Композируется через `|` без промежуточных аллокаций.

**«В чём главное преимущество ranges?»** — (1) Композиция без промежуточных контейнеров (ленивость). (2) Читаемость. (3) Проекции. (4) Concepts → внятные ошибки. (5) Невозможно перепутать итераторы разных контейнеров.

**«Почему `v | filter | transform` не аллоцирует?»** — Views ленивы: они хранят только ссылку на источник и функцию; элементы вычисляются при обходе. Компилятор инлайнит цепочку в эквивалент ручного цикла.

**«Какая главная опасность views?»** — **Dangling**: view не владеет данными и не должен переживать источник. Стандарт частично защищает через `std::ranges::dangling` (ошибка компиляции вместо UB), но не полностью.

**«Что такое проекция?»** — Функция, извлекающая значение из элемента перед применением предиката/компаратора: `rg::sort(people, {}, &Person::age)` — сортировка по полю без лямбды.

---

Отличие от Java: Stream API — **ближайший аналог** и тоже ленивый (`stream().filter().map().collect()`). Различия: (1) Java Stream **одноразовый** (после terminal operation нельзя переиспользовать), C++ view можно обходить многократно; (2) Java боксит примитивы (`Stream<Integer>` вместо `IntStream` → аллокации), C++ views работают с типами напрямую, **ноль аллокаций**; (3) Java Stream — рантайм-абстракция с виртуальными вызовами, C++ views — compile-time шаблонная композиция, инлайнится в ручной цикл; (4) Java имеет `.parallel()` из коробки, в C++ параллельные ranges только в C++26/сторонних библиотеках. Итог: C++ ranges — то же удобство, но с нулевым рантайм-оверхедом; цена — время компиляции и всё ещё непростые ошибки. Классическая для C++ сделка: zero-cost abstraction вместо рантайм-механизма.
