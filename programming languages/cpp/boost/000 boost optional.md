---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Optional

Значение, которое может отсутствовать. Аналог `std::optional`.

## Шаблон

```cpp
template <class T> class boost::optional;
```

## Конструкторы

|Конструктор|Описание|
|---|---|
|`optional()`|Пустой optional (без значения)|
|`optional(boost::none_t)`|Явно пустой через `boost::none`|
|`optional(const T& val)`|Инициализация значением|
|`optional(T&& val)`|Move-инициализация значением|
|`optional(bool cond, const T& val)`|Значение если `cond == true`, иначе пусто|
|`optional(const optional& other)`|Копирование|
|`optional(optional&& other)`|Перемещение|
|`optional(InPlaceFactory)`|Конструирование «на месте» (in-place)|

## Доступ к значению

|Метод|Описание|
|---|---|
|`T& get()` / `const T& get()`|Ссылка на значение (UB, если пусто)|
|`T& value()` / `const T& value()`|Значение; бросает `bad_optional_access`, если пусто|
|`T value_or(U&& default)`|Значение или указанное «по умолчанию», если пусто|
|`T value_or_eval(F f)`|Значение или результат вызова `f()`, если пусто|
|`T* get_ptr()` / `const T* get_ptr()`|Указатель на значение или `nullptr`, если пусто|

## Операторы

|Оператор|Описание|
|---|---|
|`operator*()`|Разыменование → ссылка на значение (UB, если пусто)|
|`operator->()`|Доступ к членам значения|
|`explicit operator bool()`|`true`, если значение есть|
|`operator!()`|`true`, если пусто|
|`operator=(const T&)` / `operator=(T&&)`|Присваивание значения|
|`operator=(boost::none_t)`|Сброс в пустое состояние|
|`operator==`, `operator!=`, `operator<`, ...|Сравнения (учитывают пустое состояние)|

## Проверка состояния

|Метод|Описание|
|---|---|
|`bool is_initialized()`|`true`, если значение присутствует|
|`explicit operator bool()`|То же, идиоматичная проверка в `if`|

## Изменение содержимого

|Метод|Описание|
|---|---|
|`void reset()`|Очистить (уничтожить значение)|
|`void reset(const T& val)`|Заменить значение (устаревший стиль, лучше `=`)|
|`T& emplace(Args&&... args)`|Сконструировать значение «на месте» из аргументов|

## Связанные сущности

|Имя|Описание|
|---|---|
|`boost::none`|Константа-маркер пустого состояния (как `std::nullopt`)|
|`boost::none_t`|Тип этой константы|
|`boost::bad_optional_access`|Исключение из `value()` при пустом optional|
|`boost::in_place(args...)`|Фабрика для конструирования значения «на месте»|
|`boost::make_optional(val)`|Создать optional с выводом типа|
|`boost::make_optional(cond, val)`|Условное создание (значение, если `cond`)|
|`boost::get_optional_value_or(opt, def)`|Свободная функция-аналог `value_or`|

## Типичные паттерны

```cpp
#include <boost/optional.hpp>

boost::optional<int> o;                 // пусто
o = 42;                                 // присвоили значение
if (o) { /* есть значение */ }          // operator bool
int a = *o;                             // разыменование
int b = o.value_or(-1);                 // безопасный доступ
o.emplace(100);                         // конструирование на месте
o = boost::none;                        // сброс
o.reset();                              // то же самое

auto m = boost::make_optional(true, 5); // optional<int> = 5
```

## Отличия от `std::optional`

- `value_or_eval(f)` и `get_ptr()` — есть в boost, нет в стандарте.
- `optional<T&>` — boost поддерживает **optional на ссылку**, `std::optional` — нет.
- `boost::none` ↔ `std::nullopt`; `is_initialized()` ↔ `has_value()`.
- In-place: boost исторически через `boost::in_place(...)`, в стандарте — `std::in_place` + `emplace`.


CMake: `find_package(Boost REQUIRED)` + `target_link_libraries(app PRIVATE Boost::boost)` (header-only интерфейс).

### include/test_optional.h
```cpp
#pragma once  

#include <boost/optional.hpp>  
  
namespace test_optional {  
    void test();  
}
```

### src/test_optional.cpp
```cpp
#include "test_optional.h"  
  
#include <iostream>  
#include <format>  
#include <string>  
  
namespace test_optional {  
  
static boost::optional<int> parse(const std::string& _line) {  
    if (_line == "42") { return 42; }  
    return boost::none;  
}  
  
static void print(boost::optional<int> _value) {  
    if (_value) {  
        std::cout << std::format("[boost::optional][HAS VALUE] {}\n", *_value);  
    } else {  
        std::cout << std::format("[boost::optional][NO VALUE] {}\n", _value.value_or(-1));  
    }  
}  
  
void test() {  
    print(parse("42"));  
    print(parse("xyz"));  
}  
  
}
```

```
[boost::optional][HAS VALUE] 42
[boost::optional][NO VALUE] -1
```
