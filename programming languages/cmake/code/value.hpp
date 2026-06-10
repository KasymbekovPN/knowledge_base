#pragma once
#include <iostream>

namespace custom_ns {

class Value {
private:
    int data{};
public:
    Value(const int _data);
    int getData() const;
};

std::ostream& operator<<(std::ostream& _os, const custom_ns::Value& _value);

} // namespace custom_ns