[[raw data/cpp/interview/_|<=]]

# Касты: `static_cast`, `dynamic_cast`, `reinterpret_cast`, `const_cast`

C++ намеренно ввёл четыре именованных каста вместо C-style `(T)x`, чтобы:

- **явно выражать намерение** (что именно за преобразование)
- **сужать разрешённое** (каждый каст умеет строго своё)
- **быть грепаемым** (`grep reinterpret_cast` находит все опасные места; `(T)x` не найти)

---

## `static_cast` — компайл-тайм преобразование

Проверяется на этапе компиляции, **без рантайм-проверок**. Самый частый.

### Что умеет

**1. Числовые преобразования** (явное сужение, подавление предупреждений):

```cpp
double d = 3.9;
int i = static_cast<int>(d);        // 3 — явно показали, что усечение намеренное
size_t n = static_cast<size_t>(-1); // явное преобразование signed→unsigned
```

**2. Upcast (вверх по иерархии) — безопасно, но обычно неявно:**

```cpp
Derived d;
Base* p = static_cast<Base*>(&d);   // корректно (но и просто Base* p = &d; работает)
```

**3. Downcast (вниз) — БЕЗ проверки:**

```cpp
Base* p = getBase();
Derived* d = static_cast<Derived*>(p);   // ⚠️ НЕТ проверки типа!
                                          //    если p на самом деле не Derived → UB
```

Это ключевое отличие от `dynamic_cast`. `static_cast` **верит тебе на слово**. Используй, только если **точно** знаешь динамический тип (например, из другого источника информации).

**4. `void*` → `T*`:**

```cpp
void* raw = malloc(sizeof(int));
int* p = static_cast<int*>(raw);   // ✅ обратно к типизированному
```

**5. Явные преобразования типов через конструктор/оператор:**

```cpp
std::string s = static_cast<std::string>("hello");   // через конструктор
```

**6. lvalue → rvalue (это и есть `std::move`):**

```cpp
static_cast<T&&>(x);   // ровно то, что делает std::move
```

**7. enum ↔ integral:**

```cpp
enum class Color { Red, Green };
int i = static_cast<int>(Color::Red);        // 0
Color c = static_cast<Color>(1);             // Green
```

### Чего НЕ умеет

- Снимать `const` (для этого `const_cast`)
- Приводить несвязанные типы (`int*` → `double*`)
- Downcast с проверкой (для этого `dynamic_cast`)

```cpp
const int x = 5;
int* p = static_cast<int*>(&x);   // ❌ ошибка компиляции — const не снимает
```

---

## `dynamic_cast` — безопасный downcast с рантайм-проверкой

Единственный каст с **рантайм-проверкой**. Требует **RTTI** и **полиморфного** типа (хотя бы одна виртуальная функция в базе).

```cpp
struct Base { virtual ~Base() = default; };   // полиморфный — есть virtual
struct Derived : Base { void special(); };
struct Other : Base { };

Base* p = getSomething();

// Указатель: при неудаче → nullptr
if (Derived* d = dynamic_cast<Derived*>(p)) {
    d->special();   // ✅ безопасно — проверили
} else {
    // p не Derived
}

// Ссылка: при неудаче → исключение std::bad_cast
try {
    Derived& d = dynamic_cast<Derived&>(*p);
    d.special();
} catch (const std::bad_cast& e) {
    // p не Derived
}
```

**Запомнить:** указатель → `nullptr`, ссылка → `std::bad_cast` (ссылка не может быть «пустой», поэтому единственный способ сообщить о неудаче — исключение).

### Как работает

Компилятор хранит **RTTI**-информацию (обычно рядом с vtable — указатель на `type_info` в отрицательном смещении от vtable). `dynamic_cast` в рантайме:

1. по vptr находит RTTI объекта → узнаёт **реальный** динамический тип
2. проверяет, есть ли целевой тип в иерархии этого объекта
3. если да — корректирует указатель (offset при множественном наследовании) и возвращает; если нет — `nullptr`/исключение

Отсюда стоимость: **обход иерархии в рантайме**, значительно дороже `static_cast` (который вообще ничего не делает в рантайме, кроме, возможно, добавления константного offset'а).

### Требования

```cpp
struct NonPoly { int x; };   // нет virtual
struct D : NonPoly { };

NonPoly* p = new D();
D* d = dynamic_cast<D*>(p);   // ❌ ОШИБКА КОМПИЛЯЦИИ: тип не полиморфный
```

Нужна хотя бы одна виртуальная функция в базе (обычно это виртуальный деструктор, который в полиморфной базе и так обязателен).

### Cross-cast — уникальная возможность

`dynamic_cast` умеет приводить «вбок» при множественном наследовании:

```cpp
struct A { virtual ~A() = default; };
struct B { virtual ~B() = default; };
struct C : A, B { };

A* pa = new C();
B* pb = dynamic_cast<B*>(pa);   // ✅ cross-cast A* → B* через общий динамический тип C
                                //    static_cast так НЕ умеет (A и B не связаны)
```

### Дизайн-замечание

Частый `dynamic_cast` — обычно **запах кода**: значит, полиморфизм используется неправильно. Вместо проверки типа стоит добавить виртуальный метод:

```cpp
// ❌ плохо
void draw(Shape* s) {
    if (auto c = dynamic_cast<Circle*>(s)) drawCircle(c);
    else if (auto r = dynamic_cast<Rect*>(s)) drawRect(r);
}

// ✅ хорошо
struct Shape { virtual void draw() = 0; };   // полиморфизм делает работу
void draw(Shape* s) { s->draw(); }
```

Легитимные применения: работа с чужой иерархией, которую нельзя менять; visitor-подобные паттерны; десериализация; отладка.

---

## `reinterpret_cast` — переинтерпретация битов

«Считай эти биты как другой тип». Практически **ничего не проверяет**, никаких преобразований не делает — просто меняет тип указателя/ссылки.

```cpp
int x = 0x41424344;
char* p = reinterpret_cast<char*>(&x);   // смотрим на int как на байты
// p[0] == 0x44 на little-endian

std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(&x);   // указатель → целое
int* back = reinterpret_cast<int*>(addr);                     // и обратно
```

### Что можно (и почти всё остальное — UB)

**Единственные надёжные применения:**

1. **`T*` ↔ `std::uintptr_t`** и обратно — гарантированно round-trip:

```cpp
int* p = &x;
auto n = reinterpret_cast<std::uintptr_t>(p);
int* q = reinterpret_cast<int*>(n);   // ✅ q == p — гарантировано
```

2. **`T*` → `char*` / `unsigned char*` / `std::byte*`** — просмотр байтового представления (явно разрешено стандартом, исключение из strict aliasing):

```cpp
float f = 3.14f;
auto* bytes = reinterpret_cast<unsigned char*>(&f);
for (size_t i = 0; i < sizeof(f); ++i) printf("%02x ", bytes[i]);   // ✅ легально
```

3. **Низкоуровневый код**: memory-mapped регистры, сетевые буферы, сериализация.

### Опасность — strict aliasing

**Обращение к объекту через указатель несовместимого типа — UB** (нарушение strict aliasing), даже если размеры совпадают:

```cpp
float f = 1.0f;
int* p = reinterpret_cast<int*>(&f);
int bits = *p;   // ⚠️ UB! читаем float через int* — нарушение strict aliasing
                 //    компилятор вправе считать, что int* и float* не алиасят,
                 //    и переупорядочить/выкинуть код
```

**Правильный способ** побитовой интерпретации:

```cpp
// C++20:
int bits = std::bit_cast<int>(f);   // ✅ безопасно, constexpr

// до C++20:
int bits;
std::memcpy(&bits, &f, sizeof(f));  // ✅ memcpy — легальный способ, оптимизируется в 0 инструкций
```

Классическая ошибка — «type punning через reinterpret_cast». Работает на практике на многих компиляторах, но формально UB и **реально ломается** при `-O2` (компилятор оптимизирует, полагаясь на strict aliasing).

Другой частый анти-пример:

```cpp
struct A { int x; };
struct B { int y; };

A a{5};
B* b = reinterpret_cast<B*>(&a);
b->y;   // ⚠️ UB, хотя layout идентичен
```

---

## `const_cast` — снятие/добавление const (и volatile)

Единственный каст, умеющий убирать `const`/`volatile`.

```cpp
const int x = 5;
int& r = const_cast<int&>(x);
```

### Критическое правило

**Модификация объекта, который изначально был объявлен `const` — UB.** Снимать const безопасно только если исходный объект **не** const:

```cpp
int y = 5;
const int& cr = y;
const_cast<int&>(cr) = 10;   // ✅ ОК: y изначально НЕ const

const int z = 5;
const_cast<int&>(z) = 10;    // ⚠️ UB! z объявлен const —
                              //    мог быть размещён в read-only памяти,
                              //    компилятор мог заинлайнить значение 5 везде
```

### Легитимные применения

**1. Устранение дублирования const/non-const перегрузок** (идиома Мейерса — мы разбирали):

```cpp
class Buffer {
    std::vector<int> data_;
public:
    const int& at(size_t i) const {   // вся логика здесь
        if (i >= data_.size()) throw std::out_of_range("");
        return data_[i];
    }
    int& at(size_t i) {               // неконстантная делегирует константной
        return const_cast<int&>(std::as_const(*this).at(i));   // ✅ безопасно:
    }                                  //    объект точно не const (мы в non-const методе)
};
```

**2. Legacy C-API без `const`:**

```cpp
void legacy_print(char* s);   // не меняет s, но const забыли

void wrapper(const std::string& s) {
    legacy_print(const_cast<char*>(s.c_str()));   // ⚠️ только если api ТОЧНО не пишет
}
```

Во всех прочих случаях `const_cast` — признак ошибки дизайна: если понадобилось снять const, вероятно, где-то неправильно расставлена константность.

---

## Сводная таблица

|Каст|Проверка|Стоимость|Основное применение|
|---|---|---|---|
|`static_cast`|compile-time|нулевая (или const. offset)|числовые, upcast, downcast без проверки, `void*`→`T*`, `std::move`|
|`dynamic_cast`|**runtime** (RTTI)|обход иерархии|безопасный downcast, cross-cast|
|`reinterpret_cast`|~никакой|нулевая|биты↔адреса, `char*`-просмотр, низкоуровневый код|
|`const_cast`|compile-time|нулевая|снять `const` (легаси API, идиома перегрузок)|

---

## C-style cast `(T)x` — почему не надо

C-style cast пробует по очереди:

1. `const_cast`
2. `static_cast`
3. `static_cast` + `const_cast`
4. `reinterpret_cast`
5. `reinterpret_cast` + `const_cast`

— и берёт первое, что скомпилируется.

Проблемы:

- **Непонятно намерение** — читая `(int*)p`, не знаешь, это безобидное преобразование или опасная переинтерпретация
- **Может незаметно превратиться в `reinterpret_cast`** — например, при рефакторинге типов
- **Не грепается** — нельзя найти все опасные касты в проекте
- **Не умеет `dynamic_cast`** — рантайм-проверки нет вообще

```cpp
Base* p = getBase();
Derived* d = (Derived*)p;   // это static_cast? reinterpret_cast? зависит от иерархии!
                            // при невиртуальном наследовании — static_cast (offset)
                            // при несвязанных типах — reinterpret_cast (UB)
```

**Правило: в C++ коде — только именованные касты.**

---

## Формулировки на собеседовании

**«Чем `static_cast` отличается от `dynamic_cast` при downcast?»** — `static_cast` не проверяет тип в рантайме (если тип неверен → UB, нулевая стоимость); `dynamic_cast` проверяет через RTTI (при неудаче → `nullptr`/`bad_cast`, но дороже).

**«Требования `dynamic_cast`?»** — Полиморфный тип (хотя бы одна виртуальная функция в базе) + включённый RTTI. Иначе ошибка компиляции.

**«Что возвращает `dynamic_cast` при неудаче?»** — Для указателя `nullptr`, для ссылки бросает `std::bad_cast` (ссылку нельзя сделать «пустой»).

**«Когда `reinterpret_cast` безопасен?»** — Практически всегда UB, кроме: round-trip `T*`↔`uintptr_t`, просмотр объекта через `char*`/`unsigned char*`/`std::byte*`, и низкоуровневые сценарии (memory-mapped IO). Для type punning нужен `std::bit_cast` (C++20) или `memcpy`.

**«Что не так с `const_cast` + модификацией?»** — Если объект **изначально** объявлен `const`, изменение через `const_cast` — UB (объект мог быть в read-only памяти, значение могло быть заинлайнено). Безопасно только когда исходный объект не const.

**«Почему не C-style cast?»** — Скрывает намерение, может тихо стать `reinterpret_cast`, не грепается, не умеет рантайм-проверку.

**«Что такое strict aliasing?»** — Правило, что доступ к объекту через указатель несовместимого типа — UB. Компилятор оптимизирует, полагаясь на то, что `int*` и `float*` не указывают на одну память. Нарушение через `reinterpret_cast` даёт код, который «работает» на `-O0` и ломается на `-O2`.

---

Отличие от Java: там всего один каст `(Type)obj`, и он **всегда** проверяется в рантайме (`ClassCastException` при неудаче) — то есть по семантике это `dynamic_cast`. Возможности сделать unchecked downcast или переинтерпретировать биты просто нет — JVM это не позволит. В C++ ты выбираешь между скоростью (`static_cast`, ноль проверок) и безопасностью (`dynamic_cast`, RTTI), и несёшь ответственность за выбор. Классический пример философии «не платить за то, что не используешь».
