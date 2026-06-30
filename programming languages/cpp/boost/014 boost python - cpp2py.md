---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.Python

Boost.Python — библиотека для связывания C++ и Python: вызов C++-кода из Python (и наоборот). **Требует линковки** и установленного Python с заголовками для разработки.

```cpp
#include <boost/python.hpp>
using namespace boost::python;
```

> Важное предупреждение сразу: для **нового** кода многие выбирают **pybind11** — это header-only библиотека с похожим API, но проще в сборке и без зависимости от Boost. Boost.Python зрелее и мощнее в ряде сценариев, но тяжелее. Разберу Boost.Python, в конце сравню.

## Что Boost.Python даёт

Двусторонний мост между языками: экспорт C++-функций и классов в Python-модуль, автоматическая конвертация типов (числа, строки, контейнеры), проброс исключений, управление временем жизни объектов между сборщиком мусора Python и RAII C++.

## 1. Минимальный модуль: `BOOST_PYTHON_MODULE`

Макрос определяет точку входа Python-модуля. Имя модуля **должно совпадать** с именем итоговой библиотеки.

```cpp
#include <boost/python.hpp>

char const* greet() {
    return "Привет из C++!";
}

int square(int x) {
    return x * x;
}

// имя модуля = hello_ext → библиотека должна называться hello_ext.so/.pyd
BOOST_PYTHON_MODULE(hello_ext) {
    using namespace boost::python;
    def("greet", greet);     // экспорт функции
    def("square", square);
}
```

Использование из Python:

```python
import hello_ext
print(hello_ext.greet())   # Привет из C++!
print(hello_ext.square(7)) # 49
```

|Элемент|Назначение|
|---|---|
|`BOOST_PYTHON_MODULE(name)`|Объявить Python-модуль `name`|
|`def("py_name", cpp_func)`|Экспортировать C++-функцию под именем для Python|
## 2. Экспорт классов: `class_<>`

```cpp
#include <boost/python.hpp>

class Animal {
public:
    Animal(std::string name) : name_(name) {}
    std::string name() const { return name_; }
    void set_name(const std::string& n) { name_ = n; }
    std::string speak() const { return name_ + " издаёт звук"; }
private:
    std::string name_;
};

BOOST_PYTHON_MODULE(zoo) {
    using namespace boost::python;

    class_<Animal>("Animal", init<std::string>())  // конструктор с аргументом
        .def("speak", &Animal::speak)              // метод
        .add_property("name",                       // свойство (getter/setter)
                      &Animal::name, &Animal::set_name);
}
```

```python
import zoo
a = zoo.Animal("Кот")
print(a.speak())   # Кот издаёт звук
print(a.name)      # Кот  (через property)
a.name = "Пёс"     # вызовет set_name
```

|Конструкция|Назначение|
|---|---|
|`class_<T>("PyName")`|Экспорт класса `T` под именем `PyName`|
|`init<Args...>()`|Объявить конструктор с указанными типами аргументов|
|`.def("method", &T::method)`|Экспорт метода|
|`.def_readonly("f", &T::field)`|Поле только для чтения|
|`.def_readwrite("f", &T::field)`|Поле для чтения и записи|
|`.add_property("p", getter, setter)`|Python-property из getter/setter|
|`.def(self + self)`|Экспорт перегруженного оператора|

## 3. Конструкторы и перегрузки

```cpp
class_<Point>("Point", init<double, double>())  // основной конструктор
    .def(init<>())                              // дополнительный (по умолчанию)
    .def(init<double>())                        // ещё один
    .def_readwrite("x", &Point::x)
    .def_readwrite("y", &Point::y);
```

Перегруженные методы требуют явного указания сигнатуры (через приведение указателя):

```cpp
// если есть int area(int) и double area(double):
.def("area", static_cast<int(Point::*)(int)>(&Point::area))
```

## 4. Наследование

Boost.Python умеет пробрасывать иерархии классов в Python:

```cpp
class Base {
public:
    virtual std::string kind() const { return "base"; }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    std::string kind() const override { return "derived"; }
};

BOOST_PYTHON_MODULE(hierarchy) {
    using namespace boost::python;

    class_<Base>("Base")
        .def("kind", &Base::kind);

    // bases<Base> сообщает Python об отношении наследования
    class_<Derived, bases<Base>>("Derived")
        .def("kind", &Derived::kind);
}
```

|Конструкция|Назначение|
|---|---|
|`class_<Derived, bases<Base>>`|Объявить наследование для Python|

## 5. Виртуальные функции с переопределением в Python

Чтобы Python-класс мог переопределить виртуальный метод C++, нужен **wrapper-класс**:

```cpp
struct BaseWrap : Base, wrapper<Base> {
    std::string kind() const override {
        // если Python переопределил метод — вызвать его
        if (override f = this->get_override("kind"))
            return f();
        // иначе — поведение по умолчанию
        return Base::kind();
    }
    std::string default_kind() const { return Base::kind(); }
};

BOOST_PYTHON_MODULE(virt) {
    class_<BaseWrap, boost::noncopyable>("Base")
        .def("kind", &Base::kind, &BaseWrap::default_kind);
}
```

Это позволяет на Python отнаследоваться от C++-класса и переопределить его виртуальные методы — вызовы из C++ будут попадать в Python-реализацию.

|Конструкция|Назначение|
|---|---|
|`wrapper<T>`|База для класса-обёртки с поддержкой override|
|`get_override("name")`|Получить Python-переопределение метода|
|`override`|Тип результата `get_override`|

## 6. Конвертация типов

Базовые типы конвертируются автоматически: `int`, `double`, `bool`, `std::string` ↔ Python-аналоги. Для контейнеров и сложных типов есть инструменты.

### Объект Python в C++: `object`

```cpp
void process(object obj) {
    // object — обёртка над любым Python-значением
    int n = extract<int>(obj);          // извлечь как int (бросит, если не int)
    std::string s = extract<std::string>(obj);
}
```

### `extract<T>` — безопасное извлечение

```cpp
extract<int> e(py_obj);
if (e.check())          // можно ли извлечь как int?
    int value = e();    // извлекаем
```

### Списки, словари, кортежи

```cpp
list make_list() {
    list result;
    result.append(1);
    result.append("two");
    result.append(3.0);
    return result;          // вернётся как Python list
}

dict make_dict() {
    dict d;
    d["key"] = "value";
    d["count"] = 42;
    return d;
}
```

|Тип Boost.Python|Python-аналог|
|---|---|
|`object`|любой объект|
|`list`|`list`|
|`dict`|`dict`|
|`tuple`|`tuple`|
|`str`|`str`|
|`extract<T>(obj)`|привести к C++-типу T|

## 7. Управление временем жизни: call policies

Главная тонкость связывания — кто владеет объектом и когда его удалять. Возврат указателей/ссылок требует явной политики, иначе утечки или висячие ссылки.

```cpp
class Factory {
public:
    Widget* create() { return new Widget(); }    // кто удалит?
    Widget& get_ref() { return widget_; }         // ссылка на член
private:
    Widget widget_;
};

class_<Factory>("Factory")
    // Python становится владельцем нового объекта:
    .def("create", &Factory::create,
         return_value_policy<manage_new_object>())
    // объект живёт, пока жив Factory:
    .def("get_ref", &Factory::get_ref,
         return_internal_reference<>());
```

|Политика|Когда применять|
|---|---|
|`return_value_policy<manage_new_object>`|Возврат `new`-объекта; Python им владеет и удалит|
|`return_value_policy<reference_existing_object>`|Возврат ссылки/указателя без передачи владения (осторожно!)|
|`return_internal_reference<>`|Возврат ссылки на член; время жизни привязано к родителю|
|`return_value_policy<copy_const_reference>`|Вернуть копию по const-ссылке|
|`with_custodian_and_ward<>`|Связать время жизни двух объектов|

> Это самая частая причина ошибок в Boost.Python: неправильная политика ведёт к падениям (use-after-free) или утечкам. Всегда продумывай владение для методов, возвращающих указатели/ссылки.

## 8. Проброс исключений

C++-исключения автоматически превращаются в Python-исключения:

```cpp
void risky() {
    throw std::runtime_error("что-то сломалось");
}
// в Python: hello_ext.risky() бросит RuntimeError
```

Можно зарегистрировать кастомный перевод:

```cpp
void translate(const MyException& e) {
    PyErr_SetString(PyExc_ValueError, e.what());  // → Python ValueError
}

BOOST_PYTHON_MODULE(mymod) {
    register_exception_translator<MyException>(&translate);
}
```

## 9. GIL — глобальная блокировка интерпретатора

Python имеет GIL: одновременно Python-код исполняет только один поток. При длительных C++-вычислениях GIL стоит **отпускать**, чтобы не блокировать другие Python-потоки:

```cpp
void heavy_computation() {
    // отпустить GIL на время тяжёлой C++-работы
    Py_BEGIN_ALLOW_THREADS
    // ... долгие вычисления без обращений к Python ...
    Py_END_ALLOW_THREADS
}
```

|Макрос|Назначение|
|---|---|
|`Py_BEGIN_ALLOW_THREADS`|Отпустить GIL (начало C++-секции)|
|`Py_END_ALLOW_THREADS`|Захватить GIL обратно|

> Внутри секции без GIL **нельзя** обращаться к Python-объектам — только к чистому C++.

## Полный практический пример

### vcpkg.json
```json
{  
    "name": "boost-learning-test",  
    "version": "0.1.0",  
    "dependencies": [  
        "boost-python"  
    ]  
}
```

### CMakePresets.json
```json
{  
    "version": 3,  
    "configurePresets": [  
        {            "name": "default",  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/.build",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",  
            "cacheVariables": {  
                "VCPKG_INSTALLED_DIR": "C:/projects/vcpkg_installed/kb_cpp2py",  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }    ],    "buildPresets": [  
        {            "name": "default",  
            "configurePreset": "default"  
        }  
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.21)  
project(geometry LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
# Python с компонентами для разработки  
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)  
find_package(Boost REQUIRED COMPONENTS python)  
  
# модуль собирается как РАЗДЕЛЯЕМАЯ библиотека особого вида  
add_library(geometry MODULE geometry.cpp)  
target_link_libraries(geometry PRIVATE  
        Boost::python  
        Python3::Python)  
  
# имя итогового файла должно совпадать с именем в BOOST_PYTHON_MODULE  
set_target_properties(geometry PROPERTIES  
        PREFIX ""                       # убрать "lib" префикс  
)  
# расширение: .pyd на Windows, .so на Linux/macOS  
if(WIN32)  
    set_target_properties(geometry PROPERTIES SUFFIX ".pyd")  
else()  
    set_target_properties(geometry PROPERTIES SUFFIX ".so")  
endif()
```

Критично: **имя в `BOOST_PYTHON_MODULE(hello_ext)`, имя CMake-цели и имя итогового файла должны совпадать** (`hello_ext`). Иначе `import hello_ext` не найдёт модуль.

```bash
cmake --preset default  
cmake --build .build

# затем из директории с hello_ext.so/.pyd:
python -c "import hello_ext; print(hello_ext.greet())"
```

### geometry.cpp
```cpp
/*  
cmake --preset default  
cmake --build .build  
& "C:\projects\vcpkg_installed\kb_cpp2py\x64-windows\tools\python3\python.exe" "C:\projects\knowledge_base\programming languages\cpp\boost\code boost cpp2py\geometry_demo.py"  
*/  
  
#include <boost/python.hpp>  
#include <cmath>  
#include <vector>  
#include <string>  
#include <format>  
  
class Point {  
public:  
    Point(): x_{0}, y_{0} {}  
    Point(const double _x, const double _y): x_{_x}, y_{_y} {}  
  
    double x() const { return x_; }  
    double y() const { return y_; }  
    void set_x(const double _x) { x_ = _x; }  
    void set_y(const double _y) { y_ = _y; }  
  
    double distance_to(const Point& _other) const {  
        double dx = x_ - _other.x_;  
        double dy = y_ - _other.y_;  
        return std::sqrt(dx * dx + dy * dy);  
    }  
    std::string to_string() const {  
        return std::format("Point({}, {})", x_, y_);  
    }private:  
    double x_, y_;  
};  
  
double polygon_perimeter(const std::vector<Point>& _points) {  
    if (_points.size() < 2) return 0.0;  
  
    double total{0.0};  
    for (size_t i{}; i < _points.size(); ++i) {  
        const Point& a = _points[i];  
        const Point& b = _points[(i + 1) % _points.size()];  
  
        total += a.distance_to(b);  
    }  
    return total;  
}  
  
BOOST_PYTHON_MODULE(geometry) {  
    boost::python::class_<Point>(  
            "Point",  
            boost::python::init<double, double>()  
        )        .def(boost::python::init<>())  
        .add_property("x", &Point::x, &Point::set_x)  
        .add_property("y", &Point::y, &Point::set_y)  
        .def("distance_to", &Point::distance_to)  
        .def("__repr__", &Point::to_string);  
  
    boost::python::def("polygon_perimeter", polygon_perimeter);  
}
```

### geometry_demo.py
```python
# & "C:\projects\vcpkg_installed\kb_cpp2py\x64-windows\tools\python3\python.exe" "C:\projects\knowledge_base\programming languages\cpp\boost\code boost cpp2py\geometry_demo.py"  
  
import os, sys  
build_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".build")  
if os.name == "nt":  
    os.add_dll_directory(build_dir)  
if build_dir not in sys.path:  
    sys.path.insert(0, build_dir)  
  
import geometry  
  
p1 = geometry.Point(0, 0)  
p2 = geometry.Point(3, 4)  
print(p1.distance_to(p2))  
print(p1)  
  
p1.x = 10  
print(p1.x)
```

## Сводка ключевых концепций

|Концепция|Суть|
|---|---|
|`BOOST_PYTHON_MODULE(name)`|Точка входа модуля (имя = имя файла)|
|`def("name", func)`|Экспорт свободной функции|
|`class_<T>("Name")`|Экспорт класса|
|`init<Args...>`|Объявление конструкторов|
|`.def(...)` / `.add_property(...)`|Методы и свойства|
|`bases<Base>`|Наследование в Python|
|`wrapper<T>` + `get_override`|Переопределение виртуальных методов на Python|
|`object`, `list`, `dict`, `extract<T>`|Работа с Python-типами из C++|
|Call policies|Управление владением при возврате указателей/ссылок|
|`register_exception_translator`|Перевод C++-исключений в Python|
|GIL (`Py_BEGIN_ALLOW_THREADS`)|Освобождение блокировки при тяжёлых вычислениях|

## Практические советы

- **Имена должны совпадать:** `BOOST_PYTHON_MODULE(X)`, CMake-цель `X`, файл `X.so`/`X.pyd`. Это причина ошибки №1 у новичков (`ImportError`).
- **Call policies — главная ловушка.** Любой метод, возвращающий указатель или ссылку, требует осознанного выбора политики владения, иначе падения или утечки. Возвращаешь `new` — `manage_new_object`; возвращаешь ссылку на член — `return_internal_reference`.
- **Начинай с функций, потом классы.** Сначала экспортируй пару свободных функций, добейся успешного `import`, и только потом усложняй классами и наследованием.
- **`__repr__` и `__str__`** делают объекты дружелюбными в Python-консоли — экспортируй их через `.def("__repr__", ...)`.
- **Отпускай GIL** в долгих чистых C++-вычислениях, иначе многопоточный Python-код встанет.
- **Версия Python имеет значение:** Boost.Python собирается под конкретную версию Python. Модуль, собранный под 3.11, не загрузится в 3.12 — следи за согласованностью версий в vcpkg и окружении.

## Boost.Python против pybind11

|Критерий|Boost.Python|pybind11|
|---|---|---|
|Подключение|Линкуемая библиотека|Header-only|
|Зависимость от Boost|Да|Нет|
|Сборка|Сложнее (линковка, версии)|Проще|
|C++-стандарт|Работает и на старых|Требует C++11+|
|API|Похожий, чуть многословнее|Современнее, лаконичнее|
|Зрелость|Очень зрелая, давно существует|Зрелая, активно развивается|
|Размер/скорость компиляции|Тяжелее|Легче|

**Когда Boost.Python:** проект уже завязан на Boost; нужна совместимость со старыми стандартами; используешь специфические возможности. **Когда pybind11:** новый проект; хочешь простую сборку без Boost; ценишь современный лаконичный API. Для большинства новых задач связывания C++/Python сегодня рекомендуют pybind11 — но API настолько похож, что знание Boost.Python переносится почти напрямую.

## Отличия от стандарта

- В стандартной библиотеке C++ средств связывания с Python **нет** — это внешняя задача по определению.
- Альтернативы: pybind11 (прямой наследник идей Boost.Python), Cython (отдельный язык-надстройка), ctypes/cffi (вызов C-функций из Python без C++-классов), SWIG (генератор обёрток для многих языков), nanobind (новая лёгкая библиотека от автора pybind11).

# Указание конкретной версии Python для сборки .pyd

Версия Python для сборки модуля (`.pyd` на Windows, `.so` на Linux) определяется тем, **какие заголовки и библиотеку Python подхватит сборка**. Управляется это в основном через CMake и/или vcpkg. Разберу способы.

## Способ 1. Через `find_package(Python3)` с указанием версии

CMake-модуль `FindPython3` принимает требования к версии:

```cmake
# точная версия
find_package(Python3 3.11 EXACT REQUIRED COMPONENTS Interpreter Development)

# минимальная версия
find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter Development)
```

| Форма         | Значение                        |
| ------------- | ------------------------------- |
| `3.11 EXACT`  | Ровно 3.11.x                    |
| `3.11`        | 3.11 или новее                  |
| `3.11...3.13` | Диапазон (3.11 ≤ версия < 3.13) |

## Способ 2. Явно указать путь к нужному интерпретатору

Самый надёжный способ — подсказать CMake, **какой именно** Python использовать, через переменные. Это снимает неоднозначность, когда в системе несколько версий:

```cmake
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
```

И при конфигурации передать:

```bash
cmake -B build -S . \
    -DPython3_EXECUTABLE=/usr/bin/python3.11
```

CMake возьмёт заголовки и библиотеку из окружения этого интерпретатора. Связанные переменные-подсказки:

|Переменная|Назначение|
|---|---|
|`Python3_EXECUTABLE`|Путь к интерпретатору (главная подсказка)|
|`Python3_ROOT_DIR`|Корень установки Python|
|`Python3_INCLUDE_DIR`|Путь к заголовкам (`Python.h`)|
|`Python3_LIBRARY`|Путь к библиотеке (`python311.lib` / `libpython3.11.so`)|

## Способ 3. Точное указание include/library (полный контроль)

Когда автопоиск ошибается, можно задать пути напрямую:

```bash
cmake -B build -S . \
    -DPython3_INCLUDE_DIR="C:/Python311/include" \
    -DPython3_LIBRARY="C:/Python311/libs/python311.lib"
```

На Windows библиотека обычно лежит в `<PythonDir>/libs/python3XX.lib`, заголовки — в `<PythonDir>/include`.

## Способ 4. Использовать активное виртуальное окружение

`FindPython3` умеет предпочитать активный venv. Это удобно: создаёшь venv нужной версии, активируешь, и сборка берёт его:

```cmake
# заставить CMake предпочесть venv/активное окружение
set(Python3_FIND_VIRTUALENV ONLY)   # ONLY / FIRST / STANDARD
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
```

```bash
python3.11 -m venv .venv
source .venv/bin/activate      # Windows: .venv\Scripts\activate
cmake -B build -S .            # подхватит Python из .venv
```

|Значение `Python3_FIND_VIRTUALENV`|Поведение|
|---|---|
|`FIRST` (по умолчанию)|Сначала venv, потом системный|
|`ONLY`|Только venv|
|`STANDARD`|Игнорировать venv|

## Полный пример CMake для модуля под конкретную версию

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_ext LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# требуем конкретную версию Python
find_package(Python3 3.11 EXACT REQUIRED COMPONENTS Interpreter Development)
find_package(Boost REQUIRED COMPONENTS python)

add_library(my_ext MODULE my_ext.cpp)
target_link_libraries(my_ext PRIVATE
    Boost::python
    Python3::Python              # подхватит выбранную версию
)

# имя файла = имя в BOOST_PYTHON_MODULE; без префикса lib
set_target_properties(my_ext PROPERTIES PREFIX "")
if(WIN32)
    set_target_properties(my_ext PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(my_ext PROPERTIES SUFFIX ".so")
endif()

# полезно для проверки: вывести найденную версию
message(STATUS "Python version: ${Python3_VERSION}")
message(STATUS "Python include: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python library: ${Python3_LIBRARIES}")
```

При конфигурации проверь сообщение `Python version:` — оно подтвердит, что выбрана нужная версия.

## Нюанс с Boost.Python и версией Python

Тут важный момент: **сам Boost.Python собирается под конкретную версию Python**. Когда vcpkg ставит `boost-python`, он компилирует библиотеку под ту версию Python, что видит при сборке. Поэтому согласовать версию нужно **в двух местах**:

1. Версия, под которую собран **Boost.Python** (в vcpkg).
2. Версия, с которой линкуется **твой модуль** (в CMake).

Если они разойдутся — будут ошибки линковки или загрузки. Самое частое расхождение: vcpkg собрал boost-python под Python 3.12, а ты в CMake указал 3.11.

### Управление версией Python в vcpkg

vcpkg при сборке `boost-python` использует свой Python. Чтобы контролировать это, иногда задают версию через настройки порта/триплета или ставят зависимость `python3` нужной версии. Это область, где детали зависят от текущей версии vcpkg — **стоит свериться с актуальной документацией vcpkg**, так как механизм выбора версии Python там менялся. Могу поискать текущий способ, если нужно.

## Проверка: под какую версию собрался модуль

После сборки убедись, что модуль линкуется с нужной версией.

**Linux/macOS** — посмотри зависимости:

```bash
ldd my_ext.so | grep python
# должно показать libpython3.11.so, а не другую версию
```

**Windows** — через Dependency Walker или dumpbin:

```bash
dumpbin /dependents my_ext.pyd
# ищи python311.dll
```

**Самая надёжная проверка** — попробовать импортировать нужной версией:

```bash
python3.11 -c "import my_ext; print('OK')"
```

Если модуль собран под другую версию — будет `ImportError` с упоминанием несовместимости.

## Частые ошибки и причины

|Симптом|Вероятная причина|
|---|---|
|`ImportError: ... module ...` при импорте|Модуль и интерпретатор разных версий Python|
|Ошибка линковки на `Py_...` символы|Boost.Python и модуль собраны под разные версии|
|CMake берёт «не тот» Python|Несколько Python в PATH; не задан `Python3_EXECUTABLE`|
|`.pyd` загружается в 3.11, но не в 3.12|Это нормально — бинарная несовместимость между минорными версиями|

> Ключевое правило: **ABI Python несовместим между минорными версиями** (3.11 ↔ 3.12). Модуль, собранный под 3.11, физически не загрузится в 3.12. Это не баг, а устройство Python — поэтому контроль версии так важен.

## Сводка действий

Чтобы собрать `.pyd`/`.so` под конкретную версию Python:

1. В CMake: `find_package(Python3 3.11 EXACT REQUIRED COMPONENTS Interpreter Development)`.
2. При конфигурации задай интерпретатор: `-DPython3_EXECUTABLE=/path/to/python3.11` (или активируй нужный venv).
3. Убедись, что **Boost.Python в vcpkg** собран под ту же версию.
4. Проверь сообщение `Python3_VERSION` при конфигурации.
5. После сборки验ифицируй через `ldd`/`dumpbin` и пробный `import` нужной версией.

Самое важное и часто упускаемое — пункт 3: согласовать версию Python между Boost.Python и твоим модулем. Расхождение здесь — причина большинства проблем.
