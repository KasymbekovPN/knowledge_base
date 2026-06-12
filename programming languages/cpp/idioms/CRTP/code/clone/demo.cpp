#include <iostream>

template<typename T>
struct Clonable {
    T* clone() const {
        return new T(static_cast<const T&>(*this));
    }
};


class Widget: public Clonable<Widget> {
public:
    std::string name;
    Widget(const std::string& _name): name{_name} {}
    Widget(const Widget&) = default;
};

int main() {
    Widget w("Main");
    Widget* pwidget = w.clone();

    std::cout << pwidget->name << std::endl;
    delete pwidget;

    return 0;
}
