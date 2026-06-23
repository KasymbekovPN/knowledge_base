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
import math2;

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
