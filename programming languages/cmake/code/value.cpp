#include "value.hpp"

namespace custom_ns
{

Value::Value(const int _data): data{_data} {}

int Value::getData() const {
    return data;
}

std::ostream& operator<<(std::ostream& _os, const custom_ns::Value& _value) {
    return _os << "{" << _value.getData() << "}";
}

} // namespace custom_ns
