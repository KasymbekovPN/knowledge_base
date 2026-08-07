// АВТОСГЕНЕРИРОВАНО generate.py из model.h. Не редактировать вручную.
#pragma once

#include "model.h"
#include <sstream>
#include <string>

inline std::string to_json(const Person& obj) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"age\":" << obj.age;
    oss << ",";
    oss << "\"name\":\"" << obj.name << "\"";
    oss << ",";
    oss << "\"salary\":\"***\"";
    oss << "}";
    return oss.str();
}

