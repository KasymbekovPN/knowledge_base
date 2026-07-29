#include "widget.hpp"

#include <format>

// Impl определён ЗДЕСЬ, в .cpp - потребитель заголовка widget.hpp никогда
// не видит этот код и не пересобирается, если тут что-то меняется.
struct Widget::Impl {
    std::string name{"unnamed"};
    int callCount{0};
};

Widget::Widget(): impl_{std::make_unique<Impl>()} {}
// теперь Impl полный тип - деструктор генерируется здесь
Widget::~Widget() = default;
Widget::Widget(Widget&&) noexcept = default;
Widget& Widget::operator=(Widget&&) noexcept = default;

void Widget::setName(const std::string& name) { impl_->name = name; }

std::string Widget::describe() const {
    ++impl_->callCount;
    return std::format("{} (calling of describe: {})\n", impl_->name, impl_->callCount);
}
