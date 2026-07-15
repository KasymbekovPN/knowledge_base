[[raw data/cpp/interview/_|<=]]

# `std::variant` (C++17)

## Что это

**Type-safe union**: хранит значение **ровно одного** из перечисленных типов, и **знает**, какого именно.

```cpp
#include <variant>

std::variant<int, double, std::string> v;

v = 42;           // теперь int
v = 3.14;         // теперь double (старое значение разрушено)
v = "hello";      // теперь std::string

v.index();        // 2 — индекс активного типа
```

Решает проблему сырого `union`, который **не помнит**, что в нём лежит:

```cpp
// ❌ сырой union — небезопасно
union U {
    int i;
    double d;
    // std::string s;   ← нетривиальные типы требуют РУЧНОГО вызова конструкторов/деструкторов!
};

U u;
u.i = 42;
double x = u.d;   // ⚠️ UB! читаем не тот член — компилятор не остановит
```

`variant` отслеживает активный тип и **автоматически** вызывает конструкторы/деструкторы.

---

## Устройство

```cpp
template<class... Types>
class variant {
    size_t index_;                        // какой тип активен
    aligned_storage_t<max_size, max_align> storage_;   // память под самый большой тип
};
```

**Размер = размер самого большого типа + дискриминант (обычно 4-8 байт) + выравнивание.**

```cpp
sizeof(std::variant<int, double>);        // ~16 (8 double + 8 index/padding)
sizeof(std::variant<char, std::string>);  // ~40 (32 string + 8)
```

**Никаких аллокаций** — объект хранится **внутри** variant (как у `optional`). Смена типа: деструктор старого + placement new нового.

---

## Доступ к значению

### `std::get` — по типу или индексу

```cpp
std::variant<int, double, std::string> v = 42;

std::get<int>(v);      // ✅ 42
std::get<0>(v);        // ✅ 42 (по индексу)

std::get<double>(v);   // ⚠️ бросает std::bad_variant_access — активен int!
```

Требует **точного** знания активного типа. Иначе исключение.

### `std::get_if` — безопасно, через указатель

```cpp
if (auto* p = std::get_if<int>(&v)) {   // ✅ nullptr, если тип не активен
    std::cout << *p;
}
```

Принимает **указатель** на variant, возвращает указатель на значение или `nullptr`. Идиоматично для «проверь и используй».

### `std::holds_alternative` — проверка типа

```cpp
if (std::holds_alternative<int>(v)) {
    int x = std::get<int>(v);   // ✅ безопасно — проверили
}
```

---

## `std::visit` — главный инструмент

Применяет вызываемый объект к **активному** значению, **какой бы тип там ни был**.

```cpp
std::variant<int, double, std::string> v = 3.14;

std::visit([](const auto& x) {
    std::cout << x;             // ✅ работает для любого типа (generic lambda)
}, v);
```

**Компилятор проверяет исчерпывающесть:** visitor должен уметь обработать **все** альтернативы, иначе ошибка компиляции. Это ключевое преимущество над `if`-цепочками — забыть тип невозможно.

### Overloaded idiom — разная обработка разных типов

Generic-лямбда обрабатывает всё одинаково. Чтобы по-разному — нужен visitor с перегрузками:

```cpp
// Классическая идиома (C++17)
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;   // ✅ подтягиваем operator() из всех баз
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;   // deduction guide

// Использование:
std::visit(overloaded{
    [](int x)                { std::cout << "int: " << x; },
    [](double x)             { std::cout << "double: " << x; },
    [](const std::string& s) { std::cout << "string: " << s; }
}, v);
```

Механика: `overloaded` наследуется **от всех лямбд** (каждая — класс с `operator()`), `using Ts::operator()...` затаскивает все перегрузки в область видимости → получаем один объект с несколькими `operator()`.

Это применение variadic templates + pack expansion в `using` (мы разбирали variadic) + CTAD через deduction guide (разбирали в шаблонах).

**C++23 упрощает:**

```cpp
// deduction guide больше не нужен (агрегатный CTAD)
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
```

### Visit нескольких variant

```cpp
std::variant<int, double> a = 1;
std::variant<int, double> b = 2.0;

std::visit([](auto x, auto y) {
    return x + y;   // ✅ обрабатывает ВСЕ 4 комбинации типов
}, a, b);
```

Компилятор генерирует таблицу переходов на все комбинации. Осторожно: N variant с M альтернативами → M^N инстанциаций (взрыв кода).

---

## Практические применения

### 1. Sum type / алгебраические типы данных

```cpp
struct Circle    { double r; };
struct Rectangle { double w, h; };
struct Triangle  { double a, b, c; };

using Shape = std::variant<Circle, Rectangle, Triangle>;

double area(const Shape& s) {
    return std::visit(overloaded{
        [](const Circle& c)    { return 3.14159 * c.r * c.r; },
        [](const Rectangle& r) { return r.w * r.h; },
        [](const Triangle& t)  { 
            double p = (t.a + t.b + t.c) / 2;
            return std::sqrt(p*(p-t.a)*(p-t.b)*(p-t.c));
        }
    }, s);
}
```

**Сравни с наследованием + virtual:**

| |`variant` + `visit`|`virtual`|
|---|---|---|
|Диспетчеризация|compile-time (jump table)|vtable|
|Аллокации|❌ нет (объект внутри)|✅ нужен `unique_ptr<Base>`|
|Добавить **тип**|нужно менять все visit'ы|✅ просто новый класс|
|Добавить **операцию**|✅ новый visitor, классы не трогаем|нужно менять базу и всех наследников|
|Набор типов|**закрытый** (фиксирован)|**открытый** (расширяется)|
|Исчерпывающесть|✅ проверяется компилятором|❌|

Это **expression problem**: `virtual` легко расширять типами, `variant` — операциями. Выбирай по тому, что меняется чаще.

**Практика:** `variant` хорош для закрытых множеств (типы AST, состояния, события), `virtual` — для расширяемых иерархий (плагины, стратегии).

### 2. Result-тип (до появления `expected`)

```cpp
struct Error { std::string message; int code; };

using Result = std::variant<int, Error>;

Result parse(const std::string& s) {
    if (bad) return Error{"invalid", 400};
    return 42;
}

std::visit(overloaded{
    [](int value)        { std::cout << "OK: " << value; },
    [](const Error& e)   { std::cout << "Error: " << e.message; }
}, parse(input));
```

С C++23 для этого есть `std::expected` — специализированный и удобнее (monadic операции, `value()`/`error()`). Но `variant` был единственным вариантом до него.

### 3. State machine

```cpp
struct Idle       { };
struct Connecting { int attempts; };
struct Connected  { Socket sock; };
struct Failed     { std::string reason; };

using State = std::variant<Idle, Connecting, Connected, Failed>;

State transition(State s, Event e) {
    return std::visit(overloaded{
        [&](Idle)             -> State { return Connecting{1}; },
        [&](Connecting c)     -> State { 
            return c.attempts < 3 ? State{Connecting{c.attempts+1}} 
                                  : State{Failed{"timeout"}};
        },
        [&](Connected& c)     -> State { return std::move(c); },
        [&](Failed f)         -> State { return f; }
    }, std::move(s));
}
```

**Ключевое:** каждое состояние несёт **свои данные** (Connecting — счётчик, Connected — сокет). С enum это невозможно — пришлось бы держать все поля во всех состояниях.

### 4. AST / парсеры

```cpp
struct Number { double value; };
struct Add;  struct Mul;

using Expr = std::variant<Number, std::unique_ptr<Add>, std::unique_ptr<Mul>>;

struct Add { Expr left, right; };
struct Mul { Expr left, right; };

double eval(const Expr& e) {
    return std::visit(overloaded{
        [](const Number& n)               { return n.value; },
        [](const std::unique_ptr<Add>& a) { return eval(a->left) + eval(a->right); },
        [](const std::unique_ptr<Mul>& m) { return eval(m->left) * eval(m->right); }
    }, e);
}
```

Рекурсивные типы требуют косвенности (`unique_ptr`) — variant не может содержать сам себя по значению (бесконечный размер).

---

## Ловушки

### 1. Неявные преобразования при присваивании

```cpp
std::variant<int, std::string> v;

v = "hello";   // ⚠️ станет... int? string?
               //    const char* → лучше подходит для string (конструктор)
               //    но исторически были случаи выбора bool/int!
```

C++17 изначально имел проблемы с разрешением перегрузок, **исправленные P0608** (C++20, но реализовано и в компиляторах для C++17): теперь выбирается тип, для которого преобразование **не сужающее**.

```cpp
std::variant<bool, std::string> v = "hello";
// C++17 (до фикса): ⚠️ bool! (const char* → bool — тоже допустимо)
// после P0608:      ✅ std::string
```

**Безопаснее — явно:**

```cpp
v = std::string("hello");                  // ✅
v.emplace<std::string>("hello");           // ✅ явно указан тип
std::variant<int, std::string> v{std::in_place_type<std::string>, "hello"};   // ✅
```

### 2. Дубликаты типов

```cpp
std::variant<int, int> v;      // ✅ компилируется
std::get<int>(v);              // ❌ ОШИБКА: неоднозначно — какой int?
std::get<0>(v);                // ✅ только по индексу
```

Дубликаты допустимы, но доступ **только по индексу**. Обычно это признак ошибки дизайна — лучше обернуть в разные типы:

```cpp
struct Width { int v; };
struct Height { int v; };
std::variant<Width, Height> v;   // ✅ явные типы
```

### 3. Первый тип должен быть default-конструируемым

```cpp
struct NoDefault { NoDefault(int); };

std::variant<NoDefault, int> v;   // ❌ ОШИБКА: default-конструктор variant
                                   //    создаёт ПЕРВЫЙ тип
```

**Решение — `std::monostate`** (пустой тип-заглушка):

```cpp
std::variant<std::monostate, NoDefault, int> v;   // ✅ default → monostate
// v теперь "пустой" в смысле monostate

if (std::holds_alternative<std::monostate>(v)) {
    // не инициализирован
}
```

`monostate` — стандартный способ дать variant «пустое» состояние.

### 4. Valueless by exception

Редкое, но реальное состояние: если при смене типа **конструктор бросил**, variant остаётся в «пустом» невалидном состоянии.

```cpp
struct Throwing {
    Throwing(int) { throw std::runtime_error("boom"); }
};

std::variant<int, Throwing> v = 42;
try {
    v.emplace<Throwing>(1);   // деструктор int вызван, конструктор Throwing бросил
} catch (...) { }

v.valueless_by_exception();   // ⚠️ true — variant НЕ содержит НИЧЕГО
v.index();                     // variant_npos
std::get<int>(v);              // ⚠️ бросает bad_variant_access
```

Крайне редко, но `visit` на valueless variant бросает. Обычно игнорируют (типы с бросающими move-конструкторами редки), но знать нужно.

### 5. Размер = максимальный тип

```cpp
std::variant<char, std::array<int, 1000>> v;   // ⚠️ ~4000 байт, даже если хранится char!
```

Для сильно разных по размеру типов — косвенность:

```cpp
std::variant<char, std::unique_ptr<BigThing>> v;   // ✅ 8-16 байт
```

---

## `visit` vs `if`-цепочка

```cpp
// ❌ if-цепочка: легко забыть тип, не проверяется компилятором
if (auto* p = std::get_if<int>(&v)) { }
else if (auto* p = std::get_if<double>(&v)) { }
// забыли string → тихо ничего не произойдёт

// ✅ visit: компилятор ТРЕБУЕТ обработать все альтернативы
std::visit(overloaded{
    [](int) { },
    [](double) { },
    // забыли string → ❌ ОШИБКА КОМПИЛЯЦИИ
}, v);
```

**Правило: используй `visit`.** Исчерпывающесть, проверяемая компилятором — главная ценность.

---

## Формулировки на собеседовании

**«Что такое `std::variant`?»** — Type-safe union: хранит значение одного из перечисленных типов **и знает, какого**. Автоматически вызывает конструкторы/деструкторы. Размер = максимальный тип + дискриминант, **без аллокаций**.

**«Чем лучше сырого `union`?»** — Сырой `union` не помнит активный член (чтение не того — **UB**) и не поддерживает нетривиальные типы без ручного управления временем жизни. `variant` отслеживает тип и вызывает конструкторы/деструкторы сам.

**«Что даёт `std::visit`?»** — Применяет visitor к активному значению. Компилятор **проверяет исчерпывающесть** — забыть альтернативу невозможно. Главное преимущество над `get_if`-цепочками.

**«`variant` vs наследование с `virtual`?»** — Expression problem: `variant` легко расширять **операциями** (новый visitor), но добавление **типа** требует правки всех visit'ов; `virtual` — наоборот. `variant` — закрытое множество типов, без аллокаций, compile-time диспетчеризация; `virtual` — открытая иерархия, vtable, обычно `unique_ptr`.

**«Зачем `std::monostate`?»** — Пустой тип-заглушка. Нужен, когда первый тип variant не default-конструируем (variant по умолчанию создаёт **первый**), или чтобы дать variant «пустое» состояние.

**«Что такое valueless_by_exception?»** — Если при смене типа конструктор бросил, старое значение уже разрушено, новое не создано → variant не содержит ничего. Редкое состояние, `get`/`visit` на нём бросают.

**«Что такое overloaded idiom?»** — `struct overloaded : Ts... { using Ts::operator()...; };` — наследуется от нескольких лямбд, подтягивая все их `operator()` → один visitor с разными перегрузками для разных типов.

---

Отличие от Java: там **нет** прямого аналога. Ближайшее — **sealed interfaces** (Java 17) + record patterns (Java 21):

```java
sealed interface Shape permits Circle, Rectangle {}
record Circle(double r) implements Shape {}
record Rectangle(double w, double h) implements Shape {}

double area(Shape s) {
    return switch (s) {                    // ✅ компилятор проверяет исчерпывающесть!
        case Circle c -> Math.PI * c.r() * c.r();
        case Rectangle r -> r.w() * r.h();
    };
}
```

Это семантически очень близко к `variant` + `visit`: закрытое множество типов + проверка исчерпывающести. Различия: (1) в Java объекты всё равно на **heap** (аллокация на каждый), C++ variant хранит внутри без heap; (2) в Java диспетчеризация — рантайм-проверка типа, в C++ — compile-time jump table; (3) в Java это надстройка над наследованием (типы связаны иерархией), в C++ типы **никак не связаны** — variant объединяет произвольные типы (`variant<int, std::string, MyClass>`). Появление sealed interfaces в Java 17 — прямое признание того, что sum types нужны; C++ дал их через библиотечный тип, Java — через языковую конструкцию.
