#include "min_stack.h"

#include <iostream>
#include <format>

namespace min_stack {

void MinStack::push(const int value) {
    data_.push(value);

    if (min_.empty() || value <= min_.top()) {
        min_.push(value);
    } else {
        min_.push(min_.top());
    }
}

void MinStack::pop() {
    data_.pop();
    min_.pop();
}

int MinStack::top() const {
    return min_.top();
}

int MinStack::get_min() const {
    return min_.top();
}

void MinStackOpt::push(const int value) {
    data_.push(value);
    if (min_.empty() || value < min_.top().first) {
        min_.push({value, 1});
    } else {
        ++min_.top().second;
    }
}

void MinStackOpt::pop() {
    if (data_.top() == min_.top().first) {
        if (--min_.top().second == 0) {
            min_.pop();
        }
    }
    data_.pop();
}

int MinStackOpt::top() const {
    return min_.top().first;
}

int MinStackOpt::get_min() const {
    return min_.top().first;
}

void demo() {
    auto stk = MinStack();
    stk.push(-2);
    stk.push(0);
    stk.push(-3);

    std::cout << std::format("STK min: {}\n", stk.get_min());
    stk.pop();
    std::cout << std::format("STK min: {}\n", stk.get_min());

    auto stk_opt = MinStackOpt();
    stk_opt.push(-2);
    stk_opt.push(0);
    stk_opt.push(-3);

    std::cout << std::format("STK opt min: {}\n", stk_opt.get_min());
    stk_opt.pop();
    std::cout << std::format("STK opt min: {}\n", stk_opt.get_min());
}

}