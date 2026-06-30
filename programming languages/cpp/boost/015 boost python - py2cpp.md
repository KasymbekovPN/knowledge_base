---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

Boost.Python умеет не только экспортировать C++ в Python, но и **встраивать** Python в C++ с типобезопасными обёртками вместо сырого C API.

### vcpkg.json
```json
{  
    "name": "boost-learning-py2cpp",  
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
project(py2cpp LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 26)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)  
find_package(Boost REQUIRED COMPONENTS python)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE Boost::python Python3::Python)
```

### script.py
```python
print(1234567890)
```

### main.cpp
```cpp
/*  
cmake --preset default  
cmake --build --preset default  
.\.build\app.exe  
*/  
  
#include <boost/python.hpp>  
  
#include <filesystem>  
#include <format>  
#include <iostream>  
#include <windows.h>  
  
int main() {  
    // Вычисляем PYTHONHOME относительно exe — vcpkg кладёт Python сюда  
    wchar_t exe_buf[MAX_PATH];  
    GetModuleFileNameW(nullptr, exe_buf, MAX_PATH);  
    static std::wstring python_home =  
        (std::filesystem::path(exe_buf).parent_path()  
         / "vcpkg_installed" / "x64-windows" / "tools" / "python3")  
        .wstring();  
    Py_SetPythonHome(python_home.c_str());  
  
    Py_Initialize();  
    try {  
        // глобальное пространство имён модуля __main__  
        boost::python::object main_module = boost::python::import("__main__");  
        boost::python::object main_namespace = main_module.attr("__dict__");  
  
        // выполнить код в этом пространстве  
        boost::python::exec(  
            "def multiply(a, b):\n"  
            "    return a * b\n",  
            main_namespace  
        );  
  
        const int A{6};  
        const int B{7};  
        // получить функцию и вызвать её  
        boost::python::object multiply = main_namespace["multiply"];  
        boost::python::object result = multiply(A, B);  
  
        // типобезопасное извлечение  
        int value{boost::python::extract<int>(result)};  
        std::cout << std::format("multiply({}, {}) = {}\n", A, B, value);  
  
        // выполнить внешний скрипт-файл  
        boost::python::exec_file("script.py", main_namespace, main_namespace);  
  
    } catch (const boost::python::error_already_set&) {  
        PyErr_Print();  
    }    Py_Finalize();  
  
    return 0;  
}
```

---

```cpp
#include <boost/python.hpp>
#include <iostream>

namespace py = boost::python;

int main() {
    Py_Initialize();
    try {
        // глобальное пространство имён модуля __main__
        py::object main_module = py::import("__main__");
        py::object main_namespace = main_module.attr("__dict__");

        // выполнить код в этом пространстве
        py::exec(
            "def multiply(a, b):\n"
            "    return a * b\n",
            main_namespace);

        // получить функцию и вызвать её
        py::object multiply = main_namespace["multiply"];
        py::object result = multiply(6, 7);

        int value = py::extract<int>(result);   // типобезопасное извлечение
        std::cout << "multiply(6, 7) = " << value << "\n";

        // выполнить внешний скрипт-файл
        py::exec_file("script.py", main_namespace, main_namespace);

    } catch (const py::error_already_set&) {
        PyErr_Print();                          // вывести Python-трейсбек
    }
    Py_Finalize();
    return 0;
}
```

```
multiply(6, 7) = 42
1234567890
```

|Конструкция|Назначение|
|---|---|
|`py::import("module")`|Импортировать модуль|
|`py::exec(code, ns)`|Выполнить строку кода|
|`py::exec_file("f.py", ns)`|Выполнить файл-скрипт|
|`obj.attr("name")`|Доступ к атрибуту|
|`func(args...)`|Прямой вызов (естественный синтаксис!)|
|`py::extract<T>(obj)`|Извлечь результат как C++-тип|
|`error_already_set`|Исключение при ошибке Python|
