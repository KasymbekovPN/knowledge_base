#include <iostream>
#include <string>
#include <stack>

namespace {
    constexpr int SIGN_MINUS{-1};
    constexpr int SIGN_PLUS{1};

    int calculate(const std::string &input) {
        std::stack<int> st;
        int result{};
        int sign{1};
        int num{};

        for (int i{}; i < static_cast<int>(input.size()); ++i) {
            if (const char c = input[i];
                isdigit(c)) {
                num = num * 10 + (c - '0');
            } else if (c == '+') {
                result += sign * num;
                num = 0;
                sign = SIGN_PLUS;
            } else if (c == '-') {
                result += sign * num;
                num = 0;
                sign = SIGN_MINUS;
            } else if (c == '(') {
                st.push(result);
                st.push(sign);
                result = 0;
                sign = SIGN_PLUS;
            } else if (c == ')') {
                result += sign * num;
                num = 0;

                const int prev_sign = st.top(); st.pop();
                const int prev_result = st.top(); st.pop();

                result = prev_result + prev_sign * result;
            }
        }

        result += sign * num;
        return result;
    }

    void run_test(const std::string& desc, const std::string& expr, const int expected) {
        const int result{calculate(expr)};
        std::cout << std::format("[{}] '{}' = {} : expected = {} : {}\n",
            desc,
            expr,
            result,
            expected,
            result == expected ? "OK" : "FAIL");
    }

}

int main() {
    run_test("Simple add", "1 + 1", 2);
    run_test("Simple sub", "10 - 5", 5);
    run_test("LC224", " 2-1 + 2 ", 3);
    run_test("Inner braces", "(1+(4+5+2)-3)+(6+8)", 23);
    run_test("Minus before brace", "1 - (2 - 3)", 2);
    run_test("Double minus", "1 - (2 + 3)", -4);
    run_test("Big number", "100 - (50 + 25)", 25);
    run_test("Only braces", "(1)", 1);
    run_test("Deep nesting", "1 + (2 - (3 + (4 - 5)))", 1);
    run_test("Negative result", "0 - 5", -5);

    return 0;
}
