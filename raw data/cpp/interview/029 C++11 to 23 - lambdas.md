[[raw data/cpp/interview/_|<=]]

# Лямбды

## Что это на самом деле

Лямбда — **синтаксический сахар** над безымянным классом с перегруженным `operator()`. Понимание этого объясняет всё поведение.

```cpp
int factor = 2;
auto f = [factor](int x) { return x * factor; };

// Компилятор генерирует примерно:
class __Lambda_1 {
    int factor;                                  // ← захваченная переменная = ЧЛЕН класса
public:
    __Lambda_1(int f) : factor(f) { }
    auto operator()(int x) const {               // ← ПО УМОЛЧАНИЮ const!
        return x * factor;
    }
};
auto f = __Lambda_1(factor);
```

Отсюда сразу три следствия:

- Тип лямбды **уникален и невыразим** (можно получить только через `auto` / `decltype`)
- Захват по значению = **член класса** → живёт столько же, сколько лямбда
- `operator()` **const** → отсюда нужда в `mutable`

---

## Анатомия

```cpp
[capture](params) specifiers -> ReturnType { body }
//  ^        ^        ^            ^          ^
//  |        |        |            |          тело
//  |        |        |            возвращаемый тип (опционален)
//  |        |        mutable, constexpr, noexcept, static (C++23)
//  |        параметры (опциональны)
//  список захвата (обязателен — только он делает это лямбдой)
```

Минимальная лямбда: `[]{}`.

---

## Списки захвата

```cpp
int a = 1, b = 2;

[]              // ничего не захватывает
[a]             // a по значению (копия)
[&a]            // a по ссылке
[a, &b]         // смешанно
[=]             // ВСЁ используемое по значению
[&]             // ВСЁ используемое по ссылке
[=, &b]         // всё по значению, кроме b
[&, a]          // всё по ссылке, кроме a
[this]          // указатель this (доступ к членам, но НЕ копия объекта!)
[*this]         // C++17: КОПИЯ объекта *this
[x = expr]      // C++14: init-capture (новая переменная)
[...args = std::move(args)]   // C++20: захват пакета параметров
```

### `[=]` и `[&]` — не рекомендуются

Выглядят удобно, но **скрывают, что именно захвачено** → источник dangling.

```cpp
auto makeCallback() {
    int local = 42;
    return [&]{ return local; };   // ⚠️ висячая ссылка! local умрёт
}   // компилятор не предупредит

auto safe() {
    int local = 42;
    return [local]{ return local; };   // ✅ копия
}
```

`[&]` захватывает по ссылке **всё**, включая то, что вот-вот умрёт. Явный список делает опасность видимой.

---

## Ловушка: `[=]` не копирует объект — он захватывает `this`

Самая частая и коварная ошибка.

```cpp
class Widget {
    int factor_ = 2;
public:
    auto getMultiplier() {
        return [=](int x) { return x * factor_; };
        //      ^^^ ⚠️ захватывается НЕ factor_, а this!
        //          эквивалентно: return [this](int x){ return x * this->factor_; };
    }
};

auto f = Widget{}.getMultiplier();   // ⚠️ Widget умер!
f(5);                                 // ⚠️ UB — обращение к this мёртвого объекта
```

Члены класса **нельзя** захватить по значению напрямую — доступ к ним всегда идёт через `this`. `[=]` захватывает `this` **по указателю**, а не копирует объект.

**Решения:**

```cpp
// 1. C++14: init-capture — копируем нужный член
auto getMultiplier() {
    return [factor = factor_](int x) { return x * factor; };   // ✅ копия int
}

// 2. C++17: [*this] — копируем весь объект
auto getMultiplier() {
    return [*this](int x) { return x * factor_; };   // ✅ копия Widget
}
```

В C++20 `[=]` с неявным захватом `this` — **deprecated** именно из-за этой ловушки (нужно писать `[=, this]` явно).

---

## `mutable` — снять const с `operator()`

По умолчанию `operator()` **const** → захваченные по значению переменные (члены класса!) менять нельзя:

```cpp
int counter = 0;

auto f = [counter]() { ++counter; };   // ❌ ОШИБКА: counter — const член в const-методе

auto g = [counter]() mutable {         // ✅ mutable убирает const с operator()
    ++counter;
    return counter;
};

g();   // 1
g();   // 2   ← лямбда хранит СОСТОЯНИЕ между вызовами (это же член класса!)
std::cout << counter;   // ⚠️ 0 — ОРИГИНАЛ не изменился (меняли копию)
```

Ключевое: `mutable` меняет **копию внутри лямбды**, а не оригинал. Лямбда — объект с состоянием.

Не путать с `mutable` у члена класса (разрешает менять в const-методе) — мы разбирали это в теме const correctness. Разные вещи, одно ключевое слово.

---

## Init-capture (C++14) — захват с инициализацией

Создаёт **новую переменную** внутри лямбды:

```cpp
int x = 5;
auto f = [y = x * 2]{ return y; };   // y — новая переменная, y == 10
```

### Главное применение: захват move-only типов

```cpp
auto p = std::make_unique<Widget>();

auto f = [p]{ };                    // ❌ ОШИБКА: unique_ptr некопируем
auto g = [p = std::move(p)]{        // ✅ ПЕРЕМЕЩАЕМ в лямбду
    p->use();
};
// p снаружи теперь nullptr
```

До C++14 это было **невозможно** — приходилось городить обёртки. Init-capture решил проблему.

### Захват результата выражения

```cpp
auto f = [data = getExpensiveData()]{ use(data); };   // ✅ вычисляем один раз
auto g = [n = v.size()]{ return n; };                 // ✅ захватываем значение, не ссылку
```

---

## Возвращаемый тип

Обычно выводится:

```cpp
auto f = [](int x) { return x * 2; };   // int
```

Правила вывода — как у `auto` (отбрасывает ref/const):

```cpp
auto bad = [](std::vector<int>& v) -> auto { return v[0]; };   // int (КОПИЯ)
auto good = [](std::vector<int>& v) -> decltype(auto) { return v[0]; };   // int& ✅
auto good2 = [](std::vector<int>& v) -> int& { return v[0]; };           // ✅ явно
```

Явный тип нужен, когда:

```cpp
auto f = [](bool c) { 
    if (c) return 1;      // int
    else return 2.0;      // ❌ ОШИБКА: разные типы в return
};

auto g = [](bool c) -> double {   // ✅ явно указали
    if (c) return 1;
    else return 2.0;
};
```

---

## Generic lambdas (C++14)

`auto` в параметрах → шаблонный `operator()`:

```cpp
auto f = [](auto x, auto y) { return x + y; };
f(1, 2);        // int
f(1.0, 2.0);    // double
f(std::string("a"), "b");   // string

// разворачивается в:
class __Lambda {
public:
    template<class T, class U>
    auto operator()(T x, U y) const { return x + y; }
};
```

### Perfect forwarding в generic-лямбде

```cpp
auto f = [](auto&& x) {                       // forwarding reference
    return process(std::forward<decltype(x)>(x));   // ✅ forward через decltype
};
```

Обрати внимание: `std::forward<decltype(x)>(x)` — потому что явного `T` нет (мы это упоминали в теме perfect forwarding).

### C++20: явные шаблонные параметры

```cpp
auto f = []<class T>(std::vector<T>& v) { return v.size(); };   // ✅ можем назвать T
auto g = []<class T>(T&& x) { return std::forward<T>(x); };     // ✅ чище, чем decltype
```

---

## Возможности по стандартам

|Стандарт|Возможность|
|---|---|
|**C++11**|базовые лямбды, `mutable`, trailing return type|
|**C++14**|generic lambda (`auto` параметры), **init-capture** (`[x = ...]`)|
|**C++17**|`constexpr` лямбды (неявно, если возможно), `[*this]`|
|**C++20**|шаблонные параметры `[]<class T>`, `[...args = std::move(args)]` (захват пакета), лямбды в unevaluated context, default-конструируемые и присваиваемые лямбды **без захвата**, `[=]` с неявным `this` — deprecated|
|**C++23**|`static operator()`, `[[attributes]]` на лямбдах, deducing `this`|

### C++17: `constexpr` лямбды

```cpp
auto square = [](int x) { return x * x; };   // неявно constexpr, если тело позволяет
static_assert(square(5) == 25);              // ✅ работает в compile-time
```

### C++20: лямбды без захвата — default-конструируемы

Важно для использования как компаратора:

```cpp
auto cmp = [](int a, int b) { return a > b; };
std::set<int, decltype(cmp)> s;   // ✅ C++20: default-конструируется
                                   //    C++17: нужно было s(cmp) — передать экземпляр
```

### C++20: захват пакета параметров

```cpp
template<class... Args>
auto delay(Args&&... args) {
    return [...args = std::forward<Args>(args)]{   // ✅ C++20
        process(args...);
    };
}
```

До C++20 — только через `std::tuple` + `std::apply`.

---

## Лямбда → указатель на функцию

Лямбда **без захвата** неявно конвертируется в указатель на функцию:

```cpp
auto f = [](int x) { return x * 2; };
int (*fp)(int) = f;   // ✅ работает — нет захвата → нет состояния

auto g = [factor](int x) { return x * factor; };
int (*gp)(int) = g;   // ❌ ОШИБКА — есть состояние, в указатель не влезет
```

Нужно для C-API:

```cpp
std::qsort(arr, n, sizeof(int), [](const void* a, const void* b) {
    return *(int*)a - *(int*)b;   // ✅ без захвата → конвертируется в указатель
});
```

---

## `std::function` — стирание типа

Тип лямбды уникален → нельзя хранить разные лямбды в одном контейнере. Решение — `std::function`:

```cpp
std::vector<std::function<int(int)>> handlers;
handlers.push_back([](int x){ return x * 2; });
handlers.push_back([factor](int x){ return x * factor; });   // ✅ разные типы — один контейнер
```

**Цена:** type erasure → виртуальный вызов + возможная аллокация (если захват не влезает в small buffer, обычно ~16 байт). Заметно медленнее прямого вызова лямбды.

```cpp
auto direct = [](int x){ return x*2; };
std::function<int(int)> erased = direct;

direct(5);   // ✅ инлайнится, ~0 стоимость
erased(5);   // ⚠️ косвенный вызов, инлайнинг невозможен
```

**Правило:** используй `auto` для лямбд (инлайнинг), `std::function` — только когда нужен единый тип (контейнеры, члены класса, API-границы).

Альтернатива без аллокации: шаблонный параметр.

```cpp
template<class F>
void forEach(const std::vector<int>& v, F f) {   // ✅ f инлайнится
    for (int x : v) f(x);
}
```

Так устроены STL-алгоритмы — они принимают предикат **по шаблону**, а не через `std::function`.

---

## Immediately Invoked Lambda (IIFE)

Инициализация сложной const-переменной:

```cpp
const auto config = []{
    Config c;
    c.load("file.cfg");
    c.validate();
    return c;
}();   // ← сразу вызываем
// ✅ config можно сделать const, хотя для инициализации нужна логика
```

Полезно для `const` членов в списке инициализации и для ограничения области видимости временных переменных.

---

## Типичные ошибки

### 1. Dangling через `[&]`

```cpp
std::function<int()> f;
{
    int x = 42;
    f = [&x]{ return x; };   // ⚠️ x умрёт
}
f();   // ⚠️ UB
```

**Правило:** захват по ссылке безопасен, только если лямбда **не переживает** захваченные объекты. Для лямбд, которые сохраняются (колбэки, `std::function`, асинхронные задачи) — **захватывай по значению**.

### 2. Захват `this` в асинхронном коде

```cpp
class Session {
    void start() {
        async_read([this](auto data) {   // ⚠️ this может умереть до колбэка!
            process(data);
        });
    }
};
```

**Решение** (мы разбирали в теме `enable_shared_from_this`):

```cpp
class Session : public std::enable_shared_from_this<Session> {
    void start() {
        auto self = shared_from_this();
        async_read([this, self](auto data) {   // ✅ self держит объект живым
            process(data);
        });
    }
};
```

Это стандартная идиома в Boost.Asio — прямо релевантно твоему стеку.

### 3. Захват по ссылке в цикле

```cpp
std::vector<std::function<int()>> fs;
for (int i = 0; i < 3; ++i) {
    fs.push_back([&i]{ return i; });   // ⚠️ ВСЕ ссылаются на ОДИН i, который умрёт
}
// fs[0]() → UB

for (int i = 0; i < 3; ++i) {
    fs.push_back([i]{ return i; });    // ✅ каждая — своя копия
}
```

---

## Формулировки на собеседовании

**«Что такое лямбда на самом деле?»** — Безымянный класс с перегруженным `operator()`. Захваченные переменные — **члены** этого класса. `operator()` по умолчанию **const**.

**«Зачем `mutable`?»** — Снимает `const` с `operator()`, позволяя менять переменные, захваченные **по значению** (они же члены). Меняется копия, не оригинал; лямбда становится stateful между вызовами.

**«Что захватывает `[=]` в методе класса?»** — **`this`** (указатель!), а не копии членов. Члены доступны только через `this`. Если объект умрёт раньше лямбды → dangling. Решения: `[member = member_]` (C++14) или `[*this]` (C++17). В C++20 неявный захват `this` через `[=]` — deprecated.

**«Как захватить `unique_ptr` в лямбду?»** — Init-capture (C++14): `[p = std::move(p)]{}`. До C++14 было невозможно.

**«Почему нельзя положить разные лямбды в один вектор?»** — У каждой лямбды **уникальный тип**. Нужен `std::function` (type erasure, ценой косвенного вызова и возможной аллокации).

**«Когда лямбда конвертируется в указатель на функцию?»** — Только **без захвата** — тогда у неё нет состояния и она укладывается в указатель. Нужно для C-API.

**«`auto` или `std::function` для хранения лямбды?»** — `auto` — точный тип, инлайнинг, ноль оверхеда. `std::function` — только когда нужен единый тип (контейнер, член класса, API), ценой производительности.

---

Отличие от Java: там лямбда — экземпляр **функционального интерфейса** (`Function<T,R>`, `Predicate<T>`), реализованный через `invokedynamic`. Ключевые различия: (1) в Java **нет выбора** захвата — только по значению, и только **effectively final** переменные (компилятор запрещает захват изменяемых); (2) отсюда нет ни `mutable`, ни dangling — GC гарантирует, что захваченный объект жив; (3) тип лямбды в Java — это интерфейс (`Runnable`, `Comparator`), то есть уже «стёртый» — прямой аналог `std::function`, с виртуальным вызовом; аналога «уникального типа + инлайнинг» просто нет. Именно поэтому в C++ STL-алгоритмы принимают предикат **по шаблону** (`std::sort(v.begin(), v.end(), cmp)` инлайнит компаратор), а в Java `Collections.sort` всегда делает виртуальный вызов через интерфейс. Это ещё один случай, где C++ покупает производительность мономорфизацией.
