[[raw data/cpp/interview/_|<=]]

# SFINAE, `enable_if`, concepts

## SFINAE — Substitution Failure Is Not An Error

**Правило:** если при подстановке шаблонных аргументов возникает ошибка **в сигнатуре** функции, это не ошибка компиляции — кандидат просто **выбывает** из разрешения перегрузок. Компилятор пробует следующего.

```cpp
template<class T>
typename T::value_type get(T t) { return t.value; }   // (1) требует T::value_type

template<class T>
T get(T t) { return t; }                              // (2) fallback

get(std::vector<int>{});   // (1) подходит (vector::value_type есть)
get(42);                    // (1): int::value_type не существует → подстановка ПРОВАЛИЛАСЬ
                            //      → НЕ ошибка, (1) выбывает → выбирается (2) ✅
```

Без SFINAE вторая строка была бы ошибкой компиляции. С SFINAE — «этот кандидат не подходит, идём дальше».

### Ключевое ограничение: только в «immediate context»

SFINAE работает **только для ошибок в сигнатуре** (параметры, возвращаемый тип, шаблонные параметры). Ошибка **внутри тела** функции — это **hard error**, компиляция падает:

```cpp
template<class T>
void f(T t) {
    t.nonexistent();   // ⚠️ HARD ERROR при инстанцировании — SFINAE НЕ спасёт
}
```

Отсюда весь «уродливый» синтаксис SFINAE: приходится тащить проверки **в сигнатуру**.

---

## `std::enable_if` — механизм SFINAE

Устроен тривиально — на частичной специализации (мы это только что разбирали):

```cpp
template<bool B, class T = void>
struct enable_if { };                       // общий случай: НЕТ члена type

template<class T>
struct enable_if<true, T> { using type = T; };   // специализация для true: type ЕСТЬ

// C++14 alias:
template<bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;
```

Логика: если условие `false` → `enable_if<false>::type` **не существует** → подстановка проваливается → кандидат выбывает (SFINAE). Если `true` → `type` есть → всё компилируется.

### Три места, куда его вставляют

**1. В шаблонный параметр (самый чистый):**

```cpp
template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T x) { std::cout << "integral\n"; }

template<class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
void process(T x) { std::cout << "floating\n"; }

process(42);     // "integral"
process(3.14);   // "floating"
process("hi");   // ❌ ошибка: ни один кандидат не подошёл
```

**2. В возвращаемый тип:**

```cpp
template<class T>
std::enable_if_t<std::is_integral_v<T>, void>
process(T x) { }
```

**3. В параметр функции (редко):**

```cpp
template<class T>
void process(T x, std::enable_if_t<std::is_integral_v<T>, int> = 0) { }
```

### Критично: разные `enable_if` в разных перегрузках — обязательны

Нельзя, чтобы условия **пересекались** — иначе ambiguous. И нельзя просто убрать `enable_if` из одной перегрузки — тогда обе будут кандидатами.

```cpp
// ❌ ОШИБКА: обе перегрузки имеют одинаковую сигнатуру после подстановки
template<class T>
std::enable_if_t<std::is_integral_v<T>> f(T);
template<class T>
std::enable_if_t<std::is_integral_v<T>> f(T);   // дубликат!
```

### Реальный пример — ограничение forwarding-конструктора

Помнишь проблему «жадного» конструктора из темы perfect forwarding?

```cpp
class Person {
    std::string name_;
public:
    template<class T,
             std::enable_if_t<!std::is_same_v<std::decay_t<T>, Person>, int> = 0>
    explicit Person(T&& name) : name_(std::forward<T>(name)) { }
    //             ^^^ отключаем конструктор, когда T — сам Person,
    //                 чтобы не перехватывать copy/move конструктор

    Person(const Person&) = default;
    Person(Person&&) = default;
};

Person a("Alice");   // ✅ forwarding-конструктор
Person b(a);         // ✅ copy-конструктор (forwarding отключён через enable_if)
```

`std::decay_t` убирает ссылки и cv-квалификаторы — нужен, потому что `T` может вывестись как `Person&`, `const Person&` и т.д.

---

## Detection idiom — проверка «есть ли у типа X»

Классический паттерн: определить, поддерживает ли тип какую-то операцию.

### Через `void_t` (C++17)

```cpp
template<class...> using void_t = void;   // всегда void, но проверяет валидность аргументов

// Проверка: есть ли у T метод size()?
template<class T, class = void>
struct has_size : std::false_type { };                      // общий: НЕТ

template<class T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type { };                                   // частичная: если size() валиден

static_assert(has_size<std::vector<int>>::value);   // true
static_assert(!has_size<int>::value);               // false
```

Механика: если `T().size()` невалидно → `decltype` проваливается → `void_t<...>` невалиден → частичная специализация не подходит (SFINAE) → выбирается общий шаблон (`false_type`).

`std::declval<T>()` — «представь, что у нас есть T» без создания объекта (работает даже без конструктора; используется только внутри `decltype`, никогда не вызывается).

### Через trailing return type

```cpp
template<class T>
auto has_size_impl(int) -> decltype(std::declval<T>().size(), std::true_type{});
//                        ^^^ comma operator: если size() невалиден → SFINAE

template<class T>
std::false_type has_size_impl(...);   // fallback (эллипсис — худший кандидат)

template<class T>
using has_size = decltype(has_size_impl<T>(0));   // 0 → int предпочтительнее ...
```

Трюк с `int` vs `...`: `int` — лучший кандидат, `...` — худший. Если первый выбывает по SFINAE, выбирается второй.

Оба паттерна работают, но синтаксис — тяжёлый, и **ошибки компиляции чудовищны**. Именно поэтому появились concepts.

---

## Tag dispatch — альтернатива SFINAE

Вместо отключения перегрузок — выбор по типу-тегу. Часто чище:

```cpp
template<class Iter>
void advance_impl(Iter& it, int n, std::random_access_iterator_tag) {
    it += n;   // O(1)
}

template<class Iter>
void advance_impl(Iter& it, int n, std::input_iterator_tag) {
    while (n--) ++it;   // O(n)
}

template<class Iter>
void advance(Iter& it, int n) {
    advance_impl(it, n, typename std::iterator_traits<Iter>::iterator_category{});
    //                  ^^^ перегрузка выбирается по ТЕГУ категории
}
```

Так реализован `std::advance` в стандартной библиотеке (мы упоминали это в теме итераторов).

Плюсы над SFINAE: понятнее, лучше ошибки, легко расширяется. Минус: нужна иерархия тегов.

---

## `if constexpr` (C++17) — часто убивает необходимость в SFINAE

Компилируется **только выбранная ветка**, отброшенная даже не проверяется:

```cpp
template<class T>
void process(T x) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "integral: " << x;
    } else if constexpr (std::is_pointer_v<T>) {
        std::cout << "pointer: " << *x;   // ✅ для int эта ветка НЕ компилируется
    } else {
        std::cout << "other";
    }
}
```

Сравни с SFINAE: три перегрузки с `enable_if` → одна функция с `if constexpr`. Радикально читаемее.

**Ограничение:** `if constexpr` работает **внутри** функции — он не может отключить саму перегрузку. Если нужно, чтобы функция **не участвовала** в разрешении перегрузок (как в примере с forwarding-конструктором), нужен SFINAE или concepts.

---

## Concepts (C++20) — правильное решение

Именованные, композируемые предикаты над типами. Заменяют SFINAE в подавляющем большинстве случаев.

### Определение

```cpp
template<class T>
concept Integral = std::is_integral_v<T>;

template<class T>
concept Numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;
```

### `requires`-выражение — проверка валидности операций

```cpp
template<class T>
concept HasSize = requires(T t) {
    t.size();                                  // выражение должно быть валидным
    { t.size() } -> std::convertible_to<size_t>;   // ...и возвращать конвертируемое в size_t
    typename T::value_type;                    // тип должен существовать
    { t.begin() } -> std::input_iterator;      // begin() возвращает итератор
};
```

Сравни с `void_t`-паттерном выше — та же проверка, но читается как обычный код.

### Четыре формы применения

```cpp
// 1. Вместо class/typename — самая короткая
template<Integral T>
void f(T x) { }

// 2. requires-клауза после template
template<class T>
requires Integral<T>
void f(T x) { }

// 3. requires-клауза после сигнатуры (можно использовать параметры!)
template<class T>
void f(T x) requires Integral<T> { }

// 4. Abbreviated function template (C++20) — самая лаконичная
void f(Integral auto x) { }
```

### Перегрузка по concepts — то, ради чего всё затевалось

```cpp
void process(Integral auto x)       { std::cout << "integral\n"; }
void process(std::floating_point auto x) { std::cout << "float\n"; }

process(42);     // "integral"
process(3.14);   // "float"
```

Сравни с `enable_if`-версией — небо и земля по читаемости.

### Subsumption — упорядочивание по «силе»

Компилятор понимает, какой concept **строже**, и выбирает более специализированную перегрузку:

```cpp
template<class T> concept Animal = requires(T t) { t.eat(); };
template<class T> concept Dog = Animal<T> && requires(T t) { t.bark(); };
//                              ^^^^^^^^^ Dog СИЛЬНЕЕ Animal

void handle(Animal auto a) { std::cout << "animal\n"; }
void handle(Dog auto d)    { std::cout << "dog\n"; }

handle(myDog);   // ✅ "dog" — Dog subsumes Animal, выбирается более специализированный
```

С `enable_if` это потребовало бы ручного разрешения неоднозначности (тегов приоритета и подобных костылей).

### Стандартные concepts (`<concepts>`)

```cpp
std::same_as<T, U>
std::derived_from<D, B>
std::convertible_to<From, To>
std::integral<T>, std::signed_integral<T>, std::floating_point<T>
std::equality_comparable<T>, std::totally_ordered<T>
std::copyable<T>, std::movable<T>, std::default_initializable<T>
std::invocable<F, Args...>, std::predicate<F, Args...>
// итераторные:
std::input_iterator, std::forward_iterator, std::random_access_iterator, std::contiguous_iterator
// диапазоны (<ranges>):
std::ranges::range, std::ranges::sized_range, std::ranges::random_access_range
```

### Главный практический выигрыш — сообщения об ошибках

```cpp
// SFINAE:
error: no matching function for call to 'process(const char*)'
note: candidate: template<class T, typename std::enable_if<std::is_integral<T>::value, int>::type <anonymous> > void process(T)
note:   template argument deduction/substitution failed:
note:   error: no type named 'type' in 'struct std::enable_if<false, int>'
[...ещё 200 строк...]

// Concepts:
error: no matching function for call to 'process(const char*)'
note: constraints not satisfied
note: the required expression 'std::is_integral_v<const char*>' evaluated to 'false'
```

Это, пожалуй, главная причина существования concepts — SFINAE технически работал, но был непригоден для нормальной разработки.

---

## Сводка: что когда использовать

|Задача|C++11/14/17|C++20|
|---|---|---|
|Ветвление внутри функции по типу|SFINAE / tag dispatch|**`if constexpr`**|
|Отключить перегрузку|**`enable_if`**|**concepts**|
|Проверить наличие метода/типа|`void_t` detection idiom|**`requires`-выражение**|
|Выбор по свойству итератора|tag dispatch|**concepts**|
|Ограничить forwarding-конструктор|`enable_if` + `decay_t`|**`requires`**|

**Практическое правило (C++20+): concepts вместо enable_if, `if constexpr` вместо SFINAE-ветвления.** SFINAE остаётся знать нужно — его полно в существующем коде и в реализации стандартной библиотеки.

---

## Формулировки на собеседовании

**«Что такое SFINAE?»** — Substitution Failure Is Not An Error: ошибка при подстановке шаблонных аргументов **в сигнатуру** не является ошибкой компиляции — кандидат выбывает из разрешения перегрузок. Работает только в immediate context (сигнатура), но не в теле функции.

**«Как устроен `enable_if`?»** — Шаблон класса с частичной специализацией: `enable_if<true, T>` имеет член `type`, `enable_if<false, T>` — не имеет. Обращение к несуществующему `::type` → провал подстановки → SFINAE → кандидат выбывает.

**«Почему SFINAE не работает для ошибок в теле функции?»** — Тело инстанцируется **после** разрешения перегрузок; ошибка там — hard error. SFINAE действует только на этапе подстановки в сигнатуру.

**«Чем concepts лучше SFINAE?»** — Читаемость (проверки выглядят как код), внятные ошибки компиляции, композируемость (`&&`, `||`), **subsumption** (компилятор упорядочивает перегрузки по «силе» ограничений), именованность и переиспользуемость.

**«Когда `if constexpr` не заменяет SFINAE?»** — Когда нужно, чтобы функция **не участвовала** в разрешении перегрузок (например, чтобы не перехватывать copy-конструктор). `if constexpr` ветвится **внутри** уже выбранной функции.

**«Что делает `std::declval`?»** — Позволяет «получить» значение типа `T` в невычисляемом контексте (`decltype`, `sizeof`), не создавая объект — работает даже для типов без конструктора по умолчанию. Вызывать его нельзя (нет определения).

---

Отличие от Java: там ограничения на generics — это `<T extends Comparable<T>>`, проверяемые компилятором, но **очень слабые**: только наследование/интерфейсы, никакой проверки «есть ли у типа метод `size()`» или «складываются ли два T». C++ concepts проверяют **произвольные выражения** — фактически структурную типизацию в compile-time («утиная типизация с проверкой»). Плюс Java-ограничения не влияют на генерацию кода (type erasure), тогда как в C++ constraints участвуют в **разрешении перегрузок** и выборе специализации. Это принципиально более выразительный механизм — и до C++20 его приходилось эмулировать через SFINAE, что и объясняет весь этот исторический уродливый синтаксис, который тебе всё равно придётся уметь читать в чужом коде.
