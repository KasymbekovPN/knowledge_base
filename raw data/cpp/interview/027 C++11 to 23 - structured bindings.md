[[raw data/cpp/interview/_|<=]]

# Structured bindings (C++17)

Распаковка составного объекта в несколько именованных переменных за один раз.

```cpp
std::pair<int, std::string> p{1, "one"};

// До C++17:
int id = p.first;
std::string name = p.second;

// C++17:
auto [id, name] = p;   // ✅
```

---

## Три случая, которые поддерживаются

### 1. Массивы

```cpp
int arr[3] = {1, 2, 3};
auto [a, b, c] = arr;   // a=1, b=2, c=3
```

Число имён должно **точно** совпадать с размером.

### 2. Tuple-like типы (`std::pair`, `std::tuple`, `std::array`)

Формально: типы, для которых определены `std::tuple_size<T>`, `std::tuple_element<I,T>` и `get<I>()`.

```cpp
std::tuple<int, double, std::string> t{1, 2.5, "hi"};
auto [i, d, s] = t;

std::array<int, 3> arr{1, 2, 3};
auto [x, y, z] = arr;
```

### 3. Структуры/классы со **всеми публичными** нестатическими членами

```cpp
struct Point { int x, y; };

Point p{1, 2};
auto [x, y] = p;   // ✅ порядок — по объявлению членов
```

**Требования:** все нестатические члены public, в одном классе (не разбросаны по базам), нет анонимных union.

```cpp
class Bad {
    int x_;         // ⚠️ private
public:
    int y_;
};
auto [a, b] = Bad{};   // ❌ ОШИБКА
```

---

## Ключевая механика: скрытый объект

Это **не** «создать N переменных». Компилятор создаёт **один скрытый объект**, а имена — просто **алиасы** к его частям.

```cpp
auto [x, y] = p;

// Разворачивается примерно в:
auto __hidden = p;              // ← КОПИЯ всего объекта
// x → __hidden.x  (это НЕ переменная, это имя для члена)
// y → __hidden.y
```

Отсюда следуют неочевидные вещи.

### `auto` копирует весь объект

```cpp
std::pair<BigObject, BigObject> p;
auto [a, b] = p;   // ⚠️ КОПИРУЕТСЯ вся пара, даже если нужен только a
```

### Модификаторы применяются к **скрытому объекту**, не к именам

```cpp
auto [x, y] = p;          // копия
auto& [x, y] = p;         // ссылка на p — изменения x меняют p.first ✅
const auto& [x, y] = p;   // const-ссылка — без копии, только чтение ✅
auto&& [x, y] = f();      // forwarding reference — продлевает жизнь временного
```

```cpp
std::pair<int, int> p{1, 2};

auto [a, b] = p;
a = 100;              // p.first всё ещё 1 (работали с копией)

auto& [c, d] = p;
c = 100;              // ✅ p.first == 100 — работали со ссылкой
```

**Практическое правило:** `const auto&` по умолчанию (без копий), `auto&` — если нужно менять, `auto` — если нужна копия.


---

# Разворачивание structured bindings через скрытый объект

Пусть есть:

```cpp
struct P { int a; std::string b; };
P p{1, "one"};
P f();   // возвращает P по значению (prvalue)
```

---

## 1. `auto [x, y] = p;` — копия

```cpp
auto [x, y] = p;

// ↓ разворачивается в:

auto __hidden = p;              // P __hidden = p;  ← COPY-конструктор, копируется ВЕСЬ объект
// x → __hidden.a
// y → __hidden.b
```

`__hidden` — независимая копия. Изменения не видны в `p`:

```cpp
x = 100;
// == __hidden.a = 100;
// p.a всё ещё 1 ✅ (работали с копией)
```

Стоимость: полное копирование `P` (включая `std::string`).

---

## 2. `auto& [x, y] = p;` — ссылка

```cpp
auto& [x, y] = p;

// ↓ разворачивается в:

auto& __hidden = p;             // P& __hidden = p;  ← ССЫЛКА, копирования НЕТ
// x → __hidden.a   (то есть p.a)
// y → __hidden.b   (то есть p.b)
```

`__hidden` — просто другое имя для `p`. Изменения проходят насквозь:

```cpp
x = 100;
// == __hidden.a = 100;
// == p.a = 100;   ✅ ИЗМЕНИЛИ ОРИГИНАЛ
```

---

## 3. `const auto& [x, y] = p;` — const-ссылка

```cpp
const auto& [x, y] = p;

// ↓ разворачивается в:

const auto& __hidden = p;       // const P& __hidden = p;  ← ссылка, копирования НЕТ
// x → __hidden.a   (тип: const int)
// y → __hidden.b   (тип: const std::string)
```

Копирования нет, но и менять нельзя:

```cpp
x = 100;   // ❌ ОШИБКА КОМПИЛЯЦИИ — __hidden const → все члены const
```

Это **дефолтный выбор** для чтения: ноль копий, защита от случайной модификации.

---

## 4. `auto&& [x, y] = f();` — forwarding reference + продление жизни

```cpp
auto&& [x, y] = f();

// ↓ разворачивается в:

auto&& __hidden = f();          // P&& __hidden = f();  ← rvalue-ссылка на ВРЕМЕННЫЙ
                                //    ⭐ lifetime extension: временный живёт,
                                //       пока живёт __hidden (до конца scope)
// x → __hidden.a
// y → __hidden.b
```

Ключевое: **продление жизни применяется к скрытому объекту**, поэтому временный, возвращённый из `f()`, не умирает в конце выражения:

```cpp
auto&& [x, y] = f();
use(x);   // ✅ временный ещё жив — __hidden держит его
```

Без structured bindings это было бы:

```cpp
auto&& __hidden = f();   // ✅ классическое продление жизни временного
```

Тот же механизм, что мы разбирали в теме lifetime — привязка временного к ссылке (`const&` или `&&`) продлевает его жизнь.

Изменять можно (не const):

```cpp
x = 100;   // ✅ меняем член временного объекта, живущего в __hidden
```

### Почему `auto&&`, а не `auto&`

```cpp
auto& [x, y] = f();   // ❌ ОШИБКА: неконстантная lvalue-ссылка не привязывается к prvalue
auto&& [x, y] = f();  // ✅ rvalue-ссылка привязывается + продлевает жизнь
const auto& [x,y] = f();  // ✅ тоже работает (const& привязывается к временному)
```

---

## Сводная таблица

|Запись|Скрытый объект|Копирование|Изменяемость|Работает с временным|
|---|---|---|---|---|
|`auto [x,y] = p;`|`auto __h = p;`|✅ **есть**|✅ (копии)|✅|
|`auto& [x,y] = p;`|`auto& __h = p;`|❌ нет|✅ (оригинала)|❌ (нельзя привязать)|
|`const auto& [x,y] = p;`|`const auto& __h = p;`|❌ нет|❌|✅ (продление жизни)|
|`auto&& [x,y] = f();`|`auto&& __h = f();`|❌ нет|✅|✅ (продление жизни)|

---

## Почему это важно: `auto&&` в range-for

Именно поэтому идиоматичный обобщённый range-for пишется через `auto&&`:

```cpp
for (auto&& [key, value] : getMap()) { }   // ✅ работает и с lvalue-контейнером,
                                            //    и с временным, и без копий
```

`auto&&` — forwarding reference: привяжется и к lvalue (`auto& __h`), и к rvalue (`auto&& __h` с продлением жизни). Универсальный вариант.

Но для обычного цикла по существующему контейнеру `const auto&` читается яснее:

```cpp
for (const auto& [key, value] : m) { }   // ✅ явно: только читаем, без копий
for (auto& [key, value] : m) { }         // ✅ явно: будем менять value
```

---

## Проверка через `decltype`

Тонкость, о которой я упоминал: имена **не являются ссылками**, даже когда скрытый объект — ссылка. `decltype` даёт тип **члена**:

```cpp
P p{1, "one"};

auto& [x, y] = p;
static_assert(std::is_same_v<decltype(x), int>);          // ✅ int, а НЕ int&
static_assert(std::is_same_v<decltype(y), std::string>);  // ✅ std::string

const auto& [cx, cy] = p;
static_assert(std::is_same_v<decltype(cx), const int>);   // ✅ const применился к члену
```

Но **выражение** `x` при этом ведёт себя как lvalue, ссылающийся на `p.a` — потому что это алиас к `__hidden.a`, а `__hidden` — ссылка на `p`. Механизм алиасинга работает на уровне имён, а не через создание ссылочных переменных.

---

## Главное применение: range-for по map

Убирает уродливые `.first`/`.second`:

```cpp
std::map<int, std::string> m{{1,"one"}, {2,"two"}};

// До C++17:
for (const auto& kv : m)
    std::cout << kv.first << ": " << kv.second;

// C++17:
for (const auto& [key, value] : m)          // ✅ читаемо
    std::cout << key << ": " << value;

// Изменение значений:
for (auto& [key, value] : m)
    value += "!";                            // ✅ key — const (ключ map неизменяем)
```

Обрати внимание: в `map` элемент — `pair<const Key, Value>`, поэтому `key` автоматически const, даже с `auto&`.

---

## Второе применение: множественный возврат

```cpp
struct Result { bool ok; int value; std::string error; };

Result parse(const std::string& s);

auto [ok, value, error] = parse(input);   // ✅ вместо out-параметров
if (!ok) log(error);
```

Это делает возврат структуры полноценной заменой out-параметрам:

```cpp
// ❌ старый стиль
bool parse(const std::string& s, int& out);

// ✅ современный
std::pair<bool, int> parse(const std::string& s);
auto [ok, value] = parse(s);
```

---

## Третье: `map::insert` / `emplace`

`insert` возвращает `pair<iterator, bool>` — теперь читается нормально:

```cpp
auto [it, inserted] = m.insert({1, "one"});
if (inserted) {
    std::cout << "new key: " << it->first;
} else {
    std::cout << "already exists";
}
```

### Особенно хорошо с `if` с инициализатором (C++17)

```cpp
if (auto [it, inserted] = m.try_emplace(key, value); inserted) {
    // ✅ область видимости it и inserted ограничена этим if
    use(it->second);
}

// то же для find:
if (auto it = m.find(key); it != m.end()) {
    use(it->second);
}   // it не «протекает» наружу
```

Эти две фичи (structured bindings + `if`-init) появились вместе и предназначены для совместного использования.

---

## Ограничения и ловушки

### 1. Нельзя пропустить элемент

```cpp
auto [x, y, z] = tuple;   // обязаны назвать ВСЕ
auto [x, _, z] = tuple;   // `_` — просто ещё одно имя, не «игнорировать»
```

В C++26 добавляется `_` как настоящий placeholder (можно переиспользовать имя). Пока — просто игнорируй ненужные (компилятор может выдать warning о неиспользуемой переменной; подавляется `[[maybe_unused]]`).

### 2. Нельзя объявить в structured binding

```cpp
auto [x, y] = p;
int [a, b] = p;          // ❌ только auto (с cv/ref модификаторами)
static auto [x, y] = p;  // ❌ (C++17); разрешено с C++20
```

### 3. Нельзя захватить в лямбду (до C++20)

```cpp
auto [x, y] = p;
auto f = [x]{ return x; };   // ❌ C++17: x — не переменная, а structured binding
                              // ✅ C++20: разрешено
```

Классическая проблема при переходе на C++17. Обходили копированием:

```cpp
auto [x, y] = p;
int x_copy = x;
auto f = [x_copy]{ return x_copy; };   // ✅
```

### 4. `const` на именах не работает

```cpp
auto [const x, y] = p;   // ❌ не синтаксис
const auto [x, y] = p;   // ✅ const применяется к СКРЫТОМУ ОБЪЕКТУ → оба const
```

### 5. Ссылка на временный — lifetime extension работает

```cpp
const auto& [x, y] = makePair();   // ✅ временный живёт, пока живёт binding
                                    //    (продление жизни применяется к скрытому объекту)
auto&& [x, y] = makePair();         // ✅ тоже
```

### 6. Типы имён — не всегда то, что кажется

`decltype` на structured binding даёт **тип члена**, а не тип ссылки:

```cpp
std::pair<int, double> p;
auto& [x, y] = p;
static_assert(std::is_same_v<decltype(x), int>);   // ✅ int, а НЕ int&!
```

Тонкость: имена не являются ссылками (даже в `auto&` варианте) — они «имена для членов скрытого объекта». `decltype` это отражает.

---

## Кастомная поддержка (tuple-like protocol)

Свой класс можно сделать «распаковываемым», реализовав три вещи:

```cpp
class Point {
    int x_, y_;   // private!
public:
    Point(int x, int y) : x_(x), y_(y) { }
    template<size_t I> auto get() const {
        if constexpr (I == 0) return x_;
        else if constexpr (I == 1) return y_;
    }
};

// 1. tuple_size
namespace std {
    template<> struct tuple_size<Point> : integral_constant<size_t, 2> { };
    // 2. tuple_element
    template<size_t I> struct tuple_element<I, Point> { using type = int; };
}

Point p(1, 2);
auto [x, y] = p;   // ✅ работает через get<I>()
```

Так `std::tuple`, `std::pair` и `std::array` и подключены к механизму. Полезно, если у класса приватные члены, но хочется распаковку.

---

## Формулировки на собеседовании

**«Что такое structured bindings?»** — C++17: распаковка массива, tuple-like типа или структуры с публичными членами в именованные переменные: `auto [a, b] = pair;`.

**«Создаются ли N новых переменных?»** — Нет. Создаётся **один скрытый объект** (копия или ссылка, в зависимости от `auto`/`auto&`), а имена — **алиасы** к его членам. Отсюда: `auto&` даёт ссылку на **весь** объект, `decltype(x)` даёт тип члена, а не ссылки.

**«Почему `auto [a,b] = p; a = 5;` не меняет p?»** — `auto` копирует весь объект в скрытый; `a` — имя для члена **копии**. Нужен `auto& [a,b] = p;`.

**«Что нужно, чтобы свой класс поддерживал распаковку?»** — Либо все публичные нестатические члены (работает автоматически), либо реализовать tuple-like протокол: `std::tuple_size`, `std::tuple_element`, `get<I>()`.

**«Можно ли захватить binding в лямбду?»** — В C++17 нет (не переменная), в C++20 — да. Частая проблема при переходе.

---

Отличие от Java: там аналогичный механизм — **record patterns** (Java 21): `if (obj instanceof Point(int x, int y))`. Плюс destructuring в switch-паттернах. Появилось намного позже и работает через pattern matching, а не через «алиасы к скрытому объекту». В C++ structured bindings — чисто **compile-time** переименование без рантайм-стоимости; в Java record pattern — часть системы паттерн-матчинга с проверкой типа в рантайме. Ещё различие: Java-версия позволяет **вложенные** паттерны (`Line(Point(var x1, var y1), Point p2)`), C++ structured bindings — только один уровень (вложенность придётся распаковывать вручную).
