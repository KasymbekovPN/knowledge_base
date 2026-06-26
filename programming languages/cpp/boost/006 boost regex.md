---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Regex

Boost.Regex — регулярные выражения. **Требует линковки** (`Boost::regex`), хотя есть и header-only режим. Прообраз `std::regex`, но часто быстрее и богаче по функциям.

```cpp
#include <boost/regex.hpp>
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS regex)
target_link_libraries(app PRIVATE Boost::regex)
```

## 1. Класс `basic_regex` (и типы-обёртки)

Скомпилированное регулярное выражение.

|Тип|Описание|
|---|---|
|`boost::regex`|Для `char` (= `basic_regex<char>`)|
|`boost::wregex`|Для `wchar_t`|

### Конструкторы

|Конструктор|Описание|
|---|---|
|`regex()`|Пустое выражение|
|`regex(const char* expr)`|Из строки шаблона|
|`regex(const string& expr)`|Из `std::string`|
|`regex(expr, flag_type flags)`|С флагами синтаксиса/поведения|

### Методы

|Метод|Описание|
|---|---|
|`mark_count()`|Число групп захвата|
|`flags()`|Текущие флаги|
|`str()`|Исходный текст шаблона|
|`empty()`|Пустое ли выражение|
|`assign(expr, flags)`|Переназначить шаблон|

```cpp
boost::regex re(R"((\d{4})-(\d{2})-(\d{2}))"); // дата YYYY-MM-DD
std::cout << re.mark_count() << "\n";          // 3 группы
```

## 2. Результаты совпадений: `match_results`

Хранит результат и группы захвата.

|Тип|Описание|
|---|---|
|`boost::smatch`|Результаты для `std::string`|
|`boost::cmatch`|Результаты для `const char*`|
|`boost::wsmatch` / `boost::wcmatch`|Для wide-строк|

### Методы

|Метод|Описание|
|---|---|
|`size()`|Число подвыражений (групп + всё совпадение)|
|`operator[](n)`|n-я группа (`[0]` — всё совпадение) как `sub_match`|
|`str(n)`|n-я группа как строка|
|`position(n)`|Позиция n-й группы в исходной строке|
|`length(n)`|Длина n-й группы|
|`prefix()`|Текст до совпадения|
|`suffix()`|Текст после совпадения|
|`empty()`|Было ли совпадение|
|`format(fmt)`|Подстановка по форматной строке|

`operator[]` возвращает `sub_match` — у него поля `first`, `second` (итераторы), `matched` (bool), методы `str()`, `length()`.

## 3. Основные алгоритмы

### `regex_match` — совпадение со **всей** строкой

```cpp
boost::regex re(R"(\d{3}-\d{4})");
bool ok = boost::regex_match("123-4567", re);   // true
bool no = boost::regex_match("abc 123-4567", re); // false (есть лишний текст)
```

> Требует, чтобы выражение покрыло строку **целиком**.

### `regex_search` — поиск совпадения **где-либо** в строке

```cpp
boost::regex re(R"((\w+)@(\w+))");
std::string text = "email: user@example";
boost::smatch m;
if (boost::regex_search(text, m, re)) {
    std::cout << "Всё: "    << m[0] << "\n"; // user@example
    std::cout << "Лог.: "   << m[1] << "\n"; // user
    std::cout << "Домен: "  << m[2] << "\n"; // example
    std::cout << "До: '"    << m.prefix() << "'\n"; // 'email: '
}
```

### `regex_replace` — замена

```cpp
boost::regex re(R"(\s+)");
std::string result = boost::regex_replace(
    std::string("a   b    c"), re, " ");        // "a b c"

// С обратными ссылками в формате ($1, $2 ...)
boost::regex date(R"((\d{4})-(\d{2})-(\d{2}))");
std::string out = boost::regex_replace(
    std::string("2025-01-15"), date, "$3/$2/$1"); // "15/01/2025"
```

|Флаг формата|Описание|
|---|---|
|`format_all`|Поддержка расширенного синтаксиса замены|
|`format_no_copy`|Не копировать неподходящие части|
|`format_first_only`|Заменить только первое совпадение|
## 4. Итераторы

### `regex_iterator` — перебор всех совпадений

|Тип|Для|
|---|---|
|`boost::sregex_iterator`|`std::string`|
|`boost::cregex_iterator`|`const char*`|

```cpp
boost::regex re(R"(\d+)");
std::string text = "a1 bb22 ccc333";
auto begin = boost::sregex_iterator(text.begin(), text.end(), re);
auto end   = boost::sregex_iterator();
for (auto it = begin; it != end; ++it) {
    std::cout << it->str() << "\n"; // 1, 22, 333
}
```

### `regex_token_iterator` — перебор групп/токенов (в т.ч. для split)

```cpp
boost::regex sep(R"(\s*,\s*)");
std::string csv = "one, two ,three,  four";
// -1 означает "выдавать части МЕЖДУ совпадениями" → split
boost::sregex_token_iterator it(csv.begin(), csv.end(), sep, -1), end;
for (; it != end; ++it)
    std::cout << "[" << *it << "]\n"; // [one][two][three][four]
```

## 5. Флаги

### Флаги синтаксиса (`boost::regex::flag_type`)

|Флаг|Описание|
|---|---|
|`boost::regex::perl` / `ECMAScript`|Perl-синтаксис (по умолчанию)|
|`boost::regex::extended`|POSIX extended|
|`boost::regex::basic`|POSIX basic|
|`boost::regex::icase`|Без учёта регистра|
|`boost::regex::nosubs`|Не сохранять группы захвата|
|`boost::regex::optimize`|Оптимизировать под скорость поиска|
|`boost::regex::collate`|Учитывать локаль в диапазонах|

```cpp
boost::regex re("hello", boost::regex::icase); // регистронезависимо
```

### Флаги сопоставления (`boost::match_flag_type`)

|Флаг|Описание|
|---|---|
|`boost::match_default`|По умолчанию|
|`boost::match_not_bol` / `match_not_eol`|`^`/`$` не совпадают на границах|
|`boost::match_continuous`|Совпадение должно начинаться с начала|
|`boost::match_any`|Любое из возможных совпадений|
|`boost::format_perl` / `format_sed`|Стиль форматной строки замены|
## 6. Обработка ошибок

|Сущность|Описание|
|---|---|
|`boost::regex_error`|Исключение при некорректном шаблоне|
|`.code()`|Код ошибки (`regex_constants::error_type`)|

```cpp
try {
    boost::regex bad("(unclosed");
} catch (const boost::regex_error& e) {
    std::cerr << "Ошибка regex: " << e.what() << "\n";
}
```

## Сводка алгоритмов

|Нужно|Функция|
|---|---|
|Проверить, что строка целиком соответствует|`regex_match`|
|Найти совпадение внутри строки|`regex_search`|
|Заменить совпадения|`regex_replace`|
|Перебрать все совпадения|`sregex_iterator`|
|Разбить строку / извлечь токены|`sregex_token_iterator`|

## Отличия от `std::regex`

- API почти идентичен: `std::regex` стандартизирован на основе Boost.Regex. Замена обычно сводится к смене `boost::` на `std::`.
- **Производительность:** Boost.Regex часто заметно быстрее, чем реализации `std::regex` (особенно старые в libstdc++/MSVC) — частая причина оставаться на Boost.
- **Юникод:** Boost интегрируется с ICU (`u32regex`, заголовок `<boost/regex/icu.hpp>`) для полноценной Unicode-обработки — у `std::regex` такого нет.
- **Синтаксисы:** Boost поддерживает больше грамматик (Perl, POSIX basic/extended, grep, egrep).
- Имена типов совпадают: `regex`, `smatch`, `sregex_iterator`, флаги в `regex_constants` похожи.
- Требует линковки; `std::regex` — часть стандартной библиотеки.

### include/test_regex.h
```cpp
#pragma once  
  
namespace test_regex {  
    void test();  
}
```

### src/test_regex.cpp
```cpp
#include "test_regex.h"  
  
#include <boost/regex.hpp>  
#include <iostream>  
#include <format>  
  
namespace test_regex {  
  
void test() {  
    std::string log =  
        "2025-01-15 ERROR disk full\n"  
        "2025-01-16 INFO ok\n"  
        "2025-01-17 ERROR timeout\n";  
  
    boost::regex re{R"((\d{4}-\d{2}-\d{2})\s+(ERROR|INFO)\s+([^\n]+))"};  
    auto begin = boost::sregex_iterator(log.begin(), log.end(), re);  
    auto end = boost::sregex_iterator();  
  
    for (auto it = begin; it != end; ++it) {  
        const boost::smatch& match = *it;  
        if (match[2] == "ERROR") {  
            std::cout << std::format("{} -> {}\n", match[1].str(), match[3].str());  
        }    }}  
  
}
```

```
2025-01-15 -> disk full
2025-01-17 -> timeout
```
