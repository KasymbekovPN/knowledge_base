[[raw data/cpp/interview/_|<=]]

# Variadic templates и fold expressions

## Variadic templates — шаблоны с переменным числом параметров

**Parameter pack** — «пачка» из нуля или более шаблонных аргументов.

```cpp
template<class... Args>        // Args — template parameter pack (типы)
void f(Args... args);          // args — function parameter pack (значения)

f();               // Args = {}, пусто
f(1);              // Args = {int}
f(1, 2.0, "s");    // Args = {int, double, const char*}
```

Обозначения:

- `class... Args` — **объявление** пачки
- `Args... args` — пачка параметров функции
- `args...` — **раскрытие** (pack expansion)
- `sizeof...(Args)` — количество элементов (compile-time)

```cpp
template<class... Args>
void count(Args... args) {
    std::cout << sizeof...(Args);   // число типов
    std::cout << sizeof...(args);   // то же самое
}
```

---

## Раскрытие пачки (pack expansion)

`pattern...` разворачивается в список, где `pattern` повторяется для каждого элемента пачки, **разделяясь запятыми**.

```cpp
template<class... Args>
void forward_all(Args&&... args) {
    inner(std::forward<Args>(args)...);
    //    ^^^^^^^^^^^^^^^^^^^^^^^^^^ разворачивается в:
    //    inner(std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3))
}
```

Паттерн может быть сложным — `...` применяется ко **всему выражению слева**:

```cpp
template<class... Args>
void f(Args... args) {
    g(args...);                    // g(a1, a2, a3)
    g(args + 1 ...);               // g(a1+1, a2+1, a3+1)
    g(h(args)...);                 // g(h(a1), h(a2), h(a3))
    g(std::forward<Args>(args)...);// g(fwd<A1>(a1), fwd<A2>(a2), ...)
    
    // раскрытие ТИПОВ:
    std::tuple<Args...> t{args...};          // tuple<int, double, string>
    std::tuple<std::vector<Args>...> vt;     // tuple<vector<int>, vector<double>, ...>
}
```

---

## Классический подход (до C++17): рекурсия

Пока не было fold expressions, пачку обрабатывали **рекурсивно**: отделяем первый элемент, рекурсивно обрабатываем хвост.

```cpp
// базовый случай — рекурсия останавливается
void print() { std::cout << "\n"; }

// рекурсивный случай
template<class T, class... Rest>
void print(T first, Rest... rest) {
    std::cout << first << " ";
    print(rest...);   // ← рекурсия с оставшейся пачкой
}

print(1, 2.5, "hi");
// print(1, 2.5, "hi")  → печатает 1, зовёт print(2.5, "hi")
// print(2.5, "hi")     → печатает 2.5, зовёт print("hi")
// print("hi")          → печатает hi, зовёт print()
// print()              → базовый случай, перевод строки
```

Важно: рекурсия здесь **compile-time** — компилятор генерирует отдельную функцию на каждый шаг. Никакого рантайм-оверхеда, но растёт время компиляции и объём кода.

С C++17 базовый случай часто заменяют на `if constexpr`:

```cpp
template<class T, class... Rest>
void print(T first, Rest... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0) {   // ✅ отдельная перегрузка не нужна
        std::cout << " ";
        print(rest...);
    } else {
        std::cout << "\n";
    }
}
```

---

## Fold expressions (C++17) — свёртка пачки

Позволяют применить **бинарный оператор** ко всей пачке **без рекурсии**. Резко упрощает код.

### Четыре формы

```cpp
(pack op ...)              // унарная ПРАВАЯ свёртка:  a1 op (a2 op (a3 op a4))
(... op pack)              // унарная ЛЕВАЯ свёртка:   ((a1 op a2) op a3) op a4
(pack op ... op init)      // бинарная ПРАВАЯ:         a1 op (a2 op (a3 op init))
(init op ... op pack)      // бинарная ЛЕВАЯ:          ((init op a1) op a2) op a3
```

Мнемоника: **`...` слева от pack → левая свёртка; справа → правая.** Скобки **обязательны**.

### Примеры

```cpp
template<class... Args>
auto sum(Args... args) {
    return (args + ...);           // унарная правая: a1 + (a2 + (a3 + ...))
}
sum(1, 2, 3);   // 6

template<class... Args>
auto sum0(Args... args) {
    return (0 + ... + args);       // бинарная левая, с начальным значением
}
sum0();         // ✅ 0 — работает для ПУСТОЙ пачки!
```

**Ключевое различие:** унарная свёртка **не компилируется для пустой пачки** (кроме трёх особых операторов), бинарная — работает всегда, начальное значение служит нейтральным элементом.

```cpp
sum();    // ❌ ошибка: пустая пачка в унарной свёртке для +
sum0();   // ✅ 0
```

**Исключения** (унарная свёртка пустой пачки допустима):

- `&&` → `true`
- `||` → `false`
- `,` → `void()`

```cpp
template<class... Args>
bool all(Args... args) { return (args && ...); }
all();   // ✅ true (нейтральный элемент для &&)
```

### Левая vs правая — когда важно

Для ассоциативных операций (`+` на числах) — не важно. Для неассоциативных — критично:

```cpp
template<class... Args>
auto sub_left(Args... args)  { return (... - args); }   // ((a1 - a2) - a3)
template<class... Args>
auto sub_right(Args... args) { return (args - ...); }   // (a1 - (a2 - a3))

sub_left(10, 3, 2);    // (10-3)-2 = 5
sub_right(10, 3, 2);   // 10-(3-2) = 9
```

**Практическое правило: по умолчанию используй ЛЕВУЮ свёртку** — она соответствует естественной ассоциативности большинства операторов (`<<`, `-`, `/`).

Пример, где это критично — `operator<<`:

```cpp
template<class... Args>
void print(Args... args) {
    (std::cout << ... << args);   // ✅ ЛЕВАЯ: ((cout << a1) << a2) << a3 — правильно
    // (std::cout << args << ...); // ❌ правая — не соберётся/неверно
}
```

---

## Практические паттерны

### 1. Печать с разделителем

```cpp
template<class... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);   // comma-свёртка: (cout<<a1<<" "), (cout<<a2<<" "), ...
    std::cout << "\n";
}
print(1, 2.5, "hi");   // "1 2.5 hi "
```

Comma-свёртка — универсальный приём: **выполнить действие для каждого элемента**.

Без завершающего пробела:

```cpp
template<class First, class... Rest>
void print(First first, Rest... rest) {
    std::cout << first;
    ((std::cout << ", " << rest), ...);   // разделитель перед каждым, кроме первого
    std::cout << "\n";
}
```

### 2. Вызвать функцию для каждого

```cpp
template<class... Args>
void call_all(Args&&... args) {
    (process(std::forward<Args>(args)), ...);   // process(a1), process(a2), ...
}
```

### 3. Push всех в контейнер

```cpp
template<class Container, class... Args>
void push_all(Container& c, Args&&... args) {
    (c.push_back(std::forward<Args>(args)), ...);
}

std::vector<int> v;
push_all(v, 1, 2, 3);   // v == {1,2,3}
```

### 4. Проверки над всеми типами

```cpp
template<class... Args>
constexpr bool all_integral() {
    return (std::is_integral_v<Args> && ...);   // конъюнкция по всем типам
}

static_assert(all_integral<int, long, char>());   // true
static_assert(!all_integral<int, double>());      // false
```

### 5. Perfect forwarding в фабрике (то самое, что мы разбирали)

```cpp
template<class T, class... Args>
std::unique_ptr<T> make(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

Так реализованы `make_unique`, `make_shared`, `emplace_back`, `std::make_tuple`.

### 6. Подсчёт совпадений

```cpp
template<class T, class... Args>
constexpr size_t count_type() {
    return (0 + ... + (std::is_same_v<T, Args> ? 1 : 0));
}

count_type<int, int, double, int>();   // 2
```

---

## Тонкости и ловушки

### 1. Порядок вычисления — гарантирован только у `,` и логических

Fold expression **не** даёт гарантии порядка вычисления для произвольных операторов... кроме тех, у которых порядок определён самим языком: `,`, `&&`, `||`.

```cpp
(f(args) + ...);      // ⚠️ порядок вызовов f() не определён!
(f(args), ...);       // ✅ comma — гарантированно слева направо
```

Поэтому для **действий с побочными эффектами** используют comma-свёртку.

### 2. Short-circuit в `&&`/`||` fold

```cpp
template<class... Args>
bool check_all(Args... args) {
    return (validate(args) && ...);   // ⚠️ ЛЕНИВО: при первом false остальные НЕ вызовутся
}
```

Обычно это плюс, но если нужны все побочные эффекты — используй comma или явное накопление.

### 3. Скобки обязательны

```cpp
return args + ...;      // ❌ ошибка компиляции
return (args + ...);    // ✅
```

### 4. Расширение пачки в списке инициализации (старый трюк)

До C++17 порядок гарантировали через `std::initializer_list` (у него порядок определён):

```cpp
template<class... Args>
void call_all(Args... args) {
    (void)std::initializer_list<int>{ (process(args), 0)... };
    //                                 ^^^^^^^^^^^^^^^^ трюк: comma → результат 0
}
```

Уродливо, но встречается в pre-C++17 коде — стоит уметь читать. С C++17 это просто `(process(args), ...)`.

### 5. Пачка может быть только в конце (для вывода)

```cpp
template<class... Args, class Last>
void f(Args... args, Last last);   // ⚠️ Args не выведется — компилятор не знает, где граница
```

Обходят через `std::tuple` или рекурсию по первому элементу.

---

## Variadic class templates

```cpp
template<class... Types>
class Tuple;   // именно так объявлен std::tuple

// Рекурсивное наследование — классическая реализация tuple:
template<class... Types> struct Tuple;

template<> struct Tuple<> { };   // базовый случай: пустой

template<class Head, class... Tail>
struct Tuple<Head, Tail...> : Tuple<Tail...> {   // наследуемся от хвоста
    Head head_;
    Tuple(Head h, Tail... t) : Tuple<Tail...>(t...), head_(h) { }
};
```

Реальные реализации `std::tuple` устроены сложнее (множественное наследование от `_Head_base<I, T>` для лучшей раскладки и доступа по индексу), но идея та же.

Работа с `std::tuple`:

```cpp
std::tuple<int, double, std::string> t{1, 2.5, "hi"};

std::get<0>(t);                        // по индексу — 1
std::get<std::string>(t);              // по типу (C++14, если тип уникален) — "hi"
std::tuple_size_v<decltype(t)>;        // 3
std::apply([](auto... xs){ (std::cout << ... << xs); }, t);   // C++17: развернуть в вызов

auto [a, b, c] = t;                    // C++17: structured bindings
```

---

## Сравнение подходов

| |Рекурсия|Fold expression|
|---|---|---|
|Стандарт|C++11|C++17|
|Читаемость|средняя|**высокая**|
|Время компиляции|медленнее (N инстанциаций)|**быстрее** (одно выражение)|
|Гибкость|**выше** (произвольная логика на каждом шаге)|только бинарный оператор|
|Порядок вычисления|явный|гарантирован только для `,`/`&&`/`|

**Правило: fold expression — если задача сводится к бинарному оператору; рекурсия (или `if constexpr`) — если нужна нетривиальная логика на каждом элементе.**

---

## Формулировки на собеседовании

**«Что такое parameter pack?»** — Шаблонный параметр, принимающий **ноль или более** аргументов (`class... Args`). Раскрывается через `args...`; количество — `sizeof...(Args)`.

**«Как обработать пачку до C++17?»** — Рекурсией: перегрузка с базовым случаем + перегрузка `(T first, Rest... rest)`, рекурсивно вызывающая себя с хвостом. Компилятор генерирует по функции на каждый шаг (compile-time рекурсия, рантайм-оверхеда нет).

**«Что такое fold expression?»** — Свёртка пачки бинарным оператором без рекурсии: `(args + ...)`. Четыре формы (унарная/бинарная × левая/правая). Скобки обязательны.

**«Чем унарная свёртка отличается от бинарной?»** — Бинарная имеет начальное значение → **работает для пустой пачки**. Унарная для пустой пачки — ошибка компиляции, кроме `&&` (→true), `||` (→false), `,` (→void()).

**«Как выполнить действие для каждого элемента пачки?»** — Comma-свёртка: `(f(args), ...)`. Только у неё (и у `&&`/`||`) **гарантирован порядок** вычисления слева направо.

**«Где variadic templates применяются в стандартной библиотеке?»** — `std::tuple`, `make_unique`/`make_shared`, `emplace_back`, `std::thread`, `std::bind`, `std::variant`, `std::function`, `printf`-подобные type-safe обёртки.

**«Почему левая свёртка предпочтительнее?»** — Соответствует естественной ассоциативности большинства операторов. Для `operator<<` правая свёртка вообще не работает корректно.

---

Отличие от Java: там **varargs** (`void f(int... args)`) — это просто **синтаксический сахар над массивом**: `args` имеет тип `int[]`, все элементы **одного** типа, разрешается в рантайме. C++ variadic templates — **гетерогенны** (разные типы в одной пачке), **compile-time** (типы известны, код генерируется для каждой комбинации), поддерживают perfect forwarding и раскрытие в типовые конструкции (`tuple<Args...>`). Java-аналог невозможен в принципе из-за type erasure. Это ещё один пример, где C++ переносит работу на этап компиляции — и получает как выразительность, так и нулевой рантайм-оверхед.
