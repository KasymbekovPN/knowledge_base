#pragma once

#include <string>

// Класс помечен для генератора — "reflect:serializable" сигнализирует,
// что для этого типа нужно сгенерировать (де)сериализацию. Компилятору
// эта строка ничего не говорит, она существует только для инструментов,
// читающих AST (libclang) — примерно как Q_OBJECT для moc.
class [[clang::annotate("reflect:serializable")]] Person {
public:
    Person() = default;
    Person(const int age, std::string name, const double salary):
        age{age}, name{std::move(name)}, salary{salary} {}

    // Поля помечены "reflect:field" — генератор включит их в вывод.
    // У salary есть модификатор "secret" — генератор подставит маску
    // вместо реального значения.
    int age [[clang::annotate("reflect:field")]] = 0;
    std::string name [[clang::annotate("reflect:field")]];
    double salary [[clang::annotate("reflect:field,secret")]] = 0.0;

private:
    // Без аннотации — генератор его вообще не увидит и не включит в вывод.
    mutable int accessCount_{0};
};
