#include "widget.h"

#include <iostream>
#include <vector>

class Widget::Impl {
public:
    std::string name {"default"};
    int value {42};

    void process() const {
        std::cout
            << "Processing: "
            << name
            << std::endl;
    };
};

Widget::Widget(): p_impl(std::make_unique<Widget::Impl>()) {}

Widget::~Widget() = default;

Widget::Widget(Widget&&) = default;

Widget& Widget::operator=(Widget&&) = default;

void Widget::do_somethong() {
    p_impl->process();
}

int Widget::get_value() const {
    return p_impl->value;
}
