#include <iostream>
#include <format>
#include <cctype>
#include <string>

namespace {
    class Solution {
        std::string s;
        size_t pos{0};

        void skip_spaces() {
            while (pos < s.size() && s[pos] == ' ') pos++;
        }

        // factor := NUMBER | '(' expr ')'
        int parse_factor() {
            skip_spaces();
            if (s[pos] == '(') {
                pos++;
                // сброс приоритета на самый низкий уровень внутри скобок
                const int result{parse_expr()};
                skip_spaces();
                // съели ')'
                pos++;

                return result;
            }

            int num{};
            while (pos < s.size() && isdigit(s[pos])) {
                num = num * 10 + (s[pos] - '0');
                pos++;
            }

            return num;
        }

        // term := factor (('*' | '/') factor)*
        int parse_term() {
            int result{parse_factor()};
            while (true) {
                skip_spaces();
                if (pos < s.size() && s[pos] == '*') {
                    pos++;
                    result *= parse_factor();
                } else if (pos < s.size() && s[pos] == '/') {
                    pos++;
                    result /= parse_factor();
                } else {
                    break;
                }
            }

            return result;
        }

        // expr := term (('+' | '-') term)*
        int parse_expr() {
            int result{parse_term()};
            while (true) {
                skip_spaces();
                if (pos < s.size() && s[pos] == '+') {
                    pos++;
                    result += parse_term();
                } else if (pos < s.size() && s[pos] == '-') {
                    pos++;
                    result -= parse_term();
                } else {
                    break;
                }
            }

            return result;
        }

    public:
        int calculate(std::string input) {
            s = std::move(input);
            pos = 0;
            return parse_expr();
        }
    };

    void run_test(const std::string& desc, const std::string& expr, const int expected) {
        Solution sol;
        const int result = sol.calculate(expr);
        std::cout << desc << ": \"" << expr << "\" = " << result
                  << " (expected " << expected << ") -> " << (result == expected ? "OK" : "FAIL") << std::endl;
    }

}

int main() {
    run_test("Simple expression", "1+1", 2);
    run_test("Division taking precedence over subtraction", "6-4/2", 4);
    run_test("Example from LC772 problem statement", "2*(5+5*2)/3+(6/2+8)", 21);
    run_test("Deeply nested parentheses", "(2+6*3+5-(3*14/7+2)*5)+3", -12);
    run_test("Multiplication only", "2*3*4", 24);
    run_test("Parentheses with no operations inside", "(42)", 42);
    run_test("Mixed whitespace", " 3 + 5 / 2 ", 5);
    run_test("Nested parentheses without whitespace", "((1+2)*3)", 9);

    return 0;
}
