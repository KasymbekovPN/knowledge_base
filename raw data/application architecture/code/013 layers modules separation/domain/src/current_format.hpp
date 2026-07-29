// ПРИВАТНЫЙ заголовок domain. Используется только внутри самой библиотеки
// domain (её .cpp файлами). CMake не добавляет эту директорию в include
// path потребителей библиотеки - значит infra_stripe и app физически
// не смогут его заинклудить, даже если попытаются.

#pragma once
#include <string>

std::string formatCurrency(const double amount);
