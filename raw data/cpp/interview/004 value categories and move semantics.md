
[[raw data/cpp/interview/_|<=]]

# lvalue / rvalue / xvalue, `std::move`, `std::forward`

## Категории значений (value categories)

С C++11 у каждого выражения есть **тип** и **категория значения**. Категории строятся из двух свойств:

- **glvalue** (generalized lvalue) — имеет идентичность (адрес, можно взять `&`)
- **rvalue** — можно «обворовать» (перемещать из него ресурсы)

```
                expression
               /          \
          glvalue         rvalue
          /     \         /     \
      lvalue   xvalue        prvalue
               (пересечение glvalue ∩ rvalue)
```

### lvalue — «имеет имя, имеет адрес»

Именованные объекты, ссылки, результат `*ptr`, `arr[i]`, `++x`. Нельзя перемещать (у него есть идентичность, кто-то ещё может им пользоваться).

```cpp
int x = 5;
x;          // lvalue
++x;        // lvalue
arr[i];     // lvalue
*ptr;       // lvalue
std::cin;   // lvalue
```

### prvalue (pure rvalue) — «временное, безымянное»

Литералы, результаты арифметики, возврат по значению, `T()`.

```cpp
42;             // prvalue
x + 1;          // prvalue
std::string("hi");   // prvalue
foo();          // prvalue, если foo возвращает по значению T (не T&)
```

### xvalue (eXpiring value) — «истекающее, но именованное место»

Объект, который **скоро умрёт**, и его ресурсы можно украсть, но у него есть идентичность. Возникает главным образом из `std::move` и возврата `T&&`.

```cpp
std::move(x);        // xvalue
std::move(obj).member;  // xvalue
static_cast<T&&>(x); // xvalue
```

### Ключевая практическая проекция

|категория|можно взять `&`?|можно move из?|пример|
|---|---|---|---|
|lvalue|да|нет|`x`, `*p`, `arr[i]`|
|xvalue|нет (напрямую)|да|`std::move(x)`|
|prvalue|нет|да|`42`, `foo()`, `T()`|

Для практики достаточно грубого деления: **lvalue** = «имеет имя, живёт дальше», **rvalue** (xvalue + prvalue) = «временное/помеченное на слом, можно красть ресурсы».

---

## rvalue-ссылки и перегрузка

`T&&` связывается с rvalue, `const T&` — с чем угодно. Это позволяет разделить копирование и перемещение:

```cpp
void f(const std::string& s);   // (1) принимает lvalue и rvalue — копирует
void f(std::string&& s);        // (2) принимает только rvalue — может украсть

std::string a = "hello";
f(a);              // → (1): a это lvalue
f("world");        // → (2): временный prvalue
f(std::move(a));   // → (2): std::move(a) это xvalue
```

Именно на этом строятся move-конструктор и move-присваивание, `push_back(T&&)` и т.д.

---

## `std::move` — это просто каст

**`std::move` ничего не перемещает.** Это `static_cast` к rvalue-ссылке, превращающий выражение в xvalue, чтобы включить move-перегрузку.

```cpp
template<class T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);
}
```

То есть:

```cpp
std::string a = "hello";
std::string b = std::move(a);   // move(a) → xvalue → вызывается move-конструктор
// само "перемещение" делает move-конструктор string, а не std::move
```

**Важные следствия:**

- `std::move` сам по себе не трогает данные — «работу» делает move-конструктор/присваивание того типа, к которому применили
- После move объект в **valid but unspecified** состоянии — им можно пользоваться (например, присвоить заново, вызвать деструктор), но нельзя полагаться на значение

```cpp
std::string a = "hello";
std::string b = std::move(a);
// a валиден, но содержимое не определено (обычно пусто); a = "new" — ОК
```

- `std::move` на `const` объекте **бесполезен** — из const нельзя красть, вызовется copy:

```cpp
const std::string c = "hi";
std::string d = std::move(c);   // move(c) → const string&& → move-ctor не подходит → COPY
```

- Ловушка: `std::move` при возврате локальной переменной **мешает** RVO/NRVO:

```cpp
std::string bad() {
    std::string s;
    return std::move(s);   // ПЛОХО: подавляет NRVO, форсирует move вместо элизии
}
std::string good() {
    std::string s;
    return s;              // NRVO: объект строится прямо в месте назначения
}
```

---

## Forwarding references (universal references)

`T&&` в **выводимом шаблонном контексте** — это НЕ rvalue-ссылка, а **forwarding reference**. Она связывается и с lvalue, и с rvalue, сохраняя категорию через reference collapsing:

```cpp
template<class T>
void f(T&& x);   // forwarding reference (T выводится)

int a = 5;
f(a);            // T = int&,  x имеет тип int&   (lvalue)
f(5);            // T = int,   x имеет тип int&&  (rvalue)
```

**Отличать от rvalue-ссылки:**

```cpp
void g(std::string&& s);      // rvalue reference — тип фиксирован, НЕ forwarding
template<class T> void h(T&& t);  // forwarding reference — T выводится
std::vector<int> v;
// v.push_back(T&&) в конкретной инстанциации — уже НЕ forwarding (T класса зафиксирован)
```

Правило: forwarding reference — только когда `T&&`, где `T` — **выводимый параметр** этой самой функции.

### Reference collapsing

Когда ссылки «складываются», действует правило: **любой `&` побеждает, `&& && → &&`**.

```
T&  &   → T&
T&  &&  → T&
T&& &   → T&
T&& &&  → T&&
```

Поэтому `f(a)` даёт `T=int&`, и `int& &&` схлопывается в `int&` → `x` это lvalue-ссылка.

---

## `std::forward` — условный каст (perfect forwarding)

Проблема: **именованный параметр — всегда lvalue**, даже если тип `T&&`:

```cpp
template<class T>
void wrapper(T&& arg) {
    inner(arg);   // arg — ИМЯ → всегда lvalue → inner всегда получает lvalue!
}                 // rvalue-ность потеряна
```

`std::forward` восстанавливает исходную категорию: возвращает rvalue, если `T` вывелся как rvalue, и lvalue — если lvalue.

```cpp
template<class T>
void wrapper(T&& arg) {
    inner(std::forward<T>(arg));   // сохраняет: rvalue→rvalue, lvalue→lvalue
}
```

Реализация — тоже условный `static_cast`:

```cpp
template<class T>
constexpr T&& forward(std::remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}
```

Магия в reference collapsing:

- `T = int&` (пришёл lvalue) → `forward<int&>` → `static_cast<int& &&>` → `static_cast<int&>` → **lvalue**
- `T = int` (пришёл rvalue) → `forward<int>` → `static_cast<int&&>` → **rvalue**

### Полный пример

```cpp
struct Widget {
    Widget(const Widget&) { std::cout << "copy\n"; }
    Widget(Widget&&)      { std::cout << "move\n"; }
};

void consume(Widget w);   // принимает по значению

template<class T>
void relay(T&& w) {
    consume(std::forward<T>(w));   // perfect forwarding
}

Widget a;
relay(a);              // lvalue → forward сохраняет lvalue → copy
relay(Widget{});       // rvalue → forward сохраняет rvalue → move
```

Без `std::forward` оба случая печатали бы `copy`.

---

## `std::move` vs `std::forward` — когда что

| |`std::move`|`std::forward<T>`|
|---|---|---|
|Что делает|**безусловно** кастует в rvalue|**условно**: rvalue если T=rvalue, иначе lvalue|
|Где применять|к rvalue-ссылке `T&&` (тип фиксирован)|к forwarding reference `T&&` (T выводится)|
|Требует явный тип|нет: `std::move(x)`|да: `std::forward<T>(x)`|
|Назначение|«я закончил с этим, забирай ресурс»|«передай дальше ровно с той категорией, что пришла»|

```cpp
// move — знаешь точно, что хочешь rvalue:
class String {
    std::string data_;
public:
    String(std::string s) : data_(std::move(s)) {}   // s локальный → move
};

// forward — прокидываешь неизвестную категорию:
template<class... Args>
auto make(Args&&... args) {
    return T(std::forward<Args>(args)...);   // сохраняем категорию каждого аргумента
}
```

---

## Частые вопросы на собеседовании

**«`std::move` перемещает?»** — Нет, это каст к rvalue-ссылке; фактическое перемещение делает move-конструктор/присваивание.

**«Почему `T&&` иногда forwarding, иногда rvalue reference?»** — Forwarding только при выводе `T` в этой функции; в остальных случаях (фиксированный тип, инстанциированный шаблон класса) — обычная rvalue-ссылка.

**«Что с объектом после move?»** — valid but unspecified: можно переприсвоить/разрушить, нельзя полагаться на значение.

**«Почему `std::move(x)` в `return` — плохо?»** — подавляет (N)RVO; компилятор и так переместит/элидирует, а явный move запрещает элизию.

**«Зачем `std::forward` требует явный `<T>`, а `std::move` — нет?»** — `forward` должен знать выведенную категорию (закодирована в `T`), чтобы решить условно; `move` кастует безусловно, тип ему выводить не нужно.

---

Отличие от Java: там нет ни value categories, ни move-семантики — всё передаётся по ссылке (для объектов) или значению (примитивы), «перемещения» ресурсов нет, так как память на GC. Move-семантика — чисто C++ механизм оптимизации владения ресурсами, и он часто спрашивается именно потому, что не имеет аналога в managed-языках.

