---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Any

Контейнер для значения любого типа (стирание типов). Аналог `std::any`.

CMake: `find_package(Boost REQUIRED)` + `target_link_libraries(app PRIVATE Boost::boost)` (header-only интерфейс).

## Класс

```cpp
class boost::any;
```

Контейнер на основе стирания типов (type erasure): хранит значение **любого** копируемого типа, помня его настоящий тип во время выполнения.

## Конструкторы

|Конструктор|Описание|
|---|---|
|`any()`|Пустой контейнер (значения нет)|
|`any(const ValueType& value)`|Сохраняет копию значения произвольного типа|
|`any(ValueType&& value)`|Move-сохранение значения|
|`any(const any& other)`|Копирование (копирует и хранимое значение)|
|`any(any&& other)`|Перемещение|

## Доступ к значению (свободные функции)

|Функция|Описание|
|---|---|
|`boost::any_cast<T>(const any&)`|Возвращает **копию** значения типа `T`; бросает `bad_any_cast` при несовпадении|
|`boost::any_cast<T>(any&)`|Возвращает значение/ссылку типа `T`; бросает `bad_any_cast` при несовпадении|
|`boost::any_cast<T>(const any*)`|Возвращает `const T*` или `nullptr`, если тип другой (без исключения)|
|`boost::any_cast<T>(any*)`|Возвращает `T*` или `nullptr`, если тип другой (без исключения)|
|`boost::unsafe_any_cast<T>(any*)`|Без проверки типа (UB при ошибке; для внутреннего/оптимизированного использования)|

## Методы

|Метод|Описание|
|---|---|
|`bool empty() const`|`true`, если значение не хранится|
|`void clear()`|Очистить (привести к пустому состоянию)|
|`const std::type_info& type() const`|RTTI-тип хранимого значения; `typeid(void)`, если пусто|
|`any& swap(any&)`|Обмен содержимым|
|`operator=(const ValueType&)` / `operator=(ValueType&&)`|Присвоить значение любого типа|
|`operator=(const any&)` / `operator=(any&&)`|Присваивание от другого `any`|

## Связанные сущности

|Имя|Описание|
|---|---|
|`boost::bad_any_cast`|Исключение (наследник `std::bad_cast`) при неверном `any_cast` по ссылке/значению|

## Ключевые правила использования

- Тип при извлечении должен **точно** совпадать с типом при сохранении — никаких неявных преобразований (`int` сохранён → только `any_cast<int>`, не `any_cast<long>`).
- Указательная форма `any_cast<T>(&a)` — безопасный способ проверки без исключений (идиома, аналогичная `dynamic_cast` по указателю).
- Хранимый тип должен быть **copy-constructible**.
## any против variant

| |`boost::any`|`boost::variant<...>`|
|---|---|---|
|Набор типов|Любой (открытый)|Заданный заранее (закрытый)|
|Хранение|Динамическая память (обычно)|Inline, по размеру наибольшего типа|
|Проверка типов|Во время выполнения|Во время выполнения, но типы известны компилятору|
|Визитация|Нет (только `any_cast`)|Есть (`apply_visitor`)|
|Когда применять|Тип заранее неизвестен|Конечный известный набор типов|

## Отличия от `std::any` (C++17)

- `boost::any` существует с давних версий, `std::any` появился в C++17 и в основном повторяет интерфейс.
- `a.empty()` ↔ `a.has_value()` (инвертировано); `a.clear()` ↔ `a.reset()`.
- `std::any` добавляет `emplace<T>(args...)` для конструирования «на месте» — у `boost::any` его нет.
- Исключение: `boost::bad_any_cast` ↔ `std::bad_any_cast`.
- `any_cast` по указателю работает одинаково в обоих; формы по ссылке/значению идентичны по семантике.
- `std::any` имеет оптимизацию малых объектов (SBO) по стандарту-рекомендации; в `boost::any` хранение почти всегда через кучу.

### include/test_any.h
```cpp
#pragma once  
  
#include <boost/any.hpp>  
  
namespace test_any {  
    void test();  
}
```

### src/test_any.cpp
```cpp
#include "test_any.h"  
  
#include <iostream>  
#include <format>  
#include <vector>  
#include <string>  
  
namespace test_any {  
  
void test() {  
    std::vector<boost::any> items;  
    items.push_back(42);  
    items.push_back(std::string{"hello"});  
    items.push_back(3.14159);  
  
    if (int* p = boost::any_cast<int>(&items[0])) {  
        std::cout << std::format("[boost::any][any_cast][0] {}\n", *p);  
    }    if (std::string* p = boost::any_cast<std::string>(&items[1])) {  
        std::cout << std::format("[boost::any][any_cast][1] {}\n", *p);  
    }  
    try {  
        boost::any_cast<int>(items[1]);  
    } catch (const boost::bad_any_cast& e) {  
        std::cout << std::format("[boost::any][boost::bad_any_cast] {}\n", e.what());  
    }  
    std::cout << std::format("[boost::any][info] {}\n", items[2].type().name());  
  
    auto&& is_empty = [](boost::any& v) {  
        std::cout << std::boolalpha << v.empty() << '\n';  
    };  
    boost::any a;  
    is_empty(a);  
  
    a = 1000;  
    is_empty(a);  
  
    a.clear();  
    is_empty(a);  
}  
  
}
```

---
---
---
---

### 1.4 Boost.SmartPtr

Умные указатели. `shared_ptr`/`weak_ptr`/`scoped_ptr`. Бóльшая часть в стандарте, но Boost даёт `intrusive_ptr`, `local_shared_ptr` и историю появления RAII-указателей.

```cpp
#include <boost/smart_ptr.hpp>
#include <iostream>

struct Node {
    int value;
    boost::shared_ptr<Node> next;
    Node(int v) : value(v) {}
};

int main() {
    auto a = boost::make_shared<Node>(1);
    a->next = boost::make_shared<Node>(2);
    std::cout << a->value << " -> " << a->next->value << "\n";
    std::cout << "use_count: " << a.use_count() << "\n";
}
```

**Ключевое:** `make_shared`, `weak_ptr` против циклических ссылок, `intrusive_ptr`, отличия от `std::shared_ptr`.

---
---
---
---

## Этап 2. Контейнеры и работа с системой

### 2.1 Boost.Container

Расширенные контейнеры: `flat_map`, `flat_set`, `small_vector`, `static_vector`, `stable_vector`. Header-only.

```cpp
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>
#include <iostream>

int main() {
    boost::container::flat_map<int, std::string> m;
    m[3] = "three"; m[1] = "one"; m[2] = "two";
    for (auto& [k, v] : m) std::cout << k << "=" << v << "\n"; // отсортировано

    boost::container::small_vector<int, 4> sv{1, 2, 3}; // первые 4 на стеке
    sv.push_back(4);
    std::cout << "size: " << sv.size() << "\n";
}
```

**Ключевое:** `flat_map` (хранение в отсортированном массиве, cache-friendly) против `std::map`; `small_vector` и оптимизация размещения на стеке.

---
---
---

### 2.2 Boost.Filesystem

Работа с путями, файлами, директориями. **Требует линковки.** Прообраз `std::filesystem`.

```cpp
#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

int main() {
    fs::path p = fs::current_path();
    std::cout << "Current: " << p << "\n";
    for (auto& entry : fs::directory_iterator(p)) {
        std::cout << (fs::is_directory(entry) ? "[D] " : "[F] ")
                  << entry.path().filename().string() << "\n";
    }
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS filesystem)
target_link_libraries(app PRIVATE Boost::filesystem)
```

**Ключевое:** `path`, обход директорий, `error_code`-перегрузки против исключений, отличия от `std::filesystem`.

---
---
---
---

## Этап 3. Текст и парсинг

### 3.1 Boost.Regex

Регулярные выражения. **Требует линковки** (хотя есть и header-only режим). Прообраз `std::regex`, но Boost-версия часто быстрее и богаче.

```cpp
#include <boost/regex.hpp>
#include <iostream>

int main() {
    boost::regex re(R"((\w+)@(\w+)\.(\w+))");
    std::string text = "contact: user@example.com";
    boost::smatch m;
    if (boost::regex_search(text, m, re)) {
        std::cout << "full: " << m[0] << "\n";
        std::cout << "user: " << m[1] << ", domain: " << m[2] << "\n";
    }
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS regex)
target_link_libraries(app PRIVATE Boost::regex)
```

**Ключевое:** `regex_search` против `regex_match`, группы захвата, `regex_replace`, почему иногда предпочитают boost вместо `std::regex`.

---
---
---

### 3.2 Boost.Spirit

Построение парсеров прямо на C++ через выражения-грамматики (EBNF в виде кода). Header-only, но тяжёлая для компилятора. Один из самых сложных модулей — изучай после уверенного владения шаблонами.

```cpp
#include <boost/spirit/home/x3.hpp>
#include <iostream>
#include <vector>

namespace x3 = boost::spirit::x3;

int main() {
    std::string input = "1, 2, 3, 42";
    std::vector<int> nums;
    auto it = input.begin();
    bool ok = x3::phrase_parse(
        it, input.end(),
        x3::int_ % ',',   // числа через запятую
        x3::space, nums);
    if (ok && it == input.end())
        for (int n : nums) std::cout << n << " ";
    std::cout << "\n";
}
```

**Ключевое:** начинай с **Spirit X3** (современная версия), оператор `%`, атрибуты парсеров, `phrase_parse`. Это самый трудоёмкий модуль — закладывай больше времени.

---
---
---
---

## Этап 4. Числа произвольной точности

### 4.1 Boost.Multiprecision

Целые, рациональные и числа с плавающей точкой произвольной точности. В основном header-only; для backend на GMP/MPFR нужны внешние библиотеки (`cpp_int` работает без них).

```cpp
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>

using boost::multiprecision::cpp_int;

int main() {
    cpp_int factorial = 1;
    for (int i = 1; i <= 50; ++i) factorial *= i;
    std::cout << "50! = " << factorial << "\n"; // огромное число целиком
}
```

**Ключевое:** `cpp_int` (без зависимостей), `cpp_dec_float`/`cpp_bin_float`, backend GMP/MPFR и когда они нужны.

---
---
---
---

## Этап 5. Графы

### 5.1 Boost.Graph (BGL)

Структуры данных и алгоритмы для графов. Header-only, но с крутой кривой обучения из-за интенсивного использования шаблонов и property maps.

```cpp
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <vector>

using namespace boost;

int main() {
    typedef adjacency_list<vecS, vecS, directedS,
        no_property, property<edge_weight_t, int>> Graph;

    Graph g(4);
    add_edge(0, 1, 1, g);
    add_edge(1, 2, 2, g);
    add_edge(0, 2, 5, g);
    add_edge(2, 3, 1, g);

    std::vector<int> dist(num_vertices(g));
    dijkstra_shortest_paths(g, 0,
        distance_map(make_iterator_property_map(
            dist.begin(), get(vertex_index, g))));

    for (size_t i = 0; i < dist.size(); ++i)
        std::cout << "0 -> " << i << " = " << dist[i] << "\n";
}
```

**Ключевое:** `adjacency_list` и выбор селекторов (`vecS`/`listS`), property maps (самая сложная концепция BGL), обходы (BFS/DFS), Dijkstra, визиторы.

---
---
---
---

## Этап 6. Сеть и асинхронность

### 6.1 Boost.Asio

Асинхронный ввод/вывод, таймеры, сокеты. Сердцевина сетевого программирования на Boost. В основном header-only, но требует системных библиотек (на Windows — ws2_32, на Linux — pthread; vcpkg/CMake подтянут зависимости автоматически).

Начни с синхронного примера, затем переходи к асинхронному.

```cpp
#include <boost/asio.hpp>
#include <iostream>

int main() {
    boost::asio::io_context io;
    boost::asio::steady_timer timer(io, std::chrono::seconds(2));

    timer.async_wait([](const boost::system::error_code& ec) {
        if (!ec) std::cout << "Таймер сработал!\n";
    });

    std::cout << "Ожидание...\n";
    io.run(); // блокируется, пока есть незавершённые операции
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

**Ключевое:** `io_context` и его роль, модель completion handlers, `async_*` операции, strands для синхронизации, корутины (`co_await` с `boost::asio::awaitable`). Это фундамент для Beast — изучай тщательно.

---
---
---

### 6.2 Boost.Beast

HTTP и WebSocket поверх Asio. Изучается **только после** уверенного владения Asio. Header-only, зависит от Asio.

```cpp
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
    net::io_context io;
    tcp::resolver resolver(io);
    beast::tcp_stream stream(io);

    auto results = resolver.resolve("example.com", "80");
    stream.connect(results);

    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::host, "example.com");
    req.set(http::field::user_agent, "Beast");
    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream, buffer, res);
    std::cout << res.base() << "\n"; // заголовки ответа

    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
}
```

CMake:

```cmake
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
target_link_libraries(app PRIVATE Boost::system Threads::Threads)
```

**Ключевое:** структуры `request`/`response`, синхронный HTTP-клиент → асинхронный → простой HTTP-сервер → WebSocket. Самый практичный модуль для веб-задач.

---
---
---
---

## Этап 7. Тестирование

### 7.1 Boost.Test

Фреймворк юнит-тестирования. **Требует линковки** (для unit_test_framework). Изучи раньше остальных по желанию — полезно писать тесты ко всем учебным примерам.

```cpp
#define BOOST_TEST_MODULE MyTests
#include <boost/test/included/unit_test.hpp>

int add(int a, int b) { return a + b; }

BOOST_AUTO_TEST_CASE(addition_works) {
    BOOST_CHECK_EQUAL(add(2, 3), 5);
    BOOST_TEST(add(-1, 1) == 0);
}

BOOST_AUTO_TEST_CASE(edge_cases) {
    BOOST_CHECK(add(0, 0) == 0);
}
```

CMake (вариант со скомпилированной библиотекой):

```cmake
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
enable_testing()
add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE Boost::unit_test_framework)
add_test(NAME tests COMMAND tests)
```

Запуск: `ctest --test-dir build --output-on-failure`.

**Ключевое:** `BOOST_CHECK` против `BOOST_REQUIRE`, `BOOST_TEST`, test suites, fixtures, интеграция с CTest. Совет: используй `<boost/test/included/...>` для header-only режима в маленьких проектах и линкуемую версию — в больших.

---
---
---
---

## Этап 8. Интеграция с Python

### 8.1 Boost.Python

Связывание C++ и Python. **Требует линковки** и установленного Python. Самый зависимый от окружения модуль — оставь напоследок.

```cpp
// hello_ext.cpp
#include <boost/python.hpp>

char const* greet() { return "Привет из C++!"; }
int square(int x) { return x * x; }

BOOST_PYTHON_MODULE(hello_ext) {
    using namespace boost::python;
    def("greet", greet);
    def("square", square);
}
```

CMake (сборка как разделяемая библиотека-модуль):

```cmake
find_package(Boost REQUIRED COMPONENTS python)
find_package(Python3 REQUIRED COMPONENTS Development)

add_library(hello_ext MODULE hello_ext.cpp)
target_link_libraries(hello_ext PRIVATE Boost::python Python3::Python)
set_target_properties(hello_ext PROPERTIES PREFIX "" SUFFIX ".pyd") # Windows
# На Linux: SUFFIX ".so"
```

Использование из Python:

```python
import hello_ext
print(hello_ext.greet())
print(hello_ext.square(7))
```

**Ключевое:** экспорт функций, классов (`class_<>`), конвертеры типов, управление GIL. Альтернатива — pybind11 (легче, header-only), стоит знать о ней.

---

## Сводная таблица

|Этап|Модуль|Линковка|Сложность|Время (ориентир)|
|---|---|---|---|---|
|1|Optional|нет|низкая|0.5 дня|
|1|Variant|нет|низкая|0.5 дня|
|1|Any|нет|низкая|0.5 дня|
|1|SmartPtr|нет|низкая|1 день|
|2|Container|нет|средняя|1 день|
|2|Filesystem|да|низкая|1 день|
|3|Regex|да|средняя|1 день|
|3|Spirit|нет|высокая|3–4 дня|
|4|Multiprecision|нет|низкая|0.5 дня|
|5|Graph (BGL)|нет|высокая|3–4 дня|
|6|Asio|частично|высокая|4–5 дней|
|6|Beast|нет*|высокая|3–4 дня|
|7|Test|да|средняя|1–2 дня|
|8|Python|да|средняя|2–3 дня|

* Beast зависит от Asio и его системных библиотек.

---

## Практические советы

Веди один репозиторий, где каждый модуль — отдельная подпапка с собственным `add_executable` и мини-README с выводами. Пиши тесты на Boost.Test ко всем примерам — заодно закрепишь и его. Если какой-то модуль уже есть в стандарте (Optional, Variant, Any, SmartPtr, Filesystem, Regex), сравни boost-версию со стандартной и зафиксируй отличия — это частый вопрос на собеседованиях и реальная проблема в legacy-коде.

Хочешь, соберу стартовый шаблон репозитория целиком — с `CMakePresets.json`, структурой папок и готовым `CMakeLists.txt` верхнего уровня, который подключает все подпроекты?