[[raw data/cpp/interview/_|<=]]

# `std::string_view` (C++17)

## Что это

**Невладеющий view** на непрерывную последовательность символов. Два поля: указатель + размер.

```cpp
#include <string_view>

std::string_view sv = "hello";   // указывает на строковый литерал, НЕ копирует

// Устройство:
class string_view {
    const char* data_;
    size_t size_;
};
sizeof(std::string_view);   // 16 на 64-bit (два слова)
```

Копирование `string_view` — O(1) (два слова). Копирование `std::string` — O(n) + возможная аллокация.

---

## Проблема, которую решает

**Без `string_view` любая функция, принимающая строку, вынуждает к компромиссу:**

```cpp
// ❌ по значению — копия ВСЕГДА
void process(std::string s);
process("hello");        // ⚠️ создаётся временный string → аллокация

// ❌ const& — лучше, но всё равно временный из литерала
void process(const std::string& s);
process("hello");        // ⚠️ const char* → создаётся ВРЕМЕННЫЙ std::string → аллокация!
                         //    (const& продлевает жизнь, но объект всё равно создан)

// ❌ const char* — теряем size(), не работает с std::string напрямую
void process(const char* s);
process(myString);       // ❌ нужен .c_str()
                         //    внутри — strlen() O(n) при каждом обращении к длине

// ✅ string_view — работает со всем, БЕЗ аллокаций
void process(std::string_view sv);
process("hello");        // ✅ ноль аллокаций
process(myString);       // ✅ ноль аллокаций (неявное преобразование)
process({buf, len});     // ✅ произвольный буфер
```

Это главное применение: **параметр функции для строк только для чтения**.

---

## Конструирование

```cpp
std::string_view a = "hello";                 // из литерала
std::string_view b = myString;                // из std::string (неявно)
std::string_view c(buffer, length);           // из указателя + длины
std::string_view d = "hello"sv;               // литерал (C++17: using namespace std::literals)

// НЕ копирует данные ни в одном случае
```

Неявное преобразование `std::string` → `string_view` — ключ к удобству. Обратное (`string_view` → `std::string`) — **явное**, чтобы копия была видна:

```cpp
std::string_view sv = "hello";
std::string s(sv);                      // ✅ явный конструктор — копирует
std::string s2 = sv;                    // ❌ ОШИБКА — explicit
std::string s3 = std::string(sv);       // ✅
```

---

## API — почти как у `std::string`, но только чтение

```cpp
std::string_view sv = "hello world";

sv.size(), sv.length();
sv.empty();
sv[0];               // 'h'
sv.at(0);            // с проверкой
sv.front(), sv.back();
sv.data();           // ⚠️ const char* — БЕЗ гарантии \0!

sv.substr(0, 5);     // ✅ "hello" — O(1)! просто новый view
sv.find("world");    // 6
sv.starts_with("hello");   // C++20
sv.ends_with("world");     // C++20
sv.contains("lo w");       // C++23

sv.remove_prefix(6);       // ✅ сдвигает начало → "world"
sv.remove_suffix(5);       // ✅ сдвигает конец
```

### `substr` — O(1) вместо O(n)

Главный выигрыш в производительности:

```cpp
std::string s = "hello world";
auto sub = s.substr(0, 5);        // ⚠️ КОПИЯ — аллокация + O(n)

std::string_view sv = s;
auto subv = sv.substr(0, 5);      // ✅ O(1) — просто сдвиг указателя + размер
```

Отсюда — токенизация без аллокаций:

```cpp
std::vector<std::string_view> split(std::string_view s, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (start < s.size()) {
        size_t end = s.find(delim, start);
        if (end == std::string_view::npos) end = s.size();
        result.push_back(s.substr(start, end - start));   // ✅ ноль аллокаций на подстроку
        start = end + 1;
    }
    return result;
}
```

Классический парсер на `std::string` аллоцировал бы на каждый токен.

---

## Ловушка №1: dangling — главная опасность

`string_view` **не владеет** данными → легко пережить источник.

```cpp
// ❌ Возврат view на локальную строку
std::string_view bad() {
    std::string s = "hello";
    return s;              // ⚠️ s умрёт → view повиснет → UB
}

// ❌ View на временный объект
std::string_view sv = getString();          // ⚠️ временный string умирает в конце выражения
std::cout << sv;                             // ⚠️ UB

std::string_view sv2 = s1 + s2;              // ⚠️ временный результат конкатенации умрёт
// (lifetime extension НЕ работает — sv не ссылка, а объект!)

// ❌ View на строку, которая изменилась
std::string s = "hello";
std::string_view sv = s;
s = "a much longer string that causes reallocation";   // ⚠️ реаллокация → sv повис
std::cout << sv;                                        // ⚠️ UB
```

Последний случай особенно коварен: `string` с SSO (small string optimization) может не аллоцировать для коротких строк, но при росте — реаллоцирует, и все `string_view` на неё умирают. Точно как итераторы `vector` (мы разбирали инвалидацию).

**Правило: `string_view` — для параметров и локальных переменных в пределах одного выражения/области. Никогда не храни в членах класса и не возвращай из функций** (если не уверен в времени жизни источника).

```cpp
// ❌ ОПАСНО
class Config {
    std::string_view name_;   // ⚠️ на что указывает? кто владеет?
public:
    Config(std::string_view n) : name_(n) { }   // ⚠️ источник может умереть
};

// ✅ БЕЗОПАСНО
class Config {
    std::string name_;                            // ✅ владеет
public:
    Config(std::string_view n) : name_(n) { }     // ✅ копирует при конструировании
};
```

Идиома: **принимай `string_view`, храни `string`.**

---

## Ловушка №2: нет гарантии `\0`

`string_view` может указывать на **подстроку** — там нет нулевого терминатора.

```cpp
std::string s = "hello world";
std::string_view sv = std::string_view(s).substr(0, 5);   // "hello", но БЕЗ \0!

printf("%s", sv.data());   // ⚠️ UB! напечатает "hello world" (до реального \0)
                            //    или уйдёт за границы

// ✅ Правильно:
printf("%.*s", (int)sv.size(), sv.data());   // ✅ явно указываем длину
std::cout << sv;                              // ✅ operator<< знает size()
std::string(sv).c_str();                      // ✅ копия с \0 (аллокация)
```

**`data()` у `string_view` ≠ `c_str()` у `string`.** У `string_view` вообще **нет** `c_str()` — именно потому, что гарантии `\0` нет.

Отсюда: **`string_view` не годится для C-API**, ожидающих null-terminated строку.

```cpp
void legacy(const char* s);   // ожидает \0

void wrapper(std::string_view sv) {
    legacy(sv.data());              // ⚠️ UB, если sv — подстрока!
    legacy(std::string(sv).c_str()); // ✅ копия (аллокация — цена совместимости)
}
```

---

## Ловушка №3: сравнение и хеширование

```cpp
std::string_view a = "hello";
std::string_view b = "hello";
a == b;   // ✅ true — сравнивается СОДЕРЖИМОЕ, не указатели

std::string s = "hello";
s == a;   // ✅ true — есть кросс-сравнение
```

Хеш согласован со `std::string`:

```cpp
std::hash<std::string>{}("hello") == std::hash<std::string_view>{}("hello");   // ✅ равны
```

Но это **не** значит, что можно искать `string_view` в `map<std::string, V>`:

```cpp
std::map<std::string, int> m;
std::string_view key = "hello";
m.find(key);   // ⚠️ создаст ВРЕМЕННЫЙ std::string → аллокация!
```

**Решение — heterogeneous lookup (C++14/C++20):**

```cpp
// map/set — C++14:
std::map<std::string, int, std::less<>> m;   // ✅ std::less<> — прозрачный компаратор
m.find(std::string_view("hello"));            // ✅ БЕЗ аллокации

// unordered_map — C++20:
struct StringHash {
    using is_transparent = void;   // ✅ маркер прозрачности
    size_t operator()(std::string_view sv) const { return std::hash<std::string_view>{}(sv); }
};
std::unordered_map<std::string, int, StringHash, std::equal_to<>> um;
um.find(std::string_view("hello"));   // ✅ без аллокации
```

Это частый практический вопрос — «как искать по `string_view` в map со `string`-ключами без копии».

---

## Ловушка №4: перегрузки становятся неоднозначными

```cpp
void f(const std::string& s);
void f(std::string_view sv);

f("hello");   // ❌ ambiguous — оба подходят через преобразование
```

**Правило: не перегружай по `string`/`string_view`.** Выбери одно — обычно `string_view`.

---

## `string_view` vs `const string&`

| |`const std::string&`|`std::string_view`|
|---|---|---|
|Из литерала|⚠️ создаёт временный string (аллокация)|✅ ноль аллокаций|
|Из `std::string`|✅ ноль|✅ ноль|
|Из `char*` + длина|❌ нужен string|✅|
|`substr`|O(n) + аллокация|**O(1)**|
|Гарантия `\0`|✅ (`c_str()`)|❌|
|Dangling риск|низкий (ссылка)|**высокий**|
|Размер|8 (ссылка)|16 (ptr + size)|
|Владение|нет (но источник обычно жив)|нет|

**Правило: `std::string_view` — дефолт для параметров-строк только для чтения.** `const string&` — если нужен `c_str()` для C-API или гарантированный `\0`.

---

## `string_view` vs `span<const char>`

`string_view` — это по сути `span<const char>` + строковые операции (`find`, `starts_with`, сравнение, хеш, `operator<<`).

```cpp
std::string_view      // строки: find, substr, сравнение, хеш
std::span<const char> // байты: просто последовательность
```

Для текста — `string_view`, для бинарных данных — `span<const std::byte>`.

---

## Практические паттерны

```cpp
// ✅ Параметр функции — дефолт
void log(std::string_view message);
bool startsWith(std::string_view s, std::string_view prefix);

// ✅ Парсинг без аллокаций
std::string_view trim(std::string_view s) {
    while (!s.empty() && std::isspace(s.front())) s.remove_prefix(1);
    while (!s.empty() && std::isspace(s.back()))  s.remove_suffix(1);
    return s;   // ✅ O(1), ноль аллокаций
}

// ✅ Constexpr-строки
constexpr std::string_view NAME = "app";
static_assert(NAME.size() == 3);   // ✅ compile-time

// ❌ НЕ храни в членах (если не гарантируешь время жизни)
class Bad { std::string_view name_; };     // ⚠️
class Good { std::string name_; };         // ✅
```

---

## Формулировки на собеседовании

**«Что такое `string_view`?»** — Невладеющий view на непрерывную последовательность символов: указатель + размер. Копирование O(1), `substr` O(1), **ноль аллокаций**.

**«Зачем, если есть `const string&`?»** — При передаче литерала или `char*` в `const string&` создаётся **временный `std::string`** → аллокация. `string_view` работает со всем без копий, и его `substr` — O(1) вместо O(n).

**«Главная опасность?»** — **Dangling**: не владеет данными. Нельзя возвращать view на локальную строку, привязывать к временному объекту, или держать после реаллокации источника. Идиома: **принимай `string_view`, храни `string`**.

**«Почему у `string_view` нет `c_str()`?»** — Нет гарантии нулевого терминатора: view может указывать на **подстроку**. `data()` даёт указатель, но передавать его в C-API — **UB**. Нужен `\0` → копируй в `std::string`.

**«Как искать `string_view` в `map<string, V>` без копии?»** — **Heterogeneous lookup**: `std::map<std::string, V, std::less<>>` (C++14) или для `unordered_map` — кастомный хеш с `using is_transparent = void;` (C++20).

**«Можно ли перегружать по `string` и `string_view`?»** — Нет: вызов с литералом станет **ambiguous** (оба подходят). Выбирай одно, обычно `string_view`.

---

Отличие от Java: там `String` **иммутабелен**, и `substring()` до Java 7 возвращал **view** на тот же char-массив (O(1), без копии) — ровно как `string_view`. Но это вызывало **утечки памяти**: маленькая подстрока держала весь исходный массив живым. В Java 7u6 `substring()` изменили на **копирующий** (O(n)) именно из-за этой проблемы. C++ `string_view` имеет ту же характеристику (view на чужие данные), но **не** держит источник живым — вместо утечки получаем **dangling**. Это очень показательное различие: GC-язык платит за view утечкой, C++ — use-after-free. Отсюда и правило «принимай view, храни владеющий тип» — оно решает обе проблемы. Ещё: в Java `String` иммутабелен, поэтому view всегда валиден по содержимому; в C++ `std::string` **мутабелен**, и изменение источника инвалидирует `string_view` — дополнительная опасность, которой в Java просто нет.
