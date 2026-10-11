/*

https://coderun.yandex.ru/problem/nop-with-response-recovery

Даны две последовательности, требуется найти и вывести их наибольшую общую подпоследовательность.

Напомним:
Последовательность x называется подпоследовательностью последовательности y, если x получается из y удалением
нескольких (возможно, нуля или всех) элементов.

Наибольшая общая подпоследовательность - последовательность наибольшей длины, которая является подпоследовательностью
обеих последовательностей.

Формат ввода
В первой строке входных данных содержится число N – длина первой последовательности (1 ≤ N ≤ 1000).
Во второй строке заданы члены первой последовательности (через пробел) – целые числа, не превосходящие 10000 по модулю.
В третьей строке записано число M – длина второй последовательности (1 ≤ M ≤ 1000).
В четвертой строке задаются члены второй последовательности (через пробел) – целые числа, не превосходящие 10000 по модулю.

Формат вывода
Требуется вывести наибольшую общую подпоследовательность данных последовательностей, через пробел.
Если таких подпоследовательностей несколько, то можно вывести любую.

Примечание
В примере 2 существует сразу три наибольшие общие подпоследовательности.
- 1
- 2
- 3

Любая из них будет правильным ответом.


 */

#include <algorithm>
#include <iostream>
#include <format>
#include <set>
#include <vector>

namespace {
    constexpr bool REAL_INPUT{false};
    constexpr bool EXTENDED_LOG{true};

    struct Input {
        int first_len{};
        std::vector<int> first_sequence;
        int second_len{};
        std::vector<int> second_sequence;

        void init() {
            init_entity_(first_len, first_sequence);
            init_entity_(second_len, second_sequence);
        }

        void init_test(const int first_len_,
                       std::vector<int> first_sequence_,
                       const int second_len_,
                       std::vector<int> second_sequence_) {
            first_len = first_len_;
            first_sequence = std::move(first_sequence_);
            second_len = second_len_;
            second_sequence = std::move(second_sequence_);
        }

        void print() const {
            print_entity_("first", first_len, first_sequence);
            print_entity_("second", second_len, second_sequence);
        }

    private:
        static void init_entity_(int& len, std::vector<int>& sequence) {
            std::cin >> len;
            sequence.resize(len);

            int buffer;
            for (int i{}; i < len; ++i) {
                std::cin >> buffer;
                sequence[i] = buffer;
            }
        }

        static void print_entity_(const std::string& label, const int len, const std::vector<int>& sequence) {
            std::cout << std::format("{} len: {}\n{} seq: ", label, len, label);
            for (const auto item: sequence) std::cout << std::format("{} ", item);
            std::cout << std::endl;
        }
    };

    struct Output {
        std::set<std::vector<int>> max_sequences;

        void init_test(std::set<std::vector<int>> max_sequence_) {
            max_sequences = std::move(max_sequence_);
        }

        void print_all() const {
            if (max_sequences.empty()) return;
            for (const auto& seq: max_sequences) {
                for (const auto item: seq) {
                    std::cout << std::format("{} ", item);
                }
                std::cout << std::endl;
            }
        }

        void print_first() const {
            if (max_sequences.empty()) return;
            for (const auto item: *max_sequences.begin()) {
                std::cout << std::format("{} ", item);
            }
        }

        [[nodiscard]] bool check(const std::vector<int>& sequence) const {
            if (sequence.empty() && max_sequences.empty()) return true;

            return  max_sequences.contains(sequence);
        }
    };

    constexpr int INPUT{0}; // +
    // constexpr int INPUT{1}; // +
    // constexpr int INPUT{2}; // +
    // constexpr int INPUT{3}; // +
    // constexpr int INPUT{4}; // +
    // constexpr int INPUT{5}; // +
    // constexpr int INPUT{6}; // +
    // constexpr int INPUT{7}; // +
    // constexpr int INPUT{8}; // +
    // constexpr int INPUT{9}; // +
    // constexpr int INPUT{10}; // +
    // constexpr int INPUT{11}; // +
    // constexpr int INPUT{12}; // +
    // constexpr int INPUT{13}; // +
    // constexpr int INPUT{14}; // +
    // constexpr int INPUT{15}; // +
    // constexpr int INPUT{16}; // +
    // constexpr int INPUT{17}; // +
    // constexpr int INPUT{18}; // +
    // constexpr int INPUT{19}; // +
    void enrich_data(Input* input, Output* output) {
        if constexpr (INPUT == 0) {
            input->init_test(3, {1, 2, 3}, 3, {2, 3, 1});
            output->init_test({{2, 3}});
        } else if constexpr (INPUT == 1) {
            input->init_test(3, {1, 2, 3}, 3, {3, 2, 1});
            output->init_test({{1}, {2}, {3}});
        } else if constexpr (INPUT == 2) {
            // N=M=1, элементы равны
            input->init_test(1, {5}, 1, {5});
            output->init_test({{5}});
        } else if constexpr (INPUT == 3) {
            // N=M=1, элементы разные -> НОП пустая
            input->init_test(1, {5}, 1, {6});
            output->init_test({});
        } else if constexpr (INPUT == 4) {
            // одинаковые последовательности
            input->init_test(5, {1, 2, 3, 4, 5}, 5, {1, 2, 3, 4, 5});
            output->init_test({{1, 2, 3, 4, 5}});
        } else if constexpr (INPUT == 5) {
            // нет общих элементов -> НОП пустая
            input->init_test(3, {1, 2, 3}, 4, {4, 5, 6, 7});
            output->init_test({});
        } else if constexpr (INPUT == 6) {
            // вторая — подпоследовательность первой
            input->init_test(7, {1, 2, 3, 4, 5, 6, 7}, 3, {2, 4, 6});
            output->init_test({{2, 4, 6}});
        } else if constexpr (INPUT == 7) {
            // повторы: [1,1,1,1] и [1,1]
            input->init_test(4, {1, 1, 1, 1}, 2, {1, 1});
            output->init_test({{1, 1}});
        } else if constexpr (INPUT == 8) {
            // N=1, элемент есть во второй (N != M)
            input->init_test(1, {3}, 5, {1, 2, 3, 4, 5});
            output->init_test({{3}});
        } else if constexpr (INPUT == 9) {
            // отрицательные числа и ноль
            input->init_test(5, {-5, 0, 7, -5, 3}, 5, {0, -5, 3, 7, -5});
            output->init_test({{-5, 7, -5}, {0, -5, 3}, {0, 7, -5}});
        } else if constexpr (INPUT == 10) {
            // граничные значения +-10000
            input->init_test(4, {10000, -10000, 0, 10000}, 5, {-10000, 10000, 0, -10000, 10000});
            output->init_test({{-10000, 0, 10000}, {10000, -10000, 10000}, {10000, 0, 10000}});
        } else if constexpr (INPUT == 11) {
            // классика из учебника (CLRS): ABCBDAB / BDCABA
            input->init_test(7, {1, 2, 3, 2, 4, 1, 2}, 6, {2, 4, 3, 1, 2, 1});
            output->init_test({{2, 3, 1, 2}, {2, 3, 2, 1}, {2, 4, 1, 2}});
        } else if constexpr (INPUT == 12) {
            // взаимно обратные, 6 различных элементов
            input->init_test(6, {1, 2, 3, 4, 5, 6}, 6, {6, 5, 4, 3, 2, 1});
            output->init_test({{1}, {2}, {3}, {4}, {5}, {6}});
        } else if constexpr (INPUT == 13) {
            // чередование 1,2,1,2,1,2 и 2,1,2,1,2,1
            input->init_test(6, {1, 2, 1, 2, 1, 2}, 6, {2, 1, 2, 1, 2, 1});
            output->init_test({{1, 2, 1, 2, 1}, {2, 1, 2, 1, 2}});
        } else if constexpr (INPUT == 14) {
            // случайный, алфавит {1,2,3}, N=12 M=10
            input->init_test(12, {2, 1, 3, 3, 2, 1, 3, 2, 3, 2, 3, 1}, 10, {3, 3, 2, 2, 2, 3, 3, 3, 1, 2});
            output->init_test({{3, 3, 2, 2, 2, 3, 1}, {3, 3, 2, 2, 3, 3, 1}, {3, 3, 2, 3, 3, 3, 1}});
        } else if constexpr (INPUT == 15) {
            // случайный, алфавит {1..6}, N=22 M=18
            input->init_test(22, {5, 6, 3, 5, 1, 6, 2, 6, 6, 4, 6, 6, 2, 5, 2, 4, 1, 3, 6, 4, 4, 1}, 18, {6, 6, 2, 3, 4, 3, 3, 2, 3, 4, 4, 3, 5, 2, 4, 2, 2, 1});
            output->init_test({{6, 6, 2, 4, 2, 3, 4, 4, 1}, {6, 6, 2, 4, 2, 4, 3, 4, 1}, {6, 6, 2, 4, 2, 4, 4, 4, 1}, {6, 6, 2, 4, 2, 5, 2, 4, 1}});
        } else if constexpr (INPUT == 16) {
            // средний, широкий алфавит (-30..30), N=30 M=25
            input->init_test(30, {-28, -8, 9, -21, 6, 3, -26, 4, 4, 12, -13, 25, 19, 14, 1, -24, 5, -2, -20, -30, -2, -14, -17, 12, 6, -6, -25, -6, -19, 21}, 25, {30, 0, 17, -26, 4, -5, 26, 5, 29, -10, -11, -18, -24, -19, -23, 5, 23, 7, -28, -30, 0, 7, 26, -14, 11});
            output->init_test({{-26, 4, -24, 5, -30, -14}});
        } else if constexpr (INPUT == 17) {
            // Максимальный размер: две одинаковые последовательности по 1000 элементов (НОП = вся последовательность)
            std::vector<int> seq;
            seq.reserve(1000);
            for (int i{}; i < 1000; ++i) seq.push_back(i - 500);   // -500 ... 499, все различны
            input->init_test(1000, seq, 1000, seq);
            output->init_test({seq});
        } else if constexpr (INPUT == 18) {
            // Максимальный размер: каждый 10-й элемент второй последовательности заменён на значение, которого нет в первой.
            // НОП единственна: 900 общих элементов в исходном порядке.
            std::vector<int> first, second, common;
            for (int i{}; i < 1000; ++i) {
                first.push_back(i + 1);
                if (i % 10 == 9) {
                    second.push_back(5000 + i);   // чужое значение, в first его нет
                } else {
                    second.push_back(i + 1);
                    common.push_back(i + 1);
                }
            }
            input->init_test(1000, first, 1000, second);
            output->init_test({common});
        } else if constexpr (INPUT == 19) {
            // N != M и много одинаковых элементов: 1000 единиц против чередования 1,2,1,2,... длины 500 -> НОП = 250 единиц
            std::vector<int> first(1000, 1), second;
            for (int i{}; i < 500; ++i) second.push_back(i % 2 == 0 ? 1 : 2);
            input->init_test(1000, first, 500, second);
            output->init_test({std::vector<int>(250, 1)});
        } else {
            std::cout << "Unexpected input" << std::endl;
        }
    }

}

int main(int argc, char *argv[]) {
    const auto input{new Input()};
    Output* expected_output{new Output()};

    if constexpr (REAL_INPUT) {
        input->init();
    } else {
        enrich_data(input, expected_output);
    }

    if constexpr (EXTENDED_LOG) {
        input->print();
        if constexpr (!REAL_INPUT) expected_output->print_all();
    }

    const auto& first{input->first_sequence};
    const auto& second{input->second_sequence};
    const int first_len{input->first_len};
    const int second_len{input->second_len};

    std::vector<std::vector<int>> dp(first_len + 1, std::vector<int>(second_len + 1, 0));
    for (int i{1}; i <= first_len; ++i) {
        for (int j{1}; j <= second_len; ++j) {
            dp[i][j] = first[i - 1] == second[j - 1]
                ? dp[i - 1][j - 1] + 1
                : std::max(dp[i - 1][j], dp[i][j - 1]);
        }
    }

    if constexpr (EXTENDED_LOG) {
        for (const auto& vec: dp) {
            for (const auto item: vec) std::cout << item << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::vector<int> lcs;
    lcs.reserve(dp[first_len][second_len]);
    for (int i{first_len}, j{second_len}; i > 0 && j > 0;) {
        if (first[i - 1] == second[j - 1]) {
            lcs.push_back(first[i - 1]);
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }
    std::reverse(lcs.begin(), lcs.end());

    if constexpr (EXTENDED_LOG) {
        std::cout << std::format("LCS: {}\n", lcs);
        std::cout << std::format("OK: {}\n", expected_output->check(lcs) ? "OK" : "FAIL");
    }

    for (const auto i: lcs) std::cout << i << " ";
    std::cout << std::endl;

    delete input;
    delete expected_output;

    return 0;
}
