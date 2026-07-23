#pragma once

#include <stack>

namespace min_stack {

    class MinStack {
    public:
        void push(int);
        void pop();
        int top() const;
        int get_min() const;
    private:
        std::stack<int> data_;
        std::stack<int> min_;
    };

    class MinStackOpt {
    public:
        void push(int);
        void pop();
        int top() const;
        int get_min() const;
    private:
        std::stack<int> data_;
        std::stack<std::pair<int, int>> min_;
    };

    void demo();
}