---
tags:
  - programming-language
  - cpp
  - syntax
  - operator
  - scope-resolution
---

[[__cpp operators index__|<=]]

Квалифицированное выражение идентификатора (_qualified identifier expression_) — это неквалифицированное выражение идентификатора, к которому добавлен оператор разрешения области действия (_scope resolution_) __::__ и, при необходимости, последовательность любых из следующих элементов, разделенных операторами разрешения области действия:

-  [[__cpp syntax namespaces__|Пространство имен]]
- Имя класса
- Имя перечисления (__C++11__)
- [спецификатор decltype](https://en.cppreference.com/w/cpp/language/decltype), обозначающий класс или тип перечисления (__C++11__) !---
- [спецификатор индексации пакета](https://en.cppreference.com/w/cpp/language/pack_indexing#Pack_indexing_specifier), обозначающий класс или тип перечисления (__C++26__) !---

!---
Например, выражение [std::string::npos](https://en.cppreference.com/w/cpp/string/basic_string/npos) — это выражение, которое называет статический член npos в классе string в пространстве имен _std_. Выражение _::tolower_ называет функцию _tolower_ в глобальном пространстве имен. Выражение [::std::cout](https://en.cppreference.com/w/cpp/io/cout) называет глобальную переменную _cout_ в пространстве имен _std_, которое является пространством имен верхнего уровня. Выражение _boost::signals2::connection_ называет тип _connection_, объявленный в пространстве имен _signals2_, который объявлен в пространстве имен _boost_.

---
[Qualified identifiers](https://en.cppreference.com/w/cpp/language/identifiers#Qualified_identifiers)
[C++ Operator Precedence](https://en.cppreference.com/w/cpp/language/operator_precedence)