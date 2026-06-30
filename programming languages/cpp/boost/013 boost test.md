---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Test

Boost.Test — фреймворк для модульного (unit) тестирования C++. Гибкий по способу подключения: header-only, статическая или динамическая линковка. Хорошо интегрируется с CMake/CTest.

```cpp
#include <boost/test/included/unit_test.hpp>  // header-only вариант
// ИЛИ
#include <boost/test/unit_test.hpp>           // линкуемый вариант
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
enable_testing()
add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE Boost::unit_test_framework)
add_test(NAME tests COMMAND tests)
```

## Три способа подключения

|Вариант|Заголовок|Особенность|
|---|---|---|
|Header-only|`<boost/test/included/unit_test.hpp>`|Ничего не линковать, но дольше компиляция; только один .cpp|
|Статическая|`<boost/test/unit_test.hpp>` + линковка|Быстрее компиляция, несколько файлов тестов|
|Динамическая|то же + `BOOST_TEST_DYN_LINK`|Линковка с разделяемой библиотекой|

> Для одного файла удобен header-only; для проекта с множеством тестовых файлов — линкуемый.

## 1. Точка входа: `BOOST_TEST_MODULE`

Определяет модуль и **автоматически генерирует `main()`**. Указывается **один раз** на всю программу тестов, до включения заголовка:

```cpp
#define BOOST_TEST_MODULE MyTestSuite
#include <boost/test/included/unit_test.hpp>
```

Без `BOOST_TEST_MODULE` `main()` не создаётся — придётся писать свой.

## 2. Тест-кейсы

|Макрос|Описание|
|---|---|
|`BOOST_AUTO_TEST_CASE(name)`|Автоматически регистрируемый тест|
|`BOOST_AUTO_TEST_SUITE(name)`|Начало группы тестов|
|`BOOST_AUTO_TEST_SUITE_END()`|Конец группы|
|`BOOST_FIXTURE_TEST_CASE(name, Fixture)`|Тест с фикстурой|
|`BOOST_DATA_TEST_CASE(name, dataset)`|Параметризованный тест (data-driven)|

```cpp
BOOST_AUTO_TEST_CASE(my_first_test) {
    int sum = 2 + 3;
    BOOST_CHECK_EQUAL(sum, 5);
}
```

## 3. Макросы проверок — три уровня строгости

### CHECK — продолжает при провале

|Макрос|Проверяет|
|---|---|
|`BOOST_CHECK(expr)`|Истинность выражения|
|`BOOST_CHECK_EQUAL(a, b)`|Равенство (с выводом обоих значений)|
|`BOOST_CHECK_NE(a, b)`|Неравенство|
|`BOOST_CHECK_LT/LE/GT/GE(a, b)`|Сравнения `< <= > >=`|
|`BOOST_CHECK_CLOSE(a, b, tol)`|Близость дробных (в процентах)|
|`BOOST_CHECK_CLOSE_FRACTION(a, b, tol)`|Близость (в долях)|
|`BOOST_CHECK_SMALL(val, tol)`|Близость к нулю|
|`BOOST_CHECK_THROW(expr, ExType)`|Выражение бросает исключение типа|
|`BOOST_CHECK_NO_THROW(expr)`|Выражение не бросает|
|`BOOST_CHECK_THROW_EQUAL` / `BOOST_CHECK_EXCEPTION`|Проверка исключения с предикатом|

### REQUIRE — прерывает тест при провале

Те же макросы, но `BOOST_REQUIRE*` вместо `BOOST_CHECK*`. При провале **тест останавливается** (дальше нет смысла — например, упал критичный предусловие).

|Макрос|Поведение|
|---|---|
|`BOOST_REQUIRE(expr)`|Провал → тест прерывается|
|`BOOST_REQUIRE_EQUAL(a, b)`|То же для равенства|
|`BOOST_REQUIRE_THROW(...)`|То же для исключений|

### WARN — только предупреждает

`BOOST_WARN*` — при провале лишь печатает предупреждение, тест считается пройденным.

|Уровень|Провал означает|
|---|---|
|`WARN`|Предупреждение, тест проходит|
|`CHECK`|Тест провален, но выполнение продолжается|
|`REQUIRE`|Тест провален и немедленно прерывается|

```cpp
BOOST_AUTO_TEST_CASE(levels_demo) {
    int* ptr = get_pointer();
    BOOST_REQUIRE(ptr != nullptr); // если null — дальше нет смысла, прерываем
    BOOST_CHECK_EQUAL(*ptr, 42);   // сюда дойдём, только если ptr валиден
}
```

## 4. Современный макрос `BOOST_TEST`

Универсальный макрос (с Boost 1.59+), сам разбирает выражение и красиво печатает значения:

```cpp
BOOST_TEST(a == b);           // покажет значения a и b при провале
BOOST_TEST(x > 0);
BOOST_TEST(vec == expected);  // умеет сравнивать контейнеры
BOOST_TEST(d == 3.14, boost::test_tools::tolerance(0.001)); // с допуском
```

|Возможность|Описание|
|---|---|
|Автоматический разбор выражения|Печатает левую/правую часть и оператор|
|`tolerance(...)`|Допуск для дробных|
|Сравнение контейнеров|Поэлементно, с указанием расхождений|
|`per_element()`|Поэлементное сравнение коллекций|

> `BOOST_TEST` — рекомендуемый современный способ; старые `BOOST_CHECK_*` остаются для совместимости и явности.

## 5. Тест-сьюты (группировка)

```cpp
BOOST_AUTO_TEST_SUITE(math_tests)

    BOOST_AUTO_TEST_CASE(addition) {
        BOOST_TEST(2 + 2 == 4);
    }

    BOOST_AUTO_TEST_CASE(subtraction) {
        BOOST_TEST(5 - 3 == 2);
    }

BOOST_AUTO_TEST_SUITE_END()
```

Сьюты можно вкладывать; они задают иерархию и позволяют запускать выборочно (`--run_test=math_tests/addition`).

## 6. Фикстуры — общая подготовка/очистка

Фикстура — структура с конструктором (setup) и деструктором (teardown):

```cpp
struct DatabaseFixture {
    Database db;
    DatabaseFixture() {
        db.connect("test_db");     // setup перед каждым тестом
    }
    ~DatabaseFixture() {
        db.disconnect();           // teardown после каждого теста
    }
};

// Фикстура для одного теста
BOOST_FIXTURE_TEST_CASE(query_test, DatabaseFixture) {
    auto result = db.query("SELECT 1");
    BOOST_TEST(result.size() == 1);
}

// Фикстура для всех тестов в сьюте
BOOST_FIXTURE_TEST_SUITE(db_suite, DatabaseFixture)
    BOOST_AUTO_TEST_CASE(test_a) { /* db уже доступна */ }
    BOOST_AUTO_TEST_CASE(test_b) { /* своя свежая db */ }
BOOST_FIXTURE_TEST_SUITE_END()
```

|Макрос|Назначение|
|---|---|
|`BOOST_FIXTURE_TEST_CASE(name, Fix)`|Фикстура для одного теста|
|`BOOST_FIXTURE_TEST_SUITE(name, Fix)`|Фикстура для всех тестов сьюта|
|`BOOST_GLOBAL_FIXTURE(Fix)`|Глобальная фикстура (один раз на всю программу)|
|`BOOST_TEST_GLOBAL_FIXTURE(Fix)`|Современный вариант глобальной фикстуры|
## 7. Параметризованные тесты (data-driven)

Запускают один тест на множестве входных данных:

```cpp
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

namespace bdata = boost::unit_test::data;

// перебор значений
BOOST_DATA_TEST_CASE(square_test,
        bdata::make({1, 2, 3, 4}), value) {
    BOOST_TEST(value * value >= value);
}

// несколько наборов (zip) — пары вход/ожидание
BOOST_DATA_TEST_CASE(pairs_test,
        bdata::make({2, 3, 4}) ^ bdata::make({4, 9, 16}), input, expected) {
    BOOST_TEST(input * input == expected);
}
```

|Генератор|Описание|
|---|---|
|`bdata::make({...})`|Из списка значений|
|`^`|Zip — попарное объединение наборов|
|`*`|Декартово произведение наборов|
|`bdata::xrange(n)`|Диапазон чисел|
|`bdata::random(...)`|Случайные значения|

---

## 8. Контроль ожидаемых провалов и тайм-аутов (декораторы)

```cpp
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(known_bug,
        * utf::expected_failures(1)) {   // ожидаем 1 провал
    BOOST_TEST(buggy_function() == 0);
}

BOOST_AUTO_TEST_CASE(slow_test,
        * utf::timeout(5)) {             // таймаут 5 секунд
    long_running_operation();
}

BOOST_AUTO_TEST_CASE(skip_me,
        * utf::disabled()) {             // временно отключить
    // ...
}
```

|Декоратор|Назначение|
|---|---|
|`* utf::expected_failures(n)`|Ожидать n провалов|
|`* utf::timeout(sec)`|Ограничить время|
|`* utf::disabled()`|Отключить тест|
|`* utf::label("name")`|Метка для выборочного запуска|
|`* utf::precondition(...)`|Условие запуска|
|`* utf::depends_on("other")`|Зависимость от другого теста|

---

## Полный практический пример

```cpp
#define BOOST_TEST_MODULE CalculatorTests
#include <boost/test/included/unit_test.hpp>
#include <stdexcept>

// Тестируемый код
class Calculator {
public:
    int add(int a, int b) { return a + b; }
    int divide(int a, int b) {
        if (b == 0) throw std::invalid_argument("деление на ноль");
        return a / b;
    }
};

// Фикстура: свежий калькулятор на каждый тест
struct CalcFixture {
    Calculator calc;
};

BOOST_FIXTURE_TEST_SUITE(calculator_suite, CalcFixture)

    BOOST_AUTO_TEST_CASE(addition_works) {
        BOOST_TEST(calc.add(2, 3) == 5);
        BOOST_TEST(calc.add(-1, 1) == 0);
    }

    BOOST_AUTO_TEST_CASE(division_works) {
        BOOST_TEST(calc.divide(10, 2) == 5);
    }

    BOOST_AUTO_TEST_CASE(division_by_zero_throws) {
        BOOST_CHECK_THROW(calc.divide(1, 0), std::invalid_argument);
    }

    BOOST_AUTO_TEST_CASE(division_no_throw_normal) {
        BOOST_CHECK_NO_THROW(calc.divide(8, 4));
    }

BOOST_FIXTURE_TEST_SUITE_END()

// Параметризованный тест вне сьюта
BOOST_DATA_TEST_CASE(addition_commutative,
        boost::unit_test::data::make({1, 2, 3}) ^
        boost::unit_test::data::make({10, 20, 30}),
        a, b) {
    Calculator calc;
    BOOST_TEST(calc.add(a, b) == calc.add(b, a));
}
```

## Запуск тестов

### Через исполняемый файл напрямую

```bash
./tests                          # все тесты
./tests --run_test=calculator_suite          # только один сьют
./tests --run_test=calculator_suite/addition_works  # один тест
./tests --log_level=all          # подробный лог
./tests --report_level=detailed  # детальный отчёт
./tests --list_content           # показать все тесты без запуска
```

|Опция|Назначение|
|---|---|
|`--run_test=...`|Выборочный запуск (имя/маска)|
|`--log_level=all/test_suite/error/...`|Уровень логирования|
|`--report_level=detailed/short/no`|Уровень отчёта|
|`--output_format=HRF/XML/JUNIT`|Формат вывода (для CI)|
|`--list_content`|Список тестов|
|`--random`|Случайный порядок|
|`--build_info`|Информация о сборке|

### Через CTest

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -R calculator   # по имени теста (регулярка)
ctest --test-dir build -V              # подробный вывод
```

---

## CMake: интеграция с автоматической регистрацией

Для крупных проектов есть модуль, автоматически добавляющий каждый `BOOST_AUTO_TEST_CASE` в CTest:

```cmake
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
enable_testing()

add_executable(tests test_main.cpp test_calc.cpp)
target_link_libraries(tests PRIVATE Boost::unit_test_framework)

# Линкуемая версия требует определить, что main даёт Boost:
target_compile_definitions(tests PRIVATE BOOST_TEST_DYN_LINK)

# Автоматическая регистрация тестов в CTest (опционально):
include(BoostTestTargets OPTIONAL) # или ручной add_test
add_test(NAME boost_tests COMMAND tests)
```

> Для нескольких .cpp с линкуемой версией: `BOOST_TEST_MODULE` определяется **только в одном** файле, остальные просто включают `<boost/test/unit_test.hpp>` без него.

## Структура многофайлового проекта тестов

```
test_main.cpp     → #define BOOST_TEST_MODULE ProjectTests
                    #include <boost/test/unit_test.hpp>
test_calc.cpp     → #include <boost/test/unit_test.hpp> (без MODULE)
                    BOOST_AUTO_TEST_CASE(...)
test_string.cpp   → то же
```

Все компилируются в один исполняемый файл; `main()` генерируется из `test_main.cpp`.

## Сводка ключевых концепций

|Концепция|Суть|
|---|---|
|`BOOST_TEST_MODULE`|Имя модуля + автогенерация `main()` (один раз)|
|`BOOST_AUTO_TEST_CASE`|Тест с автрегистрацией|
|`BOOST_AUTO_TEST_SUITE`|Группировка тестов|
|`BOOST_TEST`|Современный универсальный макрос проверки|
|WARN / CHECK / REQUIRE|Три уровня строгости провала|
|`BOOST_FIXTURE_TEST_CASE`|Тест с setup/teardown через фикстуру|
|`BOOST_DATA_TEST_CASE`|Параметризованные (data-driven) тесты|
|Декораторы (`* utf::...`)|Таймауты, отключение, зависимости|
|CTest-интеграция|Запуск через `ctest`, форматы для CI|
## Практические советы

- **Используй `BOOST_TEST`** для новых тестов — он информативнее старых `BOOST_CHECK_*` (печатает значения, сравнивает контейнеры).
- **`REQUIRE` для предусловий, `CHECK` для самих проверок.** Если предусловие провалилось (null-указатель, пустой результат), `REQUIRE` остановит тест и убережёт от каскада бессмысленных ошибок.
- **Header-only — для маленьких проектов**, линкуемая версия — для больших (быстрее перекомпиляция при множестве файлов).
- **Фикстуры вместо копипасты setup-кода** — каждый тест получает свежий экземпляр, изоляция гарантирована.
- **Для CI выводи JUNIT/XML** (`--output_format=JUNIT`) — большинство CI-систем (GitLab, Jenkins, GitHub Actions) умеют его разбирать и показывать результаты.
- **Пиши тесты ко всем учебным примерам Boost** из плана — заодно закрепишь и сам Boost.Test, и проверишь понимание остальных модулей.

## Отличия от альтернатив

- В стандартной библиотеке фреймворка тестирования **нет** — выбирают между сторонними.
- Альтернативы: **GoogleTest** (очень популярен, богатые matchers и mock через GoogleMock), **Catch2** (header-only, лаконичный синтаксис `REQUIRE(...)`), **doctest** (самый быстрый по компиляции, header-only), **CppUnit** (старый, в стиле JUnit).
- Сила Boost.Test — зрелость, гибкость подключения, мощные data-driven тесты и декораторы, естественная интеграция, если проект уже на Boost. Многие новые проекты тем не менее выбирают GoogleTest или Catch2 за экосистему (особенно мокинг — у Boost.Test своего мок-фреймворка нет).

# Гибридный подход: модуль-обёртка + тесты против внутренней библиотеки

Идея: вся реальная логика живёт в **обычной библиотеке на заголовках** (как всегда), а C++20-модуль — это тонкая `export`-обёртка над ней. Тогда:

- Внешний код использует модуль (`import calculator;`) — современный интерфейс.
- Тесты линкуются с внутренней библиотекой по-старому (`#include`) — никаких модульных сложностей в тестовой сборке.
- Тесты получают доступ и к публичному API, и к внутренним деталям реализации.

## Структура проекта

```
project/
├── src/
│   ├── impl/                    ← внутренняя реализация (обычные заголовки)
│   │   ├── calculator_impl.hpp
│   │   └── calculator_impl.cpp
│   └── calculator.cppm          ← тонкая модуль-обёртка
├── tests/
│   ├── smoke_main.cpp
│   ├── smoke_module.cpp
│   ├── test_main.cpp
│   ├── test_calculator.cpp      ← тесты против impl (через #include)
│   └── test_internal.cpp        ← тесты внутренних деталей
├── app/
│   └── main.cpp                 ← приложение (через import)
├── vcpkg.json
├── CMakePresets.json
└── CMakeLists.txt
```

## 1. Внутренняя реализация (обычная библиотека)

Здесь живёт **вся логика** — и публичная, и внутренняя. Обычные заголовки, ничего модульного.

### src/impl/calculator_impl.hpp
```cpp
#pragma once  
  
#include <stdexcept>  
  
namespace calc::detail {  
    constexpr int MIN_VALUE{-100};  
    constexpr int MAX_VALUE{100};  
  
    bool is_valid_division(int);  
    int normalize(int);  
}  
  
namespace calc {  
    class Calculator {  
    public:  
        static int add(int, int);  
        static int divide(int, int);  
        static int normalized_add(int, int);  
    };
}
```

### src/impl/calculator_impl.cpp
```cpp
#include "calculator_impl.hpp"  
  
namespace calc::detail {  
    bool is_valid_division(const int _division) { return _division != 0; }  
  
    int normalize(const int _value) {  
        if (_value < MIN_VALUE) { return MIN_VALUE; }  
        if (_value > MAX_VALUE) { return MAX_VALUE; }  
        return _value;  
    }}  
  
namespace calc {  
    int Calculator::add(const int _a, const int _b) { return _a + _b; }  
  
    int Calculator::divide(const int _a, const int _b) {  
        if (!detail::is_valid_division(_b)) {  
            throw std::invalid_argument("division by zero");  
        }        return _a / _b;  
    }  
    int Calculator::normalized_add(const int _a, const int _b) {  
        return detail::normalize(_a + _b);  
    }
}
```

## 2. Модуль-обёртка (тонкий слой)

Модуль **не содержит логики** — он только переэкспортирует публичную часть библиотеки. Вся реализация уже скомпилирована в библиотеке.

### src/calculator.cppm
```cpp
module;  
  
#include "impl/calculator_impl.hpp"  
  
export module calculator;  
  
export namespace calc {  
    using calc::Calculator;  
}
```

> Ключевой момент: модуль экспортирует `calc::Calculator` через `using`-объявление. Логики здесь нет — только проброс имени. Внутренние функции `detail::` остаются невидимыми для импортёров модуля, но доступны тестам напрямую через заголовок.

## 3. Приложение использует модуль (современный интерфейс)

### app/main.cpp
```cpp
/*  
cmake --preset default  
cmake --build .build  
ctest --test-dir .build --output-on-failure  
ctest --test-dir .build -R unit_tests  
ctest --test-dir .build -R smoke_tests  
 */  
  
import calculator;  
  
#include <iostream>  
#include <format>  
  
int main() {  
    const int A{2};  
    const int B{3};  
    const int C{42};  
    const int D{6};  
  
    std::cout << std::format("{} + {} = {}", A, B, calc::Calculator::add(A, B));  
    std::cout << std::format("{} / {} = {}", C, D, calc::Calculator::divide(A, B));  
  
    return 0;  
}
```

## 4. Тесты против внутренней библиотеки (по-старому)

Тесты **не импортируют модуль** — они включают заголовок реализации напрямую. Это даёт доступ ко всему, включая `detail::`, и избавляет тестовую сборку от модульных сложностей.

### tests/test_main.cpp
```cpp
#define BOOST_TEST_MODULE ProjectTests  
#include <boost/test/unit_test.hpp>
```

### tests/test_calculator.cpp
```cpp
#include <boost/test/unit_test.hpp>  
  
#include "../src/impl/calculator_impl.hpp"  
  
BOOST_AUTO_TEST_SUITE(public_api_tests)  
  
BOOST_AUTO_TEST_CASE(add_works) {  
    BOOST_TEST(calc::Calculator::add(2, 3) == 5);  
    BOOST_TEST(calc::Calculator::add(-5, 5) == 0);  
}  
  
BOOST_AUTO_TEST_CASE(divide_works) {  
    BOOST_TEST(calc::Calculator::divide(10, 2) == 5);  
}  
  
BOOST_AUTO_TEST_CASE(divide_by_zero_throws) {  
    BOOST_CHECK_THROW(calc::Calculator::divide(1, 0), std::invalid_argument);  
}  
  
BOOST_AUTO_TEST_CASE(normalized_add_clamps) {  
    BOOST_TEST(calc::Calculator::normalized_add(60, 60) == 100);  
    BOOST_TEST(calc::Calculator::normalized_add(10, 60) == 70);  
}  
  
BOOST_AUTO_TEST_SUITE_END()
```

### tests/test_internal.cpp
```cpp
#include <boost/test/unit_test.hpp>  
#include "../src/impl/calculator_impl.hpp"  
  
BOOST_AUTO_TEST_SUITE(internal_tests)  
  
    BOOST_AUTO_TEST_CASE(is_valid_division) {  
    BOOST_TEST(calc::detail::is_valid_division(5));  
    BOOST_TEST(!calc::detail::is_valid_division(0));  
    BOOST_TEST(calc::detail::is_valid_division(-3));  
}  
  
BOOST_AUTO_TEST_CASE(normalize_boundaries) {  
    BOOST_TEST(calc::detail::normalize(50) == 50);  
    BOOST_TEST(calc::detail::normalize(150) == 100);  
    BOOST_TEST(calc::detail::normalize(-150) == -100);  
    BOOST_TEST(calc::detail::normalize(0) == 0);  
}  
  
BOOST_AUTO_TEST_SUITE_END()
```

### tests/smoke_module.cpp

Добавлю отдельную тестовую цель, которая проверяет **именно модуль** — что `export` не сломан и публичный интерфейс доступен через `import`. Это закрывает последний пробел гибридного подхода: основные тесты проверяют логику через `#include`, а smoke-тест убеждается, что модульная обёртка корректно экспортирует API.

Он импортирует модуль (а не включает заголовок) и делает несколько базовых вызовов:

> Принцип: smoke-тест намеренно **минимален**. Он не дублирует проверку логики (это уже делают основные тесты), а только подтверждает, что модуль импортируется и публичные методы вызываются. Если `export` обёртки сломается, упадёт именно он.

```cpp
#include <boost/test/unit_test.hpp>  
  
import calculator;  
  
BOOST_AUTO_TEST_SUITE(module_smoke)  
  
// Проверка: модуль импортируется и публичный класс доступен  
BOOST_AUTO_TEST_CASE(module_exports_calculator) {  
     BOOST_TEST(calc::Calculator::add(2, 3) == 5);  
}  
  
// Проверка: публичные методы доступны через экспортированный интерфейс  
BOOST_AUTO_TEST_CASE(module_public_methods_callable) {  
    BOOST_TEST(calc::Calculator::divide(10, 2) == 5);  
    BOOST_TEST(calc::Calculator::normalized_add(60, 60) == 100);  
}  
  
// Проверка: исключения корректно проходят через границу модуля  
BOOST_AUTO_TEST_CASE(module_exceptions_propagate) {  
    calc::Calculator c;  
    BOOST_CHECK_THROW(calc::Calculator::divide(1, 0), std::invalid_argument);  
}  
  
BOOST_AUTO_TEST_SUITE_END()
```

> Важно: здесь намеренно **нет** обращений к `calc::detail::...`. Если бы мы попытались вызвать внутреннюю функцию, код **не скомпилировался бы** — и это правильно: smoke-тест заодно подтверждает, что внутренние детали **не утекли** через `export`. Это негативная проверка инкапсуляции.

### tests/smoke_main.cpp - отдельная точка входа для smoke-теста

Поскольку smoke-тест собирается в **отдельный исполняемый файл** (модульную цель), ему нужна своя точка входа с `BOOST_TEST_MODULE`:

```cpp
#define BOOST_TEST_MODULE ModuleSmokeTests  
#include <boost/test/unit_test.hpp>
```

> Напомню правило: `BOOST_TEST_MODULE` определяется ровно один раз на исполняемый файл. У основных тестов своя точка входа (`test_main.cpp`), у smoke-теста — своя (`smoke_main.cpp`), потому что это **два разных бинарника**.

## 5. CMake: три цели из одного кода

### CMakePresets.json
```json
{  
  "version": 3,  
  "configurePresets": [  
    {      "name": "default",  
      "generator": "Ninja",  
      "binaryDir": "${sourceDir}/.build",  
      "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
    }  
  ]
}
```

### vcpkg.json
```json
{  
    "name": "boost-learning-test",  
    "version": "0.1.0",  
    "dependencies": [  
        "boost-test"  
    ]  
}
```
### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.28)  
project(hybrid_modules LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
set(CMAKE_CXX_EXTENSIONS OFF)  
  
# 1. Внутренняя библиотека реализации (вся логика)  
add_library(calc_impl STATIC src/impl/calculator_impl.cpp)  
target_include_directories(calc_impl PUBLIC src/impl)  
target_compile_features(calc_impl PUBLIC cxx_std_20)  
  
# 2. Модуль-обёртка  
add_library(calc_module)  
target_sources(calc_module  
        PUBLIC FILE_SET CXX_MODULES FILES src/calculator.cppm)  
target_link_libraries(calc_module PUBLIC calc_impl)  
target_compile_features(calc_module PUBLIC cxx_std_20)  
  
# 3. Приложение — через import  
add_executable(app app/main.cpp)  
target_link_libraries(app PRIVATE calc_module)  
  
# --- Тестирование ---  
find_package(Boost REQUIRED COMPONENTS unit_test_framework)  
enable_testing()  
  
# 4. Основные тесты — против реализации (#include, полный доступ)  
add_executable(unit_tests  
        tests/test_main.cpp        tests/test_calculator.cpp        tests/test_internal.cpp)  
target_link_libraries(unit_tests PRIVATE calc_impl Boost::unit_test_framework)  
add_test(NAME unit_tests COMMAND unit_tests)  
  
# 5. Smoke-тест — против модуля (import, проверка export)  
add_executable(smoke_tests  
        tests/smoke_main.cpp        tests/smoke_module.cpp)  
target_link_libraries(smoke_tests PRIVATE calc_module Boost::unit_test_framework)  
target_compile_features(smoke_tests PRIVATE cxx_std_20)  
add_test(NAME smoke_tests COMMAND smoke_tests)
```

Обрати внимание на главный приём: **`unit_tests` линкуется с `calc_impl`, а не с `calc_module`**. Тестовая сборка вообще не касается модулей — никаких `FILE_SET CXX_MODULES`, никаких BMI-проблем, никаких требований к согласованности модульных флагов. А приложение `app` использует уже модуль.

Контраст с основными тестами:

```cmake
# основные тесты — против реализации (полный доступ, без модулей)
target_link_libraries(unit_tests PRIVATE calc_impl Boost::unit_test_framework)

# smoke-тест — против модуля (проверка export, через import)
target_link_libraries(smoke_tests PRIVATE calc_module Boost::unit_test_framework)
```

## Почему это снимает подводные камни

|Проблема модульного тестирования|Как гибридный подход её решает|
|---|---|
|`import` не даёт доступа к внутренним (`detail::`) деталям|Тесты включают заголовок напрямую — видят всё|
|BMI несовместимы между флагами/версиями|Тестовая сборка вообще не использует модули|
|Незрелость модульной поддержки в IDE/линтерах для тестов|Тесты — обычный C++ с `#include`, инструменты их понимают|
|Header-only Boost.Test обесценивает выигрыш модулей|В тестах модулей нет — выигрыш модулей сохраняется в `app`|
|Капризы `import std;` в тестах|Тесты используют обычную stdlib через `#include`|

При этом **продакшен-код (приложение) полноценно использует модули** — современный интерфейс, изоляция, потенциальный выигрыш в компиляции на стороне потребителей модуля.

## Поток зависимостей (наглядно)

```
            calc_impl (вся логика)
           /         |          \
  calc_module    unit_tests    (общая база)
       |         (#include,
       |          логика +
       |          detail::)
   ┌───┴────┐
  app    smoke_tests
(import)  (import, проверка export)
```

Одна реализация — два потребителя: модуль (для приложения) и прямой include (для тестов).

## Когда этот подход оправдан

|Ситуация|Гибрид подходит?|
|---|---|
|Внедряешь модули, но тулчейн ещё капризничает|Да — изолирует риск в одной цели|
|Нужно тестировать внутренние детали реализации|Да — `import` этого не даёт|
|Хочешь современный модульный интерфейс наружу|Да — `app` использует `import`|
|Проект уже уверенно и полностью на модулях|Нет — тогда тестируй через `import` напрямую|
|Логика тривиальна, внутренних деталей нет|Возможно избыточно — проще без обёртки|

## Что именно ловит smoke-тест

|Сценарий поломки|Поймает ли smoke-тест|
|---|---|
|Забыли `export` у `Calculator` в обёртке|**Да** — не скомпилируется (тип не виден)|
|Опечатка в имени модуля (`export module calc;` vs `import calculator;`)|**Да** — ошибка импорта|
|Внутренняя деталь случайно попала в `export`|Косвенно — структурно тест на это можно расширить|
|Сломалась логика `add`/`divide`|Да, но это уже ловят основные тесты — здесь дублируется минимально|
|Проблема сборки самого модуля (BMI, флаги)|**Да** — цель не соберётся|

Главная ценность: основные тесты проверяют **что код делает**, а smoke-тест — **что модуль корректно отдаёт этот код наружу**. Без него можно сломать `export` и не заметить, потому что основные тесты ходят в обход модуля.

## Практический совет по smoke-тестам модулей

Держи их **тонкими и быстрыми**: пара вызовов на каждый экспортированный тип, проверка, что импорт вообще работает. Не переноси сюда проверку бизнес-логики — это размывает разделение и заставляет тестовую сборку модуля расти (со всеми её капризами). Smoke-тест отвечает на один вопрос: «модуль собирается и экспортирует то, что должен?» — и этого достаточно.

## Компромиссы, о которых стоит знать

- **Дублирование границы API:** публичный класс упоминается дважды — в заголовке и в `export`-обёртке. Это цена развязки; зато обёртка тонкая и меняется редко.
- **Тесты не проверяют сам модуль.** Они проверяют логику (`calc_impl`), но не то, что модуль корректно её экспортирует. Это можно закрыть одним отдельным smoke-тестом, который всё же `import`-ит модуль и дёргает публичный метод — чтобы убедиться, что `export` не сломан. Такой тест собирается отдельной модульной целью.
- **Архитектурная дисциплина:** подход подталкивает держать логику в `impl`, а модуль — тонким. Это в целом здоровая структура, но требует осознанности.

## Итог

Гибридный подход разделяет две заботы: **модуль — это интерфейс наружу**, **библиотека — это тестируемая реализация**. Тесты работают с библиотекой по проверенной схеме `#include` + Boost.Test (с полным доступом к внутренностям и без модульных граблей), а приложение пользуется модулем. Так ты получаешь современный модульный фасад, не платя за это болью в тестовой инфраструктуре, пока экосистема модулей дозревает.
