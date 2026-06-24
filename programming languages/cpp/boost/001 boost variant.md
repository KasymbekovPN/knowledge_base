---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Variant

Типобезопасное объединение (один из нескольких типов). Предшественник `std::variant`.

## Шаблон

```cpp
template <class T1, class T2, ..., class TN> class boost::variant;
```

## Конструкторы

|Конструктор|Описание|
|---|---|
|`variant()`|Инициализирует значением **первого** типа (T1 должен быть default-constructible)|
|`variant(const T& val)`|Инициализация значением одного из допустимых типов|
|`variant(T&& val)`|Move-инициализация значением|
|`variant(const variant& other)`|Копирование|
|`variant(variant&& other)`|Перемещение|

## Доступ к значению

|Функция|Описание|
|---|---|
|`boost::get<T>(variant&)`|Ссылка на значение типа `T`; бросает `bad_get`, если хранится другой тип|
|`boost::get<T>(variant*)`|Указатель на значение типа `T` или `nullptr`, если тип другой (без исключения)|
|`boost::get<I>(variant&)`|Доступ по индексу типа (начиная с 0)|
|`boost::strict_get<T>(...)`|Строгий вариант `get` (без неявных преобразований)|

## Информация о состоянии

|Метод|Описание|
|---|---|
|`int which() const`|Индекс (0-based) текущего активного типа в списке|
|`bool empty() const`|Почти всегда `false`; `true` только в редком «singular»-состоянии после исключения|
|`const std::type_info& type() const`|RTTI-тип текущего хранимого значения|

## Изменение содержимого

|Оператор/метод|Описание|
|---|---|
|`operator=(const T&)` / `operator=(T&&)`|Присвоить значение допустимого типа|
|`operator=(const variant&)`|Копирующее присваивание|
|`operator=(variant&&)`|Перемещающее присваивание|
|`void swap(variant&)`|Обмен содержимым|

## Визитация (основной идиоматичный механизм)

|Сущность|Описание|
|---|---|
|`boost::apply_visitor(visitor, variant)`|Применяет visitor к текущему значению|
|`boost::apply_visitor(visitor, v1, v2)`|Бинарная визитация двух variant'ов|
|`boost::apply_visitor(visitor)`|Возвращает «отложенный» вызываемый объект (для алгоритмов STL)|
|`boost::static_visitor<R>`|Базовый класс для visitor'а; `R` — тип возвращаемого значения (по умолчанию `void`)|

Visitor — это объект с перегруженными `operator()` для каждого возможного типа:

```cpp
struct MyVisitor : boost::static_visitor<std::string> {
    std::string operator()(int i) const { return "int"; }
    std::string operator()(double d) const { return "double"; }
    std::string operator()(const std::string& s) const { return "string"; }
};
```

## Операторы сравнения

|Оператор|Описание|
|---|---|
|`operator==`, `operator!=`|Равны, если совпадают активный тип и значение|
|`operator<`, `operator<=`, `operator>`, `operator>=`|Сначала по `which()`, затем по значению|

## Связанные сущности

|Имя|Описание|
|---|---|
|`boost::bad_get`|Исключение при неверном `boost::get<T>` по ссылке|
|`boost::static_visitor<R>`|База для визиторов с указанием типа результата|
|`boost::recursive_variant_`|Маркер для рекурсивных вариантов (см. ниже)|
|`boost::make_recursive_variant<...>`|Создание рекурсивного типа variant|
|`boost::recursive_wrapper<T>`|Обёртка для хранения неполного/рекурсивного типа|
|`boost::variant<...>::types`|MPL-список типов, входящих в variant|

## Рекурсивные варианты

Позволяют строить древовидные структуры (JSON, AST):

```cpp
typedef boost::make_recursive_variant
    int,
    std::vector<boost::recursive_variant_>
>::type tree;
// tree = либо int, либо вектор из tree
```

## Отличия от `std::variant`

- Доступ: `boost::get<T>(v)` ↔ `std::get<T>(v)`; исключение `boost::bad_get` ↔ `std::bad_variant_access`.
- Индекс типа: `v.which()` ↔ `v.index()`.
- Визитация: `boost::apply_visitor(vis, v)` + `static_visitor<R>` ↔ `std::visit(vis, v)` (в стандарте visitor — обычный вызываемый объект, базовый класс не нужен).
- **Рекурсивные варианты:** boost имеет встроенную поддержку (`make_recursive_variant`, `recursive_wrapper`); в `std::variant` нужно делать вручную.
- **«Never empty» гарантия:** boost старается никогда не быть пустым (отсюда `recursive_wrapper` и редкое singular-состояние вместо `valueless_by_exception`); `std::variant` имеет состояние `valueless_by_exception()`.
- `boost::variant` не имеет `emplace` (значение задаётся через присваивание/конструктор); `std::variant` имеет `emplace`.

CMake: `find_package(Boost REQUIRED)` + `target_link_libraries(app PRIVATE Boost::boost)` (header-only интерфейс).

### include/test_variant.h
```cpp
#pragma once  

#include <boost/variant.hpp>  

namespace test_variant {  
    void test();  
}
```

### src/test_variant.cpp
```cpp
#include "test_variant.h"  
  
#include <iostream>  
#include <format>  
  
namespace test_variant {  
  
struct Printer: boost::static_visitor<> {  
    void operator()(int _value) {  
        std::cout << std::format("[boost::variant][print-int] {}\n", _value);  
    }    void operator()(const std::string& _value) {  
        std::cout << std::format("[boost::variant][print-str] {}\n", _value);  
    }};  
  
struct Sizer: boost::static_visitor<std::size_t> {  
    std::size_t operator()(int) { return sizeof(int); }  
    std::size_t operator()(const std::string& _value) { return _value.size(); }  
};  
  
void test() {  
    boost::variant<int, std::string> v = 42;  
    if (int* p = boost::get<int>(&v)) {  
        std::cout << std::format("[boost::variant][has-int] {}\n", *p);  
    }  
    int value = boost::get<int>(v);  
    std::cout << std::format("[boost::variant][value-int] {}\n", value);  
  
    v = std::string{"hello"};  
    std::cout << std::format("[boost::variant][which] {}\n", v.which());  
  
    Printer printer;  
    boost::apply_visitor(printer, v);  
  
    Sizer sizer;  
    std::cout << std::format("[boost::variant][size] {}]\n", boost::apply_visitor(sizer, v));  
}  
  
}
```
