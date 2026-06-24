---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.SmartPtr

Boost.SmartPtr — это набор умных указателей. Разберу каждый по отдельности.

## 1. `boost::shared_ptr<T>`

Указатель с подсчётом ссылок (разделяемое владение). Прообраз `std::shared_ptr`.

### Конструкторы

|Конструктор|Описание|
|---|---|
|`shared_ptr()`|Пустой (хранит `nullptr`, счётчик = 0)|
|`shared_ptr(Y* p)`|Берёт во владение сырой указатель|
|`shared_ptr(Y* p, Deleter d)`|С пользовательским удалителем|
|`shared_ptr(Y* p, Deleter d, Allocator a)`|С удалителем и аллокатором|
|`shared_ptr(const shared_ptr& r)`|Копия (увеличивает счётчик)|
|`shared_ptr(shared_ptr&& r)`|Перемещение|
|`shared_ptr(const weak_ptr<Y>& r)`|Из weak_ptr (бросает `bad_weak_ptr`, если объект уничтожен)|
|`shared_ptr(std::unique_ptr<Y,D>&& r)`|Перенос владения из unique_ptr|
|`shared_ptr(const shared_ptr<Y>& r, T* p)`|Aliasing-конструктор (делит счётчик r, но указывает на p)|

### Методы

|Метод|Описание|
|---|---|
|`T* get() const`|Сырой указатель (без передачи владения)|
|`T& operator*() const`|Разыменование|
|`T* operator->() const`|Доступ к членам|
|`long use_count() const`|Число shared_ptr, владеющих объектом|
|`bool unique() const`|`true`, если `use_count() == 1` (устарело)|
|`explicit operator bool() const`|`true`, если указатель не пустой|
|`void reset()`|Освободить владение (счётчик−1)|
|`void reset(Y* p)`|Заменить управляемый объект|
|`void reset(Y* p, Deleter d)`|Заменить с удалителем|
|`void swap(shared_ptr& other)`|Обмен|

### Создание

```cpp
boost::make_shared<T>(args...)        // рекомендуется: одна аллокация
boost::allocate_shared<T>(alloc, args...)
```

---

## 2. `boost::weak_ptr<T>`

Слабая (не владеющая) ссылка на объект, управляемый `shared_ptr`. Разрывает циклические ссылки.

### Методы

|Метод|Описание|
|---|---|
|`weak_ptr()`|Пустой|
|`weak_ptr(const shared_ptr<Y>&)`|Из shared_ptr|
|`shared_ptr<T> lock() const`|Получить shared_ptr (пустой, если объект уничтожен)|
|`long use_count() const`|Число владеющих shared_ptr|
|`bool expired() const`|`true`, если объект уже уничтожен|
|`void reset()`|Сбросить|
|`void swap(weak_ptr&)`|Обмен|

```cpp
boost::weak_ptr<Node> w = sp;
if (auto sp2 = w.lock()) { /* объект жив, sp2 безопасен */ }
```

---

## 3. `boost::scoped_ptr<T>`

Эксклюзивное владение в пределах области видимости. **Некопируемый, неперемещаемый.** Аналог упрощённого `unique_ptr`.

### Методы

|Метод|Описание|
|---|---|
|`scoped_ptr()` / `scoped_ptr(T* p)`|Пустой / берёт владение|
|`T& operator*()` / `T* operator->()`|Доступ к объекту|
|`T* get() const`|Сырой указатель|
|`explicit operator bool() const`|Не пустой?|
|`void reset(T* p = nullptr)`|Заменить/освободить объект|
|`void swap(scoped_ptr&)`|Обмен|

> Нет `release()` (в отличие от `unique_ptr`) — нельзя «отпустить» владение.

---

## 4. `boost::scoped_array<T>`

Как `scoped_ptr`, но для массивов (`new[]` / `delete[]`).

|Метод|Описание|
|---|---|
|`T& operator[](std::ptrdiff_t i)`|Доступ по индексу|
|`T* get() const`|Сырой указатель|
|`void reset(T* p = nullptr)`|Заменить|
|`void swap(...)`|Обмен|

---

## 5. `boost::shared_array<T>`

Разделяемое владение массивом с подсчётом ссылок.

|Метод|Описание|
|---|---|
|`T& operator[](std::ptrdiff_t i)`|Доступ по индексу|
|`T* get() const`|Сырой указатель|
|`long use_count() const`|Счётчик ссылок|
|`void reset(T* p = nullptr)`|Заменить|

---

## 6. `boost::intrusive_ptr<T>`

Указатель со счётчиком, **встроенным в сам объект** (счётчик хранится в T, а не рядом). Эффективнее по памяти, но требует поддержки от типа.

### Требования

Тип `T` должен предоставить две свободные функции:

```cpp
void intrusive_ptr_add_ref(T* p);  // увеличить счётчик
void intrusive_ptr_release(T* p);  // уменьшить, удалить при 0
```

### Методы

|Метод|Описание|
|---|---|
|`intrusive_ptr()`|Пустой|
|`intrusive_ptr(T* p, bool add_ref = true)`|Из сырого указателя|
|`T* get() const`|Сырой указатель|
|`T& operator*()` / `T* operator->()`|Доступ|
|`void reset()` / `void reset(T* p)`|Сбросить/заменить|
|`explicit operator bool() const`|Не пустой?|

> В стандарте аналога нет — это уникальная возможность Boost.

---

## 7. `boost::enable_shared_from_this<T>`

База, позволяющая объекту безопасно получить `shared_ptr` на самого себя.

|Метод|Описание|
|---|---|
|`shared_ptr<T> shared_from_this()`|shared_ptr на этот объект|
|`weak_ptr<T> weak_from_this()`|weak_ptr на этот объект|

```cpp
struct Widget : boost::enable_shared_from_this<Widget> {
    boost::shared_ptr<Widget> self() { return shared_from_this(); }
};
auto w = boost::make_shared<Widget>(); // ОБЯЗАТЕЛЬНО через shared_ptr
auto s = w->self();
```

---

## 8. `boost::local_shared_ptr<T>`

Однопоточный `shared_ptr` без атомарного счётчика — быстрее, когда многопоточность не нужна. Создаётся через `boost::make_local_shared<T>(...)`. (В стандарте аналога нет.)

---

## Связанные сущности

| Имя                                      | Описание                                                        |
| ---------------------------------------- | --------------------------------------------------------------- |
| `boost::bad_weak_ptr`                    | Исключение при конструировании shared_ptr из истёкшего weak_ptr |
| `boost::make_shared<T>(...)`             | Создание shared_ptr одной аллокацией                            |
| `boost::allocate_shared<T>(...)`         | То же с пользовательским аллокатором                            |
| `boost::make_local_shared<T>(...)`       | Создание local_shared_ptr                                       |
| `boost::static_pointer_cast<T>(sp)`      | static_cast для shared_ptr                                      |
| `boost::dynamic_pointer_cast<T>(sp)`     | dynamic_cast для shared_ptr                                     |
| `boost::const_pointer_cast<T>(sp)`       | const_cast для shared_ptr                                       |
| `boost::reinterpret_pointer_cast<T>(sp)` | reinterpret_cast для shared_ptr                                 |
| `boost::get_deleter<D>(sp)`              | Доступ к удалителю shared_ptr                                   |

## Сводка: какой указатель выбрать

|Сценарий|Указатель|
|---|---|
|Разделяемое владение|`shared_ptr`|
|Не владеющая ссылка / разрыв циклов|`weak_ptr`|
|Эксклюзивное владение в scope|`scoped_ptr`|
|Эксклюзивный массив|`scoped_array`|
|Разделяемый массив|`shared_array`|
|Счётчик внутри объекта|`intrusive_ptr`|
|`shared_ptr` на себя изнутри класса|`enable_shared_from_this`|
|Разделяемое владение в один поток|`local_shared_ptr`|

## Отличия от стандартных умных указателей

- `boost::shared_ptr` ↔ `std::shared_ptr`; `boost::weak_ptr` ↔ `std::weak_ptr` — практически идентичны по API.
- `boost::scoped_ptr` ≈ упрощённый `std::unique_ptr` (но **без** `release()` и без перемещения). В новом коде обычно берут `std::unique_ptr`.
- `boost::scoped_array` / `shared_array` ↔ `std::unique_ptr<T[]>` / `std::shared_ptr<T[]>` (последнее с C++17).
- `boost::intrusive_ptr` и `boost::local_shared_ptr` — **аналогов в стандарте нет**, это весомые причины использовать Boost.
- `enable_shared_from_this` есть в обоих; в boost добавлен `weak_from_this()` (в стандарт пришёл в C++17).
- Касты: `boost::static_pointer_cast` и др. ↔ `std::static_pointer_cast` и др.

В современном коде для базовых сценариев предпочитают `std::`-указатели; Boost.SmartPtr остаётся ценным ради `intrusive_ptr`, `local_shared_ptr` и при поддержке старых кодовых баз.


### include/test_smart_ptr.h
```cpp
#pragma once  
  
#include <boost/smart_ptr.hpp>  
  
namespace test_smart_ptr {  
    struct Node {  
        int value;  
        boost::shared_ptr<Node> next;  
        boost::weak_ptr<Node> prev;  
        Node(int _value): value{_value} {}  
    };    void test();  
}
```

### src/test_smart_ptr.cpp
```cpp
#include "test_smart_ptr.h"  
  
#include <iostream>  
#include <format>  
  
namespace test_smart_ptr {  
  
void test() {  
    auto&& a = boost::make_shared<Node>(42);  
    auto&& b = boost::make_shared<Node>(12);  
  
    a->next = b;  
    b->prev = a;  
  
    std::cout << std::format("a.use_count: {}\n", a.use_count());  
    std::cout << std::format("b.use_count: {}\n", b.use_count());  
  
    if (auto p = b->prev.lock()) {  
        std::cout << std::format("prev value: {}\n", p->value);  
    }  
    boost::shared_ptr<void> v = a;  
    auto&& back = boost::static_pointer_cast<Node>(v);  
}  
  
}
```

```
a.use_count: 1
b.use_count: 2
prev value: 42
```

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