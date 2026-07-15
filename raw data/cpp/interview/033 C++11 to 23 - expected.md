[[raw data/cpp/interview/_|<=]]

# `std::expected` (C++23)

## Что это

Тип «**значение или ошибка**»: либо `T` (успех), либо `E` (причина неудачи).

```cpp
#include <expected>

std::expected<int, std::string> parse(const std::string& s) {
    if (s.empty())
        return std::unexpected("empty input");   // ошибка
    if (!isValid(s))
        return std::unexpected("invalid format");
    return std::stoi(s);                          // успех — просто возвращаем T
}

auto r = parse("42");
if (r) {
    std::cout << *r;              // 42
} else {
    std::cout << r.error();       // причина
}
```

Закрывает пробел между `optional` (нет значения — но **почему**?) и исключениями (дорого, невидимо в сигнатуре).

---

## Зачем: три способа сообщить об ошибке

```cpp
// 1. optional — «не получилось», но причина потеряна
std::optional<int> parse(const std::string& s);
if (!parse(s)) { /* почему?? */ }

// 2. исключение — причина есть, но не видна в сигнатуре, дорогая раскрутка
int parse(const std::string& s);   // может бросить... что? не видно

// 3. expected — ✅ причина есть, видна в типе, без исключений
std::expected<int, ParseError> parse(const std::string& s);
```

`expected` — это **error code, но типобезопасный и невозможно проигнорировать по ошибке** (в отличие от возврата `int` из C-API).

---

## Устройство

```cpp
template<class T, class E>
class expected {
    bool has_value_;
    union {
        T value_;
        E error_;      // ← ОДНО ИЗ ДВУХ, не оба
    };
};
```

Размер ≈ `max(sizeof(T), sizeof(E))` + дискриминант. **Без аллокаций** — как `optional` и `variant`.

По сути это `variant<T, E>` со специализированным API «успех/ошибка».

---

## API

### Проверка и доступ

```cpp
std::expected<int, std::string> r = parse(s);

if (r) { }                  // ✅ explicit operator bool — есть ли ЗНАЧЕНИЕ
r.has_value();              // то же

*r;                         // ⚠️ БЕЗ проверки → UB, если ошибка
r.value();                  // ✅ бросает std::bad_expected_access<E>, если ошибка
r.value_or(0);              // ✅ значение или дефолт

r.error();                  // ⚠️ БЕЗ проверки → UB, если значение! (симметрично)
```

**Симметрия с `optional`**: `*` и `->` не проверяют, `value()` бросает. Но добавлен `error()` — тоже без проверки.

```cpp
std::expected<int, std::string> r = 42;
r.error();   // ⚠️ UB — есть значение, а не ошибка!
```

### Создание

```cpp
std::expected<int, std::string> ok = 42;                       // успех
std::expected<int, std::string> err = std::unexpected("bad");  // ошибка

// in-place конструирование:
std::expected<std::vector<int>, Err> v{std::in_place, 3, 5};   // vector(3,5)
std::expected<T, Err> e{std::unexpect, args...};               // сконструировать ошибку на месте
```

`std::unexpected<E>` — обёртка, помечающая «это ошибка, не значение». Нужна, чтобы различить случаи, когда `T` и `E` — один тип:

```cpp
std::expected<int, int> r = 42;                  // значение
std::expected<int, int> e = std::unexpected(42); // ошибка — тот же int!
```

### `expected<void, E>` — для функций без результата

```cpp
std::expected<void, std::string> save(const Data& d) {
    if (!file.open()) return std::unexpected("cannot open");
    // ...
    return {};   // ✅ успех, значения нет
}

auto r = save(data);
if (!r) log(r.error());
```

Специализация для `void` — операция либо удалась, либо нет.

---

## Monadic операции — главная сила

Композиция без «лестницы» проверок. Это то, ради чего `expected` и создавался.

```cpp
r.and_then(f)      // если ЗНАЧЕНИЕ → f(value), f возвращает expected  (flatMap)
r.or_else(f)       // если ОШИБКА  → f(error), f возвращает expected
r.transform(f)     // если ЗНАЧЕНИЕ → expected{f(value)}                (map)
r.transform_error(f)  // если ОШИБКА → expected{unexpected(f(error))}   (mapError)
```

### Цепочка

```cpp
std::expected<Config, Error> loadConfig(const std::string& path);
std::expected<Connection, Error> connect(const Config& c);
std::expected<Data, Error> fetch(const Connection& conn);

// ❌ без monadic — лестница
auto cfg = loadConfig(path);
if (!cfg) return std::unexpected(cfg.error());
auto conn = connect(*cfg);
if (!conn) return std::unexpected(conn.error());
auto data = fetch(*conn);
if (!data) return std::unexpected(data.error());
return process(*data);

// ✅ с monadic — плоско
return loadConfig(path)
    .and_then(connect)
    .and_then(fetch)
    .transform(process);        // ошибка на любом шаге → short-circuit, пробрасывается дальше
```

**Ключевое: ошибка короткозамыкает цепочку** — как исключение, но явно и без раскрутки стека.

### Разница `and_then` и `transform`

```cpp
// transform: функция возвращает ОБЫЧНОЕ значение → оборачивается
r.transform([](int x){ return x * 2; });   // expected<int,E> → expected<int,E>

// and_then: функция возвращает EXPECTED → НЕ оборачивается дважды (flatten)
r.and_then([](int x) -> std::expected<int, E> {
    return x > 0 ? std::expected<int,E>{x} : std::unexpected(E{});
});   // expected<int,E>, а не expected<expected<int,E>,E>
```

Это классическая пара `map` / `flatMap` из функционального программирования.

### Обработка ошибок

```cpp
auto result = parse(input)
    .transform_error([](ParseError e) {          // преобразовать тип ошибки
        return AppError{e.message, 400};
    })
    .or_else([](AppError e) -> std::expected<int, AppError> {
        log(e);
        return 0;                                 // ✅ восстановились дефолтом
    });
```

---

## `expected` vs исключения

| |`std::expected`|исключения|
|---|---|---|
|Видно в сигнатуре|✅ явно|❌ (`noexcept` только бинарно)|
|Стоимость успешного пути|~ноль|~ноль (zero-cost при отсутствии throw)|
|Стоимость ошибки|~ноль|**дорого** (раскрутка стека, RTTI)|
|Можно проигнорировать|сложнее (`[[nodiscard]]`)|нет|
|Работает с `-fno-exceptions`|✅|❌|
|Пробрасывается через слои|вручную (или monadic)|автоматически|
|Подходит для|**ожидаемых** ошибок|**исключительных** ситуаций|

**Когда `expected`:**

- Ошибка **ожидаема** и часта (парсинг, валидация, сетевые запросы, поиск)
- Нужен предсказуемый latency (embedded, gamedev, HFT — раскрутка стека недопустима)
- Проект собирается с `-fno-exceptions`
- Ошибку нужно обработать **локально**, не пробрасывая

**Когда исключения:**

- Ситуация действительно **исключительная** (out of memory, повреждённое состояние)
- Ошибка должна пройти **через много слоёв** без явного проброса
- Конструктор не может вернуть значение (единственный способ сообщить об ошибке)

Реальность: большинство C++ проектов используют **оба** — исключения для критических сбоев, `expected` для ожидаемых ошибок бизнес-логики.

---

## `expected` vs `optional` vs `variant`

```cpp
std::optional<int>                  // «значение или ничего» — причина неизвестна
std::expected<int, Error>           // «значение или ОШИБКА» — с причиной
std::variant<int, Error>            // технически то же, но без специализированного API
```

`expected` — это `variant<T, E>` + удобный API:

- `operator bool` (у variant нет — нужен `holds_alternative`)
- `value()` / `error()` вместо `get<0>` / `get<1>`
- **Monadic операции** (у variant нет вообще)
- Семантика асимметрична: `T` — «нормальный» путь, `E` — «исключительный»

Именно ради этого API `expected` и добавили — до C++23 писали `variant<T, Error>` или свои `Result<T,E>` (в каждом крупном проекте был свой).

---

## Ловушки

### 1. `error()` без проверки — UB

```cpp
std::expected<int, std::string> r = 42;
r.error();   // ⚠️ UB! есть значение, ошибки нет
```

Симметрично `*r` при ошибке. Всегда проверяй:

```cpp
if (!r) use(r.error());   // ✅
```

### 2. `[[nodiscard]]` — не забудь проверить результат

```cpp
std::expected<int, Error> parse(const std::string& s);

parse(input);   // ⚠️ результат проигнорирован — ошибка потеряна!
```

`std::expected` помечен `[[nodiscard]]` в стандарте → компилятор выдаст **warning**. Но warning ≠ error — включай `-Werror=unused-result`.

Это главное отличие от C-style error codes: там игнорирование молчаливое, здесь — предупреждение.

### 3. `T` и `E` одного типа

```cpp
std::expected<int, int> r = 42;                   // значение
std::expected<int, int> e = std::unexpected(42);  // ошибка
```

Обязателен `std::unexpected` — иначе неоднозначность. Обычно `E` делают отдельным типом (enum class, struct) — так яснее.

### 4. Размер

```cpp
sizeof(std::expected<BigStruct, SmallError>);   // ~sizeof(BigStruct) — union!
```

Как у `variant` — размер по максимальному. Для больших `E` (например, `std::string` с длинным сообщением) может быть неэффективно; в горячем коде используют компактные коды ошибок (`enum class`).

### 5. Пробрасывание требует явности

```cpp
std::expected<Data, Error> pipeline() {
    auto a = step1();
    if (!a) return std::unexpected(a.error());   // ⚠️ ручной проброс на каждом шаге
    auto b = step2(*a);
    if (!b) return std::unexpected(b.error());
    return step3(*b);
}
```

Вот почему monadic операции важны:

```cpp
std::expected<Data, Error> pipeline() {
    return step1().and_then(step2).and_then(step3);   // ✅ проброс автоматически
}
```

Без monadic-цепочки `expected` многословнее исключений — это его главная цена.

---

## Реалистичный пример

```cpp
enum class ParseError { Empty, InvalidChar, Overflow };

std::expected<int, ParseError> parseInt(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    
    int result = 0;
    for (char c : s) {
        if (!std::isdigit(c)) return std::unexpected(ParseError::InvalidChar);
        if (result > (INT_MAX - (c - '0')) / 10) 
            return std::unexpected(ParseError::Overflow);
        result = result * 10 + (c - '0');
    }
    return result;
}

std::expected<Config, std::string> loadConfig(std::string_view path) {
    return readFile(path)
        .and_then(parseJson)
        .and_then(validateSchema)
        .transform_error([](auto e){ return std::format("config error: {}", e); });
}

// Использование
if (auto cfg = loadConfig("app.json")) {
    run(*cfg);
} else {
    std::cerr << cfg.error();
    return 1;
}
```

---

## Формулировки на собеседовании

**«Что такое `std::expected`?»** — C++23: тип «значение **или ошибка**». Либо `T` (успех), либо `E` (причина). Union внутри → **без аллокаций**, размер = max(T, E) + дискриминант.

**«Чем лучше `optional`?»** — `optional` говорит «значения нет», но **не почему**. `expected` несёт причину ошибки в типе `E`.

**«`expected` или исключения?»** — `expected` для **ожидаемых** ошибок (парсинг, валидация, сеть): видно в сигнатуре, ноль стоимости на ошибке, работает без исключений. Исключения — для **исключительных** ситуаций и когда ошибка проходит через много слоёв (автоматический проброс). Обычно используют оба.

**«Что такое monadic операции?»** — `and_then` (flatMap: функция возвращает expected), `transform` (map: функция возвращает значение), `or_else` (обработать ошибку), `transform_error` (преобразовать тип ошибки). Позволяют строить **плоские цепочки** с автоматическим short-circuit при ошибке.

**«В чём разница `and_then` и `transform`?»** — `transform` оборачивает результат функции в `expected` (map); `and_then` ожидает, что функция **сама вернёт** `expected`, и не оборачивает дважды (flatMap/flatten).

**«Чем `expected<T,E>` отличается от `variant<T,E>`?»** — Технически похожи, но `expected` даёт специализированный API: `operator bool`, `value()`/`error()`, **monadic операции**, и асимметричную семантику (T — успех, E — ошибка). У `variant` этого нет.

**«Что даёт `[[nodiscard]]` у `expected`?»** — Предупреждение, если результат проигнорирован → ошибка не потеряется молча (в отличие от C-style error codes).

---

Отличие от Java: там **нет** аналога в стандартной библиотеке. Есть checked exceptions (`throws IOException`) — они делают ошибку видимой в сигнатуре, но пробрасывание всё равно через исключения (дорого) и они широко критикуются (заставляют либо ловить, либо объявлять — «exception fatigue»). Библиотечные `Either<L,R>` / `Result<T,E>` есть в Vavr и подобных, но не в стандарте. Ближайшая аналогия по духу: Rust `Result<T, E>` — там это **основной** механизм ошибок (исключений нет вообще), с оператором `?` для автоматического проброса. C++ `expected` — прямое заимствование этой модели, но без синтаксического сахара `?`, поэтому ручной проброс многословен и monadic-цепочки становятся практической необходимостью. Раз ты изучаешь Rust — заметь, что `expected` — это буквально `Result`, а `and_then`/`or_else` — те же комбинаторы; разница в том, что в Rust компилятор **заставляет** обработать `Result` (warning на `#[must_use]`), а в C++ это только `[[nodiscard]]`-предупреждение.
