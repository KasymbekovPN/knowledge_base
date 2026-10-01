/*

https://coderun.yandex.ru/problem/median-out-of-three

Рассмотрим три числа a, b, c
Упорядочим их по возрастанию.

Какое число будет стоять между двумя другими?

 */

#include <iostream>
#include <algorithm>
#include <chrono>

namespace {
    void variant0() {
        constexpr int SIZE{3};
        int arr[SIZE];
        for (int i{}; i < SIZE; ++i) {
            std::cin >> arr[i];
        }
        const auto start{std::chrono::steady_clock::now()};
        std::sort(arr, arr + SIZE);
        const auto dif{std::chrono::steady_clock::now() - start};

        std::cout << "ns: " << dif << " " << arr[1] << std::endl;
    }

    // [2][1][2][0][2][1][2]
    void variant1() {
        constexpr int SIZE{3};
        int arr[SIZE];
        for (int i{}; i < SIZE; ++i) {
            std::cin >> arr[i];
        }

        const auto start{std::chrono::steady_clock::now()};

        int arr1[2 * SIZE + 1];
        arr1[SIZE + 1] = arr1[0];
        for (int i{1}; i < SIZE; ++i) {

        }

        const auto dif{std::chrono::steady_clock::now() - start};

        std::cout << "ns: " << dif << " " << arr[1] << std::endl;
    }
}

int main(int argc, char *argv[]) {
    variant0();

    return 0;
}
