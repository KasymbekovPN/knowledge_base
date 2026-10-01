/*

https://coderun.yandex.ru/problem/cheapest-way

В каждой клетке прямоугольной таблицы N×M записано некоторое число. Изначально игрок находится в левой верхней клетке.
За один ход ему разрешается перемещаться в соседнюю клетку либо вправо, либо вниз (влево и вверх перемещаться запрещено).
При проходе через клетку с игрока берут столько килограммов еды, какое число записано в этой клетке (еду берут также
за первую и последнюю клетки его пути).

Требуется найти минимальный вес еды в килограммах, отдав которую игрок может попасть в правый нижний угол.

Формат ввода
Вводятся два числа N и M — размеры таблицы (1≤N≤20, 1≤M≤20). Затем идет N строк по M чисел в каждой — размеры штрафов
в килограммах за прохождение через соответствующие клетки (числа от 0 до 100).

Формат вывода
Выведите минимальный вес еды в килограммах, отдав которую можно попасть в правый нижний угол.

Ввод:
5 5
1 1 1 1 1
3 100 100 100 100
1 1 1 1 1
2 2 2 2 1
1 1 1 1 1

Вывод
11
 */

#include <vector>
#include <iostream>
#include <limits>

namespace {
    void test() {
        int size[2];
        for (int i{0}; i < 2; ++i) {
            std::cin >> size[i];
        }

        int buf;
        std::vector<std::vector<int>> map{};
        for (int r{}; r < size[0]; ++r) {
            map.emplace_back();
            for (int c{}; c < size[1]; ++c) {
                std::cin >> buf;
                map[r].push_back(buf);
            }
        }

        // int size[2] = {5, 5};
        // std::vector<std::vector<int>> map {
        //     {1, 1, 1, 1, 1},
        //     {3, 100, 100, 100, 100},
        //     {1, 1, 1, 1, 1},
        //     {2, 2, 2, 2, 1},
        //     {1, 1, 1, 1, 1}
        // };

        const auto& R{size[0]};
        const auto& C{size[1]};
        std::vector<std::vector<int>> buffer(R, std::vector<int>(C));
        buffer[0][0] = map[0][0];
        for (int r{0}; r < R; ++r) {
            for (int c{0}; c < C; ++c) {
                if (r == 0 && c == 0) continue;
                int best{INT_MAX};
                if (r > 0) best = std::min(best, buffer[r-1][c]);
                if (c > 0) best = std::min(best, buffer[r][c-1]);

                buffer[r][c] = best + map[r][c];
            }
        }

        std::cout << buffer[R-1][C-1] << std::endl;
    }
}

int main(int argc, char *argv[]) {
    test();
    return 0;
}
