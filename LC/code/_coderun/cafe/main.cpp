
/*

Около Петиного университета недавно открылось новое кафе, в котором действует следующая система скидок: при каждой
покупке более чем на 100 рублей покупатель получает купон, дающий право на один бесплатный обед (при покупке на сумму
100 рублей и меньше такой купон покупатель не получает).

Однажды Пете на глаза попался прейскурант на ближайшие N дней. Внимательно его изучив, он решил, что будет обедать
в этом кафе все N дней, причем каждый день он будет покупать в кафе ровно один обед. Однако стипендия у Пети небольшая,
и поэтому он хочет по максимуму использовать предоставляемую систему скидок так, чтобы его суммарные затраты были
минимальны. Требуется найти минимально возможную суммарную стоимость обедов и номера дней, в которые Пете следует
воспользоваться купонами.

Формат ввода
В первой строке входного файла записано целое число N (0<N≤100). В каждой из последующих N строк записано одно целое
число, обозначающее стоимость обеда в рублях на соответствующий день. Стоимость — неотрицательное целое число,
не превосходящее 300.

Формат вывода
В первой строке выдайте минимальную возможную суммарную стоимость обедов.
Во второй строке выдайте два числа K1 и K2 — количество купонов, которые останутся неиспользованными у Пети после
этих N дней и количество использованных им купонов соответственно.

В последующих K2 строках выдайте в возрастающем порядке номера дней, когда Пете следует воспользоваться купонами.
Если существует несколько решений с минимальной суммарной стоимостью, то выдайте то из них, в котором значение K1
максимально (на случай, если Петя когда-нибудь ещё решит заглянуть в это кафе).
Если таких решений несколько, выведите любое из них.

 */

#include <iostream>
#include <format>
#include <inttypes.h>
#include <queue>
#include <vector>
#include <set>
#include <utility>

namespace {

    constexpr int THRESHOLD{100};
    constexpr bool REAL_INPUT_ON{false};
    constexpr bool EXTENDED_LOG_ON{true};
    constexpr int INPUT{23};

    struct Input {
        int days_num{};
        std::vector<int> prices;

        void init() {
            std::cin >> days_num;
            days_num = days_num > 0 ? days_num : 0;

            if (days_num == 0) return;

            int buffer;
            prices.reserve(days_num);
            for (int i{}; i < days_num; ++i) {
                std::cin >> buffer;
                prices.emplace_back(buffer);
            }
        }

        void init_test(const int days_num_, std::vector<int> prices_) {
            days_num = days_num_;
            prices = std::move(prices_);
        }

        void print() const {
            std::cout << std::format("[INPUT] days: {}, prices: {}\n", days_num, prices);
        }
    };

    struct Output {
        int min_sum{};
        int not_used_coupon{};
        int used_coupon{};
        std::priority_queue<int, std::vector<int>, std::greater<>> coupon_days;

        void print() const {
            std::cout << min_sum << "\n";
            std::cout << not_used_coupon << " " << used_coupon;
            if (coupon_days.empty()) {
                std::cout << "\n";
                return;
            }

            auto coupon_day_copy = coupon_days;
            while (!coupon_day_copy.empty()) {
                std::cout << std::format("\n{}", coupon_day_copy.top());
                coupon_day_copy.pop();
            }
            std::cout << "\n";
        };

        void init_test(const int min_sum_,
                       const int not_used_coupon_,
                       const int used_coupon_,
                       std::vector<int> coupon_days_) {
            min_sum = min_sum_;
            not_used_coupon = not_used_coupon_;
            used_coupon = used_coupon_;
            coupon_days = std::priority_queue<int, std::vector<int>, std::greater<>>(std::greater<>(), std::move(coupon_days_));
        }
    };

    void enrich_input_data(Input* const input, Output* const output) {
        if constexpr  (INPUT == 0) {
            input->init_test(4, {150, 160, 170, 0});
            output->init_test(310,1,1, {3});
        } else if constexpr (INPUT == 1) {
            input->init_test(5, {35, 40, 101, 59, 63});
            output->init_test(235, 0, 1, {5});
        } else if constexpr (INPUT == 2) {
            input->init_test(7, {56, 254, 11, 192, 4, 37, 115});
            output->init_test(477, 1, 1, {4});
        } else if constexpr (INPUT == 3) {
            input->init_test(5, {35, 40, 101, 59, 63});
            output->init_test(235, 0, 1, {5});
        }
        else if constexpr (INPUT == 4) {
            input->init_test(1, {50});
            output->init_test(50, 0, 0, {});
        }
        else if constexpr (INPUT == 5) {
            input->init_test(1, {250});
            output->init_test(250, 1, 0, {});
        }
        else if constexpr (INPUT == 6) {
            input->init_test(1, {100});
            output->init_test(100, 0, 0, {});
        }
        else if constexpr (INPUT == 7) {
            input->init_test(1, {101});
            output->init_test(101, 1, 0, {});
        }
        else if constexpr (INPUT == 8) {
            input->init_test(5, {10, 20, 30, 100, 50});
            output->init_test(210, 0, 0, {});
        }
        else if constexpr (INPUT == 9) {
            input->init_test(5, {150, 200, 180, 300, 250});
            output->init_test(530, 1, 2, {4, 5});
        }
        else if constexpr (INPUT == 10) {
            input->init_test(6, {150, 10, 200, 20, 250, 30});
            output->init_test(380, 0, 2, {5, 6});
        }
        else if constexpr (INPUT == 11) {
            input->init_test(5, {300, 50, 60, 10, 90});
            output->init_test(420, 0, 1, {5});
        }
        else if constexpr (INPUT == 12) {
            input->init_test(4, {10, 20, 30, 300});
            output->init_test(360, 1, 0, {});
        }
        else if constexpr (INPUT == 13) {
            input->init_test(5, {50, 150, 10, 200, 30});
            output->init_test(240, 0, 1, {4});
        }
        else if constexpr (INPUT == 14) {
            input->init_test(4, {0, 0, 0, 0});
            output->init_test(0, 0, 0, {});
        }
        else if constexpr (INPUT == 15) {
            input->init_test(5, {0, 300, 0, 300, 0});
            output->init_test(300, 0, 1, {4});
        }
        else if constexpr (INPUT == 16) {
            input->init_test(7, {200, 250, 5, 10, 15, 20, 25});
            output->init_test(275, 0, 1, {2});
        }
        else if constexpr (INPUT == 17) {
            input->init_test(2, {150, 160});
            output->init_test(150, 0, 1, {2});
        }
        else if constexpr (INPUT == 18) {
            input->init_test(2, {200, 50});
            output->init_test(200, 0, 1, {2});
        }
        else if constexpr (INPUT == 19) {
            input->init_test(2, {50, 200});
            output->init_test(250, 1, 0, {});
        }
        else if constexpr (INPUT == 20) {
            input->init_test(15, {165, 77, 202, 24, 37, 274, 48, 187, 298, 29, 259, 109, 19, 44, 222});
            output->init_test(941, 0, 4, {6, 9, 11, 15});
        }
        else if constexpr (INPUT == 21) {
            input->init_test(30, {53, 242, 7, 28, 250, 6, 135, 53, 73, 13, 47, 72, 275, 181, 217, 38, 89, 248, 227, 215, 77, 65, 96, 62, 120, 278, 228, 124, 60, 288});
            output->init_test(2073, 0, 7, {5, 13, 18, 19, 26, 27, 30});
        }
        else if constexpr (INPUT == 22) {
            input->init_test(50, {295, 228, 145, 197, 177, 11, 236, 181, 86, 59, 252, 30, 111, 147, 66, 126, 203, 200, 254, 41, 85, 229, 205, 281, 142, 70, 220, 281, 142, 212, 183, 194, 118, 77, 42, 90, 77, 118, 119, 6, 248, 93, 134, 144, 2, 74, 214, 273, 189, 289});
            output->init_test(3588, 0, 17, {2, 7, 11, 17, 19, 22, 23, 24, 27, 28, 30, 32, 41, 47, 48, 49, 50});
        } else if constexpr (INPUT == 23) {
            input->init_test(8, {150, 160, 170, 180, 190, 200, 210, 0});
            output->init_test(660, 1, 3, {5, 6, 7});
        } else {
            std::cout << "Bad input\n";
        }
    }

    //<
    // struct Compare {
    //     bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) const {
    //         if (lhs.first == rhs.first) {
    //             return lhs.second > rhs.second;
    //         }
    //         return lhs.first > rhs.first;
    //     }
    // };

    enum class Direction: int {
        UNKNOWN = -1,
        LEFT = 1,
        UP = 2,
        RIGHT = 3,
    };

    void test() {
        const auto input{new Input{}};
        Output* expected_output{};
        Output* const gotten_output{new Output{}};

        if constexpr (REAL_INPUT_ON) {
            input->init();
        } else {
            expected_output = new Output{};
            enrich_input_data(input, expected_output);
        }

        if constexpr (EXTENDED_LOG_ON) {
            std::cout << "input:\n";
            input->print();
            std::cout << "expected_output:\n";
            if (expected_output) expected_output->print();
            std::cout << "gotten_output:\n";
            if (gotten_output) gotten_output->print();
        }

        std::vector<std::vector<int>> dp(input->days_num, std::vector<int>(input->days_num, -1));
        // std::vector<std::vector<int>> direction(input->days_num, std::vector<int>(input->days_num, -1));
        //<
        std::vector<std::vector<Direction>> direction(input->days_num, std::vector<Direction>(input->days_num, Direction::UNKNOWN));
        dp[0][input->prices[0] > THRESHOLD ? 1 : 0] = input->prices[0];

        // |     ||  0  |     1   |    2    |     3   |     4   |    5     |  6   |   7  |
        // -------------------------------------------------------------------------------
        // | 150 || -1  |    150  |   -1    |    -1   |    -1   |    -1    |  -1  |  -1  |
        // | 160 || 150 |    -1   |   310   |    -1   |    -1   |    -1    |  -1  |  -1  |
        // | 170 || -1  |    310  |   -1    |    480  |    -1   |    -1    |  -1  |  -1  |
        // | 180 || 310 |    -1   | 490 480 |    -1   |    660  |    -1    |  -1  |  -1  |
        // | 190 || -1  | 500 480 |   -1    | 680 660 |    -1   |    850   |  -1  |  -1  |
        // | 200 || 480 |    -1   | 680 660 |    -1   | 860 850 |    -1    | 1050 |  -1  |
        // | 210 || -1  | 690 660 |   -1    | 860 850 |    -1   | 850 1050 |  -1  | 1260 |
        // |  0  || 660 |    660  |   850   |   850   |    850  |    850   |  850 | 1260 |

        for (int p{1}; p < input->days_num; ++p) {
            const auto price{input->prices[p]};
            const auto maybe_coupon{price > THRESHOLD};
            const auto prev_p_idx{p - 1};
            for (int d{}; d < input->days_num; ++d) {
                const int prev_d{d - 1};
                const int next_d{d + 1};

                const int prev_d_coupon{
                    prev_d >= 0 && dp[prev_p_idx][prev_d] != -1
                    ? dp[prev_p_idx][prev_d]
                    : -1
                };
                const int next_d_coupon{
                    next_d < input->days_num && dp[prev_p_idx][next_d] != -1
                    ? dp[prev_p_idx][next_d]
                    : -1
                };

                if (maybe_coupon) {
                    if (prev_d_coupon != -1) {
                        auto& value{dp[p][d]};
                        if (const auto new_value{prev_d_coupon + price};
                            value == -1 || value > new_value)
                        {
                            value = new_value;
                            direction[p][d] = Direction::LEFT;
                        }
                    }
                } else {
                    if (dp[p-1][d] != -1) {
                        auto& old_value{dp[p][d]};
                        if (const auto new_value{dp[p-1][d] + price};
                            old_value == -1 || old_value > new_value)
                        {
                            old_value = new_value;
                            direction[p][d] = Direction::UP;
                        }
                    }
                }

                if (next_d_coupon != -1) {
                    if (auto& value{dp[p][d]}; value == -1 || value > next_d_coupon) {
                        value = next_d_coupon;
                        direction[p][d] = Direction::RIGHT;
                    }
                }
            }
        }

        if constexpr (EXTENDED_LOG_ON) {
            for (const auto& vec: dp) {
                for (const auto i: vec) {
                    std::cout << "\t" << i << " ";
                }
                std::cout << std::endl;
            }

            for (const auto& vec: direction) {
                for (const auto i: vec) {
                    std::cout << "\t" << static_cast<int>(i) << " ";
                }
                std::cout << std::endl;
            }
        }

        delete input;
        delete expected_output;
        delete gotten_output;
    }
}

int main(int argc, char *argv[]) {
    test();

    return 0;
}
