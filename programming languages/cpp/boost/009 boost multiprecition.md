---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Multiprecision

Boost.Multiprecision — арифметика произвольной (или просто повышенной) точности: большие целые, рациональные числа, числа с плавающей точкой с заданным числом знаков. В основном **header-only**, но некоторые backend'ы требуют внешних библиотек (GMP, MPFR).

```cpp
#include <boost/multiprecision/cpp_int.hpp>        // целые
#include <boost/multiprecision/cpp_dec_float.hpp>  // десятичные float
#include <boost/multiprecision/cpp_bin_float.hpp>  // двоичные float
#include <boost/multiprecision/cpp_rational.hpp>   // рациональные
```

## Архитектура: число = frontend + backend

Ключевая идея библиотеки — разделение на два слоя:

- **Frontend** — `boost::multiprecision::number<Backend>` — обёртка, дающая привычные операторы (`+`, `*`, сравнения и т.д.).
- **Backend** — реализация хранения и вычислений (своя на чистом C++ или обёртка над GMP/MPFR/MPC).

Это позволяет менять «движок» не трогая код вычислений.

## 1. Готовые типы для целых чисел

|Тип|Backend|Зависимости|Описание|
|---|---|---|---|
|`cpp_int`|чистый C++|нет|Целое произвольной точности|
|`int128_t`, `int256_t`, ...|чистый C++|нет|Целые фиксированной разрядности|
|`uint128_t`, `uint256_t`, ...|чистый C++|нет|Беззнаковые фиксированной разрядности|
|`mpz_int`|GMP|libgmp|Целое (быстрее cpp_int на больших числах)|
|`checked_cpp_int`|чистый C++|нет|С проверкой переполнения|

```cpp
#include <boost/multiprecision/cpp_int.hpp>
using boost::multiprecision::cpp_int;

cpp_int factorial = 1;
for (int i = 1; i <= 100; ++i) factorial *= i;
std::cout << "100! = " << factorial << "\n"; // огромное число целиком
```

`cpp_int` растёт динамически; фиксированные `int256_t` и т.п. имеют предел, но без аллокаций.

## 2. Числа с плавающей точкой

|Тип|Backend|Зависимости|Описание|
|---|---|---|---|
|`cpp_dec_float_50`|чистый C++|нет|~50 десятичных знаков|
|`cpp_dec_float_100`|чистый C++|нет|~100 десятичных знаков|
|`cpp_dec_float<N>`|чистый C++|нет|N десятичных знаков (параметризуемо)|
|`cpp_bin_float_50` / `_100`|чистый C++|нет|Двоичная мантисса (точнее для математики)|
|`mpf_float`|GMP|libgmp|Float произвольной точности|
|`mpfr_float_50` / `_100`|MPFR|libmpfr|Корректное округление, быстрые трансцендентные функции|

```cpp
#include <boost/multiprecision/cpp_dec_float.hpp>
using boost::multiprecision::cpp_dec_float_50;

cpp_dec_float_50 pi = boost::math::constants::pi<cpp_dec_float_50>();
std::cout << std::setprecision(50) << pi << "\n"; // 50 знаков π
```

> `cpp_dec_float` — десятичный (удобен для вывода без двоичных артефактов), `cpp_bin_float` — двоичный (точнее в чисто математических вычислениях).

## 3. Рациональные числа

| Тип            | Backend    | Зависимости |
| -------------- | ---------- | ----------- |
| `cpp_rational` | чистый C++ | нет         |
| `mpq_rational` | GMP        | libgmp      |

Хранят дробь как пару числитель/знаменатель — без потери точности.

```cpp
#include <boost/multiprecision/cpp_int.hpp>
using boost::multiprecision::cpp_rational;

cpp_rational r = cpp_rational(1, 3) + cpp_rational(1, 6); // ровно 1/2
std::cout << r << "\n";                  // 1/2
std::cout << numerator(r) << "/" << denominator(r) << "\n";
```

|Функция|Описание|
|---|---|
|`numerator(r)`|Числитель|
|`denominator(r)`|Знаменатель|

## 4. Использование: как обычные числа

Главное удобство — типы ведут себя как встроенные: те же операторы и функции.

```cpp
cpp_int a = 1234567890;
cpp_int b("99999999999999999999999"); // из строки — для очень больших литералов
cpp_int c = a * b + 42;
bool less = (a < b);
```

|Операция|Поддержка|
|---|---|
|Арифметика `+ - * / %`|Да|
|Сравнения `< > == ...`|Да|
|Составные `+= *= ...`|Да|
|Битовые `& \| ^ << >>` (для целых)|Да|
|Инкремент/декремент|Да|
|Потоковый ввод/вывод `<< >>`|Да|
## 5. Математические функции

Работают через перегрузки, как `std::` для встроенных типов:

|Функция|Описание|
|---|---|
|`sqrt(x)`, `cbrt(x)`|Корни|
|`pow(x, y)`|Степень|
|`abs(x)`, `fabs(x)`|Модуль|
|`log(x)`, `log10(x)`, `exp(x)`|Лог/экспонента|
|`sin`, `cos`, `tan`, ...|Тригонометрия (для float-типов)|
|`gcd(a, b)`, `lcm(a, b)`|НОД/НОК (для целых)|
|`pow(cpp_int, unsigned)`|Целочисленная степень|
|`sqrt(cpp_int)`|Целочисленный корень|

```cpp
#include <boost/multiprecision/cpp_dec_float.hpp>
using boost::multiprecision::cpp_dec_float_100;

cpp_dec_float_100 x = 2;
cpp_dec_float_100 root = sqrt(x);
std::cout << std::setprecision(100) << root << "\n"; // √2 на 100 знаков
```

## 6. Преобразования

|Метод/функция|Описание|
|---|---|
|Конструктор из строки|`cpp_int("123...")` — для больших литералов|
|`x.str()`|В `std::string`|
|`x.str(digits, flags)`|С числом знаков и флагами форматирования|
|`x.convert_to<T>()`|Привести к другому типу (`double`, `int`, другой backend)|
|`static_cast<double>(x)`|Привести к встроенному типу (с потерей точности)|

```cpp
cpp_int big("123456789012345678901234567890");
std::string s = big.str();           // в строку
double approx = big.convert_to<double>(); // приближённо в double
```

## 7. Точность: фиксированная vs динамическая

|Категория|Точность|Примеры|
|---|---|---|
|Динамическая|Растёт по мере необходимости|`cpp_int`, `cpp_rational`|
|Фиксированная произвольная|Задана типом на этапе компиляции|`cpp_dec_float_50`, `int256_t`|
|Настраиваемая|Параметр шаблона|`cpp_dec_float<N>`, `number<...>` с политиками|

Можно настроить поведение через политики (expression templates вкл/выкл, проверка переполнения):

```cpp
// Отключить expression templates (иногда упрощает отладку/совместимость)
using namespace boost::multiprecision;
typedef number<cpp_int_backend<>, et_off> my_int;
```

> **Expression templates** включены по умолчанию — они откладывают вычисления, устраняя временные объекты и ускоряя сложные выражения. Иногда их отключают (`et_off`) ради простоты типов и совместимости с шаблонным кодом.

## Выбор backend'а

|Нужно|Backend|
|---|---|
|Целые без зависимостей|`cpp_int`|
|Максимальная скорость на огромных целых|`mpz_int` (GMP)|
|Десятичные с фиксированной точностью, без зависимостей|`cpp_dec_float_N`|
|Float с корректным округлением и быстрой математикой|`mpfr_float` (MPFR)|
|Точные дроби|`cpp_rational`|
|Фиксированная разрядность без аллокаций|`int128_t`/`int256_t`/...|

> Правило: начинай с **чистых C++ backend'ов** (`cpp_*`) — они header-only и не требуют установки библиотек. Переходи на GMP/MPFR только если упёрся в производительность на действительно больших числах.

## Подключение GMP/MPFR через vcpkg (если понадобятся)

```json
// vcpkg.json
{ "dependencies": ["boost-multiprecision", "gmp", "mpfr"] }
```

```cmake
find_package(Boost REQUIRED)
find_package(PkgConfig REQUIRED)
# gmp/mpfr подтянутся; слинкуй соответствующие таргеты
target_link_libraries(app PRIVATE Boost::boost gmp mpfr)
```

Но для `cpp_int`/`cpp_dec_float` ничего этого не нужно — только заголовки.

## Отличия от стандарта

- В стандартной библиотеке C++ **аналога нет** — нет ни больших целых, ни произвольной точности. Это делает Boost.Multiprecision уникальным.
- Похожие задачи вне Boost решают: GMP/MPFR напрямую (C-API, менее удобный), `__int128` (компиляторное расширение, лишь 128 бит), сторонние библиотеки больших чисел.
- Преимущество Boost — единый удобный C++-интерфейс поверх разных backend'ов: можно прототипировать на `cpp_int` и переключиться на GMP сменой одного типа.

### include/test_multiprecision.h
```cpp
#pragma once  
  
namespace test_multiprecision {  
    void test();  
}
```

### src/test_multiprecision.cpp
```cpp
#include "test_multiprecision.h"  
  
#include <iomanip>  
#include <boost/multiprecision/cpp_int.hpp>  
#include <boost/multiprecision/cpp_dec_float.hpp>  
#include <iostream>  
  
using boost::multiprecision::cpp_int;  
using boost::multiprecision::cpp_dec_float_50;  
  
namespace test_multiprecision {  
  
void test() {  
    cpp_int power{1};  
    power <<= 256;  
    std::cout << std::format("2^256 = {}\n", power.str());  
  
    cpp_int fact{1};  
    for (int i{1}; i <= 30; ++i) fact *= i;  
    std::cout << std::format("30! = {}\n", fact.str());  
  
    cpp_dec_float_50 a{1};  
    cpp_dec_float_50 b{3};  
    std::cout  
        << std::setprecision(50)  
        << std::format(  
            "{}/{} = {}\n",  
            a.str(),  
            b.str(),  
            cpp_dec_float_50(a / b).str());  
  
    cpp_int n("1000000000000000000000000000057");  
    std::cout << std::format("sqrt(n) ~ {}\n", sqrt(n).str());  
}  
  
}
```
