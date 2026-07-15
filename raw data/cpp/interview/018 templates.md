[[raw data/cpp/interview/_|<=]]


# Шаблоны: функции, классы, специализация

## Базовая модель: шаблон — это не код, а рецепт

Шаблон сам по себе **не компилируется в код**. Компилятор **инстанцирует** его для конкретных типов — генерирует отдельную функцию/класс на каждую комбинацию аргументов.

```cpp
template<class T>
T max(T a, T b) { return a > b ? a : b; }

max(1, 2);        // инстанцирует max<int>
max(1.0, 2.0);    // инстанцирует max<double> — ОТДЕЛЬНЫЙ код
```

Это **мономорфизация** — прямая противоположность Java-generics с их type erasure. Отсюда: ноль рантайм-оверхеда (всё разрешается в compile-time), но раздувание кода (code bloat) и медленная компиляция.

Практическое следствие: **шаблоны определяются в заголовках**. Компилятор должен видеть тело, чтобы инстанцировать его в каждой TU, где шаблон используется.

```cpp
// template.h
template<class T> void f(T t) { /* тело ЗДЕСЬ, не в .cpp */ }
```

(Альтернатива — явное инстанцирование в .cpp, редко используется.)

---

## Function templates

### Вывод типов (template argument deduction)

```cpp
template<class T>
void f(T x);           // по значению — top-level const/volatile и ссылки ОТБРАСЫВАЮТСЯ

template<class T>
void g(T& x);          // по ссылке — const сохраняется

template<class T>
void h(T&& x);         // forwarding reference (мы это разбирали)
```

```cpp
const int c = 5;
f(c);   // T = int          (const отброшен — у нас своя копия)
g(c);   // T = const int    (const сохранён)
h(c);   // T = const int&   (lvalue → reference collapsing)
```

**Явное указание типа**, когда вывод не работает или даёт не то:

```cpp
template<class T> T max(T a, T b);

max(1, 2.0);        // ❌ ОШИБКА: T выводится и как int, и как double — конфликт
max<double>(1, 2.0);   // ✅ явно задали T=double, int неявно преобразуется
```

### Порядок параметров имеет значение

Выводимые параметры ставят **после** невыводимых:

```cpp
template<class R, class T>
R convert(T x) { return static_cast<R>(x); }

convert<double>(5);   // ✅ R=double задан явно, T=int выведен
                      //    если бы порядок был <T, R> — пришлось бы писать оба
```

### `auto` возвращаемого типа

```cpp
template<class T, class U>
auto add(T a, U b) { return a + b; }   // C++14: тип выводится из return

// C++11 (trailing return type):
template<class T, class U>
auto add(T a, U b) -> decltype(a + b) { return a + b; }

// когда нужно СОХРАНИТЬ ссылочность/константность:
template<class C, class I>
decltype(auto) get(C& c, I i) { return c[i]; }   // C++14: вернёт T&, а не T!
```

`auto` отбрасывает ссылки и const (как вывод по значению). `decltype(auto)` — сохраняет **точный** тип выражения. Важно, когда возвращаешь ссылку.

---

## Class templates

```cpp
template<class T, size_t N = 10>       // параметр по умолчанию
class Buffer {
    T data_[N];
    size_t size_ = 0;
public:
    void push(const T& x) { data_[size_++] = x; }
    T& operator[](size_t i) { return data_[i]; }
};

Buffer<int> b1;         // T=int, N=10
Buffer<double, 5> b2;   // T=double, N=5
```

### Виды параметров шаблона

```cpp
template<class T>                       // тип
template<size_t N>                      // non-type (значение!) — int, enum, указатель, ссылка
template<template<class> class Cont>     // template template parameter
template<auto V>                        // C++17: non-type с выводом типа
```

```cpp
template<auto V>
struct Constant { static constexpr auto value = V; };

Constant<42>;       // V = 42 (int)
Constant<'a'>;      // V = 'a' (char)
Constant<3.14>;     // C++20: floating-point non-type параметры
```

Non-type параметры должны быть **константными выражениями** (compile-time):

```cpp
Buffer<int, 10> b;     // ✅
size_t n = readInt();
Buffer<int, n> b2;     // ❌ n не compile-time константа
```

### CTAD (Class Template Argument Deduction, C++17)

Раньше для классов приходилось указывать типы явно (в отличие от функций):

```cpp
std::pair<int, std::string> p(1, "a");    // до C++17 — обязательно
std::make_pair(1, "a");                    // либо helper-функция

// C++17:
std::pair p(1, "a");                       // ✅ выводится из конструктора!
std::vector v{1, 2, 3};                    // vector<int>
std::lock_guard lock(mtx);                 // lock_guard<std::mutex>
```

При необходимости пишут **deduction guides**:

```cpp
template<class T>
struct Wrapper {
    T value_;
    Wrapper(T v) : value_(v) { }
};

Wrapper(const char*) -> Wrapper<std::string>;   // guide: char* → string, не const char*

Wrapper w("hello");   // Wrapper<std::string> благодаря guide
```

---

## Полная специализация (explicit specialization)

**Полностью** заменяет реализацию для конкретного набора аргументов. Работает и для функций, и для классов.

### Для классов

```cpp
template<class T>
struct Serializer {
    static std::string serialize(const T& x) {
        return std::to_string(x);   // общий случай
    }
};

// ПОЛНАЯ специализация для std::string
template<>                          // ← пустые угловые скобки
struct Serializer<std::string> {    // ← конкретный аргумент
    static std::string serialize(const std::string& s) {
        return "\"" + s + "\"";     // кавычки для строк
    }
};

Serializer<int>::serialize(42);            // "42"
Serializer<std::string>::serialize("hi");  // "\"hi\""
```

Специализация — **полностью независимый класс**. Может иметь другой набор членов, другой интерфейс (хотя это плохая идея):

```cpp
template<>
struct Serializer<bool> {
    static const char* serialize(bool b) { return b ? "true" : "false"; }
    static constexpr int extra = 42;   // членов может не быть в основном шаблоне!
};
```

Классический реальный пример — `std::vector<bool>`: полная специализация с битовой упаковкой, из-за которой её итераторы возвращают прокси-объекты, а не `bool&`. Историческая ошибка стандарта, но иллюстрирует силу механизма.

### Для функций

```cpp
template<class T>
void print(T x) { std::cout << x; }

template<>
void print<bool>(bool b) { std::cout << (b ? "true" : "false"); }   // полная специализация
```

**Но: для функций специализацию использовать не рекомендуется.** Вместо неё — **перегрузка**:

```cpp
template<class T>
void print(T x) { std::cout << x; }

void print(bool b) { std::cout << (b ? "true" : "false"); }   // ✅ обычная перегрузка — лучше
```

Причина: специализация функции **не участвует в разрешении перегрузок как отдельный кандидат**. Сначала выбирается основной шаблон (или перегрузка), и только потом проверяется, есть ли для него специализация. Это даёт неинтуитивные результаты:

```cpp
template<class T> void f(T);       // (1) основной шаблон
template<> void f<int*>(int*);     // (2) специализация (1)
template<class T> void f(T*);      // (3) перегрузка — БОЛЕЕ специализированная!

int* p;
f(p);   // ⚠️ вызовется (3), НЕ (2)! (3) выигрывает как более специализированный шаблон,
        //    и для (3) специализации нет
```

Порядок объявления влияет на результат — источник трудноуловимых багов. **Правило: специализируй классы, перегружай функции.**

---

## Частичная специализация (partial specialization)

Специализация для **семейства** типов, а не одного конкретного. **Только для классов!** (для функций — нельзя, используй перегрузку).

```cpp
template<class T>
struct Info {
    static constexpr const char* name = "unknown";
};

// ЧАСТИЧНАЯ: для любых указателей
template<class T>
struct Info<T*> {                              // ← T остаётся параметром!
    static constexpr const char* name = "pointer";
};

// ЧАСТИЧНАЯ: для любых vector
template<class T>
struct Info<std::vector<T>> {
    static constexpr const char* name = "vector";
};

// ПОЛНАЯ: конкретный тип
template<>
struct Info<int> {
    static constexpr const char* name = "int";
};

Info<double>::name;              // "unknown"  (основной шаблон)
Info<double*>::name;             // "pointer"  (частичная)
Info<std::vector<int>>::name;    // "vector"   (частичная)
Info<int>::name;                 // "int"      (полная)
```

### Частичная специализация по части параметров

```cpp
template<class T, class U>
struct Pair { /* общий случай */ };

template<class T>
struct Pair<T, T> { /* оба типа ОДИНАКОВЫ */ };        // частичная

template<class U>
struct Pair<int, U> { /* первый — int */ };            // частичная

template<>
struct Pair<int, int> { /* оба int */ };               // полная
```

### Правила выбора

Компилятор выбирает **наиболее специализированный** подходящий вариант:

1. Полная специализация (если точно подходит)
2. Наиболее специализированная частичная
3. Основной шаблон

```cpp
Pair<int, int>;       // → полная Pair<int,int>
Pair<int, double>;    // → частичная Pair<int, U>
Pair<char, char>;     // → частичная Pair<T, T>
Pair<char, double>;   // → основной шаблон
```

Если два варианта одинаково подходят и ни один не «более специализирован» — **ambiguous, ошибка компиляции**:

```cpp
Pair<int, int>;   // без полной специализации: подходят и Pair<T,T>, и Pair<int,U>
                  // → ❌ ambiguous
```

---

## Почему нельзя частичную специализацию функций

```cpp
template<class T> void f(T);
template<class T> void f<T*>(T*);   // ❌ ОШИБКА — частичная специализация функций запрещена
```

**Обходной путь — перегрузка**, которая делает то же самое и естественнее вписывается в разрешение перегрузок:

```cpp
template<class T> void f(T);       // общий
template<class T> void f(T*);      // ✅ перегрузка для указателей — работает как надо
```

**Второй обходной путь** (когда нужна именно специализация, например ради контроля SFINAE) — делегирование в специализируемый класс:

```cpp
template<class T>
struct Impl {
    static void call(T x) { /* общий */ }
};

template<class T>
struct Impl<T*> {                          // ✅ частичная специализация КЛАССА — можно
    static void call(T* x) { /* для указателей */ }
};

template<class T>
void f(T x) { Impl<T>::call(x); }          // функция просто делегирует
```

Эта идиома широко используется в реализации стандартной библиотеки.

---

## Практический пример: type traits через специализацию

Так устроены `<type_traits>`:

```cpp
// is_pointer
template<class T>
struct is_pointer : std::false_type { };              // общий: НЕ указатель

template<class T>
struct is_pointer<T*> : std::true_type { };           // частичная: указатель!

static_assert(is_pointer<int*>::value);       // true
static_assert(!is_pointer<int>::value);       // false
```

```cpp
// remove_reference
template<class T> struct remove_reference      { using type = T; };
template<class T> struct remove_reference<T&>  { using type = T; };   // частичная
template<class T> struct remove_reference<T&&> { using type = T; };   // частичная

using X = remove_reference<int&>::type;   // int
```

Это буквально механизм, на котором построена вся compile-time рефлексия в C++.

---

## Two-phase lookup — важная тонкость

Шаблон проверяется **дважды**:

**Фаза 1 (при определении)** — проверяется синтаксис и **независимые от T** имена. **Фаза 2 (при инстанцировании)** — проверяются **зависимые** имена.

```cpp
template<class T>
void f(T t) {
    undefined_function();   // ❌ ОШИБКА СРАЗУ (фаза 1) — имя не зависит от T
    t.someMethod();         // ✅ проверится только при инстанцировании (фаза 2)
}
```

Отсюда необходимость `typename` и `template` для зависимых имён:

```cpp
template<class T>
void f() {
    typename T::value_type x;      // ✅ typename: говорим, что T::value_type — ТИП
                                    //    без него компилятор считает это переменной!
    T::template rebind<int>();      // ✅ template: говорим, что rebind — шаблон
}
```

Без `typename` компилятор в фазе 1 не знает, что `T::value_type` — тип (это может быть статический член), и парсит неверно.

(В C++20 требования к `typename` смягчены в контекстах, где ожидается только тип.)

---

## Формулировки на собеседовании

**«Чем полная специализация отличается от частичной?»** — Полная (`template<>`) фиксирует **все** аргументы → конкретный тип. Частичная (`template<class T> struct X<T*>`) оставляет часть параметров → семейство типов. Частичная — **только для классов**.

**«Почему нельзя частичную специализацию функций?»** — Запрещено стандартом: это конфликтовало бы с разрешением перегрузок. Вместо неё — обычная перегрузка (она делает то же самое) или делегирование в специализируемый класс.

**«Почему для функций специализацию не рекомендуют?»** — Специализация не участвует в разрешении перегрузок как отдельный кандидат: сначала выбирается шаблон/перегрузка, потом ищется его специализация. Результат зависит от порядка объявлений → неинтуитивные баги. Правило: **специализируй классы, перегружай функции**.

**«Как компилятор выбирает специализацию?»** — Наиболее специализированный подходящий вариант: полная → наиболее специализированная частичная → основной шаблон. При равной «специализированности» — ambiguous.

**«Почему шаблоны в заголовках?»** — Компилятор должен видеть **определение**, чтобы инстанцировать шаблон для конкретных типов в каждой TU. Это мономорфизация, а не type erasure.

**«Зачем `typename` перед `T::value_type`?»** — Two-phase lookup: в фазе 1 компилятор не знает, тип это или статический член; `typename` явно указывает, что это тип.

---

Отличие от Java (важно для тебя): Java generics — **type erasure**: `List<Integer>` и `List<String>` в рантайме — один и тот же `List`, параметры типов стираются. Отсюда: нет специализации, нет примитивов в generics (`List<int>` невозможен — только боксинг), нельзя `new T()`, нельзя `T.class`. C++ шаблоны — **мономорфизация**: для каждого набора типов генерируется отдельный код. Отсюда: полная типовая информация в compile-time, работа с примитивами без оверхеда, специализация, non-type параметры, compile-time вычисления. Цена — code bloat и медленная компиляция. Это одно из самых глубоких различий языков, и на собеседовании на C++ после Java его почти наверняка спросят.
