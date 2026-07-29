#pragma once

#include <memory>
#include <string>

// Публичный заголовок НЕ содержит ни одного #include под детали реализации -
// никакого <vector>, никакого стороннего SDK, ничего, что мог бы захотеть
// поменять автор реализации. Потребитель компилирует этот файл быстро и
// физически не видит внутренностей Widget - только объявление указателя.
class Widget {
public:
    Widget();
    // объявлен здесь, но НЕ "= default" - Impl пока неполный тип,
    // компилятор не может сгенерировать деструктор без его определения
    ~Widget();

    // pImpl-классы либо некопируемы (тогда ничего доп. писать не нужно -
    // unique_ptr сам запретит копирование), либо явно movable - здесь move
    // объявлен явно, чтобы Widget можно было класть в std::vector<Widget>.
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    void setName(const std::string& name);
    std::string describe() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
