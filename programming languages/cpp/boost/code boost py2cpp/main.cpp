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
    }
    Py_Finalize();

    return 0;
}
