[[raw data/cpp/interview/_|<=]]

# Move constructor / assignment: когда генерируются

## Пять специальных функций и их взаимозависимость

Компилятор может неявно сгенерировать пять функций. Их генерация **связана**: наличие одних подавляет генерацию других. Move-операции — самые «хрупкие» в этом смысле.

```cpp
class T {
    T();                           // default constructor (отдельная история)
    ~T();                          // destructor
    T(const T&);                   // copy constructor
    T& operator=(const T&);        // copy assignment
    T(T&&) noexcept;               // move constructor
    T& operator=(T&&) noexcept;    // move assignment
};
```

---

## Правила генерации move-конструктора и move-присваивания

**Move-операции генерируются автоматически ТОЛЬКО если пользователь не объявил НИ ОДНОГО из следующего:**

1. copy-конструктор
2. copy-присваивание
3. **другую** move-операцию (move-конструктор ⇄ move-присваивание)
4. деструктор

Если объявлено **хоть что-то** из этого списка — move-операции **не генерируются**.

```cpp
struct A {
    // ничего не объявлено → генерируются ВСЕ пять, включая обе move-операции
    std::string s;
};

struct B {
    ~B() {}   // объявлен деструктор → move-операции НЕ генерируются
    std::string s;
    // копирование ещё генерируется (deprecated, но работает),
    // а move — нет → B будет КОПИРОВАТЬСЯ там, где мог бы перемещаться
};

struct C {
    C(const C&) {}   // объявлен copy-ctor → move-операции НЕ генерируются
};

struct D {
    D(D&&) {}   // объявлен move-ctor → move-ASSIGNMENT не генерируется,
                // а также НЕ генерируются copy-операции (они становятся deleted)
};
```

---

## Симметричная картина: что подавляет что

|Пользователь объявил|copy-ctor|copy-assign|move-ctor|move-assign|
|---|---|---|---|---|
|ничего|✅|✅|✅|✅|
|любой конструктор (не спец.)|✅|✅|✅|✅|
|**деструктор**|✅ (deprecated)|✅ (deprecated)|❌|❌|
|**copy-ctor**|—|✅ (deprecated)|❌|❌|
|**copy-assign**|✅ (deprecated)|—|❌|❌|
|**move-ctor**|❌ (deleted)|❌ (deleted)|—|❌|
|**move-assign**|❌ (deleted)|❌ (deleted)|❌|—|

Три ключевых наблюдения:

1. **Объявление move-операции удаляет copy-операции** (делает их `= delete`). Логично: если тип определил особую семантику перемещения, дефолтное копирование почти наверняка неверно.
    
2. **Объявление copy-операции или деструктора убивает move.** Тип останется копируемым, но не перемещаемым → тихая потеря производительности.
    
3. **«deprecated»** — генерация copy при наличии деструктора или другой copy-операции формально устарела (C++11), но всё ещё работает для обратной совместимости. Компилятор может предупреждать.
    

---

## Самая частая ловушка: деструктор убивает move

Это **любимый вопрос** на собеседовании.

```cpp
class Buffer {
    std::vector<int> data_;
public:
    ~Buffer() { std::cout << "dtor\n"; }   // «безобидный» деструктор
    // move-операции НЕ сгенерированы!
};

Buffer a;
Buffer b = std::move(a);   // ⚠️ вызовется COPY-конструктор, не move!
                           // (copy ещё генерируется, move — нет)
```

Хотя `std::vector` внутри прекрасно перемещается, `Buffer` будет **копироваться**, потому что объявленный деструктор подавил генерацию move. Дорогое копирование вектора вместо дешёвого перемещения — молчаливая деградация.

**Лечение — явно запросить дефолты:**

```cpp
class Buffer {
    std::vector<int> data_;
public:
    ~Buffer() { std::cout << "dtor\n"; }

    Buffer() = default;
    Buffer(const Buffer&) = default;
    Buffer& operator=(const Buffer&) = default;
    Buffer(Buffer&&) noexcept = default;              // явно вернули move
    Buffer& operator=(Buffer&&) noexcept = default;
};
```

Отсюда правило: **объявил одну из пяти — осознанно реши про остальные** (усиление Rule of Five).

---

## `= default` — генерация по запросу

`= default` просит компилятор сгенерировать функцию с дефолтным поведением, даже когда автоматически она бы не сгенерировалась:

```cpp
struct Widget {
    std::unique_ptr<Impl> impl_;   // некопируемый член!

    Widget(Widget&&) noexcept = default;             // ✅ поэлементный move членов
    Widget& operator=(Widget&&) noexcept = default;
    // copy НЕ запрашиваем — unique_ptr некопируем, copy был бы deleted
};
```

Дефолтный move-конструктор перемещает **каждый член поэлементно** (member-wise move) в порядке объявления. Дефолтное move-присваивание — аналогично move-присваивает члены.

---

## Когда сгенерированный move оказывается deleted

`= default` может дать **deleted**-функцию, если хоть один член/база не перемещаемы:

```cpp
struct HasReference {
    int& ref_;   // ссылки нельзя переприсвоить
    HasReference(HasReference&&) = default;   // будет deleted (ссылку не move-assign'нуть для assignment;
                                              // для конструктора ссылка копируется — но assignment сломан)
};

struct HasConst {
    const int x_;
    HasConst& operator=(HasConst&&) = default;   // deleted: const-член нельзя присвоить
};
```

Move считается «удалённым», если любой нестатический член или базовый класс не может быть перемещён соответствующей операцией.

---

## Тонкость: move как оптимизация copy, а не отдельная семантика

Если move не сгенерирован/deleted, но copy доступен — `std::move(x)` **не ошибка**, просто вызовется copy:

```cpp
struct OnlyCopyable {
    std::string s;
    OnlyCopyable(const OnlyCopyable&) = default;
    OnlyCopyable(OnlyCopyable&&) = delete;   // move явно удалён
};
```

А вот тут важный нюанс: если move **явно** `= delete`, то он **участвует в разрешении перегрузки** и, будучи выбранным для rvalue, даёт **ошибку** (а не откат на copy):

```cpp
OnlyCopyable a;
OnlyCopyable b = std::move(a);   // ❌ ошибка: move-ctor deleted, но он лучший кандидат для rvalue
```

Разница между «move не объявлен» (откат на copy) и «move = delete» (ошибка) — тонкий, но реальный вопрос на собеседовании.

---

## Почему move должен быть `noexcept`

Не про генерацию, но критично рядом. Контейнеры используют move при реаллокации **только если он `noexcept`** — иначе откатываются на copy ради strong exception guarantee:

```cpp
class Widget {
    std::vector<int> data_;
public:
    Widget(Widget&&) noexcept = default;   // БЕЗ noexcept vector будет копировать при росте!
};

std::vector<Widget> v;
v.push_back(...);   // при реаллокации: noexcept move → перемещение; иначе → копирование
```

Дефолтные move-операции получают `noexcept` автоматически, если все поэлементные move — `noexcept`. Но если пишешь свой move — не забудь `noexcept` явно, иначе `vector` тихо перейдёт на копирование.

---

## Полная сводка правил генерации

**Move-конструктор генерируется, если:**

- не объявлены: copy-ctor, copy-assign, move-assign, деструктор
- все члены и базы move-конструируемы

**Move-присваивание генерируется, если:**

- не объявлены: copy-ctor, copy-assign, move-ctor, деструктор
- все члены и базы move-присваиваемы

**Объявление любой move-операции:**

- подавляет генерацию **другой** move-операции
- делает **copy-операции deleted**

---

## Формулировки на собеседовании

**«Когда компилятор генерирует move-конструктор?»** — когда пользователь не объявил ни copy-ctor, ни copy-assign, ни move-assign, ни деструктор, и все члены перемещаемы.

**«Почему объявление деструктора мешает move?»** — правило Rule of Five: деструктор — признак ручного управления ресурсом, компилятор перестаёт доверять дефолтному move (мог бы оставить два владельца одного ресурса), поэтому не генерирует его; тип молча становится только-копируемым.

**«Что происходит с copy при объявлении move-конструктора?»** — copy-операции становятся deleted.

**«В чём разница между "move не сгенерирован" и "move = delete"?»** — в первом случае rvalue откатывается на copy; во втором deleted-move участвует в перегрузке, выбирается для rvalue и даёт ошибку компиляции.

**«Зачем move noexcept?»** — чтобы контейнеры использовали перемещение (а не копирование) при реаллокации, сохраняя strong exception guarantee.

---

Идиоматичный вывод: следуй **Rule of Zero** (не объявляй ничего, оборачивай ресурсы в готовые RAII-типы) — тогда все пять функций сгенерируются корректно. А если вынужден объявить хоть одну (например, полиморфный базовый класс требует `virtual ~Base()`), явно `= default` остальные, чтобы не потерять move.

---

## Неделя 1: Core C++ и память

**День 1–2: Управление памятью и RAII**

- [x] Stack vs Heap, lifetime объектов (2026.07.10)
- [x] RAII, правило 0/3/5 (2026.07.10)
- [x] Smart pointers: `unique_ptr`, `shared_ptr`, `weak_ptr` — внутреннее устройство, control block, циклические ссылки (2026.07.10)
- [x] `make_shared` vs `make_unique` (2026.07.10)

**День 3–4: Move-семантика и rvalue**

- [x] lvalue/rvalue/xvalue, `std::move`, `std::forward` (2026.07.10)
- [x] Perfect forwarding, universal (forwarding) references (2026.07.10)
- [x] RVO/NRVO, copy elision (гарантии C++17) (2026.07.10)
- [x] Move constructor/assignment, когда генерируются (2026.07.10)

**День 5: Объектная модель и виртуальность**

- [ ] vtable/vptr, как работает виртуальный вызов
- [ ] Виртуальный деструктор (зачем)
- [ ] Множественное и виртуальное наследование, ромб
- [ ] `override`, `final`, slicing

**День 6: const и type system**

- [ ] `const`/`constexpr`/`consteval`/`constinit`
- [ ] const correctness, `mutable`
- [ ] Касты: `static_cast`, `dynamic_cast`, `reinterpret_cast`, `const_cast`

**День 7: Повтор + написать руками** unique_ptr, shared_ptr, vector (push_back с реаллокацией).

## Неделя 2: STL, шаблоны, многопоточность

**День 8–9: STL контейнеры и алгоритмы**

- [ ] Сложность операций: `vector`, `deque`, `map`/`set` (RB-tree), `unordered_map` (хеши, коллизии, rehash)
- [ ] Инвалидация итераторов (важный вопрос)
- [ ] Алгоритмы: `<algorithm>`, итераторные категории
- [ ] `emplace` vs `insert`, `reserve`

**День 10–11: Шаблоны**

- [ ] Function/class templates, специализация (полная/частичная)
- [ ] SFINAE, `enable_if`, базовые concepts (C++20)
- [ ] Variadic templates, fold expressions
- [ ] CRTP, type traits

**День 12–13: Многопоточность**

- [ ] `std::thread`, `jthread`, `async`/`future`/`promise`
- [ ] `mutex`, `lock_guard`, `unique_lock`, `scoped_lock`, deadlock
- [ ] `condition_variable`, spurious wakeup
- [ ] Memory model, `std::atomic`, memory_order (хотя бы acquire/release)
- [ ] Data race vs race condition

**День 14: Повтор** + thread-safe queue, простой thread pool.

## Неделя 3: Современный C++, UB, практика

**День 15: Modern C++ обзор**

- [ ] C++11→23: structured bindings, `auto`, lambdas (capture by value/ref, mutable), ranges, `optional`/`variant`/`expected`, `string_view`, `span`

**День 16: Undefined Behavior**

- [ ] Классика: dangling, use-after-free, signed overflow, strict aliasing, нарушение порядка вычислений
- [ ] Почему UB опасно для оптимизаций

**День 17–18: Алгоритмические задачи**

- [ ] LeetCode medium на C++ (важно: писать чисто, с STL)
- [ ] Акцент на корректную работу с памятью/итераторами

**День 19: System design / архитектура** (как тимлид это твоя сильная зона — освежи именно C++-специфику: pImpl, header/cpp разделение, ABI, линковка, ODR)

**День 20: Build/tooling**

- [ ] Компиляция/линковка, ODR violations
- [ ] CMake базово, sanitizers (ASan/UBSan/TSan), valgrind
- [ ] `-O` уровни, что компилятор оптимизирует

**День 21: Mock-собеседование** — прорешай вслух, объясняя решения.

---

## Ключевые «любимые» вопросы интервьюеров

- Чем `shared_ptr` thread-safe, а чем нет (счётчик vs объект)
- Что будет при исключении в конструкторе/деструкторе
- `static` инициализация и порядок (static init order fiasco)
- Разница `int*`, `int* const`, `const int*`
- Почему нельзя возвращать ссылку на локальную переменную

## Учитывая твой бэкграунд

Раз последние годы был не на «чистом» C++ — **сделай акцент на том, что отличает C++ от Java**: ручное управление памятью, move-семантика, шаблоны вместо generics (стирание типов vs мономорфизация), отсутствие GC, деструкторы/RAII, UB. Это типичные места, где «java-программиста в C++» ловят.

Хочешь — могу расписать конкретные задачи/материалы под любой из дней или сделать набор флеш-карточек по каверзным вопросам.**