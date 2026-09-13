#include <iostream>
#include <format>
#include <string>
#include <stack>
#include <cctype>

namespace {
    constexpr char OP_PLUS{'+'};
    constexpr char OP_MINUS{'-'};
    constexpr char OP_MUL{'*'};
    constexpr char OP_DIV{'/'};

    int calculate(const std::string& input) {
        std::stack<int> st;
        int num{};
        char last_op{OP_PLUS};

        for (int i{}; i < static_cast<int>(input.size()); ++i) {
            const char c{input[i]};

            if (isdigit(c)) {
                num = num * 10 + c - '0';
                continue;
            }

            if (!isspace(c) || i == static_cast<int>(input.size()) - 1) {
                if (last_op == OP_PLUS) {
                    st.push(num);
                }
                else if (last_op == OP_MINUS) {
                    st.push(-num);
                }
                else if (last_op == OP_MUL) {
                    const int prev{st.top()}; st.pop();
                    st.push(num * prev);
                }
                else if (last_op == OP_DIV) {
                    const int prev{st.top()}; st.pop();
                    st.push(num / prev);
                }
                last_op = c;
                num = 0;
            }
        }

        int result{};
        while (!st.empty()) {
            result += st.top(); st.pop();
        }

        return result;
    }

    void run_test(const std::string& desc, const std::string& expr, const int expected) {
        const int result{calculate(expr)};
        std::cout << std::format("[{}] '{}' = {}, expected: {}, success: {}",
            desc, expr, result, expected, result != expected ? "OK" : "FAIL");
    }

}

int main() {
    run_test("Example from condition 1", "3+2*2", 7);
    run_test("Example from condition 2 (integer division)", " 3/2 ", 1);
    run_test("Example from condition 3", " 3+5 / 2 ", 5);
    run_test("Multiplication only", "2*3*4", 24);
    run_test("Mixed operations", "14-3/2", 13);
    run_test("Multiplication precedence over addition", "2+3*4-5", 9);
    run_test("Division with remainder (rounding towards zero)", "7/2", 3);
    run_test("Multi-digit numbers", "100*2+50", 250);
    run_test("Chain of subtractions", "10-2-3", 5);
    run_test("Single number without operations", "42", 42);

    return main();
}
