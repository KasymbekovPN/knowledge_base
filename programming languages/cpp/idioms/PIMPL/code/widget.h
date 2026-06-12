#pragma once
#include <memory>

struct Impl;

class Widget {
private:
    class Impl;
    std::unique_ptr<Impl> p_impl;

public:
    Widget();
    ~Widget();
    Widget(Widget&&);
    Widget& operator=(Widget&&);

    void do_somethong();
    int get_value() const;
};
