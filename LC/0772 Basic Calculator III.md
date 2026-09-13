
## Суть задачи

LC772 (Basic Calculator III) — объединение LC224 (скобки) и LC227 (`*`/`/` с приоритетом) в одну задачу: полное арифметическое выражение с `+`, `-`, `*`, `/` и скобками, без явного разделения на "два уровня приоритета через стек", как было в LC227 — теперь это делаем через **recursive descent**, что естественно продолжает предыдущий разбор.

```cpp
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

```

## Грамматика

```
expr   := term (('+' | '-') term)*
term   := factor (('*' | '/') factor)*
factor := NUMBER | '(' expr ')'
```

`expr` — самый низкий приоритет (`+`/`-`), `term` — выше (`*`/`/`), `factor` — атом (число или скобочное подвыражение, которое рекурсивно возвращает на самый низкий приоритет `expr`).

## Ключевые архитектурные решения

**Почему `pos` — член класса, а не параметр, передаваемый явно:** каждая функция парсера (`parseExpr`, `parseTerm`, `parseFactor`) должна **продвигать** общую позицию чтения и видеть её изменения, сделанные рекурсивными вызовами. Можно было бы передавать `pos` по ссылке (`size_t& pos`) как параметр в каждую функцию — это более "чистый" функциональный стиль без состояния в объекте, но хранение как member-переменной чуть удобнее синтаксически (не нужно продёргивать `&pos` через каждый вызов) — оба подхода равноценны, это вопрос стиля.

**Почему `parseFactor` при виде `(` вызывает именно `parseExpr` (а не `parseTerm` или себя же):** ровно та же логика, что была в `AND`/`OR`/`NOT` примере — скобки **сбрасывают** приоритет на самый низкий уровень грамматики. Всё, что внутри скобок, — это независимое подвыражение целиком, с собственными `+`/`-`/`*`/`/`, поэтому разбор обязан начаться заново с `parseExpr`.

**Отсутствие явной токенизации:** в отличие от предыдущего примера с `AND`/`OR`/`NOT`, где токены собирались заранее в вектор строк, здесь парсер работает **напрямую** с индексом в исходной строке (`pos`), пропуская пробелы (`skipSpaces()`) прямо в процессе разбора. Это тоже валидный и распространённый стиль — токенизация "на лету" вместо отдельного прохода — оправдан, когда токены простые (одиночные символы операторов и последовательности цифр), и не даёт выигрыша от разделения на отдельный этап.

## Сравнение с LC227 (стековый подход)

Явное сравнение двух решений одной семьи задач — хороший материал для собеседования, если спросят "а как ещё можно было решить":

| |LC227 (стек, 2 уровня)|LC772 (recursive descent, N уровней)|
|---|---|---|
|Приоритет операций|Кодируется неявно через порядок обработки в стеке|Кодируется явно через иерархию функций|
|Скобки|Не поддерживаются в этом виде решения|Поддерживаются естественно через рекурсию `factor → expr`|
|Масштабируемость на новые уровни приоритета (например, `^`)|Плохая — пришлось бы городить доп. логику|Тривиальная — добавить `parsePow()` между `parseTerm` и `parseFactor`|
|Глубина стека вызовов|`O(1)` (плоский цикл)|`O(глубина вложенности скобок)` — риск переполнения стека на экстремально вложенных выражениях|

Этот компромисс (recursive descent масштабируется лучше по приоритетам ценой риска глубокой рекурсии на скобках) стоит явно проговорить, если разговор пойдёт в сторону "а как сделать это безопасным для production, где входные данные не доверенные" — там уже встаёт вопрос ограничения максимальной глубины вложенности скобок (защита от stack overflow при злонамеренном/некорректном вводе), что снова прямая связь с темой "надёжность рантайма при обработке произвольных запросов" из вашей вакансии.
