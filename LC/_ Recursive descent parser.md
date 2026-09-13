
Разберём на классическом примере — упрощённый **язык поисковых запросов** с `AND`, `OR`, `NOT` и скобками, ровно тот DSL, который упоминался в вакансии. Грамматика с тремя уровнями приоритета (`OR` — самый слабый, `AND` — сильнее, `NOT` — сильнее всех):

```
orExpr   := andExpr ('OR' andExpr)*
andExpr  := notExpr ('AND' notExpr)*
notExpr  := 'NOT' notExpr | primary
primary  := IDENTIFIER | '(' orExpr ')'
```

```cpp
#include <algorithm>
#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <memory>
#include <set>

namespace {
    // ===================== AST =====================
    struct Node {
        static constexpr char INDENT_ELEM{' '};
        static constexpr int INDENT_STEP{2};

        virtual ~Node() = default;
        [[nodiscard]] virtual bool eval(const std::set<std::string>& doc) const = 0;
        virtual void print(std::ostream& os, int index = 0) const = 0;
    };

    struct TermNode: Node {
        std::string term;

        explicit TermNode(std::string term) : term(std::move(term)) {}

        [[nodiscard]] bool eval(const std::set<std::string> &doc) const override {
            return doc.contains(term);
        }

        void print(std::ostream &os, const int index) const override {
            os << std::format("{}TERM({})\n", std::string(index, INDENT_ELEM), term);
        }
    };

    struct NotNode: Node {
        std::unique_ptr<Node> child;

        explicit NotNode(std::unique_ptr<Node> child) : child(std::move(child)) {}

        [[nodiscard]] bool eval(const std::set<std::string> &doc) const override {
            return !child->eval(doc);
        }

        void print(std::ostream &os, const int index) const override {
            os << std::format("{}NOT\n", std::string(index, INDENT_ELEM));
            child->print(os, index + INDENT_STEP);
        }
    };

    struct AndNode: Node {
        std::unique_ptr<Node> left, right;
        explicit AndNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right):
            left(std::move(left)),
            right(std::move(right)) {}

        [[nodiscard]] bool eval(const std::set<std::string> &doc) const override {
            return left->eval(doc) && right->eval(doc);
        }

        void print(std::ostream &os, const int index) const override {
            os << std::format("{}AND\n", std::string(index, INDENT_ELEM));
            left->print(os, index + INDENT_STEP);
            right->print(os, index + INDENT_STEP);
        }
    };

    struct OrNode: Node {
        std::unique_ptr<Node> left, right;

        explicit OrNode(std::unique_ptr<Node> left, std::unique_ptr<Node> right):
            left(std::move(left)), right(std::move(right)) {}

        [[nodiscard]] bool eval(const std::set<std::string> &doc) const override {
            return left->eval(doc) || right->eval(doc);
        }

        void print(std::ostream &os, const int index) const override {
            os << std::format("{}OR\n", std::string(index, INDENT_ELEM));
            left->print(os, index + INDENT_STEP);
            right->print(os, index + INDENT_STEP);
        }
    };

    // ===================== Tokenizer =====================
    std::vector<std::string> tokenize(const std::string &input) {
        std::vector<std::string> tokens;
        size_t i{};
        while (i < input.size()) {
            const char c{input[i]};
            if (isspace(c)) {
                i++;
                continue;
            }

            if (c == '(' || c == ')') {
                tokens.emplace_back(1, c);
                i++;
                continue;
            }

            const size_t start{i};
            while (i < input.size() && !isspace(input[i]) && input[i] != '(' && input[i] != ')') i++;
            tokens.push_back(input.substr(start, i - start));
        }

        return tokens;
    }

    // ===================== Recursive descent parser =====================
    class Parser {
        std::vector<std::string> tokens;
        size_t pos{0};

        const std::string& peek() {
            static std::string end;
            return pos < tokens.size() ? tokens[pos] : end;
        }

        std::string advance() { return tokens[pos++]; }

    public:
        explicit Parser(std::vector<std::string> tokens): tokens(std::move(tokens)) {}

        std::unique_ptr<Node> parse_or() {
            auto left = parse_and();
            while (peek() == "OR") {
                advance();
                auto right = parse_and();
                left = std::make_unique<OrNode>(std::move(left), std::move(right));
            }

            return left;
        }

        std::unique_ptr<Node> parse_and() {
            auto left = parse_not();
            while (peek() == "AND") {
                advance();
                auto right = parse_not();
                left = std::make_unique<AndNode>(std::move(left), std::move(right));
            }

            return left;
        }

        std::unique_ptr<Node> parse_not() {
            if (peek() == "NOT") {
                advance();
                return std::make_unique<NotNode>(parse_not());
            }

            return parse_primary();
        }

        std::unique_ptr<Node> parse_primary() {
            if (peek() == "(") {
                advance();
                auto inner = parse_or();
                if (peek() != ")") throw std::runtime_error("Expected ')'");
                advance();

                return inner;
            }

            if (peek().empty()) throw std::runtime_error("Unexpected end of input");

            return std::make_unique<TermNode>(advance());
        }
    };

    std::unique_ptr<Node> parse_query(const std::string& query) {
        Parser parser(tokenize(query));
        return parser.parse_or();
    }

    // ===================== Демонстрация =====================
    void run_test(const std::string& query, const std::set<std::string>& doc, bool expected) {
        const auto ast{parse_query(query)};
        const bool result{ast->eval(doc)};
        std::cout << "query=\"" << query << "\" doc={";
        bool first{true};
        for (auto& t : doc) { if (!first) std::cout << ","; std::cout << t; first = false; }
        std::cout << "} -> " << result << " (expected " << expected << ") "
                  << (result == expected ? "OK" : "FAIL") << std::endl;
    }

}

int main() {
    // AND связывает крепче OR -- классическая проверка приоритета
    run_test("a OR b AND c", {"a"}, true);           // a=true -> OR коротит в true независимо от "b AND c"
    run_test("a OR b AND c", {"c"}, false);           // без a: нужно (b AND c), но b нет -> false
    run_test("a OR b AND c", {"b", "c"}, true);       // b AND c = true -> OR true

    // Скобки меняют приоритет явно
    run_test("(a OR b) AND c", {"a", "c"}, true);
    run_test("(a OR b) AND c", {"a"}, false);         // a OR b = true, но c нет -> AND false

    // NOT
    run_test("NOT a", {"b"}, true);
    run_test("NOT a AND b", {"b"}, true);             // NOT связывает крепче AND: (NOT a) AND b
    run_test("NOT (a AND b)", {"a"}, true);            // скобки меняют область действия NOT

    // Глубокая вложенность
    run_test("a AND (b OR (c AND NOT d))", {"a", "c"}, true);
    run_test("a AND (b OR (c AND NOT d))", {"a", "c", "d"}, false);

    std::cout << "\n=== Example AST for \"a OR b AND c\" ===" << std::endl;
    const auto ast = parse_query("a OR b AND c");
    ast->print(std::cout);

    return 0;
}


```

## Идея

Каждый уровень приоритета — **отдельная функция**. Функция более низкого приоритета (`parseOr`) **вызывает** функцию более высокого приоритета (`parseAnd`) для получения "атомарных" по отношению к себе операндов, а затем сама разбирает свой уровень операторов в цикле. Рекурсия **вниз** по цепочке приоритетов гарантирует, что операторы с более высоким приоритетом (`AND`, `NOT`) всегда "склеиваются" в единый операнд **раньше**, чем более слабый оператор (`OR`) их увидит — именно так и получается корректный приоритет без явных чисел-приоритетов, через саму структуру вызовов.

## Разбор ключевых моментов

**Почему `parseOr` стартует разбор, а не `parseNot` или `parsePrimary`:** входная точка парсера всегда — функция **самого низкого** приоритета. Это гарантирует, что весь диапазон операторов будет корректно "увиден" сверху вниз: `parseOr` сначала спускается через `parseAnd` → `parseNot` → `parsePrimary`, чтобы получить первый операнд, и только потом в цикле разбирает свои `OR`.

**Почему в `parsePrimary` при виде `(` снова вызывается `parseOr`, а не какая-то отдельная функция:** скобки **сбрасывают** приоритет обратно на самый низкий уровень — то, что внутри скобок, разбирается заново с нуля, как если бы это было отдельное независимое выражение. Это и есть рекурсия в парсере — не по глубине операторов, а по **вложенности скобок**.

**Почему `NOT` рекурсивно вызывает сам себя (`parseNot()`), а не `parsePrimary()`:** это позволяет корректно обработать `NOT NOT a` (двойное отрицание) и `NOT (a AND b)` — `NOT` должен уметь принять после себя либо термин, либо снова `NOT`, либо скобочное выражение (что уходит в `parsePrimary`, которая обрабатывает и то, и другое).

**Почему `left-to-right` цикл в `parseOr`/`parseAnd` (`while (peek() == "OR") {...}`), а не рекурсия:** `OR` и `AND` в данной грамматике — **левоассоциативны** (`a OR b OR c` = `(a OR b) OR c`), и цикл с накоплением в `left` — стандартный способ выразить левую ассоциативность без риска глубокой рекурсии на длинной цепочке одинаковых операторов (`a OR b OR c OR d OR ...` — с циклом это `O(1)` дополнительной глубины стека вызовов, а не `O(n)`, как было бы при наивной рекурсии `parseOr = parseAnd OR parseOr`).

## Связь с прогрессией через Basic Calculator

Обратите внимание на прямую параллель со стеком из LC227: там `*`/`/` обрабатывались "жадно и сразу" (более высокий приоритет), а `+`/`-` откладывались через стек (более низкий приоритет) — это был **плоский**, двухуровневый способ выразить приоритет без явной рекурсии, работающий только потому, что уровней ровно два. Recursive descent — это **обобщение** той же идеи на произвольное число уровней приоритета: каждый уровень явно выделен в отдельную функцию вместо неявного кодирования через порядок обработки в стеке.

Если бы попытались добавить в LC227-стиль третий уровень приоритета (например, возведение в степень `^`, которое обычно приоритетнее `*`/`/`) — плоский стековый подход стал бы неудобным (пришлось бы городить ещё один вложенный стек или условную логику), тогда как recursive descent масштабируется тривиально — просто добавляется ещё одна функция `parsePow()` между `parseMulDiv()` и `parsePrimary()`.