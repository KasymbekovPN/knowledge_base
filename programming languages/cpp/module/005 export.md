---
tags:
  - programming-language
  - cpp
  - module
---
[[programming languages/cpp/module/_|<=]]

# Что можно экспортировать из модуля

Короткий ответ: `export` можно пометить практически **любое объявление пространства имён** — функции это лишь самый частый случай. Разберём всё по категориям с примерами.

## Базовое правило

`export` ставится перед объявлением и означает «эта сущность — часть публичного интерфейса модуля, её увидят те, кто сделает `import`». Экспортировать можно только сущности из области видимости пространства имён (namespace scope) — нельзя экспортировать локальную переменную внутри функции или приватный член класса по отдельности.

## 1. Функции

Уже знакомый случай, для полноты: [[004 hello module|hello module]]

```cpp
export module math;

export int add(int a, int b) { return a + b; }   // определение
export int sub(int a, int b);                     // можно и просто объявление
```

Можно экспортировать как определение целиком, так и только объявление (тогда определение даётся где-то в реализации модуля).

## 2. Переменные и константы


```cpp
export module config;

export int version = 3;              // переменная
export const double PI = 3.14159;    // константа
export inline int counter = 0;       // inline-переменная
```

Здесь кроется важное отличие от заголовков. Помните проблему: определять переменную с внешней линковкой в заголовке нельзя — нарушение ODR (multiple definition). В модуле же `export int version = 3;` совершенно легально: модуль компилируется один раз, копий не возникает, никаких `inline`-трюков для глобальной переменной не требуется. Боль, которую раньше лечили через `extern` + определение в `.cpp`, здесь просто исчезает.

## 3. Классы и структуры

```cpp
export module shapes;

export class Circle {        // весь класс экспортирован
public:
    Circle(double r) : radius(r) {}
    double area() const;
private:
    double radius;           // private остаётся private — он не «утекает»
};

export struct Point {        // структуры тоже
    int x, y;
};
```

Когда вы экспортируете класс, наружу становится доступен сам тип и его **публичный** интерфейс. При этом `private`-члены не становятся доступны для использования снаружи — инкапсуляция класса работает как обычно. Экспорт управляет видимостью _имени типа_ для других единиц трансляции, а `public`/`private` — доступом к членам; это два независимых механизма.

## 4. Шаблоны

Шаблоны — функции, классы, переменные — экспортируются так же:

```cpp
export module containers;

export template<typename T>          // шаблон функции
T max(T a, T b) {
    return a > b ? a : b;
}

export template<typename T>          // шаблон класса
class Stack {
    T data[100];
    int top = 0;
public:
    void push(T v) { data[top++] = v; }
    T pop() { return data[--top]; }
};

export template<typename T>          // шаблон переменной (C++14+)
constexpr T zero = T{};
```

Это, кстати, отдельная победа модулей: в мире заголовков шаблоны _обязаны_ целиком жить в `.h` (их нельзя разделить на объявление и реализацию по разным файлам обычным способом), что раздувало заголовки и замедляло сборку. В модуле шаблон тоже экспортируется из интерфейса, но парсится один раз в BMI, а не заново в каждой единице трансляции.

## 5. Псевдонимы типов и using-объявления

```cpp
export module types;

export using Byte = unsigned char;        // псевдоним типа
export using Callback = void(*)(int);     // псевдоним для указателя на функцию

export template<typename T>
using Vec = std::vector<T>;               // шаблон псевдонима (alias template)
```

## 6. Пространства имён и `export namespace`

Можно экспортировать содержимое целого пространства имён сразу, обернув его в `export namespace`. Тогда экспортируется **каждый** член внутри:

```cpp
export module geometry;

export namespace geo {       // всё внутри экспортируется
    double area(double r);
    double perimeter(double r);

    class Triangle { /* ... */ };

    constexpr double PI = 3.14159;
}
```

Эквивалентно тому, как если бы вы поставили `export` перед каждым объявлением внутри `geo`. При этом само пространство имён остаётся обычным namespace — снаружи к нему обращаются как `geo::area(...)`.

Важная тонкость: можно экспортировать namespace частично. Если написать `namespace geo { ... }` (без `export`) и поставить `export` лишь перед отдельными членами внутри, то наружу попадут только они, а остальные останутся приватными деталями модуля.

```cpp
export module geometry;

namespace geo {                          // namespace не экспортирован целиком
    export double area(double r);        // ЭТО видно снаружи
    double internalHelper(double r);     // а это — приватная деталь модуля
}
```

## 7. Блок `export { ... }`

Если нужно экспортировать несколько объявлений подряд, не повторяя слово `export`, оберните их в блок:

```cpp
export module math;

export {                     // всё внутри блока экспортируется
    int add(int, int);
    int sub(int, int);
    double PI = 3.14159;
    class Vector { /* ... */ };
}

int privateHelper();         // вне блока — не экспортировано
```

Это чисто синтаксическое удобство, эквивалентное расстановке `export` перед каждым элементом.

## 8. `export import` — реэкспорт

Особый случай: можно экспортировать **целый другой модуль**. Тогда любой, кто импортирует ваш модуль, автоматически получит и тот, вложенный:

```cpp
export module app;

export import math;          // кто импортирует app, получает и math
import utils;                // а utils — только для внутреннего пользования app
```

Это удобно для построения «модуля-агрегатора», который собирает публичный API из нескольких подмодулей. Подробнее — на Этапе 5, где разбирается видимость, и Этапе 4 про разделы модуля.

## Что НЕЛЬЗЯ экспортировать

Несколько важных ограничений:

**Сущности с внутренней линковкой.** Нельзя экспортировать то, что помечено `static` или лежит в анонимном пространстве имён — это противоречие (внутренняя линковка означает «только для этой единицы трансляции», экспорт — «наружу»):

```cpp
export static int x = 5;     // ОШИБКА
export namespace { ... }     // ОШИБКА: анонимный namespace
```

**Локальные сущности.** Нельзя экспортировать переменную внутри функции или отдельный член класса.

**Объявления, не вводящие имя.** Например, `export static_assert(...);` или `export ;` бессмысленны и запрещены.

**Макросы.** Это ключевой момент: `#define` **невозможно** экспортировать в принципе. Макросы — феномен препроцессора, а модули работают на уровне семантики языка, _после_ препроцессора. Именно поэтому модули не «протекают» макросами — но это же означает, что если ваш старый API раздавал макросы, через модуль их передать нельзя (нужны header units или иной подход).

## Сводная таблица

|Можно экспортировать|Нельзя экспортировать|
|---|---|
|Функции (объявления и определения)|Макросы (`#define`)|
|Переменные и константы|Сущности со `static` / внутренней линковкой|
|Классы и структуры|Анонимные пространства имён|
|Шаблоны (функций, классов, переменных)|Локальные переменные|
|Псевдонимы типов (`using`)|Отдельные члены класса|
|Пространства имён (`export namespace`)|Объявления без имени|
|Блоки `export { ... }`||
|Другие модули (`export import`)||

## Главный вывод

Картина видимости в модулях устроена просто: **по умолчанию закрыто, `export` открывает**. Это инвертирует логику заголовков, где по умолчанию утекало всё. Управление одним ключевым словом заменяет целый набор приёмов из мира заголовков — `static`, `extern`, анонимные namespace, `inline` для переменных, разделение шаблонов. А макросы выпадают из системы намеренно — в этом и сила (нет утечек), и единственное реальное ограничение совместимости со старым кодом.

```cpp
/*

clang++ -std=c++23 --precompile config.cppm -o config.pcm;

if ($?) { clang++ -std=c++23 --precompile templates.cppm -o templates.pcm; }

if ($?) { clang++ -std=c++23 --precompile types.cppm -o types.pcm; }

if ($?) { clang++ -std=c++23 --precompile geo1.cppm -o geo1.pcm; }

if ($?) { clang++ -std=c++23 --precompile geo2.cppm -o geo2.pcm; }

if ($?) { clang++ -std=c++23 --precompile shapes.cppm -o shapes.pcm; }

if ($?) { clang++ -std=c++23 --precompile math.cppm -o math.pcm; }

if ($?) { clang++ -std=c++23 "-fmodule-file=config=config.pcm" "-fmodule-file=templates=templates.pcm" "-fmodule-file=types=types.pcm" "-fmodule-file=geo1=geo1.pcm" "-fmodule-file=geo2=geo2.pcm" "-fmodule-file=shapes=shapes.pcm" "-fmodule-file=math=math.pcm" demo.cpp config.pcm templates.pcm types.pcm geo1.pcm geo2.pcm shapes.pcm math.pcm -o app.exe }

*/

import config;
import templates;
import types;
import geo1;
import geo2;
import shapes;
import math;

#include <iostream>
#include <format>

void test_config() {
    std::cout << "### test_config\n";
    std::cout << std::format("config_version {}\n", config_version);
    std::cout << std::format("config_pi {}\n", config_pi);
    std::cout << std::format("config_counter {}\n", config_counter);
}

void test_templates() {
    std::cout << "### test_templates\n";

    int a{42};
    int b{13};
    std::cout << std::format("{} is max from [{}, {}]\n", max(42, 13), a, b);
  
    auto&& st = Stack<int>();
    st.push(1);
    st.push(2);
    st.push(3);
    st.pop();
    st.print();

    std::cout << std::format("zero<double>: {}\n", zero<double>);
}

void int_func(int _value) {
    std::cout << std::format("int_func {}\n", _value);
}

void test_types() {
    std::cout << "### test_types\n";

    Byte b{' '};
    std::cout << std::format("b {}\n", b);

    Callback c = int_func;
    c(42);

    Vec<int> v;
    v.push_back(11);
    std::cout << std::format("vec size {}\n", std::size(v));
}

void test_geo1() {
    std::cout << "### test_geo1\n";

    double a{12.45};
    std::cout << std::format("geo1::area({}) = {}\n", a, geo1::area(a));

    double s{11.11};
    std::cout << std::format("geo1::perimeter({}) = {}\n", s, geo1::perimeter(s));
    std::cout << std::format("geo1::PI = {}\n", geo1::PI);

    auto&& p = geo1::Point{1, 2};
    p.print();
}

void test_geo2() {
    std::cout << "### test_geo2\n";
    double a{12.45};
    std::cout << std::format("geo2::area({}) = {}\n", a, geo2::area(a));
}

void test_shapes() {
    std::cout << "### test_shapes\n";

    auto&& circle = Circle(4.2);
    std::cout << std::format("circle::area {}\n", circle.area());

    auto&& point3 = Point3(1, 2, 3);
    point3.print();
}

void test_math() {
    std::cout << "### test_math\n";
    int a{1};
    int b{2};
    std::cout << std::format("add({}, {}) = {}", 1, 2, add(1, 2));
}

int main() {
    test_config();
    test_templates();
    test_types();
    test_geo1();
    test_geo2();
    test_shapes();
    test_math();

    return 0;
}
```

```
### test_config
config_version 3
config_pi 3.14159
config_counter 0
### test_templates
42 is max from [42, 13]
[0, 1]
zero<double>: 0
### test_types
b 32
int_func 42
vec size 1
### test_geo1
geo1::area(12.45) = 155.00249999999997
geo1::perimeter(11.11) = 44.44
geo1::PI = 3.14159
[1, 2]
### test_geo2
geo2::area(12.45) = 155.00249999999997
### test_shapes
circle::area 110.8352952
[1, 2, 3]
### test_math
add(1, 2) = 3
```
