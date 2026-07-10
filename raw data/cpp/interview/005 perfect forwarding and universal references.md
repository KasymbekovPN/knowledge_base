[[raw data/cpp/interview/_|<=]]

# Perfect forwarding и forwarding references

Мы уже коснулись их в прошлом ответе — теперь разберём глубже, с механикой и подводными камнями, которые любят на собеседованиях.

## Задача, которую решает perfect forwarding

Написать обёртку, которая принимает аргументы и передаёт их дальше, **сохраняя ровно их характеристики**: тип, константность и категорию значения (lvalue/rvalue). Чтобы rvalue остался rvalue (и вызвал move), а lvalue — lvalue (и вызвал copy).

Без этого механизма обёртка «портит» аргументы. Наивные попытки:

```cpp
template<class T> void wrapper(T arg);         // копия — теряем и rvalue, и дороже
template<class T> void wrapper(T& arg);        // не примет rvalue: wrapper(42) не скомпилится
template<class T> void wrapper(const T& arg);  // всё становится const → нельзя move дальше
```

Единственный аргумент по значению — потеря категории и лишняя копия. `const T&` — принимает всё, но делает всё const, из const нельзя перемещать. Нужен механизм, который **не фиксирует** категорию.

---

## Forwarding reference

`T&&`, где **`T` выводится** в этой же функции, — это forwarding reference (в стандарте — «forwarding reference», термин Скотта Мейерса — «universal reference»).

```cpp
template<class T>
void wrapper(T&& arg);   // forwarding reference
```

Он связывается **и с lvalue, и с rvalue**, и — главное — кодирует исходную категорию в выведенном `T`.

### Два условия, чтобы `&&` был forwarding reference

1. Должен быть **вывод типа** именно этого параметра
2. Форма — ровно `T&&` (не `const T&&`, не `std::vector<T>&&`, не `T&`)

```cpp
template<class T> void a(T&& x);          // ✅ forwarding reference
template<class T> void b(const T&& x);    // ❌ rvalue reference (const мешает)
template<class T> void c(std::vector<T>&& x);  // ❌ rvalue reference (не голый T)
void d(std::string&& x);                  // ❌ rvalue reference (нет вывода T)

template<class T>
class Box {
    void e(T&& x);   // ❌ rvalue reference! T уже зафиксирован классом, вывода в e нет
    template<class U>
    void f(U&& x);   // ✅ forwarding reference (U выводится в f)
};

auto g = [](auto&& x) { };   // ✅ forwarding reference (auto&& выводится)
```

Последний пример важен: **`auto&&` — тоже forwarding reference**. Это часто в generic-лямбдах и range-for.

---

## Механика: вывод типа + reference collapsing

Когда вызывается `wrapper(T&& arg)`:

- передан **lvalue** типа `X` → `T` выводится как `X&` → `arg` имеет тип `X& &&` → схлопывается в `X&` (**lvalue-ссылка**)
- передан **rvalue** типа `X` → `T` выводится как `X` → `arg` имеет тип `X&&` (**rvalue-ссылка**)

```cpp
template<class T>
void wrapper(T&& arg);

std::string s = "hi";
wrapper(s);              // lvalue → T = std::string&,  arg: std::string&
wrapper(std::move(s));   // rvalue → T = std::string,   arg: std::string&&
wrapper("literal");      // rvalue → T = const char(&)[8]... → arg rvalue
```

### Reference collapsing (правило схлопывания)

Ссылка на ссылку невозможна напрямую, но возникает через шаблоны/typedef. Правило: **если хоть одна `&` — результат `&`; `&& + && → &&`**.

```
&  &   → &
&  &&  → &
&& &   → &
&& &&  → &&
```

«lvalue-ссылка заразна». Именно поэтому `T = X&` даёт `arg` типа lvalue-ссылка, а `T = X` — rvalue-ссылка. Категория оказывается закодированной в `T`.

---

## Ключевая проблема: именованный параметр — lvalue

Даже если `arg` **имеет тип** rvalue-ссылки, само выражение `arg` (использование имени) — **lvalue**. У него есть имя и адрес.

```cpp
template<class T>
void wrapper(T&& arg) {
    inner(arg);   // arg — имя → lvalue → inner ВСЕГДА получает lvalue
}
```

Если это не исправить, `wrapper(std::move(s))` внутри превратит rvalue обратно в lvalue → `inner` сделает copy вместо move. Rvalue-ность «теряется на входе».

---

## `std::forward` — восстановление категории

`std::forward<T>(arg)` условно кастует обратно к исходной категории, используя `T`:

```cpp
template<class T>
void wrapper(T&& arg) {
    inner(std::forward<T>(arg));   // rvalue→rvalue, lvalue→lvalue
}
```

Как это работает (через reference collapsing):

- пришёл lvalue → `T = X&` → `forward<X&>` → `static_cast<X& &&>` → `static_cast<X&>` → **lvalue**
- пришёл rvalue → `T = X` → `forward<X>` → `static_cast<X&&>` → **rvalue**

`T` несёт информацию о категории, `forward` её «распаковывает». Поэтому `forward` всегда пишется с явным `<T>` — без него не из чего восстановить категорию.

---

## Канонический пример

```cpp
struct Widget {
    Widget() = default;
    Widget(const Widget&) { std::cout << "copy\n"; }
    Widget(Widget&&) noexcept { std::cout << "move\n"; }
};

void consume(Widget);   // по значению

template<class T>
void relay(T&& w) {
    consume(std::forward<T>(w));
}

int main() {
    Widget a;
    relay(a);            // lvalue → forward сохраняет lvalue → "copy"
    relay(Widget{});     // rvalue → forward сохраняет rvalue → "move"
    relay(std::move(a)); // rvalue → "move"
}
```

Замени `std::forward<T>(w)` на просто `w` — оба случая напечатают `copy`.

---

## Variadic perfect forwarding — реальное применение

Так реализованы фабрики (`make_unique`, `make_shared`, `emplace_back`, `std::make_tuple`): принять любой набор аргументов и идеально прокинуть их в конструктор.

```cpp
template<class T, class... Args>
std::unique_ptr<T> my_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    //                                            ^^^ forward каждого пакета
}

struct Point {
    Point(int x, std::string name);
};

std::string n = "origin";
auto p = my_make_unique<Point>(0, std::move(n));   // 0 как rvalue, n как rvalue → идеально
auto q = my_make_unique<Point>(1, n);              // n как lvalue → copy
```

`std::forward<Args>(args)...` разворачивается по всему пакету, forward'я каждый аргумент с его собственной категорией.

### `emplace_back` — почему быстрее `push_back`

```cpp
std::vector<Point> v;
v.push_back(Point(0, "a"));          // создаёт временный Point → move в вектор
v.emplace_back(0, "a");              // конструирует Point ПРЯМО в памяти вектора
```

`emplace_back(Args&&...)` forward'ит аргументы прямо в конструктор элемента на месте — без создания и перемещения временного объекта. Это прямое следствие perfect forwarding.

---

## Подводные камни (частые вопросы)

### 1. Forwarding-конструктор «пожирает» copy-конструктор

```cpp
struct Person {
    std::string name_;
    template<class T>
    Person(T&& name) : name_(std::forward<T>(name)) {}   // «жадный» конструктор
};

Person a("Alice");
Person b(a);   // ⚠️ не copy-конструктор! T = Person& → forwarding-ctor пытается
               // сконструировать std::string из Person → ошибка компиляции
```

Forwarding reference — **лучшее совпадение** почти для всего, включая lvalue того же типа, и перехватывает вызовы, предназначенные copy-конструктору. Лечится через `std::enable_if`/concepts (ограничить, чтобы конструктор не участвовал, когда `T` — сам `Person`):

```cpp
template<class T,
    class = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Person>>>
Person(T&& name);
```

С C++20 — чище через `requires`:

```cpp
template<class T>
requires (!std::same_as<std::decay_t<T>, Person>)
Person(T&& name);
```

### 2. `std::forward` без вывода — ошибка

`forward` осмыслен **только** для forwarding reference. Применять к фиксированной rvalue-ссылке — концептуально неверно (там нужен `std::move`):

```cpp
void f(std::string&& s) {
    inner(std::move(s));     // ✅ тип фиксирован → move
    // inner(std::forward<???>(s));  // нечего подставить в <T> — вывода не было
}
```

Правило: **forwarding reference → `std::forward<T>`; фиксированная rvalue-ссылка → `std::move`.**

### 3. Форвардить дважды — баг

```cpp
template<class T>
void bad(T&& x) {
    a(std::forward<T>(x));
    b(std::forward<T>(x));   // ⚠️ если T=rvalue, x уже перемещён в a() — use-after-move
}
```

После первого forward'а (если это был rvalue) ресурс мог уйти. Форвардить каждый аргумент можно **один раз**.

### 4. `const` убивает forwarding

```cpp
template<class T> void a(const T&& x);   // это НЕ forwarding reference
```

`const` фиксирует, вывод не даёт «lvalue-ветку» → обычная const rvalue reference.

---

## `auto&&` и range-for

`auto&&` — forwarding reference вне шаблонов. Идиома для generic-кода и обобщённого range-for:

```cpp
for (auto&& elem : container) {          // работает с любым container, без копий
    process(std::forward<decltype(elem)>(elem));
}

auto lambda = [](auto&& x) {             // generic lambda
    return inner(std::forward<decltype(x)>(x));   // forward через decltype
};
```

Обрати внимание: с `auto&&`/generic-лямбдами forward делают через `std::forward<decltype(x)>(x)`, потому что явного `T` нет.

---

## Формулировки на собеседовании

**«Что такое forwarding reference и чем отличается от rvalue reference?»** — `T&&` при выводе `T` в этой функции; связывается и с lvalue, и с rvalue, кодируя категорию в `T`. Rvalue reference (`std::string&&`, `const T&&`, `T&&` при фиксированном `T`) связывается только с rvalue.

**«Зачем `std::forward`, если параметр уже `T&&`?»** — потому что именованный параметр — всегда lvalue; forward восстанавливает исходную категорию из `T`.

**«Что за reference collapsing?»** — правило схлопывания вложенных ссылок: любая `&` даёт `&`, только `&& + &&` даёт `&&`. На нём держится и вывод forwarding reference, и работа `forward`/`move`.

**«Почему forwarding-конструктор опасен?»** — перехватывает copy/move и другие вызовы как лучшее совпадение; нужно ограничивать через `enable_if`/`requires`.

**«Можно ли forward'ить аргумент дважды?»** — нет, после перемещения объект в unspecified-состоянии.
