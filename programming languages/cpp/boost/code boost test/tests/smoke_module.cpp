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