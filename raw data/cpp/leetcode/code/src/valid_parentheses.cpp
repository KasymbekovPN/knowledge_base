#include "valid_parentheses.h"

#include <stack>
#include <unordered_map>
#include <iostream>
#include <format>

namespace valid_parentheses {

bool is_valid_0(const std::string& line) {
    static const std::unordered_map<char, char> BRACKET_PAIRS {
        {')', '('},
        {']', '['},
        {'}', '{'}
    };
    std::stack<char> stk;

    for (const auto& c : line) {
        if (c == '{' || c == '[' || c == '(') {
            stk.push(c);
        } else {
            if (stk.empty() || stk.top() != BRACKET_PAIRS.at(c)) {
                return false;
            }
            stk.pop();
        }
    }

    return stk.empty();
}

bool is_valid_1(const std::string& line) {
    std::stack<char> stk;
    for (const auto& c: line) {
        switch (c) {
            case '(': case '[': case '{':
                stk.push(c);
                break;
            case ')':
                if (stk.empty() || stk.top() != '(') return false;
                stk.pop();
                break;
            case ']':
                if (stk.empty() || stk.top() != '[') return false;
                stk.pop();
                break;
            case '}':
                if (stk.empty() || stk.top() != '{') return false;
                stk.pop();
                break;
            default: break;
        }
    }

    return stk.empty();
}

void demo() {
    const std::string GOOD_LINE{"{[()]}"};
    const std::string BAD_LINE{"([)]"};

    std::cout << std::format("is_valid_0 of {} => {}\n", GOOD_LINE, is_valid_0(GOOD_LINE));
    std::cout << std::format("is_valid_0 of {} => {}\n", BAD_LINE, is_valid_0(BAD_LINE));
    std::cout << std::format("is_valid_1 of {} => {}\n", GOOD_LINE, is_valid_1(GOOD_LINE));
    std::cout << std::format("is_valid_1 of {} => {}\n", BAD_LINE, is_valid_1(BAD_LINE));
}

}
