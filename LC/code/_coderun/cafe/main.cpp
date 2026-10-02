
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
            input->init_test(8, {8, 150, 160, 170, 180, 190, 200, 210, 0});
            output->init_test(660, 1, 3, {5, 6, 7});
        } else {
            std::cout << "Bad input\n";
        }
    }

    struct Compare {
        bool operator()(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) const {
            if (lhs.first == rhs.first) {
                return lhs.second > rhs.second;
            }
            return lhs.first > rhs.first;
        }
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

        input->print();
        if (expected_output) expected_output->print();
        if (gotten_output) gotten_output->print();

// [INPUT] days: 5, prices: [35, 40, 101, 59, 63]
// 235
// 0 1
// 5

        delete input;
        delete expected_output;
        delete gotten_output;

        //<
        // if constexpr (EXTENDED_LOG_ON) {
        //     std::cout << std::format("Days: {}\nPrices:", day_quantity);
        //     for (const auto price : prices) {
        //         std::cout << std::format(" {}", price);
        //     }
        // }
        //
        // std::set<std::pair<int, int>, Compare> sorted_prices;
        // for (int i{}; i < day_quantity; ++i) {
        //     sorted_prices.insert(std::make_pair(prices[i], i));
        // }
        // if constexpr (EXTENDED_LOG_ON) {
        //     std::cout << "\nsorted_prices:";
        //     for (const auto&[fst, snd]: sorted_prices) {
        //         std::cout << std::format(" [price: {}, idx: {}]", fst, snd);
        //     }
        //     std::cout << std::endl;
        // }
        //
        //
        // auto masks = std::vector<bool>(day_quantity, false);
        // for (int day_idx{}; day_idx < day_quantity; ++day_idx) {
        //     const auto price{prices[day_idx]};
        //     if (masks[day_idx]) {
        //         ++used_coupon;
        //         //<
        //         std::cout << std::format("1: {} {}\n", day_idx, price);
        //         //<
        //         coupon_days.emplace(day_idx);
        //         continue;
        //     }
        //
        //     total_sum += price;
        //     if (price > THRESHOLD) {
        //         ++total_coupon;
        //         //<
        //         std::cout << std::format("2: {}\n", price);
        //         auto it{sorted_prices.begin()};
        //         while (it != sorted_prices.end()) {
        //             if (it->second > day_idx) {
        //                 masks[it->second] = true;
        //                 break;
        //             }
        //             sorted_prices.erase(it);
        //             it = sorted_prices.begin();
        //         }
        //     }
        // }
        //
        // print_ret(total_sum, total_coupon, used_coupon, coupon_days);
    }
}

int main(int argc, char *argv[]) {
    test();

    return 0;
}
