[[raw data/cpp/interview/_|<=]]

# `std::optional` (C++17)

## Что это

Тип-обёртка, представляющий **значение или его отсутствие**: либо есть `T`, либо ничего.

```cpp
#include <optional>

std::optional<int> a = 42;              // есть значение
std::optional<int> b = std::nullopt;    // пусто
std::optional<int> c;                    // пусто (default)
```

Решает классическую проблему: как сообщить «результата нет» без магических значений (`-1`, `nullptr`, `""`) и без исключений.

```cpp
// ❌ до C++17 — костыли
int find(const std::string& key);        // возвращает -1 если не найдено? а если -1 валиден?
bool find(const std::string& key, int& out);   // out-параметр, некрасиво
int* find(const std::string& key);       // nullptr = не найдено, но кто владеет?

// ✅ C++17
std::optional<int> find(const std::string& key);   // явно: может не быть значения
```

---

## Устройство

Никакой динамической аллокации — объект **встроен** в `optional`:

```cpp
template<class T>
class optional {
    bool has_value_;
    union { 
        char dummy_; 
        T value_;      // ← объект ЗДЕСЬ, не на heap
    };
};
```

```cpp
sizeof(std::optional<int>);   // обычно 8 (4 int + 1 bool + padding)
sizeof(std::optional<char>);  // обычно 2
```

Оверхед — **один bool + выравнивание**. Никаких аллокаций, объект строится **на месте** (placement new при присваивании).

Это принципиально: `optional<BigStruct>` содержит `BigStruct` внутри себя, а не указатель на него.

---

## API

### Проверка наличия

```cpp
std::optional<int> opt = 42;

if (opt.has_value()) { }    // явно
if (opt) { }                // ✅ explicit operator bool — идиоматично
if (opt != std::nullopt) { } // тоже работает
```

### Доступ к значению

```cpp
*opt;             // ⚠️ БЕЗ проверки! если пусто → UB
opt.operator->(); // opt->method() — тоже без проверки
opt.value();      // ✅ бросает std::bad_optional_access если пусто
opt.value_or(0);  // ✅ вернуть значение или дефолт — без исключений
```

**Ключевое различие:**

```cpp
std::optional<int> empty;

*empty;                    // ⚠️ UB — как разыменование nullptr
empty.value();             // ✅ std::bad_optional_access
empty.value_or(-1);        // ✅ -1
```

Идиоматичный доступ:

```cpp
if (auto opt = find(key)) {     // ✅ проверка
    use(*opt);                   // ✅ здесь разыменование безопасно
}
```

Или через `if`-init (C++17):

```cpp
if (auto opt = find(key); opt) {
    use(*opt);
}
```

### Модификация

```cpp
std::optional<int> opt;

opt = 42;              // присвоить значение
opt.emplace(42);       // ✅ сконструировать НА МЕСТЕ (perfect forwarding)
opt.reset();           // очистить (вызывает деструктор T)
opt = std::nullopt;    // то же
```

`emplace` — как у контейнеров: конструирует объект из аргументов, без временного:

```cpp
std::optional<std::string> opt;
opt.emplace(5, 'a');   // ✅ строит string(5, 'a') на месте → "aaaaa"
```

### Конструирование на месте — `in_place`

```cpp
std::optional<std::vector<int>> opt{std::in_place, 3, 5};   // ✅ vector(3, 5) — {5,5,5}
// без in_place: optional<vector<int>> opt{{3, 5}};  ← это initializer_list → {3, 5}!
```

`std::in_place` разрешает неоднозначность: «конструируй T из этих аргументов», а не «скопируй готовый T».

---

## Практические применения

### 1. Функция, которая может не вернуть значение

```cpp
std::optional<int> parseInt(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return std::nullopt;   // ✅ явно: не смогли
    }
}

if (auto n = parseInt("42")) {
    std::cout << *n;
}
```

### 2. Опциональные параметры

```cpp
void configure(int port, std::optional<std::string> host = std::nullopt) {
    std::string h = host.value_or("localhost");   // ✅ дефолт
    // ...
}

configure(8080);                    // host по умолчанию
configure(8080, "example.com");     // явно
```

### 3. Ленивая инициализация / отложенное создание

```cpp
class Widget {
    std::optional<ExpensiveObject> cache_;   // ✅ не создаётся до первого использования
public:
    ExpensiveObject& get() {
        if (!cache_) {
            cache_.emplace(computeExpensive());   // ✅ строим на месте
        }
        return *cache_;
    }
};
```

Раньше для этого нужен был `unique_ptr` (с аллокацией) или флаг + raw storage. `optional` даёт это без heap.

Мы упоминали этот паттерн в теме `mutable` — но с оговоркой про thread-safety:

```cpp
class Matrix {
    mutable std::optional<double> cachedDet_;
    mutable std::mutex mtx_;   // ⚠️ обязательно — иначе гонка
public:
    double determinant() const {
        std::lock_guard lock(mtx_);
        if (!cachedDet_) cachedDet_ = compute();
        return *cachedDet_;
    }
};
```

### 4. Члены класса без default-конструктора

```cpp
class NoDefault {
public:
    NoDefault(int x);   // нет default-конструктора
};

class Widget {
    std::optional<NoDefault> obj_;   // ✅ можно не инициализировать сразу
public:
    void init(int x) { obj_.emplace(x); }
};
```

Без `optional` пришлось бы либо `unique_ptr`, либо инициализировать в списке инициализации.

### 5. Возврат из контейнера

Помнишь thread-safe очередь из темы condition_variable?

```cpp
std::optional<T> pop() {
    std::unique_lock lock(mtx_);
    cv_.wait(lock, [this]{ return !queue_.empty() || stopped_; });
    if (queue_.empty()) return std::nullopt;   // ✅ остановлены — значения нет
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
}
```

Вот зачем нужен был `optional` — сообщить «значения нет» без исключений и без out-параметров.

---

## Monadic операции (C++23)

Композиция без вложенных проверок:

```cpp
std::optional<int> opt = 42;

// transform — применить функцию, если есть значение
auto r1 = opt.transform([](int x){ return x * 2; });   // optional<int>{84}

// and_then — применить функцию, ВОЗВРАЩАЮЩУЮ optional (flatMap)
auto r2 = opt.and_then([](int x) -> std::optional<int> {
    return x > 0 ? std::optional{x} : std::nullopt;
});

// or_else — вызвать, если пусто
auto r3 = opt.or_else([]{ return std::optional{0}; });
```

Позволяет строить цепочки без «лестницы» if:

```cpp
// ❌ C++17
auto opt = findUser(id);
if (opt) {
    auto addr = getAddress(*opt);
    if (addr) {
        auto city = getCity(*addr);
        if (city) return *city;
    }
}
return "unknown";

// ✅ C++23
return findUser(id)
    .and_then(getAddress)
    .and_then(getCity)
    .value_or("unknown");
```

---

## Ловушки

### 1. `optional<bool>` — двусмысленность

```cpp
std::optional<bool> flag = false;

if (flag) {              // ⚠️ TRUE! проверяется НАЛИЧИЕ, а не значение
    // сюда попадём, хотя flag == false
}

if (flag.value()) { }    // ✅ проверяем значение
if (*flag) { }           // ✅ (но UB, если пусто)
if (flag && *flag) { }   // ✅ и наличие, и значение
```

Классическая ловушка. `operator bool` у `optional` означает «есть ли значение», а не «истинно ли значение».

То же с `optional<int>`:

```cpp
std::optional<int> n = 0;
if (n) { /* ✅ выполнится — значение ЕСТЬ (оно 0) */ }
```

### 2. `optional<T&>` — НЕ поддерживается

```cpp
std::optional<int&> opt;   // ❌ ОШИБКА — ссылки не поддерживаются
```

Причина: неясная семантика присваивания (переприсвоить ссылку или значение по ссылке?). Комитет отложил решение.

**Обход:**

```cpp
std::optional<std::reference_wrapper<int>> opt;   // ✅ костыль
int* ptr = nullptr;                                // ✅ или просто указатель
```

Для «опциональной ссылки» указатель (`T*`) — идиоматичное решение: он уже nullable.

### 3. Разыменование без проверки

```cpp
std::optional<int> opt = find(key);
int x = *opt;   // ⚠️ UB, если не нашли!
```

`*` и `->` **не проверяют** — как у указателя. Используй `value()` (бросает) или проверяй.

### 4. Размер

```cpp
sizeof(std::optional<BigStruct>);   // sizeof(BigStruct) + bool + padding
```

`optional` **встраивает** объект → для больших типов размер растёт. Если нужен «либо объект, либо ничего» с малым размером — `unique_ptr` (8 байт, но с аллокацией).

### 5. `optional` в `constexpr`

```cpp
constexpr std::optional<int> f() { return 42; }   // ✅ C++17+: optional constexpr-friendly
static_assert(*f() == 42);
```

Работает, но только для литеральных типов.

---

## Когда `optional`, когда альтернативы

|Задача|Решение|
|---|---|
|«Значение или ничего», владение|**`std::optional<T>`**|
|«Значение или **причина** ошибки»|**`std::expected<T, E>`** (C++23)|
|«Один из нескольких типов»|**`std::variant<A, B, C>`**|
|Опциональная **ссылка** (не владеющая)|**`T*`** (указатель)|
|Полиморфный объект или ничего|**`std::unique_ptr<Base>`**|
|Ошибка, которую нельзя игнорировать|**исключение**|

**`optional` vs исключения:**

- `optional` — «отсутствие значения **нормально**» (не нашли ключ, парсинг опционального поля)
- исключение — «это **ошибка**, которую нельзя игнорировать» (не открылся файл, повреждены данные)

**`optional` vs `expected`:**

```cpp
std::optional<int> parse(const std::string& s);              // не смогли — но ПОЧЕМУ?
std::expected<int, ParseError> parse(const std::string& s);  // ✅ известна причина
```

Если причина важна — `expected` (C++23). Если «нет и нет» — `optional`.

---

## Формулировки на собеседовании

**«Что такое `std::optional`?»** — Тип-обёртка «значение или ничего». Объект хранится **внутри** optional (никаких аллокаций), оверхед — `bool` + выравнивание.

**«Чем `optional<T>` лучше указателя?»** — Явно выражает намерение (nullable значение, а не «указатель на что-то»); владеет объектом; не требует аллокации; нельзя случайно забыть про владение/удаление.

**«В чём разница `*opt` и `opt.value()`?»** — `*` и `->` **не проверяют** (пусто → **UB**, как разыменование nullptr). `value()` бросает `std::bad_optional_access`. `value_or(def)` — безопасный дефолт.

**«Что не так с `optional<bool>`?»** — `if (opt)` проверяет **наличие значения**, а не само значение. `optional<bool>{false}` даёт `true` в булевом контексте. Нужно `opt.value()` или `opt && *opt`.

**«Почему нет `optional<T&>`?»** — Неоднозначная семантика присваивания (переприсвоить ссылку или изменить значение?). Комитет не договорился. Для опциональной ссылки используют `T*` или `std::reference_wrapper`.

**«`optional` или исключение?»** — `optional`, когда отсутствие значения — **нормальный** исход (не найден ключ). Исключение — когда это **ошибка**. Если нужна причина ошибки — `std::expected` (C++23).

---

Отличие от Java: там `Optional<T>` (Java 8) — концептуально то же, но: (1) **всегда аллоцирует** (объект на heap, ссылка внутри Optional) — C++ `optional` встраивает значение без heap; (2) в Java `Optional` рекомендуют **только для возвращаемых значений**, не для полей и параметров (официальная позиция Oracle) — в C++ `optional` уместен везде; (3) monadic-операции (`map`, `flatMap`, `orElse`) были **с самого начала**, C++ получил их только в C++23; (4) Java `Optional` не спасает от `null` внутри (`Optional.of(null)` бросит NPE), C++ `optional` этой проблемы не имеет — в C++ нет `null` для значений. Главное для тебя: в Java `Optional` — обёртка над **ссылкой**, в C++ — над **значением**, отсюда все различия в стоимости и применимости.
