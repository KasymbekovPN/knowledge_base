[[raw data/cpp/interview/_|<=]]

# CRTP и type traits

## CRTP — Curiously Recurring Template Pattern

**Идея:** базовый класс параметризован своим **наследником**.

```cpp
template<class Derived>
class Base {
    // ...
};

class Derived : public Base<Derived> { };   // ← класс передаёт САМ СЕБЯ в базу
```

Выглядит парадоксально (класс использует себя до того, как определён), но работает: на момент инстанцирования `Base<Derived>` компилятору достаточно **имени** `Derived` — тело нужно только когда методы `Base` реально вызываются, а к тому моменту `Derived` уже полон.

---

## Зачем: статический полиморфизм

CRTP даёт полиморфизм **без vtable** — диспетчеризация разрешается на этапе компиляции.

```cpp
template<class Derived>
class Shape {
public:
    void draw() {
        static_cast<Derived*>(this)->drawImpl();   // ← «виртуальный» вызов БЕЗ virtual
    }
    double area() {
        return static_cast<Derived*>(this)->areaImpl();
    }
};

class Circle : public Shape<Circle> {
    double r_;
public:
    Circle(double r) : r_(r) { }
    void drawImpl() { std::cout << "circle\n"; }      // реализация
    double areaImpl() { return 3.14159 * r_ * r_; }
};

class Square : public Shape<Square> {
    double s_;
public:
    Square(double s) : s_(s) { }
    void drawImpl() { std::cout << "square\n"; }
    double areaImpl() { return s_ * s_; }
};

Circle c(1.0);
c.draw();   // → Circle::drawImpl — разрешено в COMPILE-TIME, можно ЗАИНЛАЙНИТЬ
```

Механика: `static_cast<Derived*>(this)` приводит `Base<Derived>*` к `Derived*`. Компилятор знает точный тип → прямой вызов → инлайнинг → ноль оверхеда.

### CRTP vs виртуальные функции

| |`virtual`|CRTP|
|---|---|---|
|Диспетчеризация|**рантайм** (vtable)|**compile-time**|
|Оверхед|vptr (8 байт/объект) + косвенный вызов|**ноль**|
|Инлайнинг|невозможен|**возможен**|
|Общий базовый тип|✅ `Base*` для всех|❌ `Shape<Circle>` и `Shape<Square>` — **разные типы**|
|Гетерогенный контейнер|✅ `vector<Base*>`|❌ невозможно|
|Рантайм-выбор типа|✅|❌ тип фиксирован в compile-time|

**Главное ограничение CRTP:** нет общего базового типа → **нельзя сложить разные наследники в один контейнер**:

```cpp
std::vector<Shape*> v;   // ❌ Shape — шаблон, не тип!
std::vector<Shape<Circle>*> v;   // только Circle, не Square
```

Если нужен гетерогенный контейнер или рантайм-полиморфизм — CRTP не подходит, нужен `virtual`.

**Когда CRTP уместен:** типы известны в compile-time, и виртуальный вызов в горячем пути слишком дорог (например, численные вычисления, где вызов в цикле должен инлайниться). Тебе как C++ разработчику это встретится в библиотеках вроде Eigen (expression templates на CRTP).

---

## Другие применения CRTP

### 1. Подмешивание функциональности (mixins)

База добавляет методы, используя интерфейс наследника:

```cpp
template<class Derived>
struct Comparable {
    bool operator!=(const Derived& o) const {
        return !static_cast<const Derived&>(*this).operator==(o);
    }
    bool operator>(const Derived& o) const {
        return o < static_cast<const Derived&>(*this);
    }
    bool operator<=(const Derived& o) const {
        return !(static_cast<const Derived&>(*this) > o);
    }
    bool operator>=(const Derived& o) const {
        return !(static_cast<const Derived&>(*this) < o);
    }
};

class Point : public Comparable<Point> {
    int x_, y_;
public:
    bool operator==(const Point& o) const { return x_ == o.x_ && y_ == o.y_; }
    bool operator<(const Point& o) const { return x_ < o.x_; }
    // остальные 4 оператора получены БЕСПЛАТНО от Comparable
};
```

Так работал `boost::operators`. С C++20 это делает `operator<=>` (spaceship), но паттерн полезно знать.

### 2. Счётчик объектов

```cpp
template<class Derived>
class Counter {
    static inline size_t count_ = 0;   // ✅ у КАЖДОГО Derived СВОЙ счётчик!
public:
    Counter() { ++count_; }
    Counter(const Counter&) { ++count_; }
    ~Counter() { --count_; }
    static size_t count() { return count_; }
};

class Widget : public Counter<Widget> { };
class Gadget : public Counter<Gadget> { };

Widget w1, w2;
Gadget g1;
Widget::count();   // 2  — отдельный счётчик
Gadget::count();   // 1  — независимый
```

Ключевое: `Counter<Widget>` и `Counter<Gadget>` — **разные инстанциации**, у каждой свой `count_`. С обычным (не-шаблонным) базовым классом счётчик был бы **один на всех**.

### 3. `enable_shared_from_this` — CRTP из стандартной библиотеки!

```cpp
class Session : public std::enable_shared_from_this<Session> { };
//                                                  ^^^^^^^ CRTP
```

Мы это разбирали. База должна знать тип наследника, чтобы `shared_from_this()` вернул `shared_ptr<Session>`, а не `shared_ptr<Base>`.

### 4. Полиморфное клонирование без дублирования

```cpp
struct Shape {
    virtual std::unique_ptr<Shape> clone() const = 0;
    virtual ~Shape() = default;
};

template<class Derived, class Base = Shape>
struct Cloneable : Base {
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
        //                               ^^^ копируем ПОЛНЫЙ Derived
    }
};

struct Circle : Cloneable<Circle> { /* clone() получен автоматически */ };
struct Square : Cloneable<Square> { /* тоже */ };
```

Гибрид: `virtual` (для гетерогенного контейнера) + CRTP (чтобы не писать `clone()` в каждом наследнике).

---

## Подводные камни CRTP

### 1. Нет проверки, что Derived действительно наследник

```cpp
class Circle : public Shape<Square> { };   // ⚠️ ОПЕЧАТКА! компилируется,
                                            //    но static_cast<Square*>(this) — UB
```

Защита — `static_assert` (но только в теле метода, не в теле класса — Derived ещё неполон):

```cpp
template<class Derived>
class Shape {
    void draw() {
        static_assert(std::is_base_of_v<Shape, Derived>, "CRTP misuse");
        static_cast<Derived*>(this)->drawImpl();
    }
};
```

Более надёжный приём — приватный конструктор + friend:

```cpp
template<class Derived>
class Shape {
    Shape() = default;               // приватный
    friend Derived;                  // ✅ только Derived может сконструировать базу
public:
    void draw() { static_cast<Derived*>(this)->drawImpl(); }
};
```

Теперь `class Circle : public Shape<Square>` не скомпилируется — `Circle` не friend для `Shape<Square>`.

### 2. Derived неполон внутри тела базы

```cpp
template<class Derived>
class Base {
    typename Derived::value_type x_;   // ❌ ОШИБКА: Derived ещё не определён!
};
```

На момент инстанцирования `Base<Derived>` (в списке наследования) `Derived` — **incomplete type**. Обращаться к его членам можно только **внутри тел методов** (они инстанцируются позже, по требованию).

Обходной путь — traits-класс:

```cpp
template<class T> struct Traits;

template<class Derived>
class Base {
    typename Traits<Derived>::value_type x_;   // ✅ Traits<Derived> специализирован ЗАРАНЕЕ
};

struct Circle;
template<> struct Traits<Circle> { using value_type = double; };
class Circle : public Base<Circle> { };
```

### 3. Code bloat

Каждая инстанциация `Shape<T>` — отдельный код. При множестве наследников бинарник растёт (обратная сторона мономорфизации).

---

## Type traits

Compile-time «рефлексия»: получить информацию о типе или преобразовать тип. Заголовок `<type_traits>`.

### Механика: всё построено на специализации

```cpp
// Пример: is_pointer
template<class T> struct is_pointer      : std::false_type { };
template<class T> struct is_pointer<T*>  : std::true_type  { };   // ← частичная специализация

// std::true_type / false_type — это:
template<class T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    constexpr operator T() const noexcept { return v; }   // неявное преобразование в bool
};
using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;
```

Это ровно тот механизм частичной специализации, который мы разбирали.

### Категории traits

**1. Предикаты типа (`::value` → bool)**

```cpp
std::is_integral_v<T>          // int, char, bool, long...
std::is_floating_point_v<T>
std::is_arithmetic_v<T>        // integral || floating_point
std::is_pointer_v<T>
std::is_reference_v<T>         // lvalue или rvalue ссылка
std::is_lvalue_reference_v<T>
std::is_const_v<T>
std::is_class_v<T>
std::is_enum_v<T>
std::is_array_v<T>
std::is_void_v<T>
std::is_polymorphic_v<T>       // есть виртуальные функции
std::is_abstract_v<T>          // есть pure virtual
std::is_final_v<T>
std::is_empty_v<T>             // нет нестатических членов (для EBO!)
std::is_trivially_copyable_v<T>   // можно memcpy
std::is_standard_layout_v<T>
```

**2. Отношения между типами**

```cpp
std::is_same_v<T, U>
std::is_base_of_v<Base, Derived>
std::is_convertible_v<From, To>
std::is_constructible_v<T, Args...>
std::is_assignable_v<To, From>
std::is_invocable_v<F, Args...>            // можно ли вызвать F(args...)
std::is_invocable_r_v<R, F, Args...>       // ...и получить R
```

**3. Свойства операций (важно для оптимизации!)**

```cpp
std::is_nothrow_move_constructible_v<T>   // ← vector использует для выбора move vs copy
std::is_nothrow_destructible_v<T>
std::is_trivially_destructible_v<T>       // деструктор можно не вызывать
std::is_copy_constructible_v<T>
std::is_move_assignable_v<T>
```

Помнишь, почему move должен быть `noexcept`? `std::vector` при реаллокации проверяет ровно `is_nothrow_move_constructible_v<T>` (через `std::move_if_noexcept`) и выбирает move или copy.

**4. Преобразования типов (`::type` → тип)**

```cpp
std::remove_reference_t<T>          // int& → int
std::remove_const_t<T>              // const int → int
std::remove_cv_t<T>                 // убрать const и volatile
std::remove_pointer_t<T>            // int* → int
std::add_const_t<T>
std::add_lvalue_reference_t<T>
std::add_pointer_t<T>

std::decay_t<T>                     // ⭐ убирает ref + cv + array→ptr, func→ptr
                                    //    (моделирует передачу по значению)
std::underlying_type_t<Enum>        // базовый тип enum
std::common_type_t<T, U, ...>       // общий тип (как в тернарном операторе)
std::conditional_t<B, T, F>         // compile-time тернарник: B ? T : F
std::enable_if_t<B, T>              // SFINAE (разбирали)
std::invoke_result_t<F, Args...>    // тип результата F(args...)
```

`std::decay_t` — самый частый в практике. Мы уже использовали его для ограничения forwarding-конструктора:

```cpp
template<class T,
    std::enable_if_t<!std::is_same_v<std::decay_t<T>, Person>, int> = 0>
Person(T&& name);   // decay убирает Person&, const Person& → сравниваем «голый» тип
```

### Суффиксы `_v` и `_t`

```cpp
std::is_integral<T>::value      // C++11 — громоздко
std::is_integral_v<T>           // C++17 — variable template ✅

typename std::remove_reference<T>::type    // C++11 — нужен typename!
std::remove_reference_t<T>                 // C++14 — alias template ✅
```

**Всегда используй `_v` / `_t` формы** — короче и не требуют `typename`.

---

## Практические применения

### 1. Compile-time проверки

```cpp
template<class T>
void process(T x) {
    static_assert(std::is_arithmetic_v<T>, "T must be numeric");
    static_assert(!std::is_pointer_v<T>, "pointers not supported");
    // ...
}
```

Внятные ошибки вместо шаблонной простыни.

### 2. Ветвление по типу (`if constexpr`)

```cpp
template<class T>
void print(T x) {
    if constexpr (std::is_pointer_v<T>) {
        std::cout << *x;              // разыменовываем
    } else if constexpr (std::is_same_v<T, bool>) {
        std::cout << (x ? "true" : "false");
    } else {
        std::cout << x;
    }
}
```

### 3. Ограничение перегрузок (SFINAE / concepts)

```cpp
// C++17
template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void f(T x);

// C++20 — concepts построены на тех же traits
template<class T> concept Integral = std::is_integral_v<T>;
void f(Integral auto x);
```

### 4. Выбор типа

```cpp
template<class T>
using StorageType = std::conditional_t
    sizeof(T) <= 16,       // условие
    T,                     // маленькие — по значению
    const T&               // большие — по ссылке
>;
```

### 5. Оптимизация по свойствам типа

```cpp
template<class T>
void copyRange(T* dst, const T* src, size_t n) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(dst, src, n * sizeof(T));   // ✅ быстрый путь
    } else {
        for (size_t i = 0; i < n; ++i) new (dst + i) T(src[i]);   // поэлементно
    }
}
```

Так устроены внутренности STL-алгоритмов.

### 6. Свои traits

```cpp
// Detection idiom (разбирали в SFINAE)
template<class T, class = void>
struct has_size : std::false_type { };

template<class T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type { };

template<class T>
inline constexpr bool has_size_v = has_size<T>::value;   // добавляем _v форму
```

С C++20 — проще через concept:

```cpp
template<class T>
concept HasSize = requires(T t) { t.size(); };
```

---

## Формулировки на собеседовании

**«Что такое CRTP?»** — Базовый класс, параметризованный своим наследником (`class D : Base<D>`). Позволяет базе вызывать методы наследника через `static_cast<Derived*>(this)` → **статический полиморфизм** без vtable, с возможностью инлайнинга.

**«CRTP vs virtual — когда что?»** — `virtual`: рантайм-диспетчеризация, общий базовый тип, гетерогенные контейнеры; цена — vptr и невозможность инлайнинга. CRTP: compile-time, нулевой оверхед, инлайнинг; цена — **нет общего базового типа**, гетерогенный контейнер невозможен, code bloat.

**«Почему в CRTP нельзя обращаться к членам Derived в теле базы?»** — На момент инстанцирования `Base<Derived>` тип `Derived` **неполон** (мы ещё внутри его списка наследования). Обращаться можно только в **телах методов** — они инстанцируются лениво, по требованию.

**«Приведи пример CRTP из стандартной библиотеки.»** — `std::enable_shared_from_this<T>`.

**«Как реализован `std::is_pointer`?»** — Основной шаблон наследует `false_type`; частичная специализация `is_pointer<T*>` наследует `true_type`. Вся библиотека traits построена на специализации шаблонов.

**«Что делает `std::decay_t`?»** — Моделирует преобразование при передаче по значению: убирает ссылки, cv-квалификаторы, превращает массив в указатель, функцию в указатель на функцию. Часто нужен для сравнения «голых» типов в SFINAE/concepts.

**«Зачем `is_nothrow_move_constructible`?»** — `std::vector` при реаллокации использует move только если он `noexcept` (иначе не может дать strong exception guarantee) → откатывается на copy. Этот trait — механизм проверки.

---

Отличие от Java: там рефлексия — **рантайм** (`Class<?>`, `getDeclaredMethods()`), с оверхедом и потерей типобезопасности. Type traits — **compile-time**: ноль рантайм-стоимости, всё разрешается компилятором, ошибки ловятся при сборке. CRTP как паттерн в Java невозможен по назначению (generics стираются, `static_cast` к параметру типа не сделать) — хотя внешне похожая конструкция `class E extends Enum<E>` существует, она нужна лишь для типизации `compareTo`, а не для статического полиморфизма: в Java **любой** вызов метода всё равно виртуальный. Философия та же, что мы видели везде: C++ переносит на этап компиляции всё, что можно, ради нулевого рантайм-оверхеда.
